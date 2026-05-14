#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TwoHeartsGameplayAbility.generated.h"

class AActor;
class ACharacter;
class UAbilitySystemComponent;

UCLASS(Abstract)
class UTwoHeartsGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UTwoHeartsGameplayAbility();

protected:
	AActor* GetAbilityOwnerActor() const;
	AActor* GetAbilityAvatarActor() const;
	ACharacter* GetAbilityCharacter() const;
	UAbilitySystemComponent* GetTwoHeartsAbilitySystemComponent() const;
	void LogAbilityMessage(const FString& Message) const;
};
