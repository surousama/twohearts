#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"

#include "twohearts.h"

namespace
{
	const TCHAR* LexToString(const ETwoHeartsHostileAttackSignalType SignalType)
	{
		switch (SignalType)
		{
		case ETwoHeartsHostileAttackSignalType::AttackStarted:
			return TEXT("AttackStarted");
		case ETwoHeartsHostileAttackSignalType::HitWindowOpened:
			return TEXT("HitWindowOpened");
		case ETwoHeartsHostileAttackSignalType::HitWindowClosed:
			return TEXT("HitWindowClosed");
		case ETwoHeartsHostileAttackSignalType::AttackContact:
			return TEXT("AttackContact");
		case ETwoHeartsHostileAttackSignalType::AttackFinished:
			return TEXT("AttackFinished");
		case ETwoHeartsHostileAttackSignalType::None:
		default:
			return TEXT("None");
		}
	}
}

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

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackSignal] receiver=%s type=%s attack=%s source=%s target=%s hit_window=%s contact=%s time=%.2f detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		LexToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		*GetNameSafe(Signal.TargetActor),
		Signal.bIsHitWindowActive ? TEXT("true") : TEXT("false"),
		Signal.bHasContact ? TEXT("true") : TEXT("false"),
		Signal.TimestampSeconds,
		*Signal.Detail);

	OnHostileAttackSignalReceived.Broadcast(Signal);
}

void UTwoHeartsHostileAttackReceiverComponent::ClearSignalHistory()
{
	bHasReceivedSignal = false;
	LastSignal = FTwoHeartsHostileAttackSignal();
	SignalHistory.Reset();

	UE_LOG(LogtwoheartsCombatTest, Display, TEXT("[HostileAttackSignal] receiver=%s history_cleared=true"), *GetNameSafe(GetOwner()));
}
