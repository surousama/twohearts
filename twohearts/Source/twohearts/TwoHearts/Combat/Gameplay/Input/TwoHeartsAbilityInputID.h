#pragma once

#include "CoreMinimal.h"
#include "TwoHeartsAbilityInputID.generated.h"

UENUM(BlueprintType)
enum class ETwoHeartsAbilityInputID : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Confirm UMETA(DisplayName="Confirm"),
	Cancel UMETA(DisplayName="Cancel"),
	NormalAttack UMETA(DisplayName="NormalAttack"),
	Dodge UMETA(DisplayName="Dodge"),
	Guard UMETA(DisplayName="Guard")
};
