#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h"

#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"
#include "twoheartsCharacter.h"

namespace
{
	void FinishGuardSuccessCooldown(
		TWeakObjectPtr<UAbilitySystemComponent> WeakAbilitySystemComponent,
		TWeakObjectPtr<AtwoheartsCharacter> WeakCharacter)
	{
		UAbilitySystemComponent* AbilitySystemComponent = WeakAbilitySystemComponent.Get();
		if (!AbilitySystemComponent)
		{
			UE_LOG(
				LogtwoheartsCombatTest,
				Warning,
				TEXT("[Guard] event=GuardCooldownClearFailed detail=\"Ability system component was invalid when cooldown timer completed.\""));
			return;
		}

		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_TwoHearts_Cooldown_Guard);

		if (AtwoheartsCharacter* Character = WeakCharacter.Get())
		{
			Character->PushGuardDebugEvent(TEXT("GuardCooldownReady"), TEXT("Guard success cooldown finished."));
		}

		UE_LOG(
			LogtwoheartsCombatTest,
			Verbose,
			TEXT("[Guard] event=GuardCooldownReady detail=\"Guard success cooldown finished and Cooldown.Guard was removed from the ASC.\""));
	}
}

UTwoHeartsGA_Guard::UTwoHeartsGA_Guard()
{
	AddDefaultAssetTag(TAG_TwoHearts_Ability_Guard);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_Action_Guard);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_CannotInput);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_Cooldown_Guard);
}


void UTwoHeartsGA_Guard::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	CachedAbilitySystemComponent.Reset();
	bGuardStarted = false;
	bGuardFinished = false;
	bGuardWindowActive = false;
	bHasAppliedSuccessCooldownThisActivation = false;
	bHasRegisteredCombatActionContext = false;
	bHasMarkedCombatLogicEnded = false;
	bInterruptedByHitReaction = false;
	CurrentGuardPhase = ETwoHeartsCombatPhase::None;
	BoundHostileAttackReceiver.Reset();
	LastEvaluatedAttackInstanceName.Reset();
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
	CachedAbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();

	if (UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent())
	{
		ActionContextComponent->ClearBufferedInput(TEXT("ClearedByGuardStart"));
	}

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

bool UTwoHeartsGA_Guard::ApplyGuardSuccessCooldown()
{
	if (bHasAppliedSuccessCooldownThisActivation)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = CachedAbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
		CachedAbilitySystemComponent = AbilitySystemComponent;
	}

	const FTwoHeartsGuardConfig* GuardConfig = GetGuardConfig();
	if (!AbilitySystemComponent || !GuardConfig)
	{
		return false;
	}

	const float CooldownSeconds = FMath::Max(0.0f, GuardConfig->GuardSuccessCooldownSeconds);
	if (CooldownSeconds <= 0.0f)
	{
		return false;
	}

	AbilitySystemComponent->AddLooseGameplayTag(TAG_TwoHearts_Cooldown_Guard);
	bCooldownActive = true;
	bHasAppliedSuccessCooldownThisActivation = true;
	RecordGuardEvent(TEXT("GuardCooldownBegin"), FString::Printf(TEXT("Guard success cooldown started for %.2f seconds."), CooldownSeconds));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GuardCooldownTimerHandle);

		const TWeakObjectPtr<UAbilitySystemComponent> WeakAbilitySystemComponent(AbilitySystemComponent);
		const TWeakObjectPtr<AtwoheartsCharacter> WeakCharacter(Cast<AtwoheartsCharacter>(GetAbilityCharacter()));
		FTimerDelegate CooldownFinishedDelegate = FTimerDelegate::CreateLambda(
			[WeakAbilitySystemComponent, WeakCharacter]()
			{
				FinishGuardSuccessCooldown(WeakAbilitySystemComponent, WeakCharacter);
			});
		World->GetTimerManager().SetTimer(GuardCooldownTimerHandle, CooldownFinishedDelegate, CooldownSeconds, false);
	}

	return true;
}

void UTwoHeartsGA_Guard::BindHostileAttackReceiver(UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent)

{
	if (!ReceiverComponent || BoundHostileAttackReceiver.Get() == ReceiverComponent)
	{
		return;
	}

	UnbindHostileAttackReceiver();
	BoundHostileAttackReceiver = ReceiverComponent;
	ReceiverComponent->OnHostileAttackSignalReceived.AddDynamic(this, &UTwoHeartsGA_Guard::HandleHostileAttackSignalReceived);
}

void UTwoHeartsGA_Guard::UnbindHostileAttackReceiver()
{
	if (UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent = BoundHostileAttackReceiver.Get())
	{
		ReceiverComponent->OnHostileAttackSignalReceived.RemoveDynamic(this, &UTwoHeartsGA_Guard::HandleHostileAttackSignalReceived);
	}

	BoundHostileAttackReceiver.Reset();
	LastEvaluatedAttackInstanceName.Reset();
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

	DrawGuardDebugGeometry();
}

void UTwoHeartsGA_Guard::DrawGuardDebugGeometry() const
{
	const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	if (!Character || !bGuardStarted || bGuardFinished)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTwoHeartsAttackMetadata* AttackMetadata = nullptr;
	if (const UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent = BoundHostileAttackReceiver.Get())
	{
		if (ReceiverComponent->HasReceivedHostileAttackSignal())
		{
			AttackMetadata = &ReceiverComponent->GetLastSignal().AttackMetadata;
		}
	}

	const float GuardDistance = AttackMetadata ? AttackMetadata->GuardMaxDistance : 220.0f;
	const float GuardHalfAngleDegrees = AttackMetadata ? AttackMetadata->GuardFacingHalfAngleDegrees : 100.0f;
	const float GuardHeightTolerance = AttackMetadata ? AttackMetadata->GuardMaxHeightDifference : 120.0f;
	const FVector Origin = Character->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector Forward2D = Character->GetActorForwardVector().GetSafeNormal2D();
	const FVector LeftDirection = FRotator(0.0f, -GuardHalfAngleDegrees, 0.0f).RotateVector(Forward2D);
	const FVector RightDirection = FRotator(0.0f, GuardHalfAngleDegrees, 0.0f).RotateVector(Forward2D);
	const FColor GuardColor = bGuardWindowActive ? FColor::Green : FColor::Yellow;
	const float LifetimeSeconds = 0.20f;

	DrawDebugDirectionalArrow(World, Origin, Origin + (Forward2D * GuardDistance), 28.0f, GuardColor, false, LifetimeSeconds, 0, 2.2f);
	DrawDebugLine(World, Origin, Origin + (LeftDirection * GuardDistance), GuardColor, false, LifetimeSeconds, 0, 1.6f);
	DrawDebugLine(World, Origin, Origin + (RightDirection * GuardDistance), GuardColor, false, LifetimeSeconds, 0, 1.6f);
	DrawDebugCircle(World, Origin, GuardDistance, 32, GuardColor, false, LifetimeSeconds, 0, 1.0f, FVector::UpVector, LeftDirection.GetSafeNormal());
	DrawDebugCircle(World, Origin + FVector(0.0f, 0.0f, GuardHeightTolerance), 24.0f, 16, FColor::Cyan, false, LifetimeSeconds, 0, 1.0f, FVector::ForwardVector, FVector::RightVector);
	DrawDebugCircle(World, Origin - FVector(0.0f, 0.0f, GuardHeightTolerance), 24.0f, 16, FColor::Cyan, false, LifetimeSeconds, 0, 1.0f, FVector::ForwardVector, FVector::RightVector);
	DrawDebugLine(World, Origin + FVector(0.0f, 0.0f, GuardHeightTolerance), Origin - FVector(0.0f, 0.0f, GuardHeightTolerance), FColor::Cyan, false, LifetimeSeconds, 0, 1.2f);
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

void UTwoHeartsGA_Guard::HandleHostileAttackSignalReceived(const FTwoHeartsHostileAttackSignal& Signal)
{
	if (!bGuardStarted || bGuardFinished)
	{
		return;
	}

	if (Signal.SignalType != ETwoHeartsHostileAttackSignalType::AttackContact || !Signal.bHasContact)
	{
		return;
	}

	if (Signal.AttackInstanceName.IsEmpty() || Signal.AttackInstanceName.Equals(TEXT("None"), ESearchCase::CaseSensitive))
	{
		RecordGuardEvent(TEXT("GuardRuleInvalid"), TEXT("Guard received an attack contact signal without a valid attack instance name."), true);
		return;
	}

	if (LastEvaluatedAttackInstanceName.Equals(Signal.AttackInstanceName, ESearchCase::CaseSensitive))
	{
		return;
	}


	LastEvaluatedAttackInstanceName = Signal.AttackInstanceName;
	TryEvaluateGuardAgainstAttackSignal(Signal);
}

bool UTwoHeartsGA_Guard::TryEvaluateGuardAgainstAttackSignal(const FTwoHeartsHostileAttackSignal& Signal)
{
	UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent = BoundHostileAttackReceiver.Get();
	AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	if (!ReceiverComponent || !Character)
	{
		RecordGuardEvent(TEXT("GuardRuleInvalid"), TEXT("Guard rule evaluation failed because the receiver or defender character was invalid."), true);
		return false;
	}

	if (!Signal.AttackMetadata.bCanBeGuarded)
	{
		RecordGuardEvent(
			TEXT("GuardAttackUnguardable"),
			FString::Printf(TEXT("Attack %s is marked as unguardable and therefore bypassed Guard."), *Signal.AttackInstanceName));
		return false;
	}

	if (!bGuardWindowActive)
	{
		RecordGuardEvent(
			CurrentGuardPhase == ETwoHeartsCombatPhase::Startup ? TEXT("GuardFailedTooEarly") : TEXT("GuardFailedTooLate"),
			BuildGuardFailureDetailFromSignal(Signal));
		return false;
	}

	const FVector DefenderLocation = Character->GetActorLocation();
	const FVector SourceLocation = Signal.AttackMetadata.SourceLocation.IsNearlyZero()
		? Signal.SourceLocation
		: Signal.AttackMetadata.SourceLocation;
	const float Distance2D = FVector::Dist2D(DefenderLocation, SourceLocation);
	const float HeightDifference = FMath::Abs(SourceLocation.Z - DefenderLocation.Z);
	float FacingDegrees = 0.0f;
	const FVector ToSource2D = (SourceLocation - DefenderLocation).GetSafeNormal2D();
	if (!ToSource2D.IsNearlyZero())
	{
		const float FacingDot = FVector::DotProduct(Character->GetActorForwardVector().GetSafeNormal2D(), ToSource2D);
		FacingDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FacingDot, -1.0f, 1.0f)));
	}

	RecordGuardEvent(
		TEXT("GuardRuleEvaluate"),
		FString::Printf(
			TEXT("attack=%s guardable=%s guard_window=%s phase=%s timing=%s/%s dist=%.1f/%.1f height=%.1f/%.1f angle=%.1f/%.1f"),
			*Signal.AttackInstanceName,
			Signal.AttackMetadata.bCanBeGuarded ? TEXT("true") : TEXT("false"),
			bGuardWindowActive ? TEXT("true") : TEXT("false"),
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentGuardPhase)),
			*StaticEnum<ETwoHeartsAttackTimingPhase>()->GetNameStringByValue(static_cast<int64>(Signal.AttackMetadata.TimingPhase)),
			Signal.AttackMetadata.TimingWindowName.IsNone() ? TEXT("None") : *Signal.AttackMetadata.TimingWindowName.ToString(),
			Distance2D,
			Signal.AttackMetadata.GuardMaxDistance,
			HeightDifference,
			Signal.AttackMetadata.GuardMaxHeightDifference,
			FacingDegrees,
			Signal.AttackMetadata.GuardFacingHalfAngleDegrees));

	if (UWorld* World = GetWorld())
	{
		const FVector DebugStart = DefenderLocation + FVector(0.0f, 0.0f, 60.0f);
		const FVector DebugEnd = SourceLocation + FVector(0.0f, 0.0f, 60.0f);
		DrawDebugLine(World, DebugStart, DebugEnd, bGuardWindowActive ? FColor::Green : FColor::Red, false, 1.2f, 0, 2.4f);
		DrawDebugSphere(World, DebugEnd, 18.0f, 12, Signal.AttackMetadata.bCanBeGuarded ? FColor::Orange : FColor::Red, false, 1.2f, 0, 1.8f);
	}

	if (Distance2D > Signal.AttackMetadata.GuardMaxDistance)
	{
		RecordGuardEvent(TEXT("GuardFailedDistance"), BuildGuardFailureDetailFromSignal(Signal));
		return false;
	}

	if (HeightDifference > Signal.AttackMetadata.GuardMaxHeightDifference)
	{
		RecordGuardEvent(TEXT("GuardFailedHeight"), BuildGuardFailureDetailFromSignal(Signal));
		return false;
	}

	if (!ToSource2D.IsNearlyZero())
	{
		const float FacingDot = FVector::DotProduct(Character->GetActorForwardVector().GetSafeNormal2D(), ToSource2D);
		const float MinFacingDot = FMath::Cos(FMath::DegreesToRadians(Signal.AttackMetadata.GuardFacingHalfAngleDegrees));
		if (FacingDot < MinFacingDot)
		{
			RecordGuardEvent(TEXT("GuardFailedAngle"), BuildGuardFailureDetailFromSignal(Signal));
			return false;
		}
	}

	const bool bAppliedGuardCooldown = ApplyGuardSuccessCooldown();
	const float GuardCooldownSeconds = Character->GetGuardConfig().GuardSuccessCooldownSeconds;
	FTwoHeartsGuardSettlementRequest SettlementRequest;
	SettlementRequest.RewrittenHitResultType = ETwoHeartsPlayerHitResultType::GuardRewritten;
	SettlementRequest.AttackInstanceName = Signal.AttackInstanceName;
	SettlementRequest.RewriteDetail = FString::Printf(

		TEXT("Guard satisfied guardable/timing/position rules and rewrote hostile attack %s into GuardRewritten."),
		*Signal.AttackInstanceName);
	SettlementRequest.bConsumesGuardResource = false;
	SettlementRequest.bRefundsGuardResource = false;
	SettlementRequest.bAppliesGuardCooldown = bAppliedGuardCooldown;
	SettlementRequest.GuardCooldownSeconds = bAppliedGuardCooldown ? FMath::Max(0.0f, GuardCooldownSeconds) : 0.0f;
	SettlementRequest.GuardCooldownTag = TAG_TwoHearts_Cooldown_Guard;

	if (!ReceiverComponent->RewriteLastPlayerHitResultForGuard(SettlementRequest))
	{
		if (bAppliedGuardCooldown)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(GuardCooldownTimerHandle);
			}
			if (UAbilitySystemComponent* AbilitySystemComponent = CachedAbilitySystemComponent.Get())
			{
				AbilitySystemComponent->RemoveLooseGameplayTag(TAG_TwoHearts_Cooldown_Guard);
			}
			bCooldownActive = false;
			bHasAppliedSuccessCooldownThisActivation = false;
		}

		RecordGuardEvent(
			TEXT("GuardRewriteFailed"),
			FString::Printf(TEXT("Guard satisfied its rules for attack %s, but the receiver rejected the rewrite."), *Signal.AttackInstanceName),
			true);
		return false;
	}


	const UEnum* DisplacementEnum = StaticEnum<ETwoHeartsGuardDisplacementResult>();
	const UEnum* DamageEnum = StaticEnum<ETwoHeartsGuardDamageResult>();
	RecordGuardEvent(
		TEXT("GuardRuleSuccess"),
		FString::Printf(
			TEXT("attack=%s displacement=%s damage=%s cooldown=%s/%.2f resource=consume:false refund:false"),
			*Signal.AttackInstanceName,
			DisplacementEnum ? *DisplacementEnum->GetNameStringByValue(static_cast<int64>(Signal.AttackMetadata.GuardSuccessDisplacementResult)) : TEXT("Unknown"),
			DamageEnum ? *DamageEnum->GetNameStringByValue(static_cast<int64>(Signal.AttackMetadata.GuardSuccessDamageResult)) : TEXT("Unknown"),
			bAppliedGuardCooldown ? TEXT("true") : TEXT("false"),
			SettlementRequest.GuardCooldownSeconds));
	return true;
}


FString UTwoHeartsGA_Guard::BuildGuardFailureDetailFromSignal(const FTwoHeartsHostileAttackSignal& Signal) const
{
	const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	const FVector DefenderLocation = Character ? Character->GetActorLocation() : FVector::ZeroVector;
	const FVector SourceLocation = Signal.AttackMetadata.SourceLocation.IsNearlyZero()
		? Signal.SourceLocation
		: Signal.AttackMetadata.SourceLocation;
	const float Distance2D = FVector::Dist2D(DefenderLocation, SourceLocation);
	const float HeightDifference = FMath::Abs(SourceLocation.Z - DefenderLocation.Z);

	float FacingDegrees = 0.0f;
	if (Character)
	{
		const FVector ToSource2D = (SourceLocation - DefenderLocation).GetSafeNormal2D();
		if (!ToSource2D.IsNearlyZero())
		{
			const float FacingDot = FVector::DotProduct(Character->GetActorForwardVector().GetSafeNormal2D(), ToSource2D);
			FacingDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FacingDot, -1.0f, 1.0f)));
		}
	}

	if (!Signal.AttackMetadata.bCanBeGuarded)
	{
		return FString::Printf(TEXT("Attack %s is configured as unguardable."), *Signal.AttackInstanceName);
	}

	if (!bGuardWindowActive)
	{
		return FString::Printf(
			TEXT("Attack %s arrived while Guard was outside its active window. phase=%s"),
			*Signal.AttackInstanceName,
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentGuardPhase)));
	}

	if (Distance2D > Signal.AttackMetadata.GuardMaxDistance)
	{
		return FString::Printf(
			TEXT("Attack %s was outside Guard distance. actual=%.1f allowed=%.1f"),
			*Signal.AttackInstanceName,
			Distance2D,
			Signal.AttackMetadata.GuardMaxDistance);
	}

	if (HeightDifference > Signal.AttackMetadata.GuardMaxHeightDifference)
	{
		return FString::Printf(
			TEXT("Attack %s was outside Guard height tolerance. actual=%.1f allowed=%.1f"),
			*Signal.AttackInstanceName,
			HeightDifference,
			Signal.AttackMetadata.GuardMaxHeightDifference);
	}

	return FString::Printf(
		TEXT("Attack %s came from outside the Guard facing sector. actual=%.1f allowed=%.1f"),
		*Signal.AttackInstanceName,
		FacingDegrees,
		Signal.AttackMetadata.GuardFacingHalfAngleDegrees);
}
