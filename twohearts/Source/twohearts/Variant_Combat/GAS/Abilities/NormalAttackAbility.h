// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "THCombatGameplayAbility.h"
#include "NormalAttackAbility.generated.h"

/**
 * Minimal normal attack ability that only plays a configured montage.
 */
UCLASS()
class TWOHEARTS_API UNormalAttackAbility : public UTHCombatGameplayAbility
{
	GENERATED_BODY()

public:

	UNormalAttackAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

private:

	void FinishAbility(bool bWasCancelled);

	FGameplayAbilitySpecHandle ActiveHandle;
	FGameplayAbilityActivationInfo ActiveActivationInfo;
	bool bHasCommittedAbility = false;
};
