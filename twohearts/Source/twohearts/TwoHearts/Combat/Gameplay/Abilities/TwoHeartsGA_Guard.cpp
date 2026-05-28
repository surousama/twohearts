#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h"

#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"
#include "twoheartsCharacter.h"

UTwoHeartsGA_Guard::UTwoHeartsGA_Guard()
{
	AddDefaultAssetTag(TAG_TwoHearts_Ability_Guard);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_Action_Guard);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_CannotInput);
}

void UTwoHeartsGA_Guard::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bGuardStarted = false;
	bGuardFinished = false;
	bGuardWindowActive = false;
	bHasRegisteredCombatActionContext = false;
	bHasMarkedCombatLogicEnded = false;
	bInterruptedByHitReaction = false;
	CurrentGuardPhase = ETwoHeartsCombatPhase::None;
	BoundHostileAttackReceiver.Reset();
	ClearGuardTimers();

	if (!CanStartGuardExecution())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanInterruptCurrentActionByGuard())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		RecordGuardEvent(TEXT("GuardRejected"), TEXT("CommitAbility failed."), true);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TryInterruptCurrentActionByGuard())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!StartGuardExecution())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UTwoHeartsGA_Guard::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearGuardTimers();
	UnbindHostileAttackReceiver();
	FinishCombatActionContext(bWasCancelled);
	UpdateGuardDebugState();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UTwoHeartsGA_Guard::TryInterruptByAction(ETwoHeartsCombatActionType InterruptingActionType, const FString& InterruptReason)
{
	if (InterruptingActionType != ETwoHeartsCombatActionType::HitReaction)
	{
		return false;
	}

	bInterruptedByHitReaction = true;
	MarkCombatActionLogicEnded(InterruptReason);
	RecordGuardEvent(TEXT("GuardInterruptedByAction"), TEXT("Guard was interrupted by hit reaction."));
	FinishGuard(true);
	return true;
}

UTwoHeartsGA_NormalAttackBase* UTwoHeartsGA_Guard::FindActiveNormalAttackAbility() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		if (UTwoHeartsGA_NormalAttackBase* ActiveNormalAttack = Cast<UTwoHeartsGA_NormalAttackBase>(AbilitySpec.GetPrimaryInstance()))
		{
			return ActiveNormalAttack;
		}
	}

	return nullptr;
}

UTwoHeartsGA_Dodge* UTwoHeartsGA_Guard::FindActiveDodgeAbility() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		if (UTwoHeartsGA_Dodge* ActiveDodge = Cast<UTwoHeartsGA_Dodge>(AbilitySpec.GetPrimaryInstance()))
		{
			return ActiveDodge;
		}
	}

	return nullptr;
}

UTwoHeartsCombatActionContextComponent* UTwoHeartsGA_Guard::GetCombatActionContextComponent() const
{
	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		return Character->GetCombatActionContextComponent();
	}

	return nullptr;
}

UTwoHeartsHostileAttackReceiverComponent* UTwoHeartsGA_Guard::GetHostileAttackReceiverComponent() const
{
	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		return Character->GetHostileAttackReceiverComponent();
	}

	return nullptr;
}

bool UTwoHeartsGA_Guard::CanStartGuardExecution() const
{
	const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	const UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	const FTwoHeartsGuardConfig* GuardConfig = GetGuardConfig();
	const UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent = GetHostileAttackReceiverComponent();
	if (!Character || !AbilitySystemComponent || !GuardConfig || !ReceiverComponent)
	{
		RecordGuardEvent(TEXT("GuardRejected"), TEXT("Character, ability system component, guard config, or hostile attack receiver was invalid."), true);
		return false;
	}

	return true;
}

bool UTwoHeartsGA_Guard::CanInterruptCurrentActionByGuard() const
{
	const UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	if (!ActionContextComponent || !ActionContextComponent->HasActiveAction())
	{
		return true;
	}

	const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
	if (CurrentContext.ActionType == ETwoHeartsCombatActionType::Guard)
	{
		RecordGuardEvent(TEXT("GuardRejected"), TEXT("Guard is already the current active action."), true);
		return false;
	}

	if (CurrentContext.ActionType != ETwoHeartsCombatActionType::NormalAttack
		&& CurrentContext.ActionType != ETwoHeartsCombatActionType::Dodge)
	{
		RecordGuardEvent(
			TEXT("GuardRejected"),
			FString::Printf(
				TEXT("Current action %s is outside the current basic guard whitelist."),
				*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType))),
			true);
		return false;
	}

	if (!ActionContextComponent->CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType::Guard))
	{
		RecordGuardEvent(
			TEXT("GuardRejected"),
			FString::Printf(
				TEXT("Public action context rejected guard interrupt for %s during phase %s."),
				*CurrentContext.ActionInstanceName,
				*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase))),
			true);
		return false;
	}

	return true;
}

bool UTwoHeartsGA_Guard::TryInterruptCurrentActionByGuard()
{
	const UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	if (!ActionContextComponent || !ActionContextComponent->HasActiveAction())
	{
		return true;
	}

	const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
	if (CurrentContext.ActionType == ETwoHeartsCombatActionType::NormalAttack)
	{
		UTwoHeartsGA_NormalAttackBase* ActiveNormalAttack = FindActiveNormalAttackAbility();
		if (!ActiveNormalAttack)
		{
			RecordGuardEvent(
				TEXT("GuardRejected"),
				FString::Printf(TEXT("Expected an active normal attack instance for %s, but none was found."), *CurrentContext.ActionInstanceName),
				true);
			return false;
		}

		if (!ActiveNormalAttack->TryInterruptByAction(ETwoHeartsCombatActionType::Guard, TEXT("InterruptedByGuard")))
		{
			RecordGuardEvent(
				TEXT("GuardRejected"),
				FString::Printf(TEXT("Guard failed to interrupt normal attack action %s."), *CurrentContext.ActionInstanceName),
				true);
			return false;
		}

		RecordGuardEvent(TEXT("GuardInterruptedAction"), FString::Printf(TEXT("Interrupted %s through the public action context."), *CurrentContext.ActionInstanceName));
		return true;
	}

	if (CurrentContext.ActionType == ETwoHeartsCombatActionType::Dodge)
	{
		UTwoHeartsGA_Dodge* ActiveDodge = FindActiveDodgeAbility();
		if (!ActiveDodge)
		{
			RecordGuardEvent(
				TEXT("GuardRejected"),
				FString::Printf(TEXT("Expected an active dodge instance for %s, but none was found."), *CurrentContext.ActionInstanceName),
				true);
			return false;
		}

		if (!ActiveDodge->TryInterruptByAction(ETwoHeartsCombatActionType::Guard, TEXT("InterruptedByGuard")))
		{
			RecordGuardEvent(
				TEXT("GuardRejected"),
				FString::Printf(TEXT("Guard failed to interrupt dodge action %s."), *CurrentContext.ActionInstanceName),
				true);
			return false;
		}

		RecordGuardEvent(TEXT("GuardInterruptedAction"), FString::Printf(TEXT("Interrupted %s through the public action context."), *CurrentContext.ActionInstanceName));
	}

	return true;
}

bool UTwoHeartsGA_Guard::StartGuardExecution()
{
	UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent = GetHostileAttackReceiverComponent();
	const FTwoHeartsGuardConfig* GuardConfig = GetGuardConfig();
	if (!ReceiverComponent || !GuardConfig)
	{
		RecordGuardEvent(TEXT("GuardRejected"), TEXT("Hostile attack receiver or guard config was invalid during startup."), true);
		return false;
	}

	bGuardStarted = true;
	bGuardFinished = false;
	bGuardWindowActive = false;

	BindHostileAttackReceiver(ReceiverComponent);
	SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::Startup, TEXT("GuardActivated"));
	UpdateGuardDebugState();
	RecordGuardEvent(
		TEXT("GuardActivate"),
		FString::Printf(
			TEXT("input_mode=%s startup=%.2f window=%.2f recovery=%.2f"),
			GuardConfig->InputMode == ETwoHeartsGuardInputMode::HoldReserved ? TEXT("HoldReserved") : TEXT("TapWindowOnly"),
			GuardConfig->GuardStartupSeconds,
			GuardConfig->GuardWindowSeconds,
			GuardConfig->GuardRecoverySeconds));

	if (UWorld* World = GetWorld())
	{
		const float StartupSeconds = FMath::Max(0.0f, GuardConfig->GuardStartupSeconds);
		if (StartupSeconds <= 0.0f)
		{
			HandleStartupFinished();
		}
		else
		{
			World->GetTimerManager().SetTimer(GuardStartupTimerHandle, this, &UTwoHeartsGA_Guard::HandleStartupFinished, StartupSeconds, false);
		}
	}

	return true;
}

void UTwoHeartsGA_Guard::SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase NewPhase, const FString& Reason)
{
	CurrentGuardPhase = NewPhase;
	UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	if (!ActionContextComponent)
	{
		return;
	}

	if (!bHasRegisteredCombatActionContext)
	{
		FTwoHeartsCombatActionRegistration Registration;
		Registration.ActionType = ETwoHeartsCombatActionType::Guard;
		Registration.InitialPhase = NewPhase;
		Registration.AbilityTag = TAG_TwoHearts_Ability_Guard;
		Registration.ActionStateTag = TAG_TwoHearts_State_Action_Guard;
		Registration.ActionInstanceName = TEXT("Guard.Basic");

		ActionContextComponent->BeginAction(Registration, Reason);
		bHasRegisteredCombatActionContext = true;
		return;
	}

	if (NewPhase == ETwoHeartsCombatPhase::LogicEnded)
	{
		ActionContextComponent->MarkLogicEnded(Reason);
		bHasMarkedCombatLogicEnded = true;
		return;
	}

	ActionContextComponent->TransitionToPhase(NewPhase, Reason);
}

void UTwoHeartsGA_Guard::MarkCombatActionLogicEnded(const FString& Reason)
{
	if (bHasMarkedCombatLogicEnded)
	{
		return;
	}

	SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::LogicEnded, Reason);
}

void UTwoHeartsGA_Guard::FinishCombatActionContext(bool bWasCancelled)
{
	if (!bHasRegisteredCombatActionContext)
	{
		return;
	}

	if (UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent())
	{
		const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
		if (CurrentContext.ActionType == ETwoHeartsCombatActionType::Guard)
		{
			const ETwoHeartsCombatActionEndReason EndReason = bWasCancelled
				? (bInterruptedByHitReaction ? ETwoHeartsCombatActionEndReason::Interrupted : ETwoHeartsCombatActionEndReason::Cancelled)
				: ETwoHeartsCombatActionEndReason::Completed;
			const FString FinishReason = bWasCancelled
				? (bInterruptedByHitReaction ? TEXT("InterruptedByHitReaction") : TEXT("GuardCancelled"))
				: TEXT("GuardEnded");
			ActionContextComponent->FinishAction(EndReason, FinishReason);
		}
	}

	if (!bWasCancelled)
	{
		if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
		{
			Character->TryConsumeReservedCombatInput(TEXT("GuardEnded"));
		}
	}

	bHasRegisteredCombatActionContext = false;
	bHasMarkedCombatLogicEnded = false;
	bInterruptedByHitReaction = false;
	CurrentGuardPhase = ETwoHeartsCombatPhase::None;
}

void UTwoHeartsGA_Guard::FinishGuard(bool bWasCancelled)
{
	if (bGuardFinished)
	{
		return;
	}

	bGuardFinished = true;
	bGuardStarted = false;
	bGuardWindowActive = false;
	MarkCombatActionLogicEnded(bWasCancelled ? TEXT("GuardCancelled") : TEXT("GuardFinished"));
	RecordGuardEvent(
		TEXT("GuardFinished"),
		bWasCancelled ? TEXT("Guard was cancelled.") : TEXT("Guard finished its active and recovery lifecycle."));

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCancelled);
}

void UTwoHeartsGA_Guard::ClearGuardTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(GuardStartupTimerHandle);
		TimerManager.ClearTimer(GuardWindowTimerHandle);
		TimerManager.ClearTimer(GuardRecoveryTimerHandle);
	}
}

void UTwoHeartsGA_Guard::BindHostileAttackReceiver(UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent)
{
	if (!ReceiverComponent || BoundHostileAttackReceiver.Get() == ReceiverComponent)
	{
		return;
	}

	UnbindHostileAttackReceiver();
	BoundHostileAttackReceiver = ReceiverComponent;
	ReceiverComponent->OnPlayerHitResultUpdated.AddDynamic(this, &UTwoHeartsGA_Guard::HandlePlayerHitResultUpdated);
}

void UTwoHeartsGA_Guard::UnbindHostileAttackReceiver()
{
	if (UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent = BoundHostileAttackReceiver.Get())
	{
		ReceiverComponent->OnPlayerHitResultUpdated.RemoveDynamic(this, &UTwoHeartsGA_Guard::HandlePlayerHitResultUpdated);
	}

	BoundHostileAttackReceiver.Reset();
}

void UTwoHeartsGA_Guard::UpdateGuardDebugState() const
{
	if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		const bool bHoldReserved = Character->GetGuardConfig().InputMode == ETwoHeartsGuardInputMode::HoldReserved;
		const FString PhaseName = StaticEnum<ETwoHeartsCombatPhase>()
			? StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentGuardPhase))
			: TEXT("Unknown");
		Character->SetGuardDebugRuntimeState(bGuardStarted && !bGuardFinished, bGuardWindowActive, PhaseName, bHoldReserved);
	}
}

void UTwoHeartsGA_Guard::RecordGuardEvent(const TCHAR* EventName, const FString& Detail, bool bWarning) const
{
	if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		Character->PushGuardDebugEvent(EventName, Detail);
	}

	if (bWarning)
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[Guard] ability=%s owner=%s avatar=%s event=%s detail=\"%s\""),
			*GetNameSafe(GetClass()),
			*GetNameSafe(GetAbilityOwnerActor()),
			*GetNameSafe(GetAbilityAvatarActor()),
			EventName,
			*Detail);
		return;
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[Guard] ability=%s owner=%s avatar=%s event=%s detail=\"%s\""),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetAbilityOwnerActor()),
		*GetNameSafe(GetAbilityAvatarActor()),
		EventName,
		*Detail);
}

const FTwoHeartsGuardConfig* UTwoHeartsGA_Guard::GetGuardConfig() const
{
	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		return &Character->GetGuardConfig();
	}

	return nullptr;
}

void UTwoHeartsGA_Guard::HandleStartupFinished()
{
	if (!bGuardStarted || bGuardFinished)
	{
		return;
	}

	const FTwoHeartsGuardConfig* GuardConfig = GetGuardConfig();
	if (!GuardConfig)
	{
		FinishGuard(true);
		return;
	}

	bGuardWindowActive = true;
	SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::Active, TEXT("GuardWindowOpened"));
	UpdateGuardDebugState();
	RecordGuardEvent(TEXT("GuardWindowOpened"), TEXT("Basic guard is now inside its active rewrite window."));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			GuardWindowTimerHandle,
			this,
			&UTwoHeartsGA_Guard::HandleGuardWindowFinished,
			FMath::Max(0.01f, GuardConfig->GuardWindowSeconds),
			false);
	}
}

void UTwoHeartsGA_Guard::HandleGuardWindowFinished()
{
	if (!bGuardStarted || bGuardFinished)
	{
		return;
	}

	const FTwoHeartsGuardConfig* GuardConfig = GetGuardConfig();
	if (!GuardConfig)
	{
		FinishGuard(true);
		return;
	}

	bGuardWindowActive = false;
	SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::Recovery, TEXT("GuardWindowClosed"));
	UpdateGuardDebugState();
	RecordGuardEvent(TEXT("GuardWindowClosed"), TEXT("Basic guard left its active rewrite window and entered recovery."));

	if (UWorld* World = GetWorld())
	{
		const float RecoverySeconds = FMath::Max(0.0f, GuardConfig->GuardRecoverySeconds);
		if (RecoverySeconds <= 0.0f)
		{
			HandleGuardRecoveryFinished();
		}
		else
		{
			World->GetTimerManager().SetTimer(
				GuardRecoveryTimerHandle,
				this,
				&UTwoHeartsGA_Guard::HandleGuardRecoveryFinished,
				RecoverySeconds,
				false);
		}
	}
}

void UTwoHeartsGA_Guard::HandleGuardRecoveryFinished()
{
	if (!bGuardStarted || bGuardFinished)
	{
		return;
	}

	FinishGuard(false);
}

void UTwoHeartsGA_Guard::HandlePlayerHitResultUpdated(const FTwoHeartsPlayerHitResult& HitResult)
{
	if (!bGuardStarted || bGuardFinished || !bGuardWindowActive)
	{
		return;
	}

	if (HitResult.ResultType != ETwoHeartsPlayerHitResultType::HitConfirmed || !HitResult.bCanBeRewrittenByGuard)
	{
		return;
	}

	UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent = BoundHostileAttackReceiver.Get();
	if (!ReceiverComponent)
	{
		return;
	}

	if (!ReceiverComponent->RewriteLastPlayerHitResultForGuard(
		ETwoHeartsPlayerHitResultType::GuardRewritten,
		TEXT("Basic guard rewrote the hostile attack contact during the active guard window.")))
	{
		RecordGuardEvent(
			TEXT("GuardRewriteFailed"),
			FString::Printf(TEXT("Guard attempted to rewrite attack %s but the receiver rejected it."), *HitResult.AttackInstanceName),
			true);
		return;
	}

	RecordGuardEvent(
		TEXT("GuardRewriteSuccess"),
		FString::Printf(TEXT("Guard rewrote hostile attack %s into GuardRewritten."), *HitResult.AttackInstanceName));
}
