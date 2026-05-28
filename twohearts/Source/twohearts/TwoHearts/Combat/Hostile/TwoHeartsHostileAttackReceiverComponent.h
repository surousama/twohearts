#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TwoHearts/Combat/TwoHeartsAttackMetadata.h"
#include "TwoHeartsHostileAttackReceiverComponent.generated.h"


class AActor;

UENUM(BlueprintType)
enum class ETwoHeartsHostileAttackSignalType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	AttackStarted UMETA(DisplayName="AttackStarted"),
	HitWindowOpened UMETA(DisplayName="HitWindowOpened"),
	HitWindowClosed UMETA(DisplayName="HitWindowClosed"),
	AttackContact UMETA(DisplayName="AttackContact"),
	AttackFinished UMETA(DisplayName="AttackFinished")
};

UENUM(BlueprintType)
enum class ETwoHeartsPlayerHitResultType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	PendingIncomingHit UMETA(DisplayName="PendingIncomingHit"),
	HitConfirmed UMETA(DisplayName="HitConfirmed"),
	HitExpired UMETA(DisplayName="HitExpired"),
	SignalInvalid UMETA(DisplayName="SignalInvalid"),
	GuardRewritten UMETA(DisplayName="GuardRewritten")
};

UENUM(BlueprintType)
enum class ETwoHeartsPlayerDamageResultType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	DamageApplied UMETA(DisplayName="DamageApplied"),
	GuardBlocked UMETA(DisplayName="GuardBlocked"),
	IgnoredNoHealth UMETA(DisplayName="IgnoredNoHealth")
};

UENUM(BlueprintType)
enum class ETwoHeartsHitReactionDirectionType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Front UMETA(DisplayName="Front"),
	Back UMETA(DisplayName="Back"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right")
};

USTRUCT(BlueprintType)
struct FTwoHeartsHostileAttackSignal
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	ETwoHeartsHostileAttackSignalType SignalType = ETwoHeartsHostileAttackSignalType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	FVector AttackDirection = FVector::ForwardVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	float TimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	bool bIsHitWindowActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	bool bHasContact = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	FString Detail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack")
	FTwoHeartsAttackMetadata AttackMetadata;
};

USTRUCT(BlueprintType)
struct FTwoHeartsPlayerHitResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	ETwoHeartsPlayerHitResultType ResultType = ETwoHeartsPlayerHitResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	bool bHitConfirmed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	bool bCanBeRewrittenByGuard = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	float ResultTimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	float ContactTimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	ETwoHeartsHostileAttackSignalType SourceSignalType = ETwoHeartsHostileAttackSignalType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	FString Detail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result")
	FTwoHeartsAttackMetadata AttackMetadata;
};

USTRUCT(BlueprintType)
struct FTwoHeartsPlayerDamageResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	ETwoHeartsPlayerDamageResultType ResultType = ETwoHeartsPlayerDamageResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	ETwoHeartsPlayerHitResultType SourceHitResultType = ETwoHeartsPlayerHitResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	float BaseDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	float FinalDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	bool bWasGuardRewritten = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	float ResultTimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	float HealthBeforeDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	float HealthAfterDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	FString Detail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result")
	FTwoHeartsAttackMetadata AttackMetadata;
};

USTRUCT(BlueprintType)
struct FTwoHeartsGuardSettlementRequest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	ETwoHeartsPlayerHitResultType RewrittenHitResultType = ETwoHeartsPlayerHitResultType::GuardRewritten;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	FString RewriteDetail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	bool bConsumesGuardResource = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	bool bRefundsGuardResource = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	bool bAppliesGuardCooldown = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	float GuardCooldownSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	FGameplayTag GuardCooldownTag;
};

USTRUCT(BlueprintType)
struct FTwoHeartsGuardOutcome
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	bool bWasGuardSuccessful = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	ETwoHeartsGuardDisplacementResult DisplacementResult = ETwoHeartsGuardDisplacementResult::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	ETwoHeartsGuardDamageResult DamageResult = ETwoHeartsGuardDamageResult::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	ETwoHeartsPlayerHitResultType ResolvedHitResultType = ETwoHeartsPlayerHitResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	ETwoHeartsPlayerDamageResultType ResolvedPlayerDamageResultType = ETwoHeartsPlayerDamageResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	float BaseDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	float FinalDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	bool bConsumedGuardResource = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	bool bRefundedGuardResource = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	bool bAppliedGuardCooldown = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	float GuardCooldownSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	FGameplayTag GuardCooldownTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	float ResultTimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	FString Detail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome")
	FTwoHeartsAttackMetadata AttackMetadata;
};

USTRUCT(BlueprintType)
struct FTwoHeartsPlayerHitReactionState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	bool bIsActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	FString HitReactionInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	FString SourceAttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	ETwoHeartsHitReactionType HitReactionType = ETwoHeartsHitReactionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	ETwoHeartsHitReactionDirectionType DirectionType = ETwoHeartsHitReactionDirectionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	FVector IncomingDirection = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	float StartTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	float ExpectedRecoveryTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	float LastEndTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	FString LastEndReason = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction")
	FString Detail = TEXT("None");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsHostileAttackSignalDelegate, const FTwoHeartsHostileAttackSignal&, Signal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsPlayerHitResultDelegate, const FTwoHeartsPlayerHitResult&, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsPlayerDamageResultDelegate, const FTwoHeartsPlayerDamageResult&, DamageResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsGuardOutcomeDelegate, const FTwoHeartsGuardOutcome&, GuardOutcome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsPlayerHitReactionStateDelegate, const FTwoHeartsPlayerHitReactionState&, HitReactionState);

UCLASS(ClassGroup=(Combat), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UTwoHeartsHostileAttackReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTwoHeartsHostileAttackReceiverComponent();

	UFUNCTION(BlueprintCallable, Category="Combat|Hostile Attack")
	void ReceiveHostileAttackSignal(const FTwoHeartsHostileAttackSignal& Signal);

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Attack")
	bool HasReceivedHostileAttackSignal() const { return bHasReceivedSignal; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Attack")
	const FTwoHeartsHostileAttackSignal& GetLastSignal() const { return LastSignal; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Attack")
	const TArray<FTwoHeartsHostileAttackSignal>& GetSignalHistory() const { return SignalHistory; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Hit Result")
	bool HasPlayerHitResult() const { return bHasPlayerHitResult; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Hit Result")
	const FTwoHeartsPlayerHitResult& GetLastPlayerHitResult() const { return LastPlayerHitResult; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Hit Result")
	const TArray<FTwoHeartsPlayerHitResult>& GetPlayerHitResultHistory() const { return PlayerHitResultHistory; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Damage Result")
	bool HasPlayerDamageResult() const { return bHasPlayerDamageResult; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Damage Result")
	const FTwoHeartsPlayerDamageResult& GetLastPlayerDamageResult() const { return LastPlayerDamageResult; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Damage Result")
	const TArray<FTwoHeartsPlayerDamageResult>& GetPlayerDamageResultHistory() const { return PlayerDamageResultHistory; }

	UFUNCTION(BlueprintPure, Category="Combat|Guard Outcome")
	bool HasGuardOutcome() const { return bHasGuardOutcome; }

	UFUNCTION(BlueprintPure, Category="Combat|Guard Outcome")
	const FTwoHeartsGuardOutcome& GetLastGuardOutcome() const { return LastGuardOutcome; }

	UFUNCTION(BlueprintPure, Category="Combat|Guard Outcome")
	const TArray<FTwoHeartsGuardOutcome>& GetGuardOutcomeHistory() const { return GuardOutcomeHistory; }

	UFUNCTION(BlueprintPure, Category="Combat|Guard Outcome")
	bool DoesLastGuardEnableFollowUpAbility() const;

	UFUNCTION(BlueprintPure, Category="Combat|Player Damage Result")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Damage Result")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Combat|Hit Reaction")
	bool IsHitReactionActive() const { return CurrentHitReactionState.bIsActive; }

	UFUNCTION(BlueprintPure, Category="Combat|Hit Reaction")
	const FTwoHeartsPlayerHitReactionState& GetCurrentHitReactionState() const { return CurrentHitReactionState; }

	UFUNCTION(BlueprintCallable, Category="Combat|Hostile Attack")
	void ClearSignalHistory();

	UFUNCTION(BlueprintCallable, Category="Combat|Player Hit Result")
	bool RewriteLastPlayerHitResultForGuard(ETwoHeartsPlayerHitResultType NewResultType, const FString& Detail);

	bool RewriteLastPlayerHitResultForGuard(const FTwoHeartsGuardSettlementRequest& SettlementRequest);

	UPROPERTY(BlueprintAssignable, Category="Combat|Hostile Attack")
	FTwoHeartsHostileAttackSignalDelegate OnHostileAttackSignalReceived;

	UPROPERTY(BlueprintAssignable, Category="Combat|Player Hit Result")
	FTwoHeartsPlayerHitResultDelegate OnPlayerHitResultUpdated;

	UPROPERTY(BlueprintAssignable, Category="Combat|Player Damage Result")
	FTwoHeartsPlayerDamageResultDelegate OnPlayerDamageResultUpdated;

	UPROPERTY(BlueprintAssignable, Category="Combat|Guard Outcome")
	FTwoHeartsGuardOutcomeDelegate OnGuardOutcomeUpdated;

	UPROPERTY(BlueprintAssignable, Category="Combat|Hit Reaction")
	FTwoHeartsPlayerHitReactionStateDelegate OnPlayerHitReactionStateUpdated;

private:
	void BeginPlay() override;
	void UpdatePlayerHitResultFromSignal(const FTwoHeartsHostileAttackSignal& Signal);
	void FinalizeCurrentPendingAttack(const FTwoHeartsHostileAttackSignal& Signal, ETwoHeartsPlayerHitResultType FinalResultType, bool bHitConfirmed, bool bCanBeRewrittenByGuard, const FString& Detail);
	bool TryConsumePendingGuardRewriteForAttack(const FString& AttackInstanceName, FTwoHeartsGuardSettlementRequest& OutSettlementRequest);
	void PushPlayerHitResult(const FTwoHeartsPlayerHitResult& HitResult);
	void UpdatePlayerDamageResultFromHitResult(const FTwoHeartsPlayerHitResult& HitResult);
	void PushPlayerDamageResult(const FTwoHeartsPlayerDamageResult& DamageResult);
	void RefreshGuardOutcomeDetail(FTwoHeartsGuardOutcome& GuardOutcome) const;
	void CommitGuardOutcome(const FTwoHeartsGuardSettlementRequest& SettlementRequest, const FTwoHeartsPlayerHitResult& HitResult);
	void PushGuardOutcome(const FTwoHeartsGuardOutcome& GuardOutcome);
	void UpdateHitReactionStateFromDamageResult(const FTwoHeartsPlayerDamageResult& DamageResult);
	void EnterHitReaction(const FTwoHeartsPlayerDamageResult& DamageResult);
	void FinishHitReaction(const FString& EndReason);
	bool InterruptCurrentActionForHitReaction();
	ETwoHeartsHitReactionDirectionType ResolveHitReactionDirectionType(const FVector& IncomingDirection) const;
	float ResolveHitReactionDuration(const FTwoHeartsPlayerDamageResult& DamageResult) const;
	float GetWorldTimeSecondsSafe() const;

	UFUNCTION()
	void HandleHitReactionRecovery();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 MaxSignalHistory = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 MaxPlayerHitResultHistory = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 MaxPlayerDamageResultHistory = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Guard Outcome|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 MaxGuardOutcomeHistory = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Player Damage Result", meta=(ClampMin="1.0", UIMin="1.0", AllowPrivateAccess="true"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction", meta=(ClampMin="0.05", UIMin="0.05", AllowPrivateAccess="true"))
	float LightHitReactionDurationSeconds = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction", meta=(ClampMin="0.05", UIMin="0.05", AllowPrivateAccess="true"))
	float HeavyHitReactionDurationSeconds = 0.50f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hit Reaction", meta=(ClampMin="0.05", UIMin="0.05", AllowPrivateAccess="true"))
	float GuardBreakHitReactionDurationSeconds = 0.65f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Damage Result", meta=(AllowPrivateAccess="true"))
	float CurrentHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack", meta=(AllowPrivateAccess="true"))
	bool bHasReceivedSignal = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack", meta=(AllowPrivateAccess="true"))
	FTwoHeartsHostileAttackSignal LastSignal;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack", meta=(AllowPrivateAccess="true"))
	TArray<FTwoHeartsHostileAttackSignal> SignalHistory;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	bool bHasPlayerHitResult = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	FTwoHeartsPlayerHitResult LastPlayerHitResult;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	TArray<FTwoHeartsPlayerHitResult> PlayerHitResultHistory;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Damage Result", meta=(AllowPrivateAccess="true"))
	bool bHasPlayerDamageResult = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Damage Result", meta=(AllowPrivateAccess="true"))
	FTwoHeartsPlayerDamageResult LastPlayerDamageResult;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Damage Result", meta=(AllowPrivateAccess="true"))
	TArray<FTwoHeartsPlayerDamageResult> PlayerDamageResultHistory;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard Outcome", meta=(AllowPrivateAccess="true"))
	bool bHasGuardOutcome = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard Outcome", meta=(AllowPrivateAccess="true"))
	FTwoHeartsGuardOutcome LastGuardOutcome;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard Outcome", meta=(AllowPrivateAccess="true"))
	TArray<FTwoHeartsGuardOutcome> GuardOutcomeHistory;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hit Reaction", meta=(AllowPrivateAccess="true"))
	FTwoHeartsPlayerHitReactionState CurrentHitReactionState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	bool bHasPendingAttack = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	FString PendingAttackInstanceName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> PendingAttackSourceActor = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	float PendingAttackStartSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	FTwoHeartsAttackMetadata PendingAttackMetadata;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	bool bHasPendingGuardRewriteRequest = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	FString PendingGuardRewriteAttackInstanceName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	FTwoHeartsGuardSettlementRequest PendingGuardSettlementRequest;

	FTimerHandle HitReactionRecoveryTimerHandle;
};
