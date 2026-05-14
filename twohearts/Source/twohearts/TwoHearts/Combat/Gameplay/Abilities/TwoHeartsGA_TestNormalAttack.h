#pragma once

#include "CoreMinimal.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_TestNormalAttack.generated.h"

UCLASS()
class UTwoHeartsGA_TestNormalAttack : public UTwoHeartsGameplayAbility
{
	GENERATED_BODY()

public:
	UTwoHeartsGA_TestNormalAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
