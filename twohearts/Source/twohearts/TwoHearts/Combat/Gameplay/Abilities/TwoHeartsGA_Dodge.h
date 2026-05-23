#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_Dodge.generated.h"

class UTwoHeartsGA_NormalAttackBase;
class UTwoHeartsCombatActionContextComponent;
struct FTwoHeartsDodgeConfig;
class UAbilitySystemComponent;
class UAnimMontage;
class UAnimInstance;
class UAbilityTask_PlayMontageAndWait;
enum class ETwoHeartsCombatPhase : uint8;
struct FBranchingPointNotifyPayload;

UCLASS()
class UTwoHeartsGA_Dodge : public UTwoHeartsGameplayAbility
{
	GENERATED_BODY()

public:
	UTwoHeartsGA_Dodge();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UTwoHeartsGA_NormalAttackBase* FindActiveNormalAttackAbility() const;
	UTwoHeartsCombatActionContextComponent* GetCombatActionContextComponent() const;
	bool TryInterruptCurrentActionByDodge();
	bool ResolveDodgeDirection(FVector& OutDirection, FString& OutDirectionName) const;
	bool StartDodgeExecution();
	void SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase NewPhase, const FString& Reason);
	void MarkCombatActionLogicEnded(const FString& Reason);
	void FinishCombatActionContext(bool bWasCancelled);
	void BeginInvulnerabilityWindow();
	void EndInvulnerabilityWindow();
	void FinishDodge(bool bWasCancelled = false);
	void ApplyDodgeCooldown();
	void ClearDodgeCooldown();
	void RestoreCharacterState();
	void CleanupDodgeExecution(bool bWasCancelled);
	void BindMontageNotifyDelegates(UAnimInstance* AnimInstance);
	void UnbindMontageNotifyDelegates();
	void ScheduleFallbackInvulnerabilityTimers();
	void ClearInvulnerabilityFallbackTimers();
	void UpdateDodgeDebugState() const;
	void RecordDodgeEvent(const TCHAR* EventName, const FString& Detail, bool bWarning = false) const;
	const FTwoHeartsDodgeConfig* GetDodgeConfig() const;
	UAnimMontage* ResolveDodgeMontage() const;

	UFUNCTION()
	void HandleInvulnerabilityWindowBegin();

	UFUNCTION()
	void HandleInvulnerabilityWindowEnd();

	UFUNCTION()
	void HandleDodgeFinished();

	UFUNCTION()
	void HandleDodgeCooldownFinished();

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	FVector DodgeDirection = FVector::ForwardVector;
	FString DodgeDirectionName = TEXT("None");
	bool bDodgeStarted = false;
	bool bDodgeFinished = false;
	bool bHasAppliedCleanup = false;
	bool bInvulnerabilityActive = false;
	bool bCooldownActive = false;
	bool bHasRegisteredCombatActionContext = false;
	bool bHasMarkedCombatLogicEnded = false;
	bool bHasReceivedInvulnerabilityBeginNotify = false;
	bool bHasReceivedInvulnerabilityEndNotify = false;
	bool bCachedOrientRotationToMovement = false;
	bool bCachedUseControllerDesiredRotation = false;
	bool bCachedUseControllerRotationYaw = false;
	bool bShouldRestoreCharacterState = false;
	TWeakObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
	TObjectPtr<UAnimInstance> BoundAnimInstance;
	TObjectPtr<UAnimMontage> ActiveDodgeMontage = nullptr;
	FTimerHandle InvulnerabilityBeginTimerHandle;
	FTimerHandle InvulnerabilityEndTimerHandle;
	FTimerHandle DodgeCooldownTimerHandle;
	FName InvulnerabilityBeginNotifyName = TEXT("Dodge_InvulnerableBegin");
	FName InvulnerabilityEndNotifyName = TEXT("Dodge_InvulnerableEnd");
	FName DodgeFinishedNotifyName = TEXT("Dodge_Finished");
};
