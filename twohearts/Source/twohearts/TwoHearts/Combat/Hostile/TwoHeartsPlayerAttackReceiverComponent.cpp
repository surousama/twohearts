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

	const TCHAR* LexHostileHitResultTypeToString(const ETwoHeartsHostileHitResultType ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsHostileHitResultType::HitConfirmed:
			return TEXT("HitConfirmed");
		case ETwoHeartsHostileHitResultType::SignalInvalid:
			return TEXT("SignalInvalid");
		case ETwoHeartsHostileHitResultType::IgnoredNoHealth:
			return TEXT("IgnoredNoHealth");
		case ETwoHeartsHostileHitResultType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexHostileDamageResultTypeToString(const ETwoHeartsHostileDamageResultType ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsHostileDamageResultType::DamageApplied:
			return TEXT("DamageApplied");
		case ETwoHeartsHostileDamageResultType::IgnoredNoHealth:
			return TEXT("IgnoredNoHealth");
		case ETwoHeartsHostileDamageResultType::None:
		default:
			return TEXT("None");
		}
	}

	template <typename TItem>
	void TrimHistory(TArray<TItem>& History, const int32 MaxEntries)
	{
		const int32 ExcessEntries = History.Num() - FMath::Max(1, MaxEntries);
		if (ExcessEntries > 0)
		{
			History.RemoveAt(0, ExcessEntries);
		}
	}
}

UTwoHeartsPlayerAttackReceiverComponent::UTwoHeartsPlayerAttackReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTwoHeartsPlayerAttackReceiverComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = FMath::Max(0.0f, MaxHealth);
}

void UTwoHeartsPlayerAttackReceiverComponent::ReceivePlayerAttackSignal(const FTwoHeartsPlayerAttackSignal& Signal)
{
	bHasReceivedSignal = true;
	LastSignal = Signal;
	SignalHistory.Add(Signal);
	TrimHistory(SignalHistory, MaxSignalHistory);

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[PlayerAttackSignal] receiver=%s type=%s attack=%s source=%s target=%s hit_window=%s contact=%s duplicate=%s base_damage=%.2f detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		LexPlayerAttackSignalTypeToString(Signal.SignalType),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		*GetNameSafe(Signal.TargetActor),
		Signal.bIsHitWindowActive ? TEXT("true") : TEXT("false"),
		Signal.bHasContact ? TEXT("true") : TEXT("false"),
		Signal.bWasDuplicateTarget ? TEXT("true") : TEXT("false"),
		FMath::Max(0.0f, Signal.AttackMetadata.BaseDamage),
		*Signal.Detail);

	OnPlayerAttackSignalReceived.Broadcast(Signal);

	FTwoHeartsHostileHitResult HitResult;
	HitResult.AttackInstanceName = Signal.AttackInstanceName;
	HitResult.SourceActor = Signal.SourceActor;
	AActor* ResolvedTargetActor = Signal.TargetActor.Get();
	if (!ResolvedTargetActor)
	{
		ResolvedTargetActor = GetOwner();
	}
	HitResult.TargetActor = ResolvedTargetActor;
	HitResult.ResultTimestampSeconds = Signal.TimestampSeconds;
	HitResult.AttackMetadata = Signal.AttackMetadata;

	if (Signal.SignalType != ETwoHeartsPlayerAttackSignalType::AttackContact || !Signal.bHasContact || !Signal.bIsHitWindowActive)
	{
		HitResult.ResultType = ETwoHeartsHostileHitResultType::SignalInvalid;
		HitResult.bCanApplyDamage = false;
		HitResult.Detail = TEXT("Incoming player attack signal was ignored because it was not a valid active hit contact.");
	}
	else if (IsDefeated())
	{
		HitResult.ResultType = ETwoHeartsHostileHitResultType::IgnoredNoHealth;
		HitResult.bCanApplyDamage = false;
		HitResult.Detail = TEXT("Incoming player attack signal was ignored because hostile health had already reached zero.");
	}
	else
	{
		HitResult.ResultType = ETwoHeartsHostileHitResultType::HitConfirmed;
		HitResult.bCanApplyDamage = true;
		HitResult.Detail = FString::Printf(
			TEXT("Hostile confirmed player attack %s for %.2f base damage."),
			*Signal.AttackInstanceName,
			FMath::Max(0.0f, Signal.AttackMetadata.BaseDamage));
	}

	LastHostileHitResult = HitResult;
	HostileHitResultHistory.Add(HitResult);
	TrimHistory(HostileHitResultHistory, MaxHostileHitResultHistory);

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileHitResult] receiver=%s result=%s attack=%s source=%s damage_gate=%s detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		LexHostileHitResultTypeToString(HitResult.ResultType),
		*HitResult.AttackInstanceName,
		*GetNameSafe(HitResult.SourceActor),
		HitResult.bCanApplyDamage ? TEXT("open") : TEXT("closed"),
		*HitResult.Detail);

	OnHostileHitResultUpdated.Broadcast(HitResult);

	if (!HitResult.bCanApplyDamage)
	{
		if (HitResult.ResultType == ETwoHeartsHostileHitResultType::IgnoredNoHealth)
		{
			FTwoHeartsHostileDamageResult DamageResult;
			DamageResult.ResultType = ETwoHeartsHostileDamageResultType::IgnoredNoHealth;
			DamageResult.AttackInstanceName = HitResult.AttackInstanceName;
			DamageResult.SourceActor = HitResult.SourceActor;
			DamageResult.TargetActor = HitResult.TargetActor;
			DamageResult.SourceHitResultType = HitResult.ResultType;
			DamageResult.BaseDamage = FMath::Max(0.0f, Signal.AttackMetadata.BaseDamage);
			DamageResult.PreviousHealth = CurrentHealth;
			DamageResult.CurrentHealth = CurrentHealth;
			DamageResult.ResultTimestampSeconds = Signal.TimestampSeconds;
			DamageResult.Detail = TEXT("Damage accounting was skipped because hostile health was already zero.");
			DamageResult.AttackMetadata = Signal.AttackMetadata;

			LastHostileDamageResult = DamageResult;
			HostileDamageResultHistory.Add(DamageResult);
			TrimHistory(HostileDamageResultHistory, MaxHostileDamageResultHistory);

			UE_LOG(
				LogtwoheartsCombatTest,
				Display,
				TEXT("[HostileDamageResult] receiver=%s result=%s attack=%s damage=%.2f health=%.2f->%.2f zero=%s detail=\"%s\""),
				*GetNameSafe(GetOwner()),
				LexHostileDamageResultTypeToString(DamageResult.ResultType),
				*DamageResult.AttackInstanceName,
				DamageResult.AppliedDamage,
				DamageResult.PreviousHealth,
				DamageResult.CurrentHealth,
				DamageResult.bReachedZeroHealth ? TEXT("true") : TEXT("false"),
				*DamageResult.Detail);

			OnHostileDamageResultUpdated.Broadcast(DamageResult);
		}

		return;
	}

	const float PreviousHealth = CurrentHealth;
	const float AppliedDamage = FMath::Max(0.0f, Signal.AttackMetadata.BaseDamage);
	CurrentHealth = FMath::Clamp(PreviousHealth - AppliedDamage, 0.0f, MaxHealth);

	FTwoHeartsHostileDamageResult DamageResult;
	DamageResult.ResultType = ETwoHeartsHostileDamageResultType::DamageApplied;
	DamageResult.AttackInstanceName = HitResult.AttackInstanceName;
	DamageResult.SourceActor = HitResult.SourceActor;
	DamageResult.TargetActor = HitResult.TargetActor;
	DamageResult.SourceHitResultType = HitResult.ResultType;
	DamageResult.BaseDamage = AppliedDamage;
	DamageResult.AppliedDamage = AppliedDamage;
	DamageResult.PreviousHealth = PreviousHealth;
	DamageResult.CurrentHealth = CurrentHealth;
	DamageResult.bReachedZeroHealth = CurrentHealth <= KINDA_SMALL_NUMBER && PreviousHealth > KINDA_SMALL_NUMBER;
	DamageResult.ResultTimestampSeconds = Signal.TimestampSeconds;
	DamageResult.Detail = FString::Printf(
		TEXT("Hostile took %.2f damage from %s and health is now %.2f."),
		AppliedDamage,
		*Signal.AttackInstanceName,
		CurrentHealth);
	DamageResult.AttackMetadata = Signal.AttackMetadata;

	LastHostileDamageResult = DamageResult;
	HostileDamageResultHistory.Add(DamageResult);
	TrimHistory(HostileDamageResultHistory, MaxHostileDamageResultHistory);

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileDamageResult] receiver=%s result=%s attack=%s damage=%.2f health=%.2f->%.2f zero=%s detail=\"%s\""),
		*GetNameSafe(GetOwner()),
		LexHostileDamageResultTypeToString(DamageResult.ResultType),
		*DamageResult.AttackInstanceName,
		DamageResult.AppliedDamage,
		DamageResult.PreviousHealth,
		DamageResult.CurrentHealth,
		DamageResult.bReachedZeroHealth ? TEXT("true") : TEXT("false"),
		*DamageResult.Detail);

	OnHostileDamageResultUpdated.Broadcast(DamageResult);

	if (DamageResult.bReachedZeroHealth)
	{
		OnZeroHealthReached.Broadcast(DamageResult);
	}
}

void UTwoHeartsPlayerAttackReceiverComponent::ClearSignalHistory()
{
	bHasReceivedSignal = false;
	LastSignal = FTwoHeartsPlayerAttackSignal();
	SignalHistory.Reset();
	LastHostileHitResult = FTwoHeartsHostileHitResult();
	HostileHitResultHistory.Reset();
	LastHostileDamageResult = FTwoHeartsHostileDamageResult();
	HostileDamageResultHistory.Reset();
	CurrentHealth = FMath::Max(0.0f, MaxHealth);
}
