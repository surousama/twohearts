#pragma once

#include "CoreMinimal.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"
#include "TwoHeartsGA_Dodge.generated.h"

class UTwoHeartsGA_NormalAttackBase;

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

private:
	UTwoHeartsGA_NormalAttackBase* FindActiveNormalAttackAbility() const;
};
