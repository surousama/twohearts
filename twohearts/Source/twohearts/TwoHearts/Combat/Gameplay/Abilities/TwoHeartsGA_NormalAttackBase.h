#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_NormalAttackBase.generated.h"

class AtwoheartsCharacter;
class UAbilityTask_PlayMontageAndWait;
class UAnimInstance;
class UTwoHeartsCombatActionContextComponent;
enum class ETwoHeartsCombatActionType : uint8;
struct FBranchingPointNotifyPayload;

UCLASS(Abstract)
class UTwoHeartsGA_NormalAttackBase : public UTwoHeartsGameplayAbility
{
	GENERATED_BODY()

public:
	UTwoHeartsGA_NormalAttackBase();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	bool CanBeInterruptedByDodge() const;
	bool TryInterruptByAction(ETwoHeartsCombatActionType InterruptingActionType, const FString& InterruptReason);
	bool TryInterruptByDodge();
	void NotifyCombatPhaseByName(FName NotifyName);

protected:
	UPROPERTY(EditDefaultsOnly, Category="Normal Attack")
	int32 NormalAttackSegment = 1;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack")
	FGameplayTag SegmentAbilityTag;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack")
	FGameplayTag NextSegmentAbilityTag;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase")
	FName ActivePhaseNotifyName = TEXT("CombatPhase_Active");

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase")
	FName RecoveryPhaseNotifyName = TEXT("CombatPhase_Recovery");

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase")
	FName LogicEndedPhaseNotifyName = TEXT("CombatPhase_LogicEnded");

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ActivePhaseFallbackNormalizedTime = 0.20f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float RecoveryPhaseFallbackNormalizedTime = 0.60f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LogicEndedFallbackNormalizedTime = 0.85f;

private:
	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

	UFUNCTION()
	void HandleFallbackEnterActive();

	UFUNCTION()
	void HandleFallbackEnterRecovery();

	UFUNCTION()
	void HandleFallbackEnterLogicEnded();

	bool CanQueueNextSegment() const;
	bool StartSegmentPlayback();
	void FinishSegment(bool bWasCancelled);
	void UpdateDebugState(bool bIsActive) const;
	void RecordAbilityEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false) const;
	void RecordAbilityFailure(const TCHAR* EventName, const FString& Detail) const;
	AtwoheartsCharacter* GetTwoHeartsCharacter() const;
	UTwoHeartsCombatActionContextComponent* GetCombatActionContextComponent() const;
	void SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase NewPhase, const FString& Reason);
	void FinishCombatActionContext(bool bWasCancelled);
	void EnterCombatPhase(ETwoHeartsCombatPhase NewPhase, const FString& Reason);
	bool CanTransitionToPhase(ETwoHeartsCombatPhase NewPhase) const;
	void BindMontageNotifyDelegates(UAnimInstance* AnimInstance);
	void UnbindMontageNotifyDelegates();
	void ClearPhaseFallbackTimers();
	void SchedulePhaseFallbacks(float SectionLength);
	bool IsLogicEndedPhase() const;

	bool bHasQueuedNextSegment = false;
	bool bHasFinishedSegment = false;
	bool bHasRegisteredCombatActionContext = false;
	bool bPreserveDebugStateUntilNextSegment = false;
	bool bInterruptedByDodge = false;
	ETwoHeartsCombatPhase CurrentCombatPhase = ETwoHeartsCombatPhase::None;
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
	TObjectPtr<UAnimInstance> BoundAnimInstance;
	FTimerHandle ActivePhaseFallbackTimerHandle;
	FTimerHandle RecoveryPhaseFallbackTimerHandle;
	FTimerHandle LogicEndedFallbackTimerHandle;
};
