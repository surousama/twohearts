// Copyright Epic Games, Inc. All Rights Reserved.

#include "twoheartsDebugHUD.h"
#include "twoheartsCharacter.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
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

	TArray<const FTwoHeartsCombatInputDebugEvent*> VisibleInputEvents;
	const TArray<FTwoHeartsCombatInputDebugEvent>& InputEvents = Character->GetCombatInputDebugEvents();
	for (int32 EventIndex = InputEvents.Num() - 1; EventIndex >= 0 && VisibleInputEvents.Num() < 3; --EventIndex)
	{
		VisibleInputEvents.Add(&InputEvents[EventIndex]);
	}

	const float PanelHeight = 270.0f + (VisibleEvents.Num() * LineHeight) + (VisibleInputEvents.Num() * LineHeight * 2.0f);
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

	DrawDebugLine(TEXT("Public Action Context"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight;

	if (const UTwoHeartsCombatActionContextComponent* ActionContextComponent = Character->GetCombatActionContextComponent())
	{
		const FTwoHeartsCombatActionContextSnapshot& ActionContext = ActionContextComponent->GetCurrentContext();
		const UEnum* ActionTypeEnum = StaticEnum<ETwoHeartsCombatActionType>();
		const UEnum* EndReasonEnum = StaticEnum<ETwoHeartsCombatActionEndReason>();

		DrawDebugLine(
			FString::Printf(
				TEXT("active=%s   type=%s   phase=%s   logic_end=%s   dodge_interrupt=%s   end=%s"),
				ActionContext.bIsActionActive ? TEXT("YES") : TEXT("NO"),
				ActionTypeEnum ? *ActionTypeEnum->GetNameStringByValue(static_cast<int64>(ActionContext.ActionType)) : TEXT("Unknown"),
				*Character->GetCombatPhaseDebugName(ActionContext.ActionPhase),
				ActionContext.bHasLogicEnded ? TEXT("YES") : TEXT("NO"),
				ActionContextComponent->CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType::Dodge) ? TEXT("YES") : TEXT("NO"),
				EndReasonEnum ? *EndReasonEnum->GetNameStringByValue(static_cast<int64>(ActionContext.LastEndReason)) : TEXT("Unknown")),
			PanelX + 12.0f,
			CurrentY,
			TextColor);
		CurrentY += LineHeight;

		DrawDebugLine(
			FString::Printf(
				TEXT("instance=%s   ability=%s   state=%s"),
				*ActionContext.ActionInstanceName,
				ActionContext.AbilityTag.IsValid() ? *ActionContext.AbilityTag.ToString() : TEXT("None"),
				ActionContext.ActionStateTag.IsValid() ? *ActionContext.ActionStateTag.ToString() : TEXT("None")),
			PanelX + 12.0f,
			CurrentY,
			TextColor);
		CurrentY += LineHeight;

		DrawDebugLine(
			FString::Printf(TEXT("reason=%s"), *ActionContext.LastReason),
			PanelX + 12.0f,
			CurrentY,
			ActionContext.LastReason == TEXT("None") ? MutedColor : TextColor);
		CurrentY += LineHeight * 1.5f;
	}
	else
	{
		DrawDebugLine(TEXT("action_context_component=None"), PanelX + 12.0f, CurrentY, FailureColor);
		CurrentY += LineHeight * 1.5f;
	}

	DrawDebugLine(TEXT("Input Evaluation"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight;

	if (VisibleInputEvents.IsEmpty())
	{
		DrawDebugLine(TEXT("no_input_evaluations_yet"), PanelX + 12.0f, CurrentY, MutedColor);
		CurrentY += LineHeight * 1.5f;
	}
	else
	{
		for (const FTwoHeartsCombatInputDebugEvent* Event : VisibleInputEvents)
		{
			DrawDebugLine(
				FString::Printf(TEXT("[%.2f] %s -> %s / %s"), Event->TimestampSeconds, *Event->InputName, *Event->ResultName, *Event->RouteName),
				PanelX + 12.0f,
				CurrentY,
				TextColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(TEXT("detail=%s"), *Event->Detail),
				PanelX + 24.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight;
		}

		CurrentY += LineHeight * 0.5f;
	}

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
