#pragma once

#include "CoreMinimal.h"
#include "TwoHearts/Combat/Gameplay/Input/TwoHeartsAbilityInputID.h"
#include "TwoHeartsAbilityGrant.generated.h"

class UTwoHeartsGameplayAbility;

USTRUCT(BlueprintType)
struct FTwoHeartsAbilityGrant
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability System")
	TSubclassOf<UTwoHeartsGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability System")
	ETwoHeartsAbilityInputID InputID = ETwoHeartsAbilityInputID::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability System", meta=(ClampMin="1", UIMin="1"))
	int32 AbilityLevel = 1;
};
