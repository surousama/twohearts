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
	const float PanelWidth = 600.0f;

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

	TArray<const FNormalAttackDebugEvent*> VisibleEvents;
	const TArray<FNormalAttackDebugEvent>& Events = Character->GetNormalAttackDebugEvents();
	for (int32 EventIndex = Events.Num() - 1; EventIndex >= 0 && VisibleEvents.Num() < 4; --EventIndex)
	{
		const FNormalAttackDebugEvent& Event = Events[EventIndex];
		if (Character->ShouldDisplayNormalAttackDebugEvent(Event))
		{
			VisibleEvents.Add(&Event);
		}
	}

	const float PanelHeight = 210.0f + (VisibleEvents.Num() * LineHeight);
	FCanvasTileItem Background(FVector2D(PanelX, PanelY), FVector2D(PanelWidth, PanelHeight), FLinearColor(0.02f, 0.02f, 0.02f, 0.78f));
	Background.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Background);

	DrawDebugLine(TEXT("Combat Debug"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight * 1.4f;

	DrawDebugLine(
		FString::Printf(
			TEXT("Log=%s   Verbose=%s"),
			Character->IsNormalAttackDebugLoggingEnabled() ? TEXT("On") : TEXT("Off"),
			Character->IsNormalAttackVerboseLoggingEnabled() ? TEXT("On") : TEXT("Off")),
		PanelX + 12.0f,
		CurrentY,
		MutedColor);
	CurrentY += LineHeight;

	DrawDebugLine(
		FString::Printf(
			TEXT("state=%s   seg=%d   phase=%s   dodge=%s   logic_end=%s"),
			Character->IsNormalAttackingDebugState() ? TEXT("Attack") : TEXT("Idle"),
			Character->GetCurrentNormalAttackSegmentDebugState(),
			*Character->GetCombatPhaseDebugName(Character->GetCurrentNormalAttackCombatPhaseDebugState()),
			Character->IsNormalAttackInterruptibleByDodgeDebugState() ? TEXT("YES") : TEXT("NO"),
			Character->IsNormalAttackLogicEndedDebugState() ? TEXT("YES") : TEXT("NO")),
		PanelX + 12.0f,
		CurrentY,
		TextColor);
	CurrentY += LineHeight;

	DrawDebugLine(
		FString::Printf(
			TEXT("section=%s   queued_next=%s"),
			*Character->GetCurrentNormalAttackSectionDebugState(),
			Character->HasQueuedNextNormalAttackSegmentDebugState() ? TEXT("YES") : TEXT("NO")),
		PanelX + 12.0f,
		CurrentY,
		TextColor);
	CurrentY += LineHeight;

	const FString FailureReason = Character->GetLastNormalAttackDebugFailureReason();
	DrawDebugLine(
		FString::Printf(TEXT("last_failure=%s"), FailureReason.IsEmpty() ? TEXT("None") : *FailureReason),
		PanelX + 12.0f,
		CurrentY,
		FailureReason.IsEmpty() ? MutedColor : FailureColor);
	CurrentY += LineHeight * 1.5f;

	DrawDebugLine(TEXT("Dodge"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight;

	DrawDebugLine(
		FString::Printf(
			TEXT("dodging=%s   direction=%s   invulnerable=%s   cooldown_ready=%s"),
			Character->IsDodgingDebugState() ? TEXT("YES") : TEXT("NO"),
			*Character->GetCurrentDodgeDirectionDebugState(),
			Character->IsDodgeInvulnerableDebugState() ? TEXT("YES") : TEXT("NO"),
			Character->IsDodgeCooldownReadyDebugState() ? TEXT("YES") : TEXT("NO")),
		PanelX + 12.0f,
		CurrentY,
		TextColor);
	CurrentY += LineHeight;

	DrawDebugLine(
		FString::Printf(
			TEXT("last_dodge_event=[%.2f] %s | %s"),
			Character->GetLastDodgeEventTimeSeconds(),
			*Character->GetLastDodgeDebugEventName(),
			Character->GetLastDodgeDebugDetail().IsEmpty() ? TEXT("None") : *Character->GetLastDodgeDebugDetail()),
		PanelX + 12.0f,
		CurrentY,
		Character->GetLastDodgeDebugDetail().IsEmpty() ? MutedColor : TextColor);
	CurrentY += LineHeight * 1.5f;

	DrawDebugLine(TEXT("Recent Key Events"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight;

	for (const FNormalAttackDebugEvent* Event : VisibleEvents)
	{
		DrawDebugLine(
			FString::Printf(
				TEXT("[%.2f] %s | phase=%s | dodge=%s"),
				Event->TimestampSeconds,
				*Event->EventName,
				*Event->PhaseName,
				Event->bInterruptibleByDodge ? TEXT("YES") : TEXT("NO")),
			PanelX + 12.0f,
			CurrentY,
			TextColor);
		CurrentY += LineHeight;
	}
}
