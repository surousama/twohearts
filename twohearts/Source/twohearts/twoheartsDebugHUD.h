// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "twoheartsDebugHUD.generated.h"

/**
 * Minimal HUD used to render combat test instrumentation for local debugging.
 */
UCLASS()
class ATwoheartsDebugHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;
};
