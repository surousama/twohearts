#pragma once

#include "CoreMinimal.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_Guard.generated.h"

class UTwoHeartsGA_NormalAttackBase;
class UTwoHeartsGA_Dodge;
class UTwoHeartsCombatActionContextComponent;
class UTwoHeartsHostileAttackReceiverComponent;
struct FTwoHeartsGuardConfig;
struct FTwoHeartsPlayerHitResult;
enum class ETwoHeartsCombatActionType : uint8;

UCLASS()
class UTwoHeartsGA_Guard : public UTwoHeartsGameplayAbility
{
	GENERATED_BODY()

public:
	UTwoHeartsGA_Guard();

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

	bool TryInterruptByAction(ETwoHeartsCombatActionType InterruptingActionType, const FString& InterruptReason);

private:
	UTwoHeartsGA_NormalAttackBase* FindActiveNormalAttackAbility() const;
	UTwoHeartsGA_Dodge* FindActiveDodgeAbility() const;
	UTwoHeartsCombatActionContextComponent* GetCombatActionContextComponent() const;
	UTwoHeartsHostileAttackReceiverComponent* GetHostileAttackReceiverComponent() const;
	bool CanStartGuardExecution() const;
	bool CanInterruptCurrentActionByGuard() const;
	bool TryInterruptCurrentActionByGuard();
	bool StartGuardExecution();
	void SyncCombatActionContextOnPhaseEntered(ETwoHeartsCombatPhase NewPhase, const FString& Reason);
	void MarkCombatActionLogicEnded(const FString& Reason);
	void FinishCombatActionContext(bool bWasCancelled);
	void FinishGuard(bool bWasCancelled = false);
	void ClearGuardTimers();
	void BindHostileAttackReceiver(UTwoHeartsHostileAttackReceiverComponent* ReceiverComponent);
	void UnbindHostileAttackReceiver();
	void UpdateGuardDebugState() const;
	void RecordGuardEvent(const TCHAR* EventName, const FString& Detail, bool bWarning = false) const;
	const FTwoHeartsGuardConfig* GetGuardConfig() const;

	UFUNCTION()
	void HandleStartupFinished();

	UFUNCTION()
	void HandleGuardWindowFinished();

	UFUNCTION()
	void HandleGuardRecoveryFinished();

	UFUNCTION()
	void HandlePlayerHitResultUpdated(const FTwoHeartsPlayerHitResult& HitResult);

	bool bGuardStarted = false;
	bool bGuardFinished = false;
	bool bGuardWindowActive = false;
	bool bHasRegisteredCombatActionContext = false;
	bool bHasMarkedCombatLogicEnded = false;
	bool bInterruptedByHitReaction = false;
	ETwoHeartsCombatPhase CurrentGuardPhase = ETwoHeartsCombatPhase::None;
	TWeakObjectPtr<UTwoHeartsHostileAttackReceiverComponent> BoundHostileAttackReceiver;
	FTimerHandle GuardStartupTimerHandle;
	FTimerHandle GuardWindowTimerHandle;
	FTimerHandle GuardRecoveryTimerHandle;
};
