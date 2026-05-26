#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"

UTwoHeartsHostileAttackReceiverComponent::UTwoHeartsHostileAttackReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTwoHeartsHostileAttackReceiverComponent::ReceiveHostileAttackSignal(const FTwoHeartsHostileAttackSignal& Signal)
{
	bHasReceivedSignal = true;
	LastSignal = Signal;
	SignalHistory.Add(Signal);

	const int32 ExcessSignals = SignalHistory.Num() - FMath::Max(1, MaxSignalHistory);
	if (ExcessSignals > 0)
	{
		SignalHistory.RemoveAt(0, ExcessSignals);
	}

	OnHostileAttackSignalReceived.Broadcast(Signal);
}

void UTwoHeartsHostileAttackReceiverComponent::ClearSignalHistory()
{
	bHasReceivedSignal = false;
	LastSignal = FTwoHeartsHostileAttackSignal();
	SignalHistory.Reset();
}
