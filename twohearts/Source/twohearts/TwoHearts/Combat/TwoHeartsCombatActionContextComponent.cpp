#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"

#include "twohearts.h"

UTwoHeartsCombatActionContextComponent::UTwoHeartsCombatActionContextComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTwoHeartsCombatActionContextComponent::BeginAction(const FTwoHeartsCombatActionRegistration& Registration, const FString& Reason)
{
	CurrentContext = FTwoHeartsCombatActionContextSnapshot();
	CurrentContext.bIsActionActive = true;
	CurrentContext.ActionType = Registration.ActionType;
	CurrentContext.ActionPhase = Registration.InitialPhase;
	CurrentContext.AbilityTag = Registration.AbilityTag;
	CurrentContext.ActionStateTag = Registration.ActionStateTag;
	CurrentContext.ActionInstanceName = Registration.ActionInstanceName.IsEmpty() ? TEXT("None") : Registration.ActionInstanceName;
	CurrentContext.LastReason = Reason.IsEmpty() ? TEXT("None") : Reason;
	CurrentContext.ActionStartTimeSeconds = GetWorldTimeSecondsSafe();
	CurrentContext.LastUpdateTimeSeconds = CurrentContext.ActionStartTimeSeconds;

	RecordContextEvent(
		TEXT("BeginAction"),
		FString::Printf(
			TEXT("type=%s phase=%s ability=%s state=%s instance=%s reason=%s"),
			*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType)),
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase)),
			*CurrentContext.AbilityTag.ToString(),
			*CurrentContext.ActionStateTag.ToString(),
			*CurrentContext.ActionInstanceName,
			*CurrentContext.LastReason));
}

void UTwoHeartsCombatActionContextComponent::TransitionToPhase(ETwoHeartsCombatPhase NewPhase, const FString& Reason)
{
	if (!CurrentContext.bIsActionActive || CurrentContext.ActionPhase == NewPhase)
	{
		return;
	}

	CurrentContext.ActionPhase = NewPhase;
	CurrentContext.bHasLogicEnded = NewPhase == ETwoHeartsCombatPhase::LogicEnded;
	CurrentContext.LastReason = Reason.IsEmpty() ? TEXT("None") : Reason;
	CurrentContext.LastUpdateTimeSeconds = GetWorldTimeSecondsSafe();

	RecordContextEvent(
		TEXT("TransitionPhase"),
		FString::Printf(
			TEXT("type=%s phase=%s reason=%s"),
			*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType)),
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase)),
			*CurrentContext.LastReason));
}

void UTwoHeartsCombatActionContextComponent::MarkLogicEnded(const FString& Reason)
{
	if (!CurrentContext.bIsActionActive)
	{
		return;
	}

	CurrentContext.ActionPhase = ETwoHeartsCombatPhase::LogicEnded;
	CurrentContext.bHasLogicEnded = true;
	CurrentContext.LastReason = Reason.IsEmpty() ? TEXT("None") : Reason;
	CurrentContext.LastUpdateTimeSeconds = GetWorldTimeSecondsSafe();

	RecordContextEvent(
		TEXT("LogicEnded"),
		FString::Printf(
			TEXT("type=%s reason=%s"),
			*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType)),
			*CurrentContext.LastReason));
}

void UTwoHeartsCombatActionContextComponent::FinishAction(ETwoHeartsCombatActionEndReason EndReason, const FString& Reason)
{
	if (CurrentContext.ActionType == ETwoHeartsCombatActionType::None)
	{
		return;
	}

	CurrentContext.bIsActionActive = false;
	CurrentContext.LastEndReason = EndReason;
	CurrentContext.LastReason = Reason.IsEmpty() ? TEXT("None") : Reason;
	CurrentContext.LastUpdateTimeSeconds = GetWorldTimeSecondsSafe();

	RecordContextEvent(
		TEXT("FinishAction"),
		FString::Printf(
			TEXT("type=%s end_reason=%s phase=%s logic_ended=%s reason=%s"),
			*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType)),
			*StaticEnum<ETwoHeartsCombatActionEndReason>()->GetNameStringByValue(static_cast<int64>(CurrentContext.LastEndReason)),
			*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase)),
			CurrentContext.bHasLogicEnded ? TEXT("true") : TEXT("false"),
			*CurrentContext.LastReason));
}

bool UTwoHeartsCombatActionContextComponent::CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType IncomingActionType) const
{
	if (!CurrentContext.bIsActionActive)
	{
		return false;
	}

	switch (CurrentContext.ActionType)
	{
	case ETwoHeartsCombatActionType::NormalAttack:
		return IncomingActionType == ETwoHeartsCombatActionType::Dodge
			&& (CurrentContext.ActionPhase == ETwoHeartsCombatPhase::Recovery
				|| CurrentContext.ActionPhase == ETwoHeartsCombatPhase::LogicEnded);

	default:
		return false;
	}
}

FTwoHeartsCombatInputEvaluation UTwoHeartsCombatActionContextComponent::EvaluateInputForAction(ETwoHeartsCombatActionType IncomingActionType) const
{
	FTwoHeartsCombatInputEvaluation Evaluation;
	Evaluation.IncomingActionType = IncomingActionType;
	Evaluation.bHasActiveAction = CurrentContext.bIsActionActive;
	Evaluation.ActiveActionType = CurrentContext.ActionType;
	Evaluation.ActiveActionPhase = CurrentContext.ActionPhase;

	if (IncomingActionType == ETwoHeartsCombatActionType::None)
	{
		Evaluation.Result = ETwoHeartsCombatInputEvaluationResult::Reject;
		Evaluation.Reason = TEXT("Incoming action type was invalid.");
		return Evaluation;
	}

	if (!CurrentContext.bIsActionActive)
	{
		Evaluation.Result = ETwoHeartsCombatInputEvaluationResult::ExecuteNow;
		Evaluation.ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::ActivateMatchingAbility;
		Evaluation.Reason = TEXT("No active combat action is currently registered.");
		return Evaluation;
	}

	if (CurrentContext.ActionType == ETwoHeartsCombatActionType::NormalAttack
		&& IncomingActionType == ETwoHeartsCombatActionType::NormalAttack)
	{
		Evaluation.Result = ETwoHeartsCombatInputEvaluationResult::BufferInput;

		if (CurrentContext.ActionPhase == ETwoHeartsCombatPhase::Startup
			|| CurrentContext.ActionPhase == ETwoHeartsCombatPhase::Active)
		{
			Evaluation.ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::ForwardToActiveAbility;
			Evaluation.bShouldForwardToActiveAbility = true;
			Evaluation.Reason = TEXT("Current normal attack can still consume combo queue input on the active ability.");
			return Evaluation;
		}

		Evaluation.ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::ReserveForFutureBufferConsumer;
		Evaluation.Reason = TEXT("Normal attack input was accepted by the minimal preinput hook, but this build does not consume late buffered attack input yet.");
		return Evaluation;
	}

	if (CanCurrentActionBeInterruptedBy(IncomingActionType))
	{
		Evaluation.Result = ETwoHeartsCombatInputEvaluationResult::ExecuteNow;
		Evaluation.ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::ActivateMatchingAbility;
		Evaluation.Reason = TEXT("Current combat action can be interrupted by the incoming action.");
		return Evaluation;
	}

	Evaluation.Result = ETwoHeartsCombatInputEvaluationResult::Reject;
	Evaluation.Reason = TEXT("Current combat action neither exposes a buffer window nor allows this interrupt.");
	return Evaluation;
}

FString UTwoHeartsCombatActionContextComponent::BuildCurrentContextDebugString() const
{
	return FString::Printf(
		TEXT("active=%s type=%s phase=%s logic_ended=%s end_reason=%s instance=%s ability=%s state=%s reason=%s"),
		CurrentContext.bIsActionActive ? TEXT("true") : TEXT("false"),
		*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType)),
		*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase)),
		CurrentContext.bHasLogicEnded ? TEXT("true") : TEXT("false"),
		*StaticEnum<ETwoHeartsCombatActionEndReason>()->GetNameStringByValue(static_cast<int64>(CurrentContext.LastEndReason)),
		*CurrentContext.ActionInstanceName,
		*CurrentContext.AbilityTag.ToString(),
		*CurrentContext.ActionStateTag.ToString(),
		*CurrentContext.LastReason);
}

float UTwoHeartsCombatActionContextComponent::GetWorldTimeSecondsSafe() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}

void UTwoHeartsCombatActionContextComponent::RecordContextEvent(const TCHAR* EventName, const FString& Detail) const
{
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[CombatActionContext] actor=%s event=%s detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		EventName,
		*Detail);
}
