#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"

#include "twohearts.h"

namespace
{
	const TCHAR* LexHostileAttackSignalTypeToString(const ETwoHeartsHostileAttackSignalType SignalType)
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

	const TCHAR* LexPlayerHitResultTypeToString(const ETwoHeartsPlayerHitResultType ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsPlayerHitResultType::PendingIncomingHit:
			return TEXT("PendingIncomingHit");
		case ETwoHeartsPlayerHitResultType::HitConfirmed:
			return TEXT("HitConfirmed");
		case ETwoHeartsPlayerHitResultType::HitExpired:
			return TEXT("HitExpired");
		case ETwoHeartsPlayerHitResultType::SignalInvalid:
			return TEXT("SignalInvalid");
		case ETwoHeartsPlayerHitResultType::GuardRewritten:
			return TEXT("GuardRewritten");
		case ETwoHeartsPlayerHitResultType::None:
		default:
			return TEXT("None");
		}
	}

	bool IsFinalPlayerHitResultType(const ETwoHeartsPlayerHitResultType ResultType)
	{
		return ResultType == ETwoHeartsPlayerHitResultType::HitConfirmed
			|| ResultType == ETwoHeartsPlayerHitResultType::HitExpired
			|| ResultType == ETwoHeartsPlayerHitResultType::GuardRewritten;
	}

	FString BuildPendingAttackDebugString(
		const bool bHasPendingAttack,
		const FString& PendingAttackInstanceName,
		const AActor* PendingAttackSourceActor,
		const float PendingAttackStartSeconds)
	{
		return FString::Printf(
			TEXT("pending=%s pending_attack=%s pending_source=%s pending_start=%.2f"),
			bHasPendingAttack ? TEXT("true") : TEXT("false"),
			bHasPendingAttack ? *PendingAttackInstanceName : TEXT("None"),
			bHasPendingAttack ? *GetNameSafe(PendingAttackSourceActor) : TEXT("None"),
			bHasPendingAttack ? PendingAttackStartSeconds : 0.0f);
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
		LexHostileAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		*GetNameSafe(Signal.TargetActor),
		Signal.bIsHitWindowActive ? TEXT("true") : TEXT("false"),
		Signal.bHasContact ? TEXT("true") : TEXT("false"),
		Signal.TimestampSeconds,
		*Signal.Detail);

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitEval] receiver=%s stage=SignalAccepted signal=%s attack=%s source=%s target=%s %s"),
		*GetNameSafe(GetOwner()),
		LexHostileAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		*GetNameSafe(Signal.TargetActor),
		*BuildPendingAttackDebugString(bHasPendingAttack, PendingAttackInstanceName, PendingAttackSourceActor, PendingAttackStartSeconds));

	UpdatePlayerHitResultFromSignal(Signal);
	OnHostileAttackSignalReceived.Broadcast(Signal);
}

void UTwoHeartsHostileAttackReceiverComponent::ClearSignalHistory()
{
	bHasReceivedSignal = false;
	LastSignal = FTwoHeartsHostileAttackSignal();
	SignalHistory.Reset();
	bHasPlayerHitResult = false;
	LastPlayerHitResult = FTwoHeartsPlayerHitResult();
	PlayerHitResultHistory.Reset();
	bHasPendingAttack = false;
	PendingAttackInstanceName = TEXT("None");
	PendingAttackSourceActor = nullptr;
	PendingAttackStartSeconds = 0.0f;

	UE_LOG(LogtwoheartsCombatTest, Display, TEXT("[HostileAttackSignal] receiver=%s history_cleared=true"), *GetNameSafe(GetOwner()));
	UE_LOG(LogtwoheartsCombatTest, Display, TEXT("[PlayerHitEval] receiver=%s stage=StateCleared"), *GetNameSafe(GetOwner()));
}

bool UTwoHeartsHostileAttackReceiverComponent::RewriteLastPlayerHitResultForGuard(
	ETwoHeartsPlayerHitResultType NewResultType,
	const FString& Detail)
{
	if (!bHasPlayerHitResult || !LastPlayerHitResult.bCanBeRewrittenByGuard)
	{
		return false;
	}

	LastPlayerHitResult.ResultType = NewResultType;
	LastPlayerHitResult.bHitConfirmed = NewResultType == ETwoHeartsPlayerHitResultType::HitConfirmed;
	LastPlayerHitResult.bCanBeRewrittenByGuard = false;
	LastPlayerHitResult.Detail = Detail.IsEmpty() ? TEXT("Guard rewrote the pending hit result.") : Detail;
	LastPlayerHitResult.SourceSignalType = ETwoHeartsHostileAttackSignalType::AttackContact;
	LastPlayerHitResult.ResultTimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : LastPlayerHitResult.ResultTimestampSeconds;

	if (!PlayerHitResultHistory.IsEmpty())
	{
		PlayerHitResultHistory.Last() = LastPlayerHitResult;
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitResult] receiver=%s event=GuardRewrite attack=%s result=%s hit=%s rewritable=%s detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		*LastPlayerHitResult.AttackInstanceName,
		LexPlayerHitResultTypeToString(LastPlayerHitResult.ResultType),
		LastPlayerHitResult.bHitConfirmed ? TEXT("true") : TEXT("false"),
		LastPlayerHitResult.bCanBeRewrittenByGuard ? TEXT("true") : TEXT("false"),
		*LastPlayerHitResult.Detail);

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitEval] receiver=%s stage=GuardRewrite attack=%s result=%s time=%.2f"),
		*GetNameSafe(GetOwner()),
		*LastPlayerHitResult.AttackInstanceName,
		LexPlayerHitResultTypeToString(LastPlayerHitResult.ResultType),
		LastPlayerHitResult.ResultTimestampSeconds);

	OnPlayerHitResultUpdated.Broadcast(LastPlayerHitResult);
	return true;
}

void UTwoHeartsHostileAttackReceiverComponent::UpdatePlayerHitResultFromSignal(const FTwoHeartsHostileAttackSignal& Signal)
{
	const bool bCanStartTrackedAttackFromSignal =
		Signal.SignalType == ETwoHeartsHostileAttackSignalType::AttackStarted
		|| Signal.SignalType == ETwoHeartsHostileAttackSignalType::HitWindowOpened;
	const bool bStartsNewAttackInstance =
		bCanStartTrackedAttackFromSignal
		&& !Signal.AttackInstanceName.IsEmpty()
		&& Signal.AttackInstanceName != TEXT("None")
		&& (!bHasPendingAttack || PendingAttackInstanceName != Signal.AttackInstanceName);

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitEval] receiver=%s stage=EvaluateSignal signal=%s attack=%s can_start=%s starts_new=%s %s"),
		*GetNameSafe(GetOwner()),
		LexHostileAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		bCanStartTrackedAttackFromSignal ? TEXT("true") : TEXT("false"),
		bStartsNewAttackInstance ? TEXT("true") : TEXT("false"),
		*BuildPendingAttackDebugString(bHasPendingAttack, PendingAttackInstanceName, PendingAttackSourceActor, PendingAttackStartSeconds));

	if (bStartsNewAttackInstance && bHasPendingAttack && PendingAttackInstanceName != Signal.AttackInstanceName)
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[PlayerHitEval] receiver=%s stage=PendingSuperseded previous_attack=%s incoming_attack=%s"),
			*GetNameSafe(GetOwner()),
			*PendingAttackInstanceName,
			*Signal.AttackInstanceName);

		FTwoHeartsHostileAttackSignal SyntheticExpiredSignal = Signal;
		SyntheticExpiredSignal.AttackInstanceName = PendingAttackInstanceName;
		SyntheticExpiredSignal.SourceActor = PendingAttackSourceActor;
		FinalizeCurrentPendingAttack(
			SyntheticExpiredSignal,
			ETwoHeartsPlayerHitResultType::HitExpired,
			false,
			false,
			TEXT("Previous hostile attack instance was superseded before reaching a player-side final hit result."));
	}

	if (Signal.SignalType == ETwoHeartsHostileAttackSignalType::AttackStarted || bStartsNewAttackInstance)
	{
		bHasPendingAttack = true;
		PendingAttackInstanceName = Signal.AttackInstanceName;
		PendingAttackSourceActor = Signal.SourceActor;
		PendingAttackStartSeconds = Signal.TimestampSeconds;

		FTwoHeartsPlayerHitResult PendingResult;
		PendingResult.ResultType = ETwoHeartsPlayerHitResultType::PendingIncomingHit;
		PendingResult.AttackInstanceName = Signal.AttackInstanceName;
		PendingResult.SourceActor = Signal.SourceActor;
		PendingResult.TargetActor = Signal.TargetActor;
		PendingResult.bHitConfirmed = false;
		PendingResult.bCanBeRewrittenByGuard = false;
		PendingResult.ResultTimestampSeconds = Signal.TimestampSeconds;
		PendingResult.ContactTimestampSeconds = 0.0f;
		PendingResult.SourceSignalType = Signal.SignalType;
		PendingResult.Detail = Signal.Detail.IsEmpty()
			? TEXT("Incoming hostile attack became pending on the player side.")
			: Signal.Detail;

		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[PlayerHitEval] receiver=%s stage=PendingOpened attack=%s source_signal=%s target=%s time=%.2f"),
			*GetNameSafe(GetOwner()),
			*PendingResult.AttackInstanceName,
			LexHostileAttackSignalTypeToString(PendingResult.SourceSignalType),
			*GetNameSafe(PendingResult.TargetActor),
			PendingResult.ResultTimestampSeconds);

		PushPlayerHitResult(PendingResult);

		if (Signal.SignalType == ETwoHeartsHostileAttackSignalType::AttackStarted)
		{
			return;
		}
	}

	if (!bHasPendingAttack)
	{
		const bool bIsLateLifecycleSignal =
			Signal.SignalType == ETwoHeartsHostileAttackSignalType::HitWindowClosed
			|| Signal.SignalType == ETwoHeartsHostileAttackSignalType::AttackFinished;
		const bool bMatchesLastResolvedAttack =
			bHasPlayerHitResult
			&& LastPlayerHitResult.AttackInstanceName == Signal.AttackInstanceName
			&& IsFinalPlayerHitResultType(LastPlayerHitResult.ResultType);

		if (bIsLateLifecycleSignal && bMatchesLastResolvedAttack)
		{
			UE_LOG(
				LogtwoheartsCombatTest,
				Display,
				TEXT("[PlayerHitEval] receiver=%s stage=LateLifecycleSignalIgnored signal=%s attack=%s last_result=%s"),
				*GetNameSafe(GetOwner()),
				LexHostileAttackSignalTypeToString(Signal.SignalType),
				*Signal.AttackInstanceName,
				LexPlayerHitResultTypeToString(LastPlayerHitResult.ResultType));
			return;
		}

		FTwoHeartsPlayerHitResult InvalidResult;
		InvalidResult.ResultType = ETwoHeartsPlayerHitResultType::SignalInvalid;
		InvalidResult.AttackInstanceName = Signal.AttackInstanceName;
		InvalidResult.SourceActor = Signal.SourceActor;
		InvalidResult.TargetActor = Signal.TargetActor;
		InvalidResult.bHitConfirmed = false;
		InvalidResult.bCanBeRewrittenByGuard = false;
		InvalidResult.ResultTimestampSeconds = Signal.TimestampSeconds;
		InvalidResult.ContactTimestampSeconds = 0.0f;
		InvalidResult.SourceSignalType = Signal.SignalType;
		InvalidResult.Detail = FString::Printf(
			TEXT("Received %s without a tracked hostile attack instance. %s"),
			LexHostileAttackSignalTypeToString(Signal.SignalType),
			Signal.Detail.IsEmpty() ? TEXT("No detail.") : *Signal.Detail);

		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[PlayerHitEval] receiver=%s stage=InvalidSignal signal=%s attack=%s reason=\"%s\""),
			*GetNameSafe(GetOwner()),
			LexHostileAttackSignalTypeToString(Signal.SignalType),
			*Signal.AttackInstanceName,
			*InvalidResult.Detail);

		PushPlayerHitResult(InvalidResult);
		return;
	}

	switch (Signal.SignalType)
	{
	case ETwoHeartsHostileAttackSignalType::HitWindowOpened:
	{
		FTwoHeartsPlayerHitResult PendingResult;
		PendingResult.ResultType = ETwoHeartsPlayerHitResultType::PendingIncomingHit;
		PendingResult.AttackInstanceName = PendingAttackInstanceName;
		PendingResult.SourceActor = PendingAttackSourceActor;
		PendingResult.TargetActor = Signal.TargetActor;
		PendingResult.bHitConfirmed = false;
		PendingResult.bCanBeRewrittenByGuard = false;
		PendingResult.ResultTimestampSeconds = Signal.TimestampSeconds;
		PendingResult.ContactTimestampSeconds = 0.0f;
		PendingResult.SourceSignalType = Signal.SignalType;
		PendingResult.Detail = TEXT("Hostile attack hit window is active and waiting for a player-side hit result.");

		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[PlayerHitEval] receiver=%s stage=HitWindowActive attack=%s target=%s time=%.2f"),
			*GetNameSafe(GetOwner()),
			*PendingResult.AttackInstanceName,
			*GetNameSafe(PendingResult.TargetActor),
			PendingResult.ResultTimestampSeconds);

		PushPlayerHitResult(PendingResult);
		return;
	}

	case ETwoHeartsHostileAttackSignalType::AttackContact:
		FinalizeCurrentPendingAttack(
			Signal,
			ETwoHeartsPlayerHitResultType::HitConfirmed,
			true,
			true,
			TEXT("Hostile attack contact reached the player and can now be rewritten by Guard."));
		return;

	case ETwoHeartsHostileAttackSignalType::HitWindowClosed:
	case ETwoHeartsHostileAttackSignalType::AttackFinished:
		FinalizeCurrentPendingAttack(
			Signal,
			ETwoHeartsPlayerHitResultType::HitExpired,
			false,
			false,
			Signal.SignalType == ETwoHeartsHostileAttackSignalType::HitWindowClosed
				? TEXT("Hostile attack hit window closed before contact reached the player.")
				: TEXT("Hostile attack finished without confirming a player hit."));
		return;

	case ETwoHeartsHostileAttackSignalType::AttackStarted:
	case ETwoHeartsHostileAttackSignalType::None:
	default:
		return;
	}
}

void UTwoHeartsHostileAttackReceiverComponent::FinalizeCurrentPendingAttack(
	const FTwoHeartsHostileAttackSignal& Signal,
	ETwoHeartsPlayerHitResultType FinalResultType,
	bool bHitConfirmed,
	bool bCanBeRewrittenByGuard,
	const FString& Detail)
{
	FTwoHeartsPlayerHitResult Result;
	Result.ResultType = FinalResultType;
	Result.AttackInstanceName = PendingAttackInstanceName;
	Result.SourceActor = PendingAttackSourceActor ? PendingAttackSourceActor : Signal.SourceActor;
	Result.TargetActor = Signal.TargetActor;
	Result.bHitConfirmed = bHitConfirmed;
	Result.bCanBeRewrittenByGuard = bCanBeRewrittenByGuard;
	Result.ResultTimestampSeconds = Signal.TimestampSeconds;
	Result.ContactTimestampSeconds = Signal.SignalType == ETwoHeartsHostileAttackSignalType::AttackContact ? Signal.TimestampSeconds : 0.0f;
	Result.SourceSignalType = Signal.SignalType;
	Result.Detail = Detail.IsEmpty() ? Signal.Detail : Detail;

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitEval] receiver=%s stage=Finalize attack=%s final_result=%s source_signal=%s hit=%s rewritable=%s finalize_time=%.2f contact_time=%.2f pending_lifetime=%.2f"),
		*GetNameSafe(GetOwner()),
		*Result.AttackInstanceName,
		LexPlayerHitResultTypeToString(Result.ResultType),
		LexHostileAttackSignalTypeToString(Result.SourceSignalType),
		Result.bHitConfirmed ? TEXT("true") : TEXT("false"),
		Result.bCanBeRewrittenByGuard ? TEXT("true") : TEXT("false"),
		Result.ResultTimestampSeconds,
		Result.ContactTimestampSeconds,
		PendingAttackStartSeconds > 0.0f ? Signal.TimestampSeconds - PendingAttackStartSeconds : 0.0f);

	PushPlayerHitResult(Result);

	bHasPendingAttack = false;
	PendingAttackInstanceName = TEXT("None");
	PendingAttackSourceActor = nullptr;
	PendingAttackStartSeconds = 0.0f;
}

void UTwoHeartsHostileAttackReceiverComponent::PushPlayerHitResult(const FTwoHeartsPlayerHitResult& HitResult)
{
	bHasPlayerHitResult = true;
	LastPlayerHitResult = HitResult;
	PlayerHitResultHistory.Add(HitResult);

	const int32 ExcessResults = PlayerHitResultHistory.Num() - FMath::Max(1, MaxPlayerHitResultHistory);
	if (ExcessResults > 0)
	{
		PlayerHitResultHistory.RemoveAt(0, ExcessResults);
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitResult] receiver=%s attack=%s result=%s hit=%s rewritable=%s time=%.2f source_signal=%s detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		*HitResult.AttackInstanceName,
		LexPlayerHitResultTypeToString(HitResult.ResultType),
		HitResult.bHitConfirmed ? TEXT("true") : TEXT("false"),
		HitResult.bCanBeRewrittenByGuard ? TEXT("true") : TEXT("false"),
		HitResult.ResultTimestampSeconds,
		LexHostileAttackSignalTypeToString(HitResult.SourceSignalType),
		*HitResult.Detail);

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitEval] receiver=%s stage=ResultCommitted attack=%s result=%s history_count=%d"),
		*GetNameSafe(GetOwner()),
		*HitResult.AttackInstanceName,
		LexPlayerHitResultTypeToString(HitResult.ResultType),
		PlayerHitResultHistory.Num());

	OnPlayerHitResultUpdated.Broadcast(HitResult);
}
