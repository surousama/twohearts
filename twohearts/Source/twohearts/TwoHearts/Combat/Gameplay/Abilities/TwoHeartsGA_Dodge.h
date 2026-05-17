#pragma once

#include "CoreMinimal.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_Dodge.generated.h"

class UTwoHeartsGA_NormalAttackBase;
struct FTwoHeartsDodgeConfig;

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
	bool ResolveDodgeDirection(FVector& OutDirection, FString& OutDirectionName) const;
	bool StartDodgeExecution();
	void BeginInvulnerabilityWindow();
	void EndInvulnerabilityWindow();
	void FinishDodge(bool bWasCancelled = false);
	void ApplyDodgeCooldown();
	void ClearDodgeCooldown();
	void UpdateDodgeDebugState() const;
	void RecordDodgeEvent(const TCHAR* EventName, const FString& Detail, bool bWarning = false) const;
	const FTwoHeartsDodgeConfig* GetDodgeConfig() const;

	UFUNCTION()
	void HandleDodgeTick();

	UFUNCTION()
	void HandleInvulnerabilityWindowBegin();

	UFUNCTION()
	void HandleInvulnerabilityWindowEnd();

	UFUNCTION()
	void HandleDodgeFinished();

	UFUNCTION()
	void HandleDodgeCooldownFinished();

	FVector DodgeDirection = FVector::ForwardVector;
	FVector DodgeStartLocation = FVector::ZeroVector;
	FVector DodgeTargetLocation = FVector::ZeroVector;
	FString DodgeDirectionName = TEXT("None");
	float DodgeDurationSeconds = 0.0f;
	float DodgeElapsedSeconds = 0.0f;
	bool bDodgeStarted = false;
	bool bDodgeFinished = false;
	bool bInvulnerabilityActive = false;
	bool bCooldownActive = false;
	FTimerHandle DodgeTickTimerHandle;
	FTimerHandle DodgeFinishTimerHandle;
	FTimerHandle InvulnerabilityBeginTimerHandle;
	FTimerHandle InvulnerabilityEndTimerHandle;
	FTimerHandle DodgeCooldownTimerHandle;
};
