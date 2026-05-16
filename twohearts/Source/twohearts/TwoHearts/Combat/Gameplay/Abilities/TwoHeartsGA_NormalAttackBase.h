#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_NormalAttackBase.generated.h"

class AtwoheartsCharacter;
class UAbilityTask_PlayMontageAndWait;

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

protected:
	UPROPERTY(EditDefaultsOnly, Category="Normal Attack")
	int32 NormalAttackSegment = 1;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack")
	FGameplayTag SegmentAbilityTag;

	UPROPERTY(EditDefaultsOnly, Category="Normal Attack")
	FGameplayTag NextSegmentAbilityTag;

private:
	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	bool CanQueueNextSegment() const;
	bool StartSegmentPlayback();
	void FinishSegment(bool bWasCancelled);
	void UpdateDebugState(bool bIsActive) const;
	void RecordAbilityEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false) const;
	void RecordAbilityFailure(const TCHAR* EventName, const FString& Detail) const;
	AtwoheartsCharacter* GetTwoHeartsCharacter() const;

	bool bHasQueuedNextSegment = false;
	bool bHasFinishedSegment = false;
};
