#pragma once

#include "CoreMinimal.h"
#include "TwoHeartsCombatPhase.generated.h"

UENUM(BlueprintType)
enum class ETwoHeartsCombatPhase : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Startup UMETA(DisplayName="Startup"),
	Active UMETA(DisplayName="Active"),
	Recovery UMETA(DisplayName="Recovery"),
	LogicEnded UMETA(DisplayName="LogicEnded")
};
