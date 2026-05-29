#include "TwoHearts/Combat/Hostile/TwoHeartsPlayerAttackReceiverComponent.h"

#include "twohearts.h"

namespace
{
	const TCHAR* LexPlayerAttackSignalTypeToString(const ETwoHeartsPlayerAttackSignalType SignalType)
	{
		switch (SignalType)
		{
		case ETwoHeartsPlayerAttackSignalType::AttackContact:
			return TEXT("AttackContact");
		case ETwoHeartsPlayerAttackSignalType::None:
		default:
			return TEXT("None");
		}
	}
}

UTwoHeartsPlayerAttackReceiverComponent::UTwoHeartsPlayerAttackReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTwoHeartsPlayerAttackReceiverComponent::ReceivePlayerAttackSignal(const FTwoHeartsPlayerAttackSignal& Signal)
{
	bHasReceivedSignal = true;
	LastSignal = Signal;
	SignalHistory.Add(Signal);

	const int32 ExcessSignals = SignalHistory.Num() - FMath::Max(1, MaxSignalHistory);
	if (ExcessSignals > 0)
	{
		SignalHistory.RemoveAt(0, ExcessSignals);
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerAttackSignal] receiver=%s type=%s attack=%s source=%s target=%s hit_window=%s contact=%s duplicate=%s detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		LexPlayerAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		*GetNameSafe(Signal.TargetActor),
		Signal.bIsHitWindowActive ? TEXT("true") : TEXT("false"),
		Signal.bHasContact ? TEXT("true") : TEXT("false"),
		Signal.bWasDuplicateTarget ? TEXT("true") : TEXT("false"),
		*Signal.Detail);

	OnPlayerAttackSignalReceived.Broadcast(Signal);
}

void UTwoHeartsPlayerAttackReceiverComponent::ClearSignalHistory()
{
	bHasReceivedSignal = false;
	LastSignal = FTwoHeartsPlayerAttackSignal();
	SignalHistory.Reset();
}
