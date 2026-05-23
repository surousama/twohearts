#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
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
	bInterruptedByDodge = false;
	CurrentCombatPhase = ETwoHeartsCombatPhase::None;
	ActiveMontageTask = nullptr;

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
	}
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageCompleted()
{
	FinishSegment(false);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageInterrupted()
{
	FinishSegment(true);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageCancelled()
{
	FinishSegment(true);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
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

bool UTwoHeartsGA_NormalAttackBase::CanQueueNextSegment() const
{
	const bool bCanAcceptQueueByPhase = CurrentCombatPhase == ETwoHeartsCombatPhase::Startup || CurrentCombatPhase == ETwoHeartsCombatPhase::Active;
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
		FString::Printf(TEXT("Started segment %d with section %s."), NormalAttackSegment, *SectionName.ToString()));

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
	ClearPhaseFallbackTimers();

	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();

	if (!bWasCancelled)
	{
		EnterCombatPhase(ETwoHeartsCombatPhase::LogicEnded, TEXT("SegmentCompleted"));
	}

	const bool bShouldActivateNext = !bWasCancelled && bHasQueuedNextSegment && NextSegmentAbilityTag.IsValid() && AbilitySystemComponent;

	if (bWasCancelled)
	{
		RecordAbilityFailure(TEXT("SegmentInterrupted"), FString::Printf(TEXT("Segment %d was interrupted before completion."), NormalAttackSegment));
	}
	else
	{
		RecordAbilityEvent(
			TEXT("SegmentFinished"),
			FString::Printf(TEXT("Segment %d finished. QueuedNext=%s."), NormalAttackSegment, bHasQueuedNextSegment ? TEXT("true") : TEXT("false")));
	}

	bPreserveDebugStateUntilNextSegment = bShouldActivateNext;
	EndAbility(Handle, ActorInfo, ActivationInfo, true, bWasCancelled);

	if (!bShouldActivateNext)
	{
		return;
	}

	FGameplayTagContainer NextAbilityTags;
	NextAbilityTags.AddTag(NextSegmentAbilityTag);
	if (!AbilitySystemComponent->TryActivateAbilitiesByTag(NextAbilityTags, true))
	{
		UpdateDebugState(false);
		RecordAbilityFailure(
			TEXT("AdvanceSegmentFailed"),
			FString::Printf(TEXT("Failed to activate next normal attack segment from segment %d."), NormalAttackSegment));
		return;
	}

	RecordAbilityEvent(
		TEXT("AdvanceSegment"),
		FString::Printf(TEXT("Advancing from segment %d to next segment tag %s."), NormalAttackSegment, *NextSegmentAbilityTag.ToString()),
		true);
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
		TEXT("[GameplayAbility] ability=%s owner=%s avatar=%s detail=\"%s\""),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetAbilityOwnerActor()),
		*GetNameSafe(GetAbilityAvatarActor()),
		*Detail);
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
		TimerManager.SetTimer(ActivePhaseFallbackTimerHandle, this, &UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterActive, SectionLength * ActivePhaseFallbackNormalizedTime, false);
		TimerManager.SetTimer(RecoveryPhaseFallbackTimerHandle, this, &UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterRecovery, SectionLength * RecoveryPhaseFallbackNormalizedTime, false);
		TimerManager.SetTimer(LogicEndedFallbackTimerHandle, this, &UTwoHeartsGA_NormalAttackBase::HandleFallbackEnterLogicEnded, SectionLength * LogicEndedFallbackNormalizedTime, false);
	}
}

bool UTwoHeartsGA_NormalAttackBase::IsLogicEndedPhase() const
{
	return CurrentCombatPhase == ETwoHeartsCombatPhase::LogicEnded;
}
