#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"
#include "twoheartsCharacter.h"

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
	DodgeDirection = FVector::ForwardVector;
	DodgeStartLocation = FVector::ZeroVector;
	DodgeTargetLocation = FVector::ZeroVector;
	DodgeDirectionName = TEXT("None");
	DodgeDurationSeconds = 0.0f;
	DodgeElapsedSeconds = 0.0f;
	bDodgeStarted = false;
	bDodgeFinished = false;
	bInvulnerabilityActive = false;

	if (UTwoHeartsGA_NormalAttackBase* ActiveNormalAttack = FindActiveNormalAttackAbility())
	{
		if (!ActiveNormalAttack->TryInterruptByDodge())
		{
			RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("The active normal attack phase is not interruptible by dodge."), true);
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		RecordDodgeEvent(TEXT("DodgeInterruptedNormalAttack"), TEXT("Interrupted the active normal attack and entered dodge."));
	}

	if (!ResolveDodgeDirection(DodgeDirection, DodgeDirectionName))
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("Failed to resolve a valid dodge direction."), true);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("CommitAbility failed."), true);
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
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(DodgeTickTimerHandle);
		TimerManager.ClearTimer(DodgeFinishTimerHandle);
		TimerManager.ClearTimer(InvulnerabilityBeginTimerHandle);
		TimerManager.ClearTimer(InvulnerabilityEndTimerHandle);
	}

	EndInvulnerabilityWindow();
	UpdateDodgeDebugState();

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
	if (!Character || !AbilitySystemComponent || !DodgeConfig)
	{
		RecordDodgeEvent(TEXT("DodgeRejected"), TEXT("Character, ability system component, or dodge config is invalid."), true);
		return false;
	}

	DodgeDurationSeconds = FMath::Max(DodgeConfig->DodgeDurationSeconds, 0.05f);
	DodgeElapsedSeconds = 0.0f;
	DodgeStartLocation = Character->GetActorLocation();
	DodgeTargetLocation = DodgeStartLocation + (DodgeDirection * FMath::Max(0.0f, DodgeConfig->DodgeDistance));
	bDodgeStarted = true;

	ApplyDodgeCooldown();
	UpdateDodgeDebugState();

	RecordDodgeEvent(
		TEXT("DodgeActivate"),
		FString::Printf(
			TEXT("direction=%s duration=%.2f distance=%.1f"),
			*DodgeDirectionName,
			DodgeDurationSeconds,
			DodgeConfig->DodgeDistance));
	RecordDodgeEvent(
		TEXT("DodgeDirectionResolved"),
		FString::Printf(TEXT("Resolved dodge direction as %s."), *DodgeDirectionName));

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}

	Character->SetActorRotation(DodgeDirection.Rotation());

	if (UAnimMontage* DodgeMontage = DodgeConfig->DodgeMontage)
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_Play(DodgeMontage, 1.0f);
		}
	}

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.SetTimer(DodgeTickTimerHandle, this, &UTwoHeartsGA_Dodge::HandleDodgeTick, 0.016f, true);
		TimerManager.SetTimer(DodgeFinishTimerHandle, this, &UTwoHeartsGA_Dodge::HandleDodgeFinished, DodgeDurationSeconds, false);

		const float InvulnerabilityStart = FMath::Max(0.0f, DodgeConfig->DodgeInvulnerableStartSeconds);
		const float InvulnerabilityDuration = FMath::Max(0.0f, DodgeConfig->DodgeInvulnerableDurationSeconds);
		if (InvulnerabilityStart <= 0.0f)
		{
			BeginInvulnerabilityWindow();
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

	return true;
}

void UTwoHeartsGA_Dodge::BeginInvulnerabilityWindow()
{
	if (bInvulnerabilityActive)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent())
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_TwoHearts_State_Dodge_Invulnerable);
	}

	bInvulnerabilityActive = true;
	UpdateDodgeDebugState();
	RecordDodgeEvent(TEXT("DodgeInvulnerableBegin"), TEXT("Entered dodge invulnerability window."));
}

void UTwoHeartsGA_Dodge::EndInvulnerabilityWindow()
{
	if (!bInvulnerabilityActive)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_TwoHearts_State_Dodge_Invulnerable);
	}

	bInvulnerabilityActive = false;
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

	AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	if (Character)
	{
		if (Character->GetCharacterMovement())
		{
			Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		}

		const FTwoHeartsDodgeConfig* DodgeConfig = GetDodgeConfig();
		if (DodgeConfig && DodgeConfig->DodgeMontage)
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
			{
				AnimInstance->Montage_Stop(0.08f, DodgeConfig->DodgeMontage);
			}
		}
	}

	EndInvulnerabilityWindow();
	UpdateDodgeDebugState();
	RecordDodgeEvent(TEXT("DodgeFinished"), bWasCancelled ? TEXT("Dodge was cancelled.") : TEXT("Dodge finished and can chain into follow-up actions."));

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

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeCooldownTimerHandle);
		World->GetTimerManager().SetTimer(DodgeCooldownTimerHandle, this, &UTwoHeartsGA_Dodge::HandleDodgeCooldownFinished, CooldownSeconds, false);
	}
}

void UTwoHeartsGA_Dodge::ClearDodgeCooldown()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(TAG_TwoHearts_Cooldown_Dodge);
	}

	bCooldownActive = false;
	UpdateDodgeDebugState();
}

void UTwoHeartsGA_Dodge::UpdateDodgeDebugState() const
{
	if (AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter()))
	{
		bool bIsCooldownReady = !bCooldownActive;
		if (const UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent())
		{
			bIsCooldownReady = !AbilitySystemComponent->HasMatchingGameplayTag(TAG_TwoHearts_Cooldown_Dodge);
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
		Display,
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

void UTwoHeartsGA_Dodge::HandleDodgeTick()
{
	AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetAbilityCharacter());
	if (!Character || DodgeDurationSeconds <= 0.0f)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	DodgeElapsedSeconds = FMath::Min(DodgeElapsedSeconds + DeltaSeconds, DodgeDurationSeconds);

	const float Alpha = FMath::Clamp(DodgeElapsedSeconds / DodgeDurationSeconds, 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(DodgeStartLocation, DodgeTargetLocation, Alpha);
	Character->SetActorLocation(NewLocation, true);
}

void UTwoHeartsGA_Dodge::HandleInvulnerabilityWindowBegin()
{
	BeginInvulnerabilityWindow();
}

void UTwoHeartsGA_Dodge::HandleInvulnerabilityWindowEnd()
{
	EndInvulnerabilityWindow();
}

void UTwoHeartsGA_Dodge::HandleDodgeFinished()
{
	FinishDodge(false);
}

void UTwoHeartsGA_Dodge::HandleDodgeCooldownFinished()
{
	ClearDodgeCooldown();
	RecordDodgeEvent(TEXT("DodgeCooldownReady"), TEXT("Dodge cooldown finished."));
}
