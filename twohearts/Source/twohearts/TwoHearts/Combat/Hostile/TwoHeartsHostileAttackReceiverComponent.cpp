#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "TimerManager.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"
#include "twohearts.h"
#include "twoheartsCharacter.h"

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

	const TCHAR* LexPlayerDamageResultTypeToString(const ETwoHeartsPlayerDamageResultType ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsPlayerDamageResultType::DamageApplied:
			return TEXT("DamageApplied");
		case ETwoHeartsPlayerDamageResultType::GuardBlocked:
			return TEXT("GuardBlocked");
		case ETwoHeartsPlayerDamageResultType::IgnoredNoHealth:
			return TEXT("IgnoredNoHealth");
		case ETwoHeartsPlayerDamageResultType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexGuardDisplacementResultToString(const ETwoHeartsGuardDisplacementResult ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsGuardDisplacementResult::DefenderPushedBack:
			return TEXT("DefenderPushedBack");
		case ETwoHeartsGuardDisplacementResult::AttackerPushedBack:
			return TEXT("AttackerPushedBack");
		case ETwoHeartsGuardDisplacementResult::NoDisplacement:
			return TEXT("NoDisplacement");
		case ETwoHeartsGuardDisplacementResult::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexGuardDamageResultToString(const ETwoHeartsGuardDamageResult ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsGuardDamageResult::FullyBlocked:
			return TEXT("FullyBlocked");
		case ETwoHeartsGuardDamageResult::PartialDamageTaken:
			return TEXT("PartialDamageTaken");
		case ETwoHeartsGuardDamageResult::PenetrationFailed:
			return TEXT("PenetrationFailed");
		case ETwoHeartsGuardDamageResult::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexHitReactionDirectionTypeToString(const ETwoHeartsHitReactionDirectionType DirectionType)
	{
		switch (DirectionType)
		{
		case ETwoHeartsHitReactionDirectionType::Front:
			return TEXT("Front");
		case ETwoHeartsHitReactionDirectionType::Back:
			return TEXT("Back");
		case ETwoHeartsHitReactionDirectionType::Left:
			return TEXT("Left");
		case ETwoHeartsHitReactionDirectionType::Right:
			return TEXT("Right");
		case ETwoHeartsHitReactionDirectionType::None:
		default:
			return TEXT("None");
		}
	}

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

	FString BuildAttackMetadataDebugString(const FTwoHeartsAttackMetadata& AttackMetadata)
	{
		const FString DamageMechanicTags = AttackMetadata.DamageMechanicTags.IsEmpty()
			? TEXT("None")
			: AttackMetadata.DamageMechanicTags.ToStringSimple();
		const FString TimingWindowName = AttackMetadata.TimingWindowName.IsNone()
			? TEXT("None")
			: AttackMetadata.TimingWindowName.ToString();
		return FString::Printf(
			TEXT("reaction=%s damage=%.2f tags=%s guard=%s dist=%.1f height=%.1f angle=%.1f settlement=%s/%s chip=%.2f dodge=%s timing=%s/%s"),
			LexHitReactionTypeToString(AttackMetadata.HitReactionType),
			AttackMetadata.BaseDamage,
			*DamageMechanicTags,
			AttackMetadata.bCanBeGuarded ? TEXT("true") : TEXT("false"),
			AttackMetadata.GuardMaxDistance,
			AttackMetadata.GuardMaxHeightDifference,
			AttackMetadata.GuardFacingHalfAngleDegrees,
			LexGuardDisplacementResultToString(AttackMetadata.GuardSuccessDisplacementResult),
			LexGuardDamageResultToString(AttackMetadata.GuardSuccessDamageResult),
			AttackMetadata.GuardPartialDamageMultiplier,
			AttackMetadata.bCanBeDodged ? TEXT("true") : TEXT("false"),
			LexAttackTimingPhaseToString(AttackMetadata.TimingPhase),
			*TimingWindowName);
	}

	bool IsFinalPlayerHitResultType(const ETwoHeartsPlayerHitResultType ResultType)
	{

		return ResultType == ETwoHeartsPlayerHitResultType::HitConfirmed
			|| ResultType == ETwoHeartsPlayerHitResultType::HitExpired
			|| ResultType == ETwoHeartsPlayerHitResultType::GuardRewritten;
	}

	bool ShouldLogPlayerHitResultAtDisplay(const ETwoHeartsPlayerHitResultType ResultType)
	{
		return ResultType == ETwoHeartsPlayerHitResultType::HitConfirmed
			|| ResultType == ETwoHeartsPlayerHitResultType::HitExpired
			|| ResultType == ETwoHeartsPlayerHitResultType::SignalInvalid
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
	CurrentHealth = MaxHealth;
}

void UTwoHeartsHostileAttackReceiverComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
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
		Verbose,
		TEXT("[HostileAttackSignal] receiver=%s type=%s attack=%s source=%s target=%s hit_window=%s contact=%s time=%.2f detail=\"%s\" metadata=\"%s\""),

		*GetNameSafe(GetOwner()),
		LexHostileAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		*GetNameSafe(Signal.TargetActor),
		Signal.bIsHitWindowActive ? TEXT("true") : TEXT("false"),
		Signal.bHasContact ? TEXT("true") : TEXT("false"),
		Signal.TimestampSeconds,
		*Signal.Detail,
		*BuildAttackMetadataDebugString(Signal.AttackMetadata));


	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[PlayerHitEval] receiver=%s stage=SignalAccepted signal=%s attack=%s source=%s target=%s %s"),

		*GetNameSafe(GetOwner()),
		LexHostileAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		*GetNameSafe(Signal.TargetActor),
		*BuildPendingAttackDebugString(bHasPendingAttack, PendingAttackInstanceName, PendingAttackSourceActor, PendingAttackStartSeconds));

	// Guard rules must see the raw hostile contact before hit confirmation/damage/hit reaction consumes the active guard state.
	OnHostileAttackSignalReceived.Broadcast(Signal);
	UpdatePlayerHitResultFromSignal(Signal);
}

void UTwoHeartsHostileAttackReceiverComponent::ClearSignalHistory()
{
	bHasReceivedSignal = false;
	LastSignal = FTwoHeartsHostileAttackSignal();
	SignalHistory.Reset();
	bHasPlayerHitResult = false;
	LastPlayerHitResult = FTwoHeartsPlayerHitResult();
	PlayerHitResultHistory.Reset();
	bHasPlayerDamageResult = false;
	LastPlayerDamageResult = FTwoHeartsPlayerDamageResult();
	PlayerDamageResultHistory.Reset();
	bHasGuardOutcome = false;
	LastGuardOutcome = FTwoHeartsGuardOutcome();
	GuardOutcomeHistory.Reset();
	bHasPendingAttack = false;
	PendingAttackInstanceName = TEXT("None");
	PendingAttackSourceActor = nullptr;
	PendingAttackStartSeconds = 0.0f;
	PendingAttackMetadata = FTwoHeartsAttackMetadata();
	bHasPendingGuardRewriteRequest = false;
	PendingGuardRewriteAttackInstanceName = TEXT("None");
	PendingGuardSettlementRequest = FTwoHeartsGuardSettlementRequest();
	CurrentHealth = MaxHealth;
	CurrentHitReactionState = FTwoHeartsPlayerHitReactionState();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitReactionRecoveryTimerHandle);
	}

	UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackSignal] receiver=%s history_cleared=true"), *GetNameSafe(GetOwner()));
	UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[PlayerHitEval] receiver=%s stage=StateCleared"), *GetNameSafe(GetOwner()));
}

bool UTwoHeartsHostileAttackReceiverComponent::RewriteLastPlayerHitResultForGuard(
	ETwoHeartsPlayerHitResultType NewResultType,
	const FString& Detail)
{
	FTwoHeartsGuardSettlementRequest SettlementRequest;
	SettlementRequest.RewrittenHitResultType = NewResultType;
	SettlementRequest.AttackInstanceName = bHasPendingAttack ? PendingAttackInstanceName : TEXT("None");
	SettlementRequest.RewriteDetail = Detail;
	return RewriteLastPlayerHitResultForGuard(SettlementRequest);
}

bool UTwoHeartsHostileAttackReceiverComponent::RewriteLastPlayerHitResultForGuard(const FTwoHeartsGuardSettlementRequest& SettlementRequest)
{
	const bool bHasRequestedAttackInstance = !SettlementRequest.AttackInstanceName.IsEmpty()
		&& !SettlementRequest.AttackInstanceName.Equals(TEXT("None"), ESearchCase::CaseSensitive);
	if (!bHasRequestedAttackInstance)
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[PlayerHitEval] receiver=%s stage=GuardRewriteRejected reason=MissingAttackInstance result=%s"),
			*GetNameSafe(GetOwner()),
			LexPlayerHitResultTypeToString(SettlementRequest.RewrittenHitResultType));
		return false;
	}

	if (bHasPlayerHitResult && LastPlayerHitResult.bCanBeRewrittenByGuard)
	{
		if (LastPlayerHitResult.AttackInstanceName.Equals(SettlementRequest.AttackInstanceName, ESearchCase::CaseSensitive))
		{
			LastPlayerHitResult.ResultType = SettlementRequest.RewrittenHitResultType;
			LastPlayerHitResult.bHitConfirmed = SettlementRequest.RewrittenHitResultType == ETwoHeartsPlayerHitResultType::HitConfirmed;
			LastPlayerHitResult.bCanBeRewrittenByGuard = false;
			LastPlayerHitResult.Detail = SettlementRequest.RewriteDetail.IsEmpty() ? TEXT("Guard rewrote the pending hit result.") : SettlementRequest.RewriteDetail;
			LastPlayerHitResult.SourceSignalType = ETwoHeartsHostileAttackSignalType::AttackContact;
			LastPlayerHitResult.ResultTimestampSeconds = GetWorldTimeSecondsSafe();

			if (!PlayerHitResultHistory.IsEmpty())
			{
				PlayerHitResultHistory.Last() = LastPlayerHitResult;
			}

			UE_LOG(
				LogtwoheartsCombatTest,
				Display,
				TEXT("[PlayerHitResult] receiver=%s event=GuardRewrite attack=%s result=%s hit=%s rewritable=%s detail=\"%s\" metadata=\"%s\""),
				*GetNameSafe(GetOwner()),
				*LastPlayerHitResult.AttackInstanceName,
				LexPlayerHitResultTypeToString(LastPlayerHitResult.ResultType),
				LastPlayerHitResult.bHitConfirmed ? TEXT("true") : TEXT("false"),
				LastPlayerHitResult.bCanBeRewrittenByGuard ? TEXT("true") : TEXT("false"),
				*LastPlayerHitResult.Detail,
				*BuildAttackMetadataDebugString(LastPlayerHitResult.AttackMetadata));

			UE_LOG(
				LogtwoheartsCombatTest,
				Verbose,
				TEXT("[PlayerHitEval] receiver=%s stage=GuardRewrite attack=%s result=%s time=%.2f"),
				*GetNameSafe(GetOwner()),
				*LastPlayerHitResult.AttackInstanceName,
				LexPlayerHitResultTypeToString(LastPlayerHitResult.ResultType),
				LastPlayerHitResult.ResultTimestampSeconds);

			UpdatePlayerDamageResultFromHitResult(LastPlayerHitResult);
			CommitGuardOutcome(SettlementRequest, LastPlayerHitResult);
			OnPlayerHitResultUpdated.Broadcast(LastPlayerHitResult);
			return true;
		}

		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[PlayerHitEval] receiver=%s stage=GuardRewriteRejected reason=AttackInstanceMismatch requested_attack=%s last_attack=%s"),
			*GetNameSafe(GetOwner()),
			*SettlementRequest.AttackInstanceName,
			*LastPlayerHitResult.AttackInstanceName);
	}

	if (!bHasPendingAttack
		|| PendingAttackInstanceName.IsEmpty()
		|| PendingAttackInstanceName.Equals(TEXT("None"), ESearchCase::CaseSensitive)
		|| !PendingAttackInstanceName.Equals(SettlementRequest.AttackInstanceName, ESearchCase::CaseSensitive))
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[PlayerHitEval] receiver=%s stage=GuardRewriteRejected reason=PendingAttackMismatch requested_attack=%s pending_attack=%s pending=%s"),
			*GetNameSafe(GetOwner()),
			*SettlementRequest.AttackInstanceName,
			bHasPendingAttack ? *PendingAttackInstanceName : TEXT("None"),
			bHasPendingAttack ? TEXT("true") : TEXT("false"));
		return false;
	}

	bHasPendingGuardRewriteRequest = true;
	PendingGuardRewriteAttackInstanceName = SettlementRequest.AttackInstanceName;
	PendingGuardSettlementRequest = SettlementRequest;
	if (PendingGuardSettlementRequest.RewriteDetail.IsEmpty())
	{
		PendingGuardSettlementRequest.RewriteDetail = TEXT("Guard rewrote the pending hostile attack before hit confirmation committed.");
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitEval] receiver=%s stage=GuardRewriteQueued attack=%s result=%s detail=\"%s\" cooldown=%s/%.2f resource_consume=%s resource_refund=%s"),
		*GetNameSafe(GetOwner()),
		*PendingGuardRewriteAttackInstanceName,
		LexPlayerHitResultTypeToString(PendingGuardSettlementRequest.RewrittenHitResultType),
		*PendingGuardSettlementRequest.RewriteDetail,
		PendingGuardSettlementRequest.bAppliesGuardCooldown ? TEXT("true") : TEXT("false"),
		PendingGuardSettlementRequest.GuardCooldownSeconds,
		PendingGuardSettlementRequest.bConsumesGuardResource ? TEXT("true") : TEXT("false"),
		PendingGuardSettlementRequest.bRefundsGuardResource ? TEXT("true") : TEXT("false"));
	return true;
}


bool UTwoHeartsHostileAttackReceiverComponent::TryConsumePendingGuardRewriteForAttack(
	const FString& AttackInstanceName,
	FTwoHeartsGuardSettlementRequest& OutSettlementRequest)
{
	if (!bHasPendingGuardRewriteRequest || !PendingGuardRewriteAttackInstanceName.Equals(AttackInstanceName, ESearchCase::CaseSensitive))
	{
		return false;
	}

	OutSettlementRequest = PendingGuardSettlementRequest;

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerHitEval] receiver=%s stage=GuardRewriteConsume attack=%s result=%s cooldown=%s/%.2f resource_consume=%s resource_refund=%s"),
		*GetNameSafe(GetOwner()),
		*AttackInstanceName,
		LexPlayerHitResultTypeToString(OutSettlementRequest.RewrittenHitResultType),
		OutSettlementRequest.bAppliesGuardCooldown ? TEXT("true") : TEXT("false"),
		OutSettlementRequest.GuardCooldownSeconds,
		OutSettlementRequest.bConsumesGuardResource ? TEXT("true") : TEXT("false"),
		OutSettlementRequest.bRefundsGuardResource ? TEXT("true") : TEXT("false"));

	bHasPendingGuardRewriteRequest = false;
	PendingGuardRewriteAttackInstanceName = TEXT("None");
	PendingGuardSettlementRequest = FTwoHeartsGuardSettlementRequest();
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
		&& !Signal.AttackInstanceName.Equals(TEXT("None"))
		&& (!bHasPendingAttack || !PendingAttackInstanceName.Equals(Signal.AttackInstanceName, ESearchCase::CaseSensitive));

	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[PlayerHitEval] receiver=%s stage=EvaluateSignal signal=%s attack=%s can_start=%s starts_new=%s %s"),

		*GetNameSafe(GetOwner()),
		LexHostileAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		bCanStartTrackedAttackFromSignal ? TEXT("true") : TEXT("false"),
		bStartsNewAttackInstance ? TEXT("true") : TEXT("false"),
		*BuildPendingAttackDebugString(bHasPendingAttack, PendingAttackInstanceName, PendingAttackSourceActor, PendingAttackStartSeconds));

	if (bStartsNewAttackInstance && bHasPendingAttack && !PendingAttackInstanceName.Equals(Signal.AttackInstanceName, ESearchCase::CaseSensitive))
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
		SyntheticExpiredSignal.AttackMetadata = PendingAttackMetadata;

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
		PendingAttackMetadata = Signal.AttackMetadata;

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
		PendingResult.AttackMetadata = Signal.AttackMetadata;


		UE_LOG(
			LogtwoheartsCombatTest,
			Verbose,
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
			&& LastPlayerHitResult.AttackInstanceName.Equals(Signal.AttackInstanceName, ESearchCase::CaseSensitive)
			&& IsFinalPlayerHitResultType(LastPlayerHitResult.ResultType);

		if (bIsLateLifecycleSignal && bMatchesLastResolvedAttack)
		{
			UE_LOG(
				LogtwoheartsCombatTest,
				Verbose,
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
		InvalidResult.AttackMetadata = Signal.AttackMetadata;
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
		PendingAttackMetadata = Signal.AttackMetadata;

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
		PendingResult.AttackMetadata = Signal.AttackMetadata;


		UE_LOG(
			LogtwoheartsCombatTest,
			Verbose,
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
	FTwoHeartsAttackMetadata ResolvedAttackMetadata = Signal.AttackMetadata;
	if (ResolvedAttackMetadata.AttackInstanceName.IsEmpty() || ResolvedAttackMetadata.AttackInstanceName.Equals(TEXT("None")))
	{
		ResolvedAttackMetadata.AttackInstanceName = PendingAttackInstanceName;
	}
	if (!ResolvedAttackMetadata.SourceActor)
	{
		ResolvedAttackMetadata.SourceActor = PendingAttackSourceActor ? PendingAttackSourceActor : Signal.SourceActor;
	}

	FTwoHeartsPlayerHitResult Result;
	Result.ResultType = FinalResultType;
	Result.AttackMetadata = ResolvedAttackMetadata;
	Result.AttackInstanceName = Result.AttackMetadata.AttackInstanceName;
	Result.SourceActor = Result.AttackMetadata.SourceActor;
	Result.TargetActor = Signal.TargetActor;
	Result.bHitConfirmed = bHitConfirmed;
	Result.bCanBeRewrittenByGuard = bCanBeRewrittenByGuard && Result.AttackMetadata.bCanBeGuarded;
	Result.ResultTimestampSeconds = Signal.TimestampSeconds;
	Result.ContactTimestampSeconds = Signal.SignalType == ETwoHeartsHostileAttackSignalType::AttackContact ? Signal.TimestampSeconds : 0.0f;
	Result.SourceSignalType = Signal.SignalType;
	Result.Detail = Detail.IsEmpty() ? Signal.Detail : Detail;

	bool bHasGuardSettlementRequest = false;
	FTwoHeartsGuardSettlementRequest GuardSettlementRequest;
	if (TryConsumePendingGuardRewriteForAttack(Result.AttackInstanceName, GuardSettlementRequest))
	{
		bHasGuardSettlementRequest = true;
		Result.ResultType = GuardSettlementRequest.RewrittenHitResultType;
		Result.bHitConfirmed = GuardSettlementRequest.RewrittenHitResultType == ETwoHeartsPlayerHitResultType::HitConfirmed;
		Result.bCanBeRewrittenByGuard = false;
		Result.Detail = GuardSettlementRequest.RewriteDetail.IsEmpty() ? Result.Detail : GuardSettlementRequest.RewriteDetail;
	}



	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[PlayerHitEval] receiver=%s stage=Finalize attack=%s final_result=%s source_signal=%s hit=%s rewritable=%s finalize_time=%.2f contact_time=%.2f pending_lifetime=%.2f metadata=\"%s\""),

		*GetNameSafe(GetOwner()),
		*Result.AttackInstanceName,
		LexPlayerHitResultTypeToString(Result.ResultType),
		LexHostileAttackSignalTypeToString(Result.SourceSignalType),
		Result.bHitConfirmed ? TEXT("true") : TEXT("false"),
		Result.bCanBeRewrittenByGuard ? TEXT("true") : TEXT("false"),
		Result.ResultTimestampSeconds,
		Result.ContactTimestampSeconds,
		PendingAttackStartSeconds > 0.0f ? Signal.TimestampSeconds - PendingAttackStartSeconds : 0.0f,
		*BuildAttackMetadataDebugString(Result.AttackMetadata));


	PushPlayerHitResult(Result);
	if (bHasGuardSettlementRequest)
	{
		CommitGuardOutcome(GuardSettlementRequest, LastPlayerHitResult);
	}

	bHasPendingAttack = false;
	PendingAttackInstanceName = TEXT("None");
	PendingAttackSourceActor = nullptr;
	PendingAttackStartSeconds = 0.0f;
	PendingAttackMetadata = FTwoHeartsAttackMetadata();


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

	if (ShouldLogPlayerHitResultAtDisplay(HitResult.ResultType))
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[PlayerHitResult] receiver=%s attack=%s result=%s hit=%s rewritable=%s time=%.2f source_signal=%s detail=\"%s\" metadata=\"%s\""),
			*GetNameSafe(GetOwner()),
			*HitResult.AttackInstanceName,
			LexPlayerHitResultTypeToString(HitResult.ResultType),
			HitResult.bHitConfirmed ? TEXT("true") : TEXT("false"),
			HitResult.bCanBeRewrittenByGuard ? TEXT("true") : TEXT("false"),
			HitResult.ResultTimestampSeconds,
			LexHostileAttackSignalTypeToString(HitResult.SourceSignalType),
			*HitResult.Detail,
			*BuildAttackMetadataDebugString(HitResult.AttackMetadata));
	}
	else
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Verbose,
			TEXT("[PlayerHitResult] receiver=%s attack=%s result=%s hit=%s rewritable=%s time=%.2f source_signal=%s detail=\"%s\" metadata=\"%s\""),
			*GetNameSafe(GetOwner()),
			*HitResult.AttackInstanceName,
			LexPlayerHitResultTypeToString(HitResult.ResultType),
			HitResult.bHitConfirmed ? TEXT("true") : TEXT("false"),
			HitResult.bCanBeRewrittenByGuard ? TEXT("true") : TEXT("false"),
			HitResult.ResultTimestampSeconds,
			LexHostileAttackSignalTypeToString(HitResult.SourceSignalType),
			*HitResult.Detail,
			*BuildAttackMetadataDebugString(HitResult.AttackMetadata));
	}



	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[PlayerHitEval] receiver=%s stage=ResultCommitted attack=%s result=%s history_count=%d"),

		*GetNameSafe(GetOwner()),
		*HitResult.AttackInstanceName,
		LexPlayerHitResultTypeToString(HitResult.ResultType),
		PlayerHitResultHistory.Num());

	UpdatePlayerDamageResultFromHitResult(HitResult);
	OnPlayerHitResultUpdated.Broadcast(HitResult);
}

void UTwoHeartsHostileAttackReceiverComponent::UpdatePlayerDamageResultFromHitResult(const FTwoHeartsPlayerHitResult& HitResult)
{
	if (HitResult.ResultType == ETwoHeartsPlayerHitResultType::PendingIncomingHit
		|| HitResult.ResultType == ETwoHeartsPlayerHitResultType::HitExpired
		|| HitResult.ResultType == ETwoHeartsPlayerHitResultType::SignalInvalid
		|| HitResult.ResultType == ETwoHeartsPlayerHitResultType::None)
	{
		return;
	}

	FTwoHeartsPlayerDamageResult DamageResult;
	DamageResult.AttackInstanceName = HitResult.AttackInstanceName;
	DamageResult.SourceActor = HitResult.SourceActor;
	DamageResult.TargetActor = HitResult.TargetActor;
	DamageResult.SourceHitResultType = HitResult.ResultType;
	DamageResult.ResultTimestampSeconds = HitResult.ResultTimestampSeconds;
	DamageResult.AttackMetadata = HitResult.AttackMetadata;
	DamageResult.BaseDamage = FMath::Max(0.0f, HitResult.AttackMetadata.BaseDamage);

	if (HitResult.ResultType == ETwoHeartsPlayerHitResultType::GuardRewritten)
	{
		const ETwoHeartsGuardDamageResult GuardDamageResult = HitResult.AttackMetadata.GuardSuccessDamageResult;
		const float PartialDamageMultiplier = FMath::Clamp(HitResult.AttackMetadata.GuardPartialDamageMultiplier, 0.0f, 1.0f);
		const bool bHadPreviousAppliedDamage = bHasPlayerDamageResult
			&& LastPlayerDamageResult.AttackInstanceName.Equals(HitResult.AttackInstanceName, ESearchCase::CaseSensitive)
			&& LastPlayerDamageResult.ResultType == ETwoHeartsPlayerDamageResultType::DamageApplied;

		DamageResult.bWasGuardRewritten = true;
		if (bHadPreviousAppliedDamage)
		{
			CurrentHealth = FMath::Clamp(CurrentHealth + LastPlayerDamageResult.FinalDamage, 0.0f, MaxHealth);
			DamageResult.HealthBeforeDamage = LastPlayerDamageResult.HealthBeforeDamage;
		}
		else
		{
			DamageResult.HealthBeforeDamage = CurrentHealth;
		}

		switch (GuardDamageResult)
		{
		case ETwoHeartsGuardDamageResult::PartialDamageTaken:
			DamageResult.ResultType = ETwoHeartsPlayerDamageResultType::DamageApplied;
			DamageResult.FinalDamage = DamageResult.BaseDamage * PartialDamageMultiplier;
			DamageResult.Detail = bHadPreviousAppliedDamage
				? TEXT("Guard rewrote a previously applied full damage result into configured partial chip damage.")
				: TEXT("Guard succeeded, but the attack still dealt configured partial chip damage.");
			break;

		case ETwoHeartsGuardDamageResult::PenetrationFailed:
			DamageResult.ResultType = ETwoHeartsPlayerDamageResultType::GuardBlocked;
			DamageResult.FinalDamage = 0.0f;
			DamageResult.Detail = TEXT("Guard succeeded and the attack's penetration attempt failed, so no lasting damage was applied.");
			break;

		case ETwoHeartsGuardDamageResult::FullyBlocked:
		case ETwoHeartsGuardDamageResult::None:
		default:
			DamageResult.ResultType = ETwoHeartsPlayerDamageResultType::GuardBlocked;
			DamageResult.FinalDamage = 0.0f;
			DamageResult.Detail = bHadPreviousAppliedDamage
				? TEXT("Guard rewrote a previously applied damage result and restored the deducted health.")
				: TEXT("Guard rewrote the hostile hit before any lasting damage was applied.");
			break;
		}

		if (DamageResult.FinalDamage > 0.0f)
		{
			CurrentHealth = FMath::Clamp(CurrentHealth - DamageResult.FinalDamage, 0.0f, MaxHealth);
		}
		DamageResult.HealthAfterDamage = CurrentHealth;

		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[PlayerDamageRewrite] receiver=%s attack=%s path=%s guard_damage=%s base=%.2f final=%.2f health_before=%.2f health_after=%.2f"),
			*GetNameSafe(GetOwner()),
			*DamageResult.AttackInstanceName,
			bHadPreviousAppliedDamage ? TEXT("RollbackAfterDamage") : TEXT("PreDamageRewrite"),
			LexGuardDamageResultToString(GuardDamageResult),
			DamageResult.BaseDamage,
			DamageResult.FinalDamage,
			DamageResult.HealthBeforeDamage,
			DamageResult.HealthAfterDamage);

		PushPlayerDamageResult(DamageResult);
		return;
	}


	if (HitResult.ResultType != ETwoHeartsPlayerHitResultType::HitConfirmed)
	{
		return;
	}

	if (CurrentHealth <= 0.0f)
	{
		DamageResult.ResultType = ETwoHeartsPlayerDamageResultType::IgnoredNoHealth;
		DamageResult.FinalDamage = 0.0f;
		DamageResult.bWasGuardRewritten = false;
		DamageResult.HealthBeforeDamage = CurrentHealth;
		DamageResult.HealthAfterDamage = CurrentHealth;
		DamageResult.Detail = TEXT("Ignored confirmed hit because the minimum health pool had already reached zero.");
		PushPlayerDamageResult(DamageResult);
		return;
	}

	DamageResult.ResultType = ETwoHeartsPlayerDamageResultType::DamageApplied;
	DamageResult.FinalDamage = DamageResult.BaseDamage;
	DamageResult.bWasGuardRewritten = false;
	DamageResult.HealthBeforeDamage = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageResult.FinalDamage, 0.0f, MaxHealth);
	DamageResult.HealthAfterDamage = CurrentHealth;
	DamageResult.Detail = TEXT("Confirmed hostile hit produced a formal player damage result and reduced the minimum health pool.");
	PushPlayerDamageResult(DamageResult);
}

void UTwoHeartsHostileAttackReceiverComponent::PushPlayerDamageResult(const FTwoHeartsPlayerDamageResult& DamageResult)
{
	bHasPlayerDamageResult = true;
	LastPlayerDamageResult = DamageResult;

	const bool bShouldReplacePreviousResult = !PlayerDamageResultHistory.IsEmpty()
		&& PlayerDamageResultHistory.Last().AttackInstanceName.Equals(DamageResult.AttackInstanceName, ESearchCase::CaseSensitive)
		&& (DamageResult.ResultType == ETwoHeartsPlayerDamageResultType::GuardBlocked || DamageResult.bWasGuardRewritten);
	if (bShouldReplacePreviousResult)
	{
		PlayerDamageResultHistory.Last() = DamageResult;
	}
	else
	{
		PlayerDamageResultHistory.Add(DamageResult);
	}

	const int32 ExcessResults = PlayerDamageResultHistory.Num() - FMath::Max(1, MaxPlayerDamageResultHistory);
	if (ExcessResults > 0)
	{
		PlayerDamageResultHistory.RemoveAt(0, ExcessResults);
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerDamageResult] receiver=%s attack=%s result=%s hit_result=%s base=%.2f final=%.2f guard_rewritten=%s time=%.2f health_before=%.2f health_after=%.2f detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		*DamageResult.AttackInstanceName,
		LexPlayerDamageResultTypeToString(DamageResult.ResultType),
		LexPlayerHitResultTypeToString(DamageResult.SourceHitResultType),
		DamageResult.BaseDamage,
		DamageResult.FinalDamage,
		DamageResult.bWasGuardRewritten ? TEXT("true") : TEXT("false"),
		DamageResult.ResultTimestampSeconds,
		DamageResult.HealthBeforeDamage,
		DamageResult.HealthAfterDamage,
		*DamageResult.Detail);

	UpdateHitReactionStateFromDamageResult(DamageResult);
	OnPlayerDamageResultUpdated.Broadcast(DamageResult);
}







bool UTwoHeartsHostileAttackReceiverComponent::DoesLastGuardEnableFollowUpAbility() const
{
	return bHasGuardOutcome && LastGuardOutcome.bWasGuardSuccessful;
}

void UTwoHeartsHostileAttackReceiverComponent::RefreshGuardOutcomeDetail(FTwoHeartsGuardOutcome& GuardOutcome) const
{
	GuardOutcome.Detail = FString::Printf(
		TEXT("displacement=%s damage=%s resolved_hit=%s resolved_damage=%s cooldown=%s/%.2f resource=consume:%s refund:%s"),
		LexGuardDisplacementResultToString(GuardOutcome.DisplacementResult),
		LexGuardDamageResultToString(GuardOutcome.DamageResult),
		LexPlayerHitResultTypeToString(GuardOutcome.ResolvedHitResultType),
		LexPlayerDamageResultTypeToString(GuardOutcome.ResolvedPlayerDamageResultType),
		GuardOutcome.bAppliedGuardCooldown ? TEXT("true") : TEXT("false"),
		GuardOutcome.GuardCooldownSeconds,
		GuardOutcome.bConsumedGuardResource ? TEXT("true") : TEXT("false"),
		GuardOutcome.bRefundedGuardResource ? TEXT("true") : TEXT("false"));
}

void UTwoHeartsHostileAttackReceiverComponent::CommitGuardOutcome(
	const FTwoHeartsGuardSettlementRequest& SettlementRequest,
	const FTwoHeartsPlayerHitResult& HitResult)
{

	FTwoHeartsGuardOutcome GuardOutcome;
	GuardOutcome.bWasGuardSuccessful = true;
	GuardOutcome.AttackInstanceName = HitResult.AttackInstanceName;
	GuardOutcome.SourceActor = HitResult.SourceActor;
	GuardOutcome.TargetActor = HitResult.TargetActor;
	GuardOutcome.DisplacementResult = HitResult.AttackMetadata.GuardSuccessDisplacementResult;
	GuardOutcome.DamageResult = HitResult.AttackMetadata.GuardSuccessDamageResult;
	GuardOutcome.ResolvedHitResultType = HitResult.ResultType;
	GuardOutcome.BaseDamage = FMath::Max(0.0f, HitResult.AttackMetadata.BaseDamage);

	GuardOutcome.AttackMetadata = HitResult.AttackMetadata;
	GuardOutcome.bConsumedGuardResource = SettlementRequest.bConsumesGuardResource;
	GuardOutcome.bRefundedGuardResource = SettlementRequest.bRefundsGuardResource;
	GuardOutcome.bAppliedGuardCooldown = SettlementRequest.bAppliesGuardCooldown && SettlementRequest.GuardCooldownSeconds > 0.0f;
	GuardOutcome.GuardCooldownSeconds = SettlementRequest.GuardCooldownSeconds;
	GuardOutcome.GuardCooldownTag = SettlementRequest.GuardCooldownTag;

	if (bHasPlayerDamageResult && LastPlayerDamageResult.AttackInstanceName.Equals(HitResult.AttackInstanceName, ESearchCase::CaseSensitive))
	{
		GuardOutcome.ResolvedPlayerDamageResultType = LastPlayerDamageResult.ResultType;
		GuardOutcome.BaseDamage = LastPlayerDamageResult.BaseDamage;
		GuardOutcome.FinalDamage = LastPlayerDamageResult.FinalDamage;
		GuardOutcome.ResultTimestampSeconds = LastPlayerDamageResult.ResultTimestampSeconds;
	}
	else
	{
		GuardOutcome.ResultTimestampSeconds = HitResult.ResultTimestampSeconds;
	}

	RefreshGuardOutcomeDetail(GuardOutcome);

	PushGuardOutcome(GuardOutcome);
}


void UTwoHeartsHostileAttackReceiverComponent::PushGuardOutcome(const FTwoHeartsGuardOutcome& GuardOutcome)
{
	bHasGuardOutcome = true;
	LastGuardOutcome = GuardOutcome;

	const bool bShouldReplacePreviousOutcome = !GuardOutcomeHistory.IsEmpty()
		&& GuardOutcomeHistory.Last().AttackInstanceName.Equals(GuardOutcome.AttackInstanceName, ESearchCase::CaseSensitive);
	if (bShouldReplacePreviousOutcome)
	{
		GuardOutcomeHistory.Last() = GuardOutcome;
	}
	else
	{
		GuardOutcomeHistory.Add(GuardOutcome);
	}

	const int32 ExcessResults = GuardOutcomeHistory.Num() - FMath::Max(1, MaxGuardOutcomeHistory);
	if (ExcessResults > 0)
	{
		GuardOutcomeHistory.RemoveAt(0, ExcessResults);
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[GuardOutcome] receiver=%s attack=%s displacement=%s damage=%s resolved_hit=%s resolved_damage=%s final=%.2f cooldown=%s/%.2f resource=consume:%s refund:%s detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		*GuardOutcome.AttackInstanceName,
		LexGuardDisplacementResultToString(GuardOutcome.DisplacementResult),
		LexGuardDamageResultToString(GuardOutcome.DamageResult),
		LexPlayerHitResultTypeToString(GuardOutcome.ResolvedHitResultType),
		LexPlayerDamageResultTypeToString(GuardOutcome.ResolvedPlayerDamageResultType),
		GuardOutcome.FinalDamage,
		GuardOutcome.bAppliedGuardCooldown ? TEXT("true") : TEXT("false"),
		GuardOutcome.GuardCooldownSeconds,
		GuardOutcome.bConsumedGuardResource ? TEXT("true") : TEXT("false"),
		GuardOutcome.bRefundedGuardResource ? TEXT("true") : TEXT("false"),
		*GuardOutcome.Detail);


	OnGuardOutcomeUpdated.Broadcast(GuardOutcome);
}







void UTwoHeartsHostileAttackReceiverComponent::UpdateHitReactionStateFromDamageResult(const FTwoHeartsPlayerDamageResult& DamageResult)
{
	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[HitReaction] receiver=%s stage=EvaluateDamageResult attack=%s damage_result=%s final_damage=%.2f active_before=%s"),
		*GetNameSafe(GetOwner()),
		*DamageResult.AttackInstanceName,
		LexPlayerDamageResultTypeToString(DamageResult.ResultType),
		DamageResult.FinalDamage,
		CurrentHitReactionState.bIsActive ? TEXT("true") : TEXT("false"));

	if (DamageResult.ResultType == ETwoHeartsPlayerDamageResultType::GuardBlocked)
	{
		if (CurrentHitReactionState.bIsActive
			&& CurrentHitReactionState.SourceAttackInstanceName.Equals(DamageResult.AttackInstanceName, ESearchCase::CaseSensitive))
		{
			UE_LOG(
				LogtwoheartsCombatTest,
				Display,
				TEXT("[HitReaction] receiver=%s stage=GuardBlockedResolve attack=%s detail=\"Active hit reaction will be cleared because Guard blocked the same attack.\""),
				*GetNameSafe(GetOwner()),
				*DamageResult.AttackInstanceName);
			FinishHitReaction(TEXT("GuardBlocked"));
		}
		return;
	}

	if (DamageResult.ResultType != ETwoHeartsPlayerDamageResultType::DamageApplied
		|| DamageResult.FinalDamage <= 0.0f)
	{
		return;
	}

	EnterHitReaction(DamageResult);
}

void UTwoHeartsHostileAttackReceiverComponent::EnterHitReaction(const FTwoHeartsPlayerDamageResult& DamageResult)
{
	if (CurrentHitReactionState.bIsActive)
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[HitReaction] receiver=%s stage=Supersede attack=%s previous_attack=%s"),
			*GetNameSafe(GetOwner()),
			*DamageResult.AttackInstanceName,
			*CurrentHitReactionState.SourceAttackInstanceName);
		FinishHitReaction(TEXT("SupersededByNewHit"));
	}

	if (!InterruptCurrentActionForHitReaction())
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[HitReaction] receiver=%s attack=%s detail=\"Failed to interrupt current action before entering hit reaction.\""),
			*GetNameSafe(GetOwner()),
			*DamageResult.AttackInstanceName);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitReactionRecoveryTimerHandle);
	}

	UTwoHeartsCombatActionContextComponent* ActionContextComponent = nullptr;
	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetOwner()))
	{
		ActionContextComponent = Character->GetCombatActionContextComponent();
	}

	const float StartTimeSeconds = GetWorldTimeSecondsSafe();

	const float DurationSeconds = ResolveHitReactionDuration(DamageResult);
	const FVector IncomingDirection = DamageResult.AttackMetadata.AttackDirection.GetSafeNormal2D();
	const ETwoHeartsHitReactionDirectionType DirectionType = ResolveHitReactionDirectionType(IncomingDirection);

	CurrentHitReactionState.bIsActive = true;
	CurrentHitReactionState.HitReactionInstanceName = FString::Printf(TEXT("HitReaction.%s"), *DamageResult.AttackInstanceName);
	CurrentHitReactionState.SourceActor = DamageResult.SourceActor;
	CurrentHitReactionState.SourceAttackInstanceName = DamageResult.AttackInstanceName;
	CurrentHitReactionState.HitReactionType = DamageResult.AttackMetadata.HitReactionType;
	CurrentHitReactionState.DirectionType = DirectionType;
	CurrentHitReactionState.IncomingDirection = IncomingDirection;
	CurrentHitReactionState.StartTimeSeconds = StartTimeSeconds;
	CurrentHitReactionState.ExpectedRecoveryTimeSeconds = StartTimeSeconds + DurationSeconds;
	CurrentHitReactionState.LastEndReason = TEXT("None");
	CurrentHitReactionState.Detail = TEXT("Formal hit reaction entered from a confirmed player damage result.");

	if (ActionContextComponent)
	{
		ActionContextComponent->ClearBufferedInput(TEXT("ClearedByHitReaction"));

		FTwoHeartsCombatActionRegistration Registration;
		Registration.ActionType = ETwoHeartsCombatActionType::HitReaction;
		Registration.InitialPhase = ETwoHeartsCombatPhase::Active;
		Registration.ActionInstanceName = CurrentHitReactionState.HitReactionInstanceName;
		ActionContextComponent->BeginAction(Registration, TEXT("DamageApplied"));
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HitReaction] receiver=%s event=Enter attack=%s reaction=%s direction=%s duration=%.2f source=%s"),
		*GetNameSafe(GetOwner()),
		*CurrentHitReactionState.SourceAttackInstanceName,
		LexHitReactionTypeToString(CurrentHitReactionState.HitReactionType),
		LexHitReactionDirectionTypeToString(CurrentHitReactionState.DirectionType),
		DurationSeconds,
		*GetNameSafe(CurrentHitReactionState.SourceActor));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HitReactionRecoveryTimerHandle,
			this,
			&UTwoHeartsHostileAttackReceiverComponent::HandleHitReactionRecovery,
			DurationSeconds,
			false);
	}

	OnPlayerHitReactionStateUpdated.Broadcast(CurrentHitReactionState);
}

void UTwoHeartsHostileAttackReceiverComponent::FinishHitReaction(const FString& EndReason)
{
	if (!CurrentHitReactionState.bIsActive)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitReactionRecoveryTimerHandle);
	}

	if (const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetOwner()))
	{
		if (UTwoHeartsCombatActionContextComponent* ActionContextComponent = Character->GetCombatActionContextComponent())
		{
			const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
			if (CurrentContext.ActionType == ETwoHeartsCombatActionType::HitReaction)
			{
				ActionContextComponent->FinishAction(ETwoHeartsCombatActionEndReason::Completed, EndReason);
			}
		}
	}

	CurrentHitReactionState.bIsActive = false;
	CurrentHitReactionState.LastEndTimeSeconds = GetWorldTimeSecondsSafe();
	CurrentHitReactionState.LastEndReason = EndReason.IsEmpty() ? TEXT("None") : EndReason;
	CurrentHitReactionState.Detail = EndReason.Equals(TEXT("GuardBlocked"), ESearchCase::CaseSensitive)
		? TEXT("Hit reaction was cleared because Guard rewrote the incoming hit.")
		: TEXT("Hit reaction recovered automatically.");

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HitReaction] receiver=%s event=Exit attack=%s reason=%s end_time=%.2f"),
		*GetNameSafe(GetOwner()),
		*CurrentHitReactionState.SourceAttackInstanceName,
		*CurrentHitReactionState.LastEndReason,
		CurrentHitReactionState.LastEndTimeSeconds);

	OnPlayerHitReactionStateUpdated.Broadcast(CurrentHitReactionState);
}

bool UTwoHeartsHostileAttackReceiverComponent::InterruptCurrentActionForHitReaction()
{
	const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetOwner());
	if (!Character)
	{
		UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[HitReaction] receiver=%s stage=InterruptAction owner_invalid=true"), *GetNameSafe(GetOwner()));
		return false;
	}

	UTwoHeartsCombatActionContextComponent* ActionContextComponent = Character->GetCombatActionContextComponent();
	if (!ActionContextComponent || !ActionContextComponent->HasActiveAction())
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Verbose,
			TEXT("[HitReaction] receiver=%s stage=InterruptAction active_action=false detail=\"No active combat action needed interruption.\""),
			*GetNameSafe(GetOwner()));
		return true;
	}

	const FTwoHeartsCombatActionContextSnapshot& CurrentContext = ActionContextComponent->GetCurrentContext();
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HitReaction] receiver=%s stage=InterruptAction active_type=%s phase=%s instance=%s"),
		*GetNameSafe(GetOwner()),
		*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType)),
		*StaticEnum<ETwoHeartsCombatPhase>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionPhase)),
		*CurrentContext.ActionInstanceName);

	switch (CurrentContext.ActionType)
	{
	case ETwoHeartsCombatActionType::NormalAttack:
	{
		if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(Character))
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
			{
				for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
				{
					if (!AbilitySpec.IsActive())
					{
						continue;
					}

					if (UTwoHeartsGA_NormalAttackBase* ActiveNormalAttack = Cast<UTwoHeartsGA_NormalAttackBase>(AbilitySpec.GetPrimaryInstance()))
					{
						const bool bInterrupted = ActiveNormalAttack->TryInterruptByAction(ETwoHeartsCombatActionType::HitReaction, TEXT("InterruptedByHitReaction"));
						UE_LOG(
							LogtwoheartsCombatTest,
							Display,
							TEXT("[HitReaction] receiver=%s stage=InterruptNormalAttack success=%s attack_instance=%s"),
							*GetNameSafe(GetOwner()),
							bInterrupted ? TEXT("true") : TEXT("false"),
							*CurrentContext.ActionInstanceName);
						return bInterrupted;
					}
				}
			}
		}
		UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[HitReaction] receiver=%s stage=InterruptNormalAttack success=false detail=\"No active normal attack ability instance was found.\""), *GetNameSafe(GetOwner()));
		return false;
	}

	case ETwoHeartsCombatActionType::Dodge:
	{
		if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(Character))
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
			{
				for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
				{
					if (!AbilitySpec.IsActive())
					{
						continue;
					}

					if (UTwoHeartsGA_Dodge* ActiveDodge = Cast<UTwoHeartsGA_Dodge>(AbilitySpec.GetPrimaryInstance()))
					{
						const bool bInterrupted = ActiveDodge->TryInterruptByAction(ETwoHeartsCombatActionType::HitReaction, TEXT("InterruptedByHitReaction"));
						UE_LOG(
							LogtwoheartsCombatTest,
							Display,
							TEXT("[HitReaction] receiver=%s stage=InterruptDodge success=%s action_instance=%s"),
							*GetNameSafe(GetOwner()),
							bInterrupted ? TEXT("true") : TEXT("false"),
							*CurrentContext.ActionInstanceName);
						return bInterrupted;
					}
				}
			}
		}
		UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[HitReaction] receiver=%s stage=InterruptDodge success=false detail=\"No active dodge ability instance was found.\""), *GetNameSafe(GetOwner()));
		return false;
	}

	case ETwoHeartsCombatActionType::Guard:
	{
		if (const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(Character))
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent())
			{
				for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
				{
					if (!AbilitySpec.IsActive())
					{
						continue;
					}

					if (UTwoHeartsGA_Guard* ActiveGuard = Cast<UTwoHeartsGA_Guard>(AbilitySpec.GetPrimaryInstance()))
					{
						const bool bInterrupted = ActiveGuard->TryInterruptByAction(ETwoHeartsCombatActionType::HitReaction, TEXT("InterruptedByHitReaction"));
						UE_LOG(
							LogtwoheartsCombatTest,
							Display,
							TEXT("[HitReaction] receiver=%s stage=InterruptGuard success=%s action_instance=%s"),
							*GetNameSafe(GetOwner()),
							bInterrupted ? TEXT("true") : TEXT("false"),
							*CurrentContext.ActionInstanceName);
						return bInterrupted;
					}
				}
			}
		}
		UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[HitReaction] receiver=%s stage=InterruptGuard success=false detail=\"No active guard ability instance was found.\""), *GetNameSafe(GetOwner()));
		return false;
	}

	case ETwoHeartsCombatActionType::HitReaction:
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HitReaction] receiver=%s stage=InterruptHitReaction detail=\"Already inside hit reaction.\""), *GetNameSafe(GetOwner()));
		return true;

	default:
		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[HitReaction] receiver=%s stage=InterruptFallback active_type=%s detail=\"Finishing unsupported active action directly through action context.\""),
			*GetNameSafe(GetOwner()),
			*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(CurrentContext.ActionType)));
		ActionContextComponent->FinishAction(ETwoHeartsCombatActionEndReason::Interrupted, TEXT("InterruptedByHitReaction"));
		return true;
	}
}

ETwoHeartsHitReactionDirectionType UTwoHeartsHostileAttackReceiverComponent::ResolveHitReactionDirectionType(const FVector& IncomingDirection) const
{
	const AtwoheartsCharacter* Character = Cast<AtwoheartsCharacter>(GetOwner());
	if (!Character)
	{
		return ETwoHeartsHitReactionDirectionType::None;
	}

	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = Character->GetActorRightVector().GetSafeNormal2D();
	const FVector Direction = (-IncomingDirection).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		return ETwoHeartsHitReactionDirectionType::None;
	}

	const float ForwardDot = FVector::DotProduct(Forward, Direction);
	const float RightDot = FVector::DotProduct(Right, Direction);
	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f ? ETwoHeartsHitReactionDirectionType::Front : ETwoHeartsHitReactionDirectionType::Back;
	}

	return RightDot >= 0.0f ? ETwoHeartsHitReactionDirectionType::Right : ETwoHeartsHitReactionDirectionType::Left;
}

float UTwoHeartsHostileAttackReceiverComponent::ResolveHitReactionDuration(const FTwoHeartsPlayerDamageResult& DamageResult) const
{
	switch (DamageResult.AttackMetadata.HitReactionType)
	{
	case ETwoHeartsHitReactionType::Heavy:
		return HeavyHitReactionDurationSeconds;
	case ETwoHeartsHitReactionType::GuardBreak:
		return GuardBreakHitReactionDurationSeconds;
	case ETwoHeartsHitReactionType::Light:
	case ETwoHeartsHitReactionType::None:
	default:
		return LightHitReactionDurationSeconds;
	}
}

float UTwoHeartsHostileAttackReceiverComponent::GetWorldTimeSecondsSafe() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}

void UTwoHeartsHostileAttackReceiverComponent::HandleHitReactionRecovery()
{
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HitReaction] receiver=%s stage=RecoveryTimerFired attack=%s expected_recover_time=%.2f"),
		*GetNameSafe(GetOwner()),
		*CurrentHitReactionState.SourceAttackInstanceName,
		CurrentHitReactionState.ExpectedRecoveryTimeSeconds);
	FinishHitReaction(TEXT("AutoRecovered"));
}
