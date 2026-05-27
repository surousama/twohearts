#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsHostileAttackSignalDelegate, const FTwoHeartsHostileAttackSignal&, Signal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsPlayerHitResultDelegate, const FTwoHeartsPlayerHitResult&, HitResult);

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

	UFUNCTION(BlueprintCallable, Category="Combat|Hostile Attack")
	void ClearSignalHistory();

	UFUNCTION(BlueprintCallable, Category="Combat|Player Hit Result")
	bool RewriteLastPlayerHitResultForGuard(ETwoHeartsPlayerHitResultType NewResultType, const FString& Detail);

	UPROPERTY(BlueprintAssignable, Category="Combat|Hostile Attack")
	FTwoHeartsHostileAttackSignalDelegate OnHostileAttackSignalReceived;

	UPROPERTY(BlueprintAssignable, Category="Combat|Player Hit Result")
	FTwoHeartsPlayerHitResultDelegate OnPlayerHitResultUpdated;

private:
	void UpdatePlayerHitResultFromSignal(const FTwoHeartsHostileAttackSignal& Signal);
	void FinalizeCurrentPendingAttack(const FTwoHeartsHostileAttackSignal& Signal, ETwoHeartsPlayerHitResultType FinalResultType, bool bHitConfirmed, bool bCanBeRewrittenByGuard, const FString& Detail);
	void PushPlayerHitResult(const FTwoHeartsPlayerHitResult& HitResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 MaxSignalHistory = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Player Hit Result|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 MaxPlayerHitResultHistory = 8;

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	bool bHasPendingAttack = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	FString PendingAttackInstanceName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> PendingAttackSourceActor = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Player Hit Result", meta=(AllowPrivateAccess="true"))
	float PendingAttackStartSeconds = 0.0f;
};
