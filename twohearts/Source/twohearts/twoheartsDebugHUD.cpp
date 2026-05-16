// Copyright Epic Games, Inc. All Rights Reserved.

#include "twoheartsDebugHUD.h"
#include "twoheartsCharacter.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

void ATwoheartsDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !PlayerOwner)
	{
		return;
	}

	APawn* Pawn = PlayerOwner->GetPawn();
	AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(Pawn);
	if (!Character || !Character->IsNormalAttackDebugPanelEnabled())
	{
		return;
	}

	UFont* DebugFont = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!DebugFont)
	{
		return;
	}

	const float PanelX = 40.0f;
	const float PanelY = 40.0f;
	const float LineHeight = 18.0f;
	const float PanelWidth = 720.0f;
	const float PanelHeight = 220.0f + (Character->GetNormalAttackDebugEvents().Num() * LineHeight);

	FCanvasTileItem Background(FVector2D(PanelX, PanelY), FVector2D(PanelWidth, PanelHeight), FLinearColor(0.02f, 0.02f, 0.02f, 0.78f));
	Background.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Background);

	float CurrentY = PanelY + 12.0f;
	const FLinearColor HeaderColor(0.90f, 0.90f, 0.25f, 1.0f);
	const FLinearColor TextColor(0.90f, 0.90f, 0.90f, 1.0f);
	const FLinearColor MutedColor(0.65f, 0.65f, 0.65f, 1.0f);
	const FLinearColor FailureColor(1.0f, 0.45f, 0.35f, 1.0f);

	auto DrawDebugLine = [this, DebugFont](const FString& Text, float X, float Y, const FLinearColor& Color)
	{
		FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), DebugFont, Color);
		TextItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TextItem);
	};

	DrawDebugLine(TEXT("Normal Attack Test Panel"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight * 1.4f;

	DrawDebugLine(
		FString::Printf(
			TEXT("Panel=%s   Log=%s   Verbose=%s"),
			Character->IsNormalAttackDebugPanelEnabled() ? TEXT("On") : TEXT("Off"),
			Character->IsNormalAttackDebugLoggingEnabled() ? TEXT("On") : TEXT("Off"),
			Character->IsNormalAttackVerboseLoggingEnabled() ? TEXT("On") : TEXT("Off")),
		PanelX + 12.0f,
		CurrentY,
		MutedColor);
	CurrentY += LineHeight;

	const TArray<FNormalAttackDebugEvent>& Events = Character->GetNormalAttackDebugEvents();
	DrawDebugLine(
		FString::Printf(
			TEXT("Current State: attacking=%s segment=%d queued_next=%s latest_section=%s"),
			Character->IsNormalAttackingDebugState() ? TEXT("true") : TEXT("false"),
			Character->GetCurrentNormalAttackSegmentDebugState(),
			Character->HasQueuedNextNormalAttackSegmentDebugState() ? TEXT("true") : TEXT("false"),
			*Character->GetCurrentNormalAttackSectionDebugState()),
		PanelX + 12.0f,
		CurrentY,
		TextColor);
	CurrentY += LineHeight;

	const FString FailureReason = Character->GetLastNormalAttackDebugFailureReason();
	DrawDebugLine(
		FString::Printf(TEXT("Last Failure: %s"), FailureReason.IsEmpty() ? TEXT("None") : *FailureReason),
		PanelX + 12.0f,
		CurrentY,
		FailureReason.IsEmpty() ? MutedColor : FailureColor);
	CurrentY += LineHeight * 1.5f;

	DrawDebugLine(TEXT("Recent Events"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight;

	for (int32 EventIndex = Events.Num() - 1; EventIndex >= 0; --EventIndex)
	{
		const FNormalAttackDebugEvent& Event = Events[EventIndex];
		DrawDebugLine(
			FString::Printf(
				TEXT("[%.3f] %s | seg=%d | attacking=%s | queued=%s | section=%s | %s"),
				Event.TimestampSeconds,
				*Event.EventName,
				Event.Segment,
				Event.bIsAttacking ? TEXT("true") : TEXT("false"),
				Event.bHasQueuedNextSegment ? TEXT("true") : TEXT("false"),
				*Event.SectionName,
				*Event.Detail),
			PanelX + 12.0f,
			CurrentY,
			TextColor);
		CurrentY += LineHeight;
	}
}
