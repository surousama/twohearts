#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"
#include "twoheartsCharacter.h"

UTwoHeartsGA_NormalAttackBase::UTwoHeartsGA_NormalAttackBase()
{
	AddDefaultAssetTag(TAG_TwoHearts_Ability_NormalAttack);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_Action_NormalAttack);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_State_CannotAttack);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_State_CannotInput);
}

void UTwoHeartsGA_NormalAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bHasQueuedNextSegment = false;
	bHasFinishedSegment = false;
	bHasRegisteredCombatActionContext = false;
	bPreserveDebugStateUntilNextSegment = false;
	bAdvanceStopInProgress = false;
	bInterruptedByDodge = false;
	CurrentCombatPhase = ETwoHeartsCombatPhase::None;
	ActiveMontageTask = nullptr;
	PendingNextSegmentAbilityTag = FGameplayTag();
	PendingNextSegmentSourceSegment = 0;
	bHasOpenedNextSegmentAdvanceWindow = false;
	ClearPendingLateBufferedInputRestore();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeferredNextSegmentActivationTimerHandle);
	}

	if (!StartSegmentPlayback())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UTwoHeartsGA_NormalAttackBase::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (!CanQueueNextSegment())
	{
		RecordAbilityEvent(
			TEXT("InputIgnored"),
			FString::Printf(
				TEXT("Segment %d cannot queue a next segment during phase %s."),
				NormalAttackSegment,
				*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentCombatPhase))),
			true);
		return;
	}

	if (bHasQueuedNextSegment)
	{
		RecordAbilityEvent(TEXT("InputIgnored"), FString::Printf(TEXT("Segment %d already has a queued next segment."), NormalAttackSegment), true);
		return;
	}

	bHasQueuedNextSegment = true;
	UpdateDebugState(true);
	RecordAbilityEvent(TEXT("QueueNextSegment"), FString::Printf(TEXT("Queued next segment after %d."), NormalAttackSegment), true);
	TryAdvanceToNextSegment(TEXT("ForwardedInput"), false);
}

void UTwoHeartsGA_NormalAttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearPhaseFallbackTimers();
	UnbindMontageNotifyDelegates();
	ActiveMontageTask = nullptr;
	FinishCombatActionContext(bWasCancelled);

	if (!bPreserveDebugStateUntilNextSegment)
	{
		UpdateDebugState(false);
	}

	bPreserveDebugStateUntilNextSegment = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UTwoHeartsGA_NormalAttackBase::CanBeInterruptedByDodge() const
{
	if (const UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent())
	{
		return ActionContextComponent->CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType::Dodge);
	}

	return CurrentCombatPhase == ETwoHeartsCombatPhase::Recovery || CurrentCombatPhase == ETwoHeartsCombatPhase::LogicEnded;
}

bool UTwoHeartsGA_NormalAttackBase::TryInterruptByAction(ETwoHeartsCombatActionType InterruptingActionType, const FString& InterruptReason)
{
	const UEnum* ActionTypeEnum = StaticEnum<ETwoHeartsCombatActionType>();
	const FString InterruptingActionName = ActionTypeEnum
		? ActionTypeEnum->GetNameStringByValue(static_cast<int64>(InterruptingActionType))
		: TEXT("Unknown");
	const bool bCanInterrupt =
		InterruptingActionType == ETwoHeartsCombatActionType::Dodge
			? CanBeInterruptedByDodge()
			: false;

	RecordAbilityEvent(
		TEXT("InterruptCheck"),
		FString::Printf(
			TEXT("%s interrupt check on segment %d during phase %s. Allowed=%s."),
			*InterruptingActionName,
			NormalAttackSegment,
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentCombatPhase)),
			bCanInterrupt ? TEXT("true") : TEXT("false")));

	if (!bCanInterrupt)
	{
		return false;
	}

	bInterruptedByDodge = InterruptingActionType == ETwoHeartsCombatActionType::Dodge;
	EnterCombatPhase(ETwoHeartsCombatPhase::LogicEnded, InterruptReason);
	RecordAbilityEvent(
		TEXT("InterruptedByAction"),
		FString::Printf(
			TEXT("Normal attack segment %d was interrupted by %s."),
			NormalAttackSegment,
			*InterruptingActionName));

	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
		{
			if (UAnimMontage* Montage = Character->GetNormalAttackMontage())
			{
				AnimInstance->Montage_Stop(0.05f, Montage);
				return true;
			}
		}
	}

	FinishSegment(true);
	return true;
}

bool UTwoHeartsGA_NormalAttackBase::TryInterruptByDodge()
{
	return TryInterruptByAction(ETwoHeartsCombatActionType::Dodge, TEXT("InterruptedByDodge"));
}

void UTwoHeartsGA_NormalAttackBase::NotifyCombatPhaseByName(FName NotifyName)
{
	RecordAbilityEvent(
		TEXT("MontageNotify"),
		FString::Printf(
			TEXT("Segment %d received notify %s. %s"),
			NormalAttackSegment,
			*NotifyName.ToString(),
			*BuildMontageDebugSnapshot()),
		false);

	if (NotifyName == ActivePhaseNotifyName)
	{
		EnterCombatPhase(ETwoHeartsCombatPhase::Active, TEXT("MontageNotify"));
		return;
	}

	if (NotifyName == RecoveryPhaseNotifyName)
	{
		EnterCombatPhase(ETwoHeartsCombatPhase::Recovery, TEXT("MontageNotify"));
		return;
	}

	if (NotifyName == LogicEndedPhaseNotifyName)
	{
		EnterCombatPhase(ETwoHeartsCombatPhase::LogicEnded, TEXT("MontageNotify"));
		return;
	}

	if (NotifyName == NextSegmentAdvanceNotifyName)
	{
		OpenNextSegmentAdvanceWindow(TEXT("MontageNotify"), true);
	}
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageCompleted()
{
	FinishSegment(false);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageInterrupted()
{
	if (bAdvanceStopInProgress)
	{
		RecordAbilityEvent(
			TEXT("AdvanceStopInterrupted"),
			FString::Printf(
				TEXT("Ignored montage interrupted callback while segment %d was intentionally stopping for early advance."),
				NormalAttackSegment),
			true);
		return;
	}

	FinishSegment(true);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageCancelled()
{
	if (bAdvanceStopInProgress)
	{
		RecordAbilityEvent(
			TEXT("AdvanceStopCancelled"),
			FString::Printf(
				TEXT("Ignored montage cancelled callback while segment %d was intentionally stopping for early advance."),
				NormalAttackSegment),
			true);
		return;
	}

	FinishSegment(true);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	const FString NotifyAssetName = GetNameSafe(BranchingPointNotifyPayload.SequenceAsset);
	const FString NotifyAssetPath = BranchingPointNotifyPayload.SequenceAsset
		? BranchingPointNotifyPayload.SequenceAsset->GetPathName()
		: TEXT("None");
	const FString NotifyObjectName =
		(BranchingPointNotifyPayload.NotifyEvent && BranchingPointNotifyPayload.NotifyEvent->Notify)
			? BranchingPointNotifyPayload.NotifyEvent->Notify->GetName()
			: TEXT("None");
	RecordAbilityEvent(
		TEXT("MontageNotifyBegin"),
		FString::Printf(
			TEXT("Segment %d notify callback begin. Notify=%s NotifyObject=%s Sequence=%s SequencePath=%s MontageInstanceID=%d ReachedEnd=%s. %s"),
			NormalAttackSegment,
			*NotifyName.ToString(),
			*NotifyObjectName,
			*NotifyAssetName,
			*NotifyAssetPath,
			BranchingPointNotifyPayload.MontageInstanceID,
			BranchingPointNotifyPayload.bReachedEnd ? TEXT("true") : TEXT("false"),
			*BuildMontageDebugSnapshot()));
	NotifyCombatPhaseByName(NotifyName);
}

void UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterActive()
{
	EnterCombatPhase(ETwoHeartsCombatPhase::Active, TEXT("FallbackTime"));
}

void UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterRecovery()
{
	EnterCombatPhase(ETwoHeartsCombatPhase::Recovery, TEXT("FallbackTime"));
}

void UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterLogicEnded()
{
	EnterCombatPhase(ETwoHeartsCombatPhase::LogicEnded, TEXT("FallbackTime"));
}

void UTwoHeartsGA_NormalAttackBase::HandleFallbackOpenNextSegmentAdvanceWindow()
{
	OpenNextSegmentAdvanceWindow(TEXT("FallbackTime"), true);
}

bool UTwoHeartsGA_NormalAttackBase::CanQueueNextSegment() const
{
	const bool bCanAcceptQueueByPhase =
		CurrentCombatPhase == ETwoHeartsCombatPhase::Startup
		|| CurrentCombatPhase == ETwoHeartsCombatPhase::Active
		|| CurrentCombatPhase == ETwoHeartsCombatPhase::Recovery;
	return bCanAcceptQueueByPhase && !IsLogicEndedPhase() && NormalAttackSegment < 3 && NextSegmentAbilityTag.IsValid();
}

bool UTwoHeartsGA_NormalAttackBase::StartSegmentPlayback()
{
	AtwoheartsCharacter* Character = GetTwoHeartsCharacter();
	if (!Character)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), TEXT("Normal attack Ability could not find a valid twohearts character."));
		return false;
	}

	UAnimMontage* Montage = Character->GetNormalAttackMontage();
	if (!Montage)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("NormalAttackMontage is not configured on %s."), *GetNameSafe(Character)));
		return false;
	}

	UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("No AnimInstance found for normal attack on %s."), *GetNameSafe(Character)));
		return false;
	}

	const FName SectionName = Character->GetNormalAttackSectionName(NormalAttackSegment);
	if (SectionName.IsNone() || Montage->GetSectionIndex(SectionName) == INDEX_NONE)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("Normal attack section %s is missing on %s."), *SectionName.ToString(), *GetNameSafe(Montage)));
		return false;
	}

	BindMontageNotifyDelegates(AnimInstance);
	EnterCombatPhase(ETwoHeartsCombatPhase::Startup, TEXT("AbilityActivated"));
	Character->SetLastNormalAttackDebugFailureReason(TEXT(""));
	RecordAbilityEvent(
		TEXT("PlaySegment"),
		FString::Printf(
			TEXT("Started segment %d with section %s using montage %s path=%s."),
			NormalAttackSegment,
			*SectionName.ToString(),
			*GetNameSafe(Montage),
			*Montage->GetPathName()));

	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.0f,
		SectionName,
		false,
		1.0f);

	if (!ActiveMontageTask)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("Failed to create montage task for segment %d."), NormalAttackSegment));
		return false;
	}

	const float SectionLength = Character->GetNormalAttackSectionLength(NormalAttackSegment);
	if (SectionLength <= 0.0f)
	{
		RecordAbilityFailure(
			TEXT("PhaseFallbackMissing"),
			FString::Printf(
				TEXT("Segment %d resolved a non-positive section length from %s; fallback phase timers will not be scheduled."),
				NormalAttackSegment,
				*SectionName.ToString()));
	}
	else
	{
		RecordAbilityEvent(
			TEXT("PhaseFallbackScheduled"),
			FString::Printf(
				TEXT("Segment %d scheduled fallback timers from section length %.3f (active=%.3f recovery=%.3f advance=%.3f logicEnded=%.3f)."),
				NormalAttackSegment,
				SectionLength,
				SectionLength * ActivePhaseFallbackNormalizedTime,
				SectionLength * RecoveryPhaseFallbackNormalizedTime,
				SectionLength * NextSegmentAdvanceFallbackNormalizedTime,
				SectionLength * LogicEndedFallbackNormalizedTime),
			true);
	}
	SchedulePhaseFallbacks(SectionLength);

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageInterrupted);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageCancelled);
	ActiveMontageTask->ReadyForActivation();

	return true;
}

void UTwoHeartsGA_NormalAttackBase::FinishSegment(bool bWasCancelled)
{
	if (bHasFinishedSegment)
	{
		return;
	}

	bHasFinishedSegment = true;
	bAdvanceStopInProgress = false;
	ClearPhaseFallbackTimers();

	RecordAbilityEvent(
		TEXT("FinishSegmentBegin"),
		FString::Printf(
			TEXT("Finishing segment %d. Cancelled=%s QueuedNext=%s AdvanceWindowOpened=%s PendingNextTag=%s. %s"),
			NormalAttackSegment,
			bWasCancelled ? TEXT("true") : TEXT("false"),
			bHasQueuedNextSegment ? TEXT("true") : TEXT("false"),
			bHasOpenedNextSegmentAdvanceWindow ? TEXT("true") : TEXT("false"),
			*NextSegmentAbilityTag.ToString(),
			*BuildMontageDebugSnapshot()),
		true);

	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();

	if (!bWasCancelled)
	{
		EnterCombatPhase(ETwoHeartsCombatPhase::LogicEnded, TEXT("SegmentCompleted"));
	}

	const bool bConsumedLateBufferedNext = !bWasCancelled && !bHasQueuedNextSegment && TryConsumeLateBufferedNextSegment();
	const bool bShouldActivateNext =
		!bWasCancelled
		&& (bHasQueuedNextSegment || bConsumedLateBufferedNext)
		&& NextSegmentAbilityTag.IsValid()
		&& AbilitySystemComponent;

	if (bWasCancelled)
	{
		RecordAbilityFailure(TEXT("SegmentInterrupted"), FString::Printf(TEXT("Segment %d was interrupted before completion."), NormalAttackSegment));
	}
	else
	{
		RecordAbilityEvent(
			TEXT("SegmentFinished"),
			FString::Printf(
				TEXT("Segment %d finished. QueuedNext=%s LateBufferedNext=%s."),
				NormalAttackSegment,
				bHasQueuedNextSegment ? TEXT("true") : TEXT("false"),
				bConsumedLateBufferedNext ? TEXT("true") : TEXT("false")));
	}

	bPreserveDebugStateUntilNextSegment = bShouldActivateNext;
	EndAbility(Handle, ActorInfo, ActivationInfo, true, bWasCancelled);

	if (!bShouldActivateNext)
	{
		if (!bWasCancelled)
		{
			if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
			{
				Character->TryConsumeReservedCombatInput(TEXT("NormalAttackEnded"));
			}
		}
		return;
	}

	PendingNextSegmentAbilityTag = NextSegmentAbilityTag;
	PendingNextSegmentSourceSegment = NormalAttackSegment;

	if (UWorld* World = GetWorld())
	{
		RecordAbilityEvent(
			TEXT("AdvanceSegmentImmediate"),
			FString::Printf(
				TEXT("Attempting immediate follow-up activation from segment %d to next segment tag %s after ending the current segment."),
				NormalAttackSegment,
				*NextSegmentAbilityTag.ToString()),
			true);
	}

	AttemptDeferredNextSegmentActivation();
}

bool UTwoHeartsGA_NormalAttackBase::TryConsumeLateBufferedNextSegment()
{
	if (!NextSegmentAbilityTag.IsValid())
	{
		return false;
	}

	UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	AtwoheartsCharacter* Character = GetTwoHeartsCharacter();
	if (!ActionContextComponent || !Character)
	{
		return false;
	}

	FTwoHeartsBufferedCombatInput BufferedInput;
	if (!ActionContextComponent->ConsumeBufferedInput(BufferedInput, TEXT("NormalAttackLateFollowUp")))
	{
		return false;
	}

	if (BufferedInput.IncomingActionType != ETwoHeartsCombatActionType::NormalAttack)
	{
		ActionContextComponent->RestoreBufferedInput(BufferedInput, TEXT("NormalAttackLateFollowUp.WrongActionType"));
		return false;
	}

	PendingLateBufferedInputToRestore = BufferedInput;
	bHasPendingLateBufferedInputRestore = true;

	const FString Detail = FString::Printf(
		TEXT("Consumed late buffered normal attack after segment %d; scheduling %s."),
		NormalAttackSegment,
		*NextSegmentAbilityTag.ToString());
	Character->PushCombatInputDebugEvent(
		TEXT("NormalAttack"),
		TEXT("BufferedConsumed"),
		StaticEnum<ETwoHeartsCombatInputConsumptionRoute>()
			? StaticEnum<ETwoHeartsCombatInputConsumptionRoute>()->GetNameStringByValue(static_cast<int64>(BufferedInput.ConsumptionRoute))
			: TEXT("Unknown"),
		Detail);
	RecordAbilityEvent(TEXT("ConsumeLateBufferedNextSegment"), Detail, true);
	return true;
}

void UTwoHeartsGA_NormalAttackBase::UpdateDebugState(bool bShouldBeActive) const
{
	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		const FString SectionName = bShouldBeActive ? Character->GetNormalAttackSectionName(NormalAttackSegment).ToString() : TEXT("None");
		Character->SetNormalAttackDebugRuntimeState(
			bShouldBeActive,
			bShouldBeActive ? NormalAttackSegment : 0,
			bShouldBeActive ? bHasQueuedNextSegment : false,
			SectionName,
			bShouldBeActive ? CurrentCombatPhase : ETwoHeartsCombatPhase::None,
			bShouldBeActive ? CanBeInterruptedByDodge() : false,
			bShouldBeActive ? IsLogicEndedPhase() : false);
	}
}

void UTwoHeartsGA_NormalAttackBase::OpenNextSegmentAdvanceWindow(const FString& Reason, bool bCanConsumeLateBufferedInput)
{
	if (bHasOpenedNextSegmentAdvanceWindow || !NextSegmentAbilityTag.IsValid() || NormalAttackSegment >= 3)
	{
		RecordAbilityEvent(
			TEXT("AdvanceWindowIgnored"),
			FString::Printf(
				TEXT("Ignored advance window open for segment %d. Reason=%s AlreadyOpened=%s NextTagValid=%s SegmentCanAdvance=%s. %s"),
				NormalAttackSegment,
				*Reason,
				bHasOpenedNextSegmentAdvanceWindow ? TEXT("true") : TEXT("false"),
				NextSegmentAbilityTag.IsValid() ? TEXT("true") : TEXT("false"),
				NormalAttackSegment < 3 ? TEXT("true") : TEXT("false"),
				*BuildMontageDebugSnapshot()),
			true);
		return;
	}

	bHasOpenedNextSegmentAdvanceWindow = true;
	RecordAbilityEvent(
		TEXT("AdvanceWindowOpened"),
		FString::Printf(
			TEXT("Segment %d opened next-segment advance window. Reason=%s CanConsumeLateBuffered=%s QueuedNext=%s. %s"),
			NormalAttackSegment,
			*Reason,
			bCanConsumeLateBufferedInput ? TEXT("true") : TEXT("false"),
			bHasQueuedNextSegment ? TEXT("true") : TEXT("false"),
			*BuildMontageDebugSnapshot()));
	TryAdvanceToNextSegment(Reason, bCanConsumeLateBufferedInput);
}

bool UTwoHeartsGA_NormalAttackBase::TryAdvanceToNextSegment(const FString& Reason, bool bCanConsumeLateBufferedInput)
{
	if (bHasFinishedSegment || !bHasOpenedNextSegmentAdvanceWindow || !NextSegmentAbilityTag.IsValid())
	{
		RecordAbilityEvent(
			TEXT("AdvanceSegmentBlocked"),
			FString::Printf(
				TEXT("Blocked early advance for segment %d. Reason=%s Finished=%s WindowOpened=%s NextTagValid=%s. %s"),
				NormalAttackSegment,
				*Reason,
				bHasFinishedSegment ? TEXT("true") : TEXT("false"),
				bHasOpenedNextSegmentAdvanceWindow ? TEXT("true") : TEXT("false"),
				NextSegmentAbilityTag.IsValid() ? TEXT("true") : TEXT("false"),
				*BuildMontageDebugSnapshot()),
			true);
		return false;
	}

	bool bConsumedLateBufferedNext = false;
	if (!bHasQueuedNextSegment && bCanConsumeLateBufferedInput)
	{
		bConsumedLateBufferedNext = TryConsumeLateBufferedNextSegment();
		if (bConsumedLateBufferedNext)
		{
			bHasQueuedNextSegment = true;
		}
	}

	if (!bHasQueuedNextSegment)
	{
		RecordAbilityEvent(
			TEXT("AdvanceSegmentWaitingInput"),
			FString::Printf(
				TEXT("Advance window is open for segment %d but no next input is ready yet. Reason=%s CanConsumeLateBuffered=%s. %s"),
				NormalAttackSegment,
				*Reason,
				bCanConsumeLateBufferedInput ? TEXT("true") : TEXT("false"),
				*BuildMontageDebugSnapshot()),
			true);
		return false;
	}

	RecordAbilityEvent(
		TEXT("AdvanceSegmentReady"),
		FString::Printf(
			TEXT("Segment %d is advancing early. Reason=%s LateBuffered=%s NextTag=%s. %s"),
			NormalAttackSegment,
			*Reason,
			bConsumedLateBufferedNext ? TEXT("true") : TEXT("false"),
			*NextSegmentAbilityTag.ToString(),
			*BuildMontageDebugSnapshot()));

	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
		{
			if (UAnimMontage* Montage = Character->GetNormalAttackMontage())
			{
				bAdvanceStopInProgress = true;
				AnimInstance->Montage_Stop(0.0f, Montage);
			}
		}
	}

	EnterCombatPhase(ETwoHeartsCombatPhase::LogicEnded, FString::Printf(TEXT("AdvanceWindow.%s"), *Reason));
	FinishSegment(false);
	return true;
}

void UTwoHeartsGA_NormalAttackBase::RecordAbilityEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly) const
{
	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		Character->PushNormalAttackDebugEvent(EventName, Detail, bVerboseOnly);
	}

	LogAbilityMessage(Detail);
}

void UTwoHeartsGA_NormalAttackBase::RecordAbilityFailure(const TCHAR* EventName, const FString& Detail) const
{
	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		Character->PushNormalAttackDebugFailure(EventName, Detail);
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Warning,
		TEXT("[GameplayAbility] ability=%s owner=%s avatar=%s event=%s detail=\"%s\""),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetAbilityOwnerActor()),
		*GetNameSafe(GetAbilityAvatarActor()),
		EventName,
		*Detail);
}

FString UTwoHeartsGA_NormalAttackBase::BuildMontageDebugSnapshot() const
{
	const AtwoheartsCharacter* Character = GetTwoHeartsCharacter();
	const UAnimInstance* AnimInstance = Character && Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage* Montage = Character ? Character->GetNormalAttackMontage() : nullptr;
	if (!AnimInstance || !Montage)
	{
		return TEXT("MontageState=Unavailable");
	}

	const float PositionSeconds = AnimInstance->Montage_GetPosition(Montage);
	float SectionLocalTime = 0.0f;
	const int32 SectionIndex = Montage->GetAnimCompositeSectionIndexFromPos(PositionSeconds, SectionLocalTime);
	const FName CurrentSectionName = SectionIndex != INDEX_NONE ? Montage->GetSectionName(SectionIndex) : NAME_None;
	const float SectionLength = SectionIndex != INDEX_NONE ? Montage->GetSectionLength(SectionIndex) : 0.0f;
	const float SectionNormalizedTime = SectionLength > KINDA_SMALL_NUMBER ? SectionLocalTime / SectionLength : 0.0f;
	return FString::Printf(
		TEXT("Montage=%s Position=%.3f Section=%s Local=%.3f Length=%.3f Normalized=%.3f IsPlaying=%s"),
		*GetNameSafe(Montage),
		PositionSeconds,
		*CurrentSectionName.ToString(),
		SectionLocalTime,
		SectionLength,
		SectionNormalizedTime,
		AnimInstance->Montage_IsPlaying(Montage) ? TEXT("true") : TEXT("false"));
}

AtwoheartsCharacter* UTwoHeartsGA_NormalAttackBase::GetTwoHeartsCharacter() const
{
	return Cast<AtwoheartsCharacter>(GetAbilityCharacter());
}

UTwoHeartsCombatActionContextComponent* UTwoHeartsGA_NormalAttackBase::GetCombatActionContextComponent() const
{
	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		return Character->GetCombatActionContextComponent();
	}

	return nullptr;
}

void UTwoHeartsGA_NormalAttackBase::SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase NewPhase, const FString& Reason)
{
	UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	if (!ActionContextComponent)
	{
		return;
	}

	if (!bHasRegisteredCombatActionContext)
	{
		FTwoHeartsCombatActionRegistration Registration;
		Registration.ActionType = ETwoHeartsCombatActionType::NormalAttack;
		Registration.InitialPhase = NewPhase;
		Registration.AbilityTag = SegmentAbilityTag.IsValid() ? SegmentAbilityTag : TAG_TwoHearts_Ability_NormalAttack;
		Registration.ActionStateTag = TAG_TwoHearts_State_Action_NormalAttack;
		Registration.ActionInstanceName = FString::Printf(TEXT("NormalAttack.Segment%d"), NormalAttackSegment);

		ActionContextComponent->BeginAction(Registration, Reason);
		bHasRegisteredCombatActionContext = true;
		return;
	}

	if (NewPhase == ETwoHeartsCombatPhase::LogicEnded)
	{
		ActionContextComponent->MarkLogicEnded(Reason);
		return;
	}

	ActionContextComponent->TransitionToPhase(NewPhase, Reason);
}

void UTwoHeartsGA_NormalAttackBase::FinishCombatActionContext(bool bWasCancelled)
{
	if (!bHasRegisteredCombatActionContext)
	{
		return;
	}

	if (UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent())
	{
		const ETwoHeartsCombatActionEndReason EndReason =
			!bWasCancelled ? ETwoHeartsCombatActionEndReason::Completed :
			bInterruptedByDodge ? ETwoHeartsCombatActionEndReason::Interrupted :
			ETwoHeartsCombatActionEndReason::Cancelled;

		const FString FinishReason =
			!bWasCancelled ? TEXT("NormalAttackEnded") :
			bInterruptedByDodge ? TEXT("InterruptedByDodge") :
			TEXT("NormalAttackCancelled");

		ActionContextComponent->FinishAction(EndReason, FinishReason);
	}

	bHasRegisteredCombatActionContext = false;
}

void UTwoHeartsGA_NormalAttackBase::EnterCombatPhase(ETwoHeartsCombatPhase NewPhase, const FString& Reason)
{
	if (!CanTransitionToPhase(NewPhase))
	{
		return;
	}

	if (CurrentCombatPhase != ETwoHeartsCombatPhase::None)
	{
		RecordAbilityEvent(
			TEXT("LeavePhase"),
			FString::Printf(
				TEXT("Leaving phase %s because %s."),
				*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentCombatPhase)),
				*Reason),
			true);
	}

	CurrentCombatPhase = NewPhase;
	SyncCombatActionContextOnPhaseEntered(NewPhase, Reason);
	UpdateDebugState(true);

	const TCHAR* EventName = NewPhase == ETwoHeartsCombatPhase::LogicEnded ? TEXT("LogicEnded") : TEXT("EnterPhase");
	RecordAbilityEvent(
		EventName,
		FString::Printf(
			TEXT("Entered phase %s for segment %d. Reason=%s."),
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentCombatPhase)),
			NormalAttackSegment,
			*Reason));
}

bool UTwoHeartsGA_NormalAttackBase::CanTransitionToPhase(ETwoHeartsCombatPhase NewPhase) const
{
	if (NewPhase == CurrentCombatPhase)
	{
		return false;
	}

	return static_cast<uint8>(NewPhase) >= static_cast<uint8>(CurrentCombatPhase);
}

void UTwoHeartsGA_NormalAttackBase::BindMontageNotifyDelegates(UAnimInstance* AnimInstance)
{
	if (!AnimInstance || BoundAnimInstance == AnimInstance)
	{
		return;
	}

	UnbindMontageNotifyDelegates();
	BoundAnimInstance = AnimInstance;
	BoundAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageNotifyBegin);
}

void UTwoHeartsGA_NormalAttackBase::UnbindMontageNotifyDelegates()
{
	if (!BoundAnimInstance)
	{
		return;
	}

	BoundAnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageNotifyBegin);
	BoundAnimInstance = nullptr;
}

void UTwoHeartsGA_NormalAttackBase::ClearPhaseFallbackTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActivePhaseFallbackTimerHandle);
		World->GetTimerManager().ClearTimer(RecoveryPhaseFallbackTimerHandle);
		World->GetTimerManager().ClearTimer(LogicEndedFallbackTimerHandle);
		World->GetTimerManager().ClearTimer(NextSegmentAdvanceFallbackTimerHandle);
		World->GetTimerManager().ClearTimer(DeferredNextSegmentActivationTimerHandle);
	}
}

void UTwoHeartsGA_NormalAttackBase::SchedulePhaseFallbacks(float SectionLength)
{
	if (SectionLength <= 0.0f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		RecordAbilityEvent(
			TEXT("PhaseFallbackScheduleDetail"),
			FString::Printf(
				TEXT("Scheduling segment %d fallback timers. SectionLength=%.3f Active=%.3f Recovery=%.3f Advance=%.3f LogicEnded=%.3f."),
				NormalAttackSegment,
				SectionLength,
				SectionLength * ActivePhaseFallbackNormalizedTime,
				SectionLength * RecoveryPhaseFallbackNormalizedTime,
				SectionLength * NextSegmentAdvanceFallbackNormalizedTime,
				SectionLength * LogicEndedFallbackNormalizedTime),
			true);
		TimerManager.SetTimer(ActivePhaseFallbackTimerHandle, this, &UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterActive, SectionLength * ActivePhaseFallbackNormalizedTime, false);
		TimerManager.SetTimer(RecoveryPhaseFallbackTimerHandle, this, &UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterRecovery, SectionLength * RecoveryPhaseFallbackNormalizedTime, false);
		TimerManager.SetTimer(NextSegmentAdvanceFallbackTimerHandle, this, &UTwoHeartsGA_NormalAttackBase::HandleFallbackOpenNextSegmentAdvanceWindow, SectionLength * NextSegmentAdvanceFallbackNormalizedTime, false);
		TimerManager.SetTimer(LogicEndedFallbackTimerHandle, this, &UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterLogicEnded, SectionLength * LogicEndedFallbackNormalizedTime, false);
	}
}

bool UTwoHeartsGA_NormalAttackBase::IsLogicEndedPhase() const
{
	return CurrentCombatPhase == ETwoHeartsCombatPhase::LogicEnded;
}

void UTwoHeartsGA_NormalAttackBase::HandleDeferredNextSegmentActivation()
{
	AttemptDeferredNextSegmentActivation();
}

void UTwoHeartsGA_NormalAttackBase::AttemptDeferredNextSegmentActivation()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	const FGameplayTag NextAbilityTag = PendingNextSegmentAbilityTag;
	const int32 SourceSegment = PendingNextSegmentSourceSegment;

	PendingNextSegmentAbilityTag = FGameplayTag();
	PendingNextSegmentSourceSegment = 0;

	if (!AbilitySystemComponent || !NextAbilityTag.IsValid())
	{
		RestorePendingLateBufferedInput(TEXT("NormalAttackAdvance.InvalidDeferredState"));
		UpdateDebugState(false);
		RecordAbilityFailure(
			TEXT("AdvanceSegmentFailed"),
			FString::Printf(
				TEXT("Deferred next-segment activation from segment %d aborted because ability system or next tag was invalid."),
				SourceSegment));
		return;
	}

	RecordAbilityEvent(
		TEXT("AdvanceSegmentAttempt"),
		FString::Printf(
			TEXT("Deferred activation from segment %d targeting %s with %d activatable abilities."),
			SourceSegment,
			*NextAbilityTag.ToString(),
			AbilitySystemComponent->GetActivatableAbilities().Num()));

	TArray<FString> MatchingAbilityNames;
	TArray<FString> ActivationFailureReasons;
	bool bAttemptedActivation = false;

	auto BuildFailureReason = [](const FGameplayTagContainer& FailureTags) -> FString
	{
		return FailureTags.IsEmpty()
			? TEXT("CanActivateAbility returned false with no failure tags.")
			: FString::Printf(TEXT("Blocked by tags: %s"), *FailureTags.ToStringSimple());
	};

	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability || !AbilitySpec.Ability->GetAssetTags().HasTagExact(NextAbilityTag))
		{
			continue;
		}

		MatchingAbilityNames.Add(GetNameSafe(AbilitySpec.Ability));
		bAttemptedActivation = true;
		RecordAbilityEvent(
			TEXT("AdvanceSegmentCandidate"),
			FString::Printf(
				TEXT("Evaluating deferred candidate %s for segment %d. InputID=%d Active=%s Tags=%s."),
				*GetNameSafe(AbilitySpec.Ability),
				SourceSegment,
				AbilitySpec.InputID,
				AbilitySpec.IsActive() ? TEXT("true") : TEXT("false"),
				*AbilitySpec.Ability->GetAssetTags().ToStringSimple()),
			true);

		FGameplayTagContainer FailureTags;
		const bool bCanActivate = AbilitySpec.Ability->CanActivateAbility(
			AbilitySpec.Handle,
			AbilitySystemComponent->AbilityActorInfo.Get(),
			nullptr,
			nullptr,
			&FailureTags);

		if (!bCanActivate)
		{
			ActivationFailureReasons.Add(
				FString::Printf(TEXT("%s -> %s"), *GetNameSafe(AbilitySpec.Ability), *BuildFailureReason(FailureTags)));
			continue;
		}

		if (AbilitySystemComponent->TryActivateAbility(AbilitySpec.Handle))
		{
			ClearPendingLateBufferedInputRestore();
			RecordAbilityEvent(
				TEXT("AdvanceSegment"),
				FString::Printf(
					TEXT("Advanced from segment %d to %s via deferred activation."),
					SourceSegment,
					*GetNameSafe(AbilitySpec.Ability)),
				true);
			return;
		}

		ActivationFailureReasons.Add(
			FString::Printf(
				TEXT("%s -> TryActivateAbility failed after CanActivateAbility succeeded."),
				*GetNameSafe(AbilitySpec.Ability)));
	}

	UpdateDebugState(false);

	if (!bAttemptedActivation)
	{
		RestorePendingLateBufferedInput(TEXT("NormalAttackAdvance.NoMatchingAbility"));
		RecordAbilityFailure(
			TEXT("AdvanceSegmentFailed"),
			FString::Printf(
				TEXT("Deferred next-segment activation from segment %d found no ability with tag %s."),
				SourceSegment,
				*NextAbilityTag.ToString()));
		return;
	}

	RestorePendingLateBufferedInput(TEXT("NormalAttackAdvance.TryActivateFailed"));
	RecordAbilityFailure(
		TEXT("AdvanceSegmentFailed"),
		FString::Printf(
			TEXT("Failed to activate next normal attack segment from segment %d. Tag=%s. Matching=%s. Reasons=%s"),
			SourceSegment,
			*NextAbilityTag.ToString(),
			MatchingAbilityNames.IsEmpty() ? TEXT("None") : *FString::Join(MatchingAbilityNames, TEXT(", ")),
			ActivationFailureReasons.IsEmpty() ? TEXT("None") : *FString::Join(ActivationFailureReasons, TEXT(" | "))));
}

void UTwoHeartsGA_NormalAttackBase::ClearPendingLateBufferedInputRestore()
{
	bHasPendingLateBufferedInputRestore = false;
	PendingLateBufferedInputToRestore = FTwoHeartsBufferedCombatInput();
}

bool UTwoHeartsGA_NormalAttackBase::RestorePendingLateBufferedInput(const FString& Reason)
{
	if (!bHasPendingLateBufferedInputRestore)
	{
		return false;
	}

	UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	AtwoheartsCharacter* Character = GetTwoHeartsCharacter();
	const FTwoHeartsBufferedCombatInput BufferedInputToRestore = PendingLateBufferedInputToRestore;
	ClearPendingLateBufferedInputRestore();

	if (!ActionContextComponent)
	{
		return false;
	}

	const bool bRestored = ActionContextComponent->RestoreBufferedInput(BufferedInputToRestore, Reason);
	if (Character)
	{
		Character->PushCombatInputDebugEvent(
			TEXT("NormalAttack"),
			bRestored ? TEXT("BufferedRestored") : TEXT("BufferedConsumeFailed"),
			StaticEnum<ETwoHeartsCombatInputConsumptionRoute>()
				? StaticEnum<ETwoHeartsCombatInputConsumptionRoute>()->GetNameStringByValue(static_cast<int64>(BufferedInputToRestore.ConsumptionRoute))
				: TEXT("Unknown"),
			bRestored
				? FString::Printf(TEXT("Restored late buffered normal attack after deferred activation failure. Reason=%s."), *Reason)
				: FString::Printf(TEXT("Late buffered normal attack could not be restored after deferred activation failure. Reason=%s."), *Reason));
	}

	return bRestored;
}
