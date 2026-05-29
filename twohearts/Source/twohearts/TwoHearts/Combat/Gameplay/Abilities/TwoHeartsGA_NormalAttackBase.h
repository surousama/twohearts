#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/TwoHeartsAttackMetadata.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_NormalAttackBase.generated.h"


class AtwoheartsCharacter;
class UAbilityTask_PlayMontageAndWait;
class UAnimInstance;
class UTwoHeartsCombatActionContextComponent;
class UTwoHeartsPlayerAttackReceiverComponent;
enum class ETwoHeartsCombatActionType : uint8;
struct FBranchingPointNotifyPayload;
struct FTwoHeartsPlayerAttackSignal;

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
	UFUNCTION(BlueprintPure, Category="Normal Attack|Attack Metadata")
	FTwoHeartsAttackMetadata BuildCurrentAttackMetadata() const;


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

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase")
	FName NextSegmentAdvanceNotifyName = TEXT("CombatPhase_AdvanceNextSegment");

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Attack Metadata")
	FTwoHeartsAttackMetadata AttackMetadataTemplate;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Hit Delivery", meta=(ClampMin="0.0", UIMin="0.0"))
	float HitDeliveryForwardDistance = 140.0f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Hit Delivery", meta=(ClampMin="1.0", UIMin="1.0"))
	float HitDeliveryRadius = 85.0f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Hit Delivery")
	FVector HitDeliveryLocalOffset = FVector(0.0f, 0.0f, 70.0f);

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Hit Delivery", meta=(ClampMin="0.01", UIMin="0.01"))
	float HitDeliveryScanIntervalSeconds = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))

	float ActivePhaseFallbackNormalizedTime = 0.20f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float RecoveryPhaseFallbackNormalizedTime = 0.60f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LogicEndedFallbackNormalizedTime = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float NextSegmentAdvanceFallbackNormalizedTime = 0.70f;

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

	UFUNCTION()
	void HandleFallbackOpenNextSegmentAdvanceWindow();

	UFUNCTION()
	void HandleDeferredNextSegmentActivation();

	UFUNCTION()
	void HandleHitDeliveryScan();

	bool CanQueueNextSegment() const;
	bool StartSegmentPlayback();
	void OpenNextSegmentAdvanceWindow(const FString& Reason, bool bCanConsumeLateBufferedInput);
	void FinishSegment(bool bWasCancelled);
	bool TryAdvanceToNextSegment(const FString& Reason, bool bCanConsumeLateBufferedInput);
	void AttemptDeferredNextSegmentActivation();
	bool TryConsumeLateBufferedNextSegment();
	void ClearPendingLateBufferedInputRestore();
	bool RestorePendingLateBufferedInput(const FString& Reason);
	void UpdateDebugState(bool bIsActive) const;
	void RecordAbilityEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false) const;
	void RecordAbilityFailure(const TCHAR* EventName, const FString& Detail) const;
	FString BuildMontageDebugSnapshot() const;
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
	void InitializeAttackInstance();
	void OpenHitDeliveryWindow(const FString& Reason);
	void CloseHitDeliveryWindow(const FString& Reason);
	void ScanHitDeliveryTargets(const FString& Reason);
	FTwoHeartsPlayerAttackSignal BuildPlayerAttackSignal(AActor* TargetActor, const FString& Detail, bool bWasDuplicateTarget) const;

	bool bHasQueuedNextSegment = false;
	bool bHasFinishedSegment = false;
	bool bHasRegisteredCombatActionContext = false;
	bool bPreserveDebugStateUntilNextSegment = false;
	bool bAdvanceStopInProgress = false;
	bool bInterruptedByDodge = false;
	bool bInterruptedByGuard = false;
	bool bInterruptedByHitReaction = false;
	ETwoHeartsCombatPhase CurrentCombatPhase = ETwoHeartsCombatPhase::None;
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
	TObjectPtr<UAnimInstance> BoundAnimInstance;
	FTimerHandle ActivePhaseFallbackTimerHandle;
	FTimerHandle RecoveryPhaseFallbackTimerHandle;
	FTimerHandle LogicEndedFallbackTimerHandle;
	FTimerHandle NextSegmentAdvanceFallbackTimerHandle;
	FTimerHandle DeferredNextSegmentActivationTimerHandle;
	FGameplayTag PendingNextSegmentAbilityTag;
	int32 PendingNextSegmentSourceSegment = 0;
	bool bHasPendingLateBufferedInputRestore = false;
	bool bHasOpenedNextSegmentAdvanceWindow = false;
	bool bHitDeliveryWindowActive = false;
	FString CurrentAttackInstanceName = TEXT("None");
	FTwoHeartsBufferedCombatInput PendingLateBufferedInputToRestore;
	FTimerHandle HitDeliveryScanTimerHandle;
	TSet<TWeakObjectPtr<AActor>> DeliveredTargetsThisAttack;
};
