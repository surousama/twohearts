// Copyright Epic Games, Inc. All Rights Reserved.

#include "twoheartsDebugHUD.h"
#include "twoheartsCharacter.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

namespace
{
	const TCHAR* LexHitReactionTypeToString(const ETwoHeartsHitReactionType HitReactionType)
	{
		switch (HitReactionType)
		{
		case ETwoHeartsHitReactionType::Light:
			return TEXT("Light");
		case ETwoHeartsHitReactionType::Heavy:
			return TEXT("Heavy");
		case ETwoHeartsHitReactionType::GuardBreak:
			return TEXT("GuardBreak");
		case ETwoHeartsHitReactionType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexAttackTimingPhaseToString(const ETwoHeartsAttackTimingPhase TimingPhase)
	{
		switch (TimingPhase)
		{
		case ETwoHeartsAttackTimingPhase::Startup:
			return TEXT("Startup");
		case ETwoHeartsAttackTimingPhase::HitWindow:
			return TEXT("HitWindow");
		case ETwoHeartsAttackTimingPhase::Recovery:
			return TEXT("Recovery");
		case ETwoHeartsAttackTimingPhase::Finished:
			return TEXT("Finished");
		case ETwoHeartsAttackTimingPhase::None:
		default:
			return TEXT("None");
		}
	}
}

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

	const float PanelHeight = 640.0f + (VisibleEvents.Num() * LineHeight) + (VisibleInputEvents.Num() * LineHeight * 2.0f);

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
				TEXT("active=%s   type=%s   phase=%s   logic_end=%s   dodge_interrupt=%s   guard_interrupt=%s   end=%s"),
				ActionContext.bIsActionActive ? TEXT("YES") : TEXT("NO"),
				ActionTypeEnum ? *ActionTypeEnum->GetNameStringByValue(static_cast<int64>(ActionContext.ActionType)) : TEXT("Unknown"),
				*Character->GetCombatPhaseDebugName(ActionContext.ActionPhase),
				ActionContext.bHasLogicEnded ? TEXT("YES") : TEXT("NO"),
				ActionContextComponent->CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType::Dodge) ? TEXT("YES") : TEXT("NO"),
				ActionContextComponent->CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType::Guard) ? TEXT("YES") : TEXT("NO"),
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

		const UEnum* RouteEnum = StaticEnum<ETwoHeartsCombatInputConsumptionRoute>();
		DrawDebugLine(
			FString::Printf(
				TEXT("buffered=%s   input=%s   route=%s"),
				ActionContext.bHasBufferedInput ? TEXT("YES") : TEXT("NO"),
				ActionTypeEnum ? *ActionTypeEnum->GetNameStringByValue(static_cast<int64>(ActionContext.BufferedInputActionType)) : TEXT("Unknown"),
				RouteEnum ? *RouteEnum->GetNameStringByValue(static_cast<int64>(ActionContext.BufferedInputRoute)) : TEXT("Unknown")),
			PanelX + 12.0f,
			CurrentY,
			ActionContext.bHasBufferedInput ? HeaderColor : MutedColor);
		CurrentY += LineHeight;

		DrawDebugLine(
			FString::Printf(TEXT("buffer_reason=%s"), *ActionContext.BufferedInputReason),
			PanelX + 12.0f,
			CurrentY,
			ActionContext.bHasBufferedInput ? TextColor : MutedColor);
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

	DrawDebugLine(TEXT("Guard"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight;

	DrawDebugLine(
		FString::Printf(
			TEXT("guarding=%s   window=%s   phase=%s   hold_reserved=%s"),
			Character->IsGuardingDebugState() ? TEXT("YES") : TEXT("NO"),
			Character->IsGuardWindowActiveDebugState() ? TEXT("YES") : TEXT("NO"),
			*Character->GetCurrentGuardPhaseDebugState(),
			Character->IsGuardHoldInputReservedDebugState() ? TEXT("YES") : TEXT("NO")),
		PanelX + 12.0f,
		CurrentY,
		TextColor);
	CurrentY += LineHeight;

	DrawDebugLine(
		FString::Printf(
			TEXT("last_guard_event=[%.2f] %s | %s"),
			Character->GetLastGuardEventTimeSeconds(),
			*Character->GetLastGuardDebugEventName(),
			Character->GetLastGuardDebugDetail().IsEmpty() ? TEXT("None") : *Character->GetLastGuardDebugDetail()),
		PanelX + 12.0f,
		CurrentY,
		Character->GetLastGuardDebugDetail().IsEmpty() ? MutedColor : TextColor);
	CurrentY += LineHeight * 1.5f;

	DrawDebugLine(TEXT("Incoming Hostile Attack"), PanelX + 12.0f, CurrentY, HeaderColor);
	CurrentY += LineHeight;

	if (const UTwoHeartsHostileAttackReceiverComponent* HostileAttackReceiver = Character->GetHostileAttackReceiverComponent())
	{
		if (!HostileAttackReceiver->HasReceivedHostileAttackSignal())
		{
			DrawDebugLine(TEXT("no_hostile_attack_signal_yet"), PanelX + 12.0f, CurrentY, MutedColor);
			CurrentY += LineHeight * 1.5f;
		}
		else
		{
			const FTwoHeartsHostileAttackSignal& LastSignal = HostileAttackReceiver->GetLastSignal();
			const UEnum* SignalEnum = StaticEnum<ETwoHeartsHostileAttackSignalType>();

			DrawDebugLine(
				FString::Printf(
					TEXT("type=%s   attack=%s   hit_window=%s   contact=%s"),
					SignalEnum ? *SignalEnum->GetNameStringByValue(static_cast<int64>(LastSignal.SignalType)) : TEXT("Unknown"),
					*LastSignal.AttackInstanceName,
					LastSignal.bIsHitWindowActive ? TEXT("YES") : TEXT("NO"),
					LastSignal.bHasContact ? TEXT("YES") : TEXT("NO")),
				PanelX + 12.0f,
				CurrentY,
				TextColor);
			CurrentY += LineHeight;

			const FString SignalDamageMechanicTags = LastSignal.AttackMetadata.DamageMechanicTags.IsEmpty()
				? TEXT("None")
				: LastSignal.AttackMetadata.DamageMechanicTags.ToStringSimple();
			const FString SignalTimingWindowName = LastSignal.AttackMetadata.TimingWindowName.IsNone()
				? TEXT("None")
				: LastSignal.AttackMetadata.TimingWindowName.ToString();

			DrawDebugLine(
				FString::Printf(
					TEXT("source=%s   dir=(%.2f, %.2f, %.2f)"),
					*GetNameSafe(LastSignal.SourceActor),
					LastSignal.AttackDirection.X,
					LastSignal.AttackDirection.Y,
					LastSignal.AttackDirection.Z),
				PanelX + 12.0f,
				CurrentY,
				TextColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(
					TEXT("reaction=%s   guard=%s   dodge=%s"),
					LexHitReactionTypeToString(LastSignal.AttackMetadata.HitReactionType),
					LastSignal.AttackMetadata.bCanBeGuarded ? TEXT("YES") : TEXT("NO"),
					LastSignal.AttackMetadata.bCanBeDodged ? TEXT("YES") : TEXT("NO")),
				PanelX + 12.0f,
				CurrentY,
				TextColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(
					TEXT("timing=%s / %s"),
					LexAttackTimingPhaseToString(LastSignal.AttackMetadata.TimingPhase),
					*SignalTimingWindowName),
				PanelX + 12.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(TEXT("tags=%s"), *SignalDamageMechanicTags),
				PanelX + 12.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(TEXT("detail=%s"), *LastSignal.Detail),
				PanelX + 12.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight * 1.5f;

		}

		DrawDebugLine(TEXT("Player Hit Result"), PanelX + 12.0f, CurrentY, HeaderColor);
		CurrentY += LineHeight;

		if (!HostileAttackReceiver->HasPlayerHitResult())
		{
			DrawDebugLine(TEXT("no_player_hit_result_yet"), PanelX + 12.0f, CurrentY, MutedColor);
			CurrentY += LineHeight * 1.5f;
		}
		else
		{
			const FTwoHeartsPlayerHitResult& LastHitResult = HostileAttackReceiver->GetLastPlayerHitResult();
			const UEnum* ResultEnum = StaticEnum<ETwoHeartsPlayerHitResultType>();
			const UEnum* SignalEnum = StaticEnum<ETwoHeartsHostileAttackSignalType>();
			const FLinearColor ResultColor =
				LastHitResult.ResultType == ETwoHeartsPlayerHitResultType::GuardRewritten
					? FLinearColor(0.35f, 1.0f, 0.55f, 1.0f)
					: (LastHitResult.bHitConfirmed
						? FLinearColor(1.0f, 0.65f, 0.35f, 1.0f)
						: (LastHitResult.ResultType == ETwoHeartsPlayerHitResultType::SignalInvalid ? FailureColor : TextColor));

			DrawDebugLine(
				FString::Printf(
					TEXT("result=%s   hit=%s   guard_rewrite=%s"),
					ResultEnum ? *ResultEnum->GetNameStringByValue(static_cast<int64>(LastHitResult.ResultType)) : TEXT("Unknown"),
					LastHitResult.bHitConfirmed ? TEXT("YES") : TEXT("NO"),
					LastHitResult.bCanBeRewrittenByGuard ? TEXT("YES") : TEXT("NO")),
				PanelX + 12.0f,
				CurrentY,
				ResultColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(
					TEXT("attack=%s   source=%s   time=%.2f"),
					*LastHitResult.AttackInstanceName,
					*GetNameSafe(LastHitResult.SourceActor),
					LastHitResult.ResultTimestampSeconds),
				PanelX + 12.0f,
				CurrentY,
				TextColor);
			CurrentY += LineHeight;

			const FString HitResultDamageMechanicTags = LastHitResult.AttackMetadata.DamageMechanicTags.IsEmpty()
				? TEXT("None")
				: LastHitResult.AttackMetadata.DamageMechanicTags.ToStringSimple();
			const FString HitResultTimingWindowName = LastHitResult.AttackMetadata.TimingWindowName.IsNone()
				? TEXT("None")
				: LastHitResult.AttackMetadata.TimingWindowName.ToString();

			DrawDebugLine(
				FString::Printf(
					TEXT("source_signal=%s   contact_time=%.2f"),
					SignalEnum ? *SignalEnum->GetNameStringByValue(static_cast<int64>(LastHitResult.SourceSignalType)) : TEXT("Unknown"),
					LastHitResult.ContactTimestampSeconds),
				PanelX + 12.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(
					TEXT("reaction=%s   guard=%s   dodge=%s"),
					LexHitReactionTypeToString(LastHitResult.AttackMetadata.HitReactionType),
					LastHitResult.AttackMetadata.bCanBeGuarded ? TEXT("YES") : TEXT("NO"),
					LastHitResult.AttackMetadata.bCanBeDodged ? TEXT("YES") : TEXT("NO")),
				PanelX + 12.0f,
				CurrentY,
				TextColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(
					TEXT("timing=%s / %s"),
					LexAttackTimingPhaseToString(LastHitResult.AttackMetadata.TimingPhase),
					*HitResultTimingWindowName),
				PanelX + 12.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(TEXT("tags=%s"), *HitResultDamageMechanicTags),
				PanelX + 12.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight;

			DrawDebugLine(
				FString::Printf(TEXT("detail=%s"), *LastHitResult.Detail),
				PanelX + 12.0f,
				CurrentY,
				MutedColor);
			CurrentY += LineHeight * 1.5f;

		}
	}
	else
	{
		DrawDebugLine(TEXT("hostile_attack_receiver=None"), PanelX + 12.0f, CurrentY, FailureColor);
		CurrentY += LineHeight * 1.5f;
	}

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
