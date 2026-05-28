#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"
#include "twoheartsCharacter.h"

namespace
{
void FinishDodgeCooldown(
	TWeakObjectPtr<UAbilitySystemComponent> WeakAbilitySystemComponent,
	TWeakObjectPtr<AtwoheartsCharacter> WeakCharacter)
{
	UAbilitySystemComponent* AbilitySystemComponent = WeakAbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[Dodge] event=DodgeCooldownClearFailed detail=\"Ability system component was invalid when cooldown timer completed.\""));
		return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(TAG_TwoHearts_Cooldown_Dodge);

	if (AtwoheartsCharacter* Character = WeakCharacter.Get())
	{
		Character->SetDodgeDebugRuntimeState(
			Character->IsDodgingDebugState(),
			Character->IsDodgeInvulnerableDebugState(),
			true,
			Character->GetCurrentDodgeDirectionDebugState());
		Character->PushDodgeDebugEvent(TEXT("DodgeCooldownReady"), TEXT("Dodge cooldown finished."));
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[Dodge] event=DodgeCooldownReady detail=\"Dodge cooldown finished and Cooldown.Dodge was removed from the ASC.\""));
}
}

UTwoHeartsGA_Dodge::UTwoHeartsGA_Dodge()
{
	AddDefaultAssetTag(TAG_TwoHearts_Ability_Dodge);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_Action_Dodge);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_CannotInput);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_State_CannotInput);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_State_CannotDodge);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_Cooldown_Dodge);
}

void UTwoHeartsGA_Dodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	CachedAbilitySystemComponent.Reset();
	ActiveMontageTask = nullptr;
	BoundAnimInstance = nullptr;
	ActiveDodgeMontage = nullptr;
	DodgeDirection = FVector::ForwardVector;
	DodgeDirectionName = TEXT("None");
	bDodgeStarted = false;
	bDodgeFinished = false;
	bHasAppliedCleanup = false;
	bInvulnerabilityActive = false;
	bHasRegisteredCombatActionContext = false;
	bHasMarkedCombatLogicEnded = false;
	bHasReceivedInvulnerabilityBeginNotify = false;
	bHasReceivedInvulnerabilityEndNotify = false;
	bInterruptedByGuard = false;
	bShouldRestoreCharacterState = false;

	if (!ResolveDodgeDirection(DodgeDirection, DodgeDirectionName))
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("Failed to resolve a valid dodge direction."), true);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanStartDodgeExecution())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CanInterruptCurrentActionByDodge())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("CommitAbility failed."), true);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TryInterruptCurrentActionByDodge())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!StartDodgeExecution())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UTwoHeartsGA_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	CleanupDodgeExecution(bWasCancelled);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UTwoHeartsGA_NormalAttackBase* UTwoHeartsGA_Dodge::FindActiveNormalAttackAbility() const
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

UTwoHeartsCombatActionContextComponent* UTwoHeartsGA_Dodge::GetCombatActionContextComponent() const
{
	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		return Character->GetCombatActionContextComponent();
	}

	return nullptr;
}

bool UTwoHeartsGA_Dodge::CanStartDodgeExecution() const
{
	const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	const UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	const FTwoHeartsDodgeConfig* DodgeConfig = GetDodgeConfig();
	UAnimMontage* DodgeMontage = ResolveDodgeMontage();
	if (!Character || !AbilitySystemComponent || !DodgeConfig)
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("Character, ability system component, or dodge config is invalid."), true);
		return false;
	}

	if (!DodgeMontage)
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("No dodge montage was configured for the resolved direction."), true);
		return false;
	}

	if (!DodgeMontage->HasRootMotion())
	{
		RecordDodgeEvent(
			TEXT("DodgeRejected"),
			FString::Printf(TEXT("Selected dodge montage %s does not have Root Motion enabled."), *GetNameSafe(DodgeMontage)),
			true);
		return false;
	}

	return true;
}

bool UTwoHeartsGA_Dodge::CanInterruptCurrentActionByDodge() const
{
	const UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	if (!ActionContextComponent || !ActionContextComponent->HasActiveAction())
	{
		return true;
	}

	const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
	if (CurrentContext.ActionType != ETwoHeartsCombatActionType::NormalAttack)
	{
		return true;
	}

	if (!ActionContextComponent->CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType::Dodge))
	{
		RecordDodgeEvent(
			TEXT("DodgeRejected"),
			FString::Printf(
				TEXT("Public action context rejected dodge interrupt for %s during phase %s."),
				*CurrentContext.ActionInstanceName,
				*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase))),
			true);
		return false;
	}

	if (!FindActiveNormalAttackAbility())
	{
		RecordDodgeEvent(
			TEXT("DodgeRejected"),
			FString::Printf(
				TEXT("Public action context reported interruptible action %s but no active normal attack instance was found."),
				*CurrentContext.ActionInstanceName),
			true);
		return false;
	}

	return true;
}

bool UTwoHeartsGA_Dodge::TryInterruptCurrentActionByDodge()
{
	const UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	if (!ActionContextComponent || !ActionContextComponent->HasActiveAction())
	{
		return true;
	}

	const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
	if (CurrentContext.ActionType != ETwoHeartsCombatActionType::NormalAttack)
	{
		return true;
	}

	if (!ActionContextComponent->CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType::Dodge))
	{
		RecordDodgeEvent(
			TEXT("DodgeRejected"),
			FString::Printf(
				TEXT("Public action context rejected dodge interrupt for %s during phase %s."),
				*CurrentContext.ActionInstanceName,
				*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase))),
			true);
		return false;
	}

	UTwoHeartsGA_NormalAttackBase* ActiveNormalAttack = FindActiveNormalAttackAbility();
	if (!ActiveNormalAttack)
	{
		RecordDodgeEvent(
			TEXT("DodgeRejected"),
			FString::Printf(
				TEXT("Public action context reported interruptible action %s but no active normal attack instance was found."),
				*CurrentContext.ActionInstanceName),
			true);
		return false;
	}

	if (!ActiveNormalAttack->TryInterruptByAction(ETwoHeartsCombatActionType::Dodge, TEXT("InterruptedByDodge")))
	{
		RecordDodgeEvent(
			TEXT("DodgeRejected"),
			FString::Printf(
				TEXT("Public action context allowed dodge interrupt but applying it to %s failed."),
				*CurrentContext.ActionInstanceName),
			true);
		return false;
	}

	RecordDodgeEvent(
		TEXT("DodgeInterruptedAction"),
		FString::Printf(TEXT("Interrupted %s through the public action context."), *CurrentContext.ActionInstanceName));
	return true;
}

bool UTwoHeartsGA_Dodge::CanBeInterruptedByAction(ETwoHeartsCombatActionType InterruptingActionType) const
{
	return InterruptingActionType == ETwoHeartsCombatActionType::Guard;
}

bool UTwoHeartsGA_Dodge::TryInterruptByAction(ETwoHeartsCombatActionType InterruptingActionType, const FString& InterruptReason)
{
	const UEnum* ActionTypeEnum = StaticEnum<ETwoHeartsCombatActionType>();
	const FString InterruptingActionName = ActionTypeEnum
		? ActionTypeEnum->GetNameStringByValue(static_cast<int64>(InterruptingActionType))
		: TEXT("Unknown");
	const bool bCanInterrupt = CanBeInterruptedByAction(InterruptingActionType);

	RecordDodgeEvent(
		TEXT("DodgeInterruptCheck"),
		FString::Printf(
			TEXT("%s interrupt check on dodge during phase %s. Allowed=%s."),
			*InterruptingActionName,
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(
				GetCombatActionContextComponent() ? GetCombatActionContextComponent()->GetCurrentContext().ActionPhase : ETwoHeartsCombatPhase::None)),
			bCanInterrupt ? TEXT("true") : TEXT("false")),
		!bCanInterrupt);

	if (!bCanInterrupt)
	{
		return false;
	}

	bInterruptedByGuard = InterruptingActionType == ETwoHeartsCombatActionType::Guard;
	MarkCombatActionLogicEnded(InterruptReason);
	RecordDodgeEvent(
		TEXT("DodgeInterruptedByAction"),
		FString::Printf(TEXT("Dodge was interrupted by %s."), *InterruptingActionName));

	if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
		{
			if (ActiveDodgeMontage)
			{
				AnimInstance->Montage_Stop(0.05f, ActiveDodgeMontage);
				return true;
			}
		}
	}

	FinishDodge(true);
	return true;
}

bool UTwoHeartsGA_Dodge::ResolveDodgeDirection(FVector& OutDirection, FString& OutDirectionName) const
{
	const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	if (!Character)
	{
		return false;
	}

	OutDirection = Character->GetDesiredDodgeDirectionWorld();
	OutDirectionName = Character->GetDesiredDodgeDirectionName();
	return !OutDirection.IsNearlyZero();
}

bool UTwoHeartsGA_Dodge::StartDodgeExecution()
{
	AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	const FTwoHeartsDodgeConfig* DodgeConfig = GetDodgeConfig();
	UAnimMontage* DodgeMontage = ResolveDodgeMontage();
	if (!Character || !AbilitySystemComponent || !DodgeConfig)
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("Character, ability system component, or dodge config is invalid."), true);
		return false;
	}

	CachedAbilitySystemComponent = AbilitySystemComponent;
	ActiveDodgeMontage = DodgeMontage;
	bDodgeStarted = true;
	bDodgeFinished = false;
	bHasAppliedCleanup = false;
	bHasReceivedInvulnerabilityBeginNotify = false;
	bHasReceivedInvulnerabilityEndNotify = false;

	ApplyDodgeCooldown();
	SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::Startup, TEXT("DodgeActivated"));
	UpdateDodgeDebugState();

	RecordDodgeEvent(
		TEXT("DodgeActivate"),
		FString::Printf(
			TEXT("direction=%s montage=%s root_motion=true"),
			*DodgeDirectionName,
			*GetNameSafe(ActiveDodgeMontage)));
	RecordDodgeEvent(
		TEXT("DodgeDirectionResolved"),
		FString::Printf(TEXT("Resolved dodge direction as %s."), *DodgeDirectionName));

	if (Character->GetCharacterMovement())
	{
		bCachedOrientRotationToMovement = Character->GetCharacterMovement()->bOrientRotationToMovement;
		bCachedUseControllerDesiredRotation = Character->GetCharacterMovement()->bUseControllerDesiredRotation;
		Character->GetCharacterMovement()->StopMovementImmediately();
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}

	bCachedUseControllerRotationYaw = Character->bUseControllerRotationYaw;
	bShouldRestoreCharacterState = true;
	Character->bUseControllerRotationYaw = false;
	RecordDodgeEvent(
		TEXT("DodgeFacingPreserved"),
		FString::Printf(TEXT("Preserving current facing while executing %s dodge montage."), *DodgeDirectionName));

	if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
	{
		BindMontageNotifyDelegates(AnimInstance);
	}

	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ActiveDodgeMontage,
		1.0f,
		NAME_None,
		false,
		1.0f);

	if (!ActiveMontageTask)
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("Failed to create dodge montage task."), true);
		return false;
	}

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UTwoHeartsGA_Dodge::HandleMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UTwoHeartsGA_Dodge::HandleMontageInterrupted);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UTwoHeartsGA_Dodge::HandleMontageCancelled);
	ActiveMontageTask->ReadyForActivation();

	RecordDodgeEvent(
		TEXT("DodgeMontageSelected"),
		FString::Printf(
			TEXT("Selected dodge montage %s for direction %s."),
			*GetNameSafe(ActiveDodgeMontage),
			*DodgeDirectionName));

	ScheduleFallbackInvulnerabilityTimers();

	return true;
}

void UTwoHeartsGA_Dodge::BeginInvulnerabilityWindow()
{
	if (bInvulnerabilityActive)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = CachedAbilitySystemComponent.Get())
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_TwoHearts_State_Dodge_Invulnerable);
	}

	bInvulnerabilityActive = true;
	SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::Active, TEXT("DodgeInvulnerableBegin"));
	UpdateDodgeDebugState();
	RecordDodgeEvent(TEXT("DodgeInvulnerableBegin"), TEXT("Entered dodge invulnerability window."));
}

void UTwoHeartsGA_Dodge::EndInvulnerabilityWindow()
{
	if (!bInvulnerabilityActive)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = CachedAbilitySystemComponent.Get())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_TwoHearts_State_Dodge_Invulnerable);
	}

	bInvulnerabilityActive = false;
	if (!bHasMarkedCombatLogicEnded)
	{
		SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::Recovery, TEXT("DodgeInvulnerableEnd"));
	}
	UpdateDodgeDebugState();
	RecordDodgeEvent(TEXT("DodgeInvulnerableEnd"), TEXT("Exited dodge invulnerability window."));
}

void UTwoHeartsGA_Dodge::FinishDodge(bool bWasCancelled)
{
	if (bDodgeFinished)
	{
		return;
	}

	bDodgeFinished = true;
	bDodgeStarted = false;
	MarkCombatActionLogicEnded(bWasCancelled ? TEXT("DodgeCancelled") : TEXT("DodgeFinished"));
	RecordDodgeEvent(
		TEXT("DodgeFinished"),
		bInterruptedByGuard
			? TEXT("Dodge ended because Guard interrupted it.")
			: (bWasCancelled ? TEXT("Dodge was cancelled.") : TEXT("Dodge finished and can chain into follow-up actions.")));

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCancelled);
}

void UTwoHeartsGA_Dodge::ApplyDodgeCooldown()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	const FTwoHeartsDodgeConfig* DodgeConfig = GetDodgeConfig();
	if (!AbilitySystemComponent || !DodgeConfig)
	{
		return;
	}

	const float CooldownSeconds = FMath::Max(0.0f, DodgeConfig->DodgeCooldownSeconds);
	if (CooldownSeconds <= 0.0f)
	{
		bCooldownActive = false;
		UpdateDodgeDebugState();
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(TAG_TwoHearts_Cooldown_Dodge);
	bCooldownActive = true;
	UpdateDodgeDebugState();
	RecordDodgeEvent(TEXT("DodgeCooldownBegin"), FString::Printf(TEXT("Dodge cooldown started for %.2f seconds."), CooldownSeconds));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeCooldownTimerHandle);

		const TWeakObjectPtr<UAbilitySystemComponent> WeakAbilitySystemComponent(AbilitySystemComponent);
		const TWeakObjectPtr<AtwoheartsCharacter> WeakCharacter(Cast<AtwoheartsCharacter>(GetAbilityCharacter()));
		FTimerDelegate CooldownFinishedDelegate = FTimerDelegate::CreateLambda(
			[WeakAbilitySystemComponent, WeakCharacter]()
			{
				FinishDodgeCooldown(WeakAbilitySystemComponent, WeakCharacter);
			});
		World->GetTimerManager().SetTimer(DodgeCooldownTimerHandle, CooldownFinishedDelegate, CooldownSeconds, false);
	}
}

void UTwoHeartsGA_Dodge::ClearDodgeCooldown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeCooldownTimerHandle);
	}

	if (UAbilitySystemComponent* CachedASC = CachedAbilitySystemComponent.Get())
	{
		CachedASC->RemoveLooseGameplayTag(TAG_TwoHearts_Cooldown_Dodge);
	}
	else if (UAbilitySystemComponent* LiveASC = GetTwoHeartsAbilitySystemComponent())
	{
		LiveASC->RemoveLooseGameplayTag(TAG_TwoHearts_Cooldown_Dodge);
	}
	else
	{
		RecordDodgeEvent(TEXT("DodgeCooldownClearFailed"), TEXT("Cached ASC was invalid when clearing dodge cooldown."), true);
	}

	bCooldownActive = false;
	UpdateDodgeDebugState();
}

void UTwoHeartsGA_Dodge::UpdateDodgeDebugState() const
{
	if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		bool bIsCooldownReady = !bCooldownActive;
		if (const UAbilitySystemComponent* CachedASC = CachedAbilitySystemComponent.Get())
		{
			bIsCooldownReady = !CachedASC->HasMatchingGameplayTag(TAG_TwoHearts_Cooldown_Dodge);
		}
		else if (const UAbilitySystemComponent* LiveASC = GetTwoHeartsAbilitySystemComponent())
		{
			bIsCooldownReady = !LiveASC->HasMatchingGameplayTag(TAG_TwoHearts_Cooldown_Dodge);
		}

		Character->SetDodgeDebugRuntimeState(bDodgeStarted && !bDodgeFinished, bInvulnerabilityActive, bIsCooldownReady, DodgeDirectionName);
	}
}

void UTwoHeartsGA_Dodge::RecordDodgeEvent(const TCHAR* EventName, const FString& Detail, bool bWarning) const
{
	if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		Character->PushDodgeDebugEvent(EventName, Detail);
	}

	if (bWarning)
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[Dodge] ability=%s owner=%s avatar=%s event=%s detail=\"%s\""),
			*GetNameSafe(GetClass()),
			*GetNameSafe(GetAbilityOwnerActor()),
			*GetNameSafe(GetAbilityAvatarActor()),
			EventName,
			*Detail);
		return;
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[Dodge] ability=%s owner=%s avatar=%s event=%s detail=\"%s\""),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetAbilityOwnerActor()),
		*GetNameSafe(GetAbilityAvatarActor()),
		EventName,
		*Detail);

}

const FTwoHeartsDodgeConfig* UTwoHeartsGA_Dodge::GetDodgeConfig() const
{
	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		return &Character->GetDodgeConfig();
	}

	return nullptr;
}

UAnimMontage* UTwoHeartsGA_Dodge::ResolveDodgeMontage() const
{
	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		return Character->GetDodgeMontageForDirection(DodgeDirectionName);
	}

	return nullptr;
}

void UTwoHeartsGA_Dodge::SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase NewPhase, const FString& Reason)
{
	UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent();
	if (!ActionContextComponent)
	{
		return;
	}

	if (!bHasRegisteredCombatActionContext)
	{
		FTwoHeartsCombatActionRegistration Registration;
		Registration.ActionType = ETwoHeartsCombatActionType::Dodge;
		Registration.InitialPhase = NewPhase;
		Registration.AbilityTag = TAG_TwoHearts_Ability_Dodge;
		Registration.ActionStateTag = TAG_TwoHearts_State_Action_Dodge;
		Registration.ActionInstanceName = FString::Printf(TEXT("Dodge.%s"), *DodgeDirectionName);

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

void UTwoHeartsGA_Dodge::MarkCombatActionLogicEnded(const FString& Reason)
{
	if (bHasMarkedCombatLogicEnded)
	{
		return;
	}

	SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase::LogicEnded, Reason);
}

void UTwoHeartsGA_Dodge::FinishCombatActionContext(bool bWasCancelled)
{
	if (!bHasRegisteredCombatActionContext)
	{
		return;
	}

	if (UTwoHeartsCombatActionContextComponent* ActionContextComponent = GetCombatActionContextComponent())
	{
		const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
		if (CurrentContext.ActionType == ETwoHeartsCombatActionType::Dodge)
		{
			const ETwoHeartsCombatActionEndReason EndReason = bInterruptedByGuard
				? ETwoHeartsCombatActionEndReason::Interrupted
				: (bWasCancelled ? ETwoHeartsCombatActionEndReason::Cancelled : ETwoHeartsCombatActionEndReason::Completed);
			const FString FinishReason = bInterruptedByGuard
				? TEXT("InterruptedByGuard")
				: (bWasCancelled ? TEXT("DodgeCancelled") : TEXT("DodgeEnded"));
			ActionContextComponent->FinishAction(EndReason, FinishReason);
		}
	}

	if (!bWasCancelled)
	{
		if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
		{
			Character->TryConsumeReservedCombatInput(TEXT("DodgeEnded"));
		}
	}

	bHasRegisteredCombatActionContext = false;
	bHasMarkedCombatLogicEnded = false;
	bInterruptedByGuard = false;
}

void UTwoHeartsGA_Dodge::BindMontageNotifyDelegates(UAnimInstance* AnimInstance)
{
	if (!AnimInstance || BoundAnimInstance == AnimInstance)
	{
		return;
	}

	UnbindMontageNotifyDelegates();
	BoundAnimInstance = AnimInstance;
	BoundAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UTwoHeartsGA_Dodge::HandleMontageNotifyBegin);
}

void UTwoHeartsGA_Dodge::UnbindMontageNotifyDelegates()
{
	if (!BoundAnimInstance)
	{
		return;
	}

	BoundAnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UTwoHeartsGA_Dodge::HandleMontageNotifyBegin);
	BoundAnimInstance = nullptr;
}

void UTwoHeartsGA_Dodge::ScheduleFallbackInvulnerabilityTimers()
{
	const FTwoHeartsDodgeConfig* DodgeConfig = GetDodgeConfig();
	if (!DodgeConfig)
	{
		return;
	}

	ClearInvulnerabilityFallbackTimers();

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		const float InvulnerabilityStart = FMath::Max(0.0f, DodgeConfig->DodgeInvulnerableStartSeconds);
		const float InvulnerabilityDuration = FMath::Max(0.0f, DodgeConfig->DodgeInvulnerableDurationSeconds);

		if (InvulnerabilityStart <= 0.0f)
		{
			HandleInvulnerabilityWindowBegin();
		}
		else
		{
			TimerManager.SetTimer(InvulnerabilityBeginTimerHandle, this, &UTwoHeartsGA_Dodge::HandleInvulnerabilityWindowBegin, InvulnerabilityStart, false);
		}

		if (InvulnerabilityDuration > 0.0f)
		{
			TimerManager.SetTimer(InvulnerabilityEndTimerHandle, this, &UTwoHeartsGA_Dodge::HandleInvulnerabilityWindowEnd, InvulnerabilityStart + InvulnerabilityDuration, false);
		}
	}
}

void UTwoHeartsGA_Dodge::ClearInvulnerabilityFallbackTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(InvulnerabilityBeginTimerHandle);
		TimerManager.ClearTimer(InvulnerabilityEndTimerHandle);
	}
}

void UTwoHeartsGA_Dodge::HandleInvulnerabilityWindowBegin()
{
	if (bHasReceivedInvulnerabilityBeginNotify)
	{
		return;
	}

	BeginInvulnerabilityWindow();
}

void UTwoHeartsGA_Dodge::HandleInvulnerabilityWindowEnd()
{
	if (bHasReceivedInvulnerabilityEndNotify)
	{
		return;
	}

	EndInvulnerabilityWindow();
}

void UTwoHeartsGA_Dodge::HandleMontageCompleted()
{
	FinishDodge(false);
}

void UTwoHeartsGA_Dodge::HandleMontageInterrupted()
{
	FinishDodge(true);
}

void UTwoHeartsGA_Dodge::HandleMontageCancelled()
{
	FinishDodge(true);
}

void UTwoHeartsGA_Dodge::HandleDodgeFinished()
{
	MarkCombatActionLogicEnded(TEXT("DodgeFinishedNotify"));
	RecordDodgeEvent(TEXT("DodgeLogicEnded"), TEXT("Dodge reached its logical follow-up point."));
}

void UTwoHeartsGA_Dodge::HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName == InvulnerabilityBeginNotifyName)
	{
		bHasReceivedInvulnerabilityBeginNotify = true;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InvulnerabilityBeginTimerHandle);
		}

		BeginInvulnerabilityWindow();
		return;
	}

	if (NotifyName == InvulnerabilityEndNotifyName)
	{
		bHasReceivedInvulnerabilityEndNotify = true;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InvulnerabilityEndTimerHandle);
		}

		EndInvulnerabilityWindow();
		return;
	}

	if (NotifyName == DodgeFinishedNotifyName)
	{
		FinishDodge(false);
	}
}

void UTwoHeartsGA_Dodge::RestoreCharacterState()
{
	if (!bShouldRestoreCharacterState)
	{
		return;
	}

	if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		Character->bUseControllerRotationYaw = bCachedUseControllerRotationYaw;
		if (UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement())
		{
			CharacterMovement->bOrientRotationToMovement = bCachedOrientRotationToMovement;
			CharacterMovement->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
		}
	}

	bShouldRestoreCharacterState = false;
}

void UTwoHeartsGA_Dodge::CleanupDodgeExecution(bool bWasCancelled)
{
	if (bHasAppliedCleanup)
	{
		return;
	}

	bHasAppliedCleanup = true;
	ClearInvulnerabilityFallbackTimers();
	UnbindMontageNotifyDelegates();
	ActiveMontageTask = nullptr;

	if (bWasCancelled && ActiveDodgeMontage)
	{
		if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
			{
				AnimInstance->Montage_Stop(0.08f, ActiveDodgeMontage);
			}
		}
	}

	EndInvulnerabilityWindow();
	FinishCombatActionContext(bWasCancelled);
	RestoreCharacterState();
	bDodgeStarted = false;
	UpdateDodgeDebugState();
}
