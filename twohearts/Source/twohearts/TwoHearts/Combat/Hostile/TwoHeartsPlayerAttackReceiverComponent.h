#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TwoHearts/Combat/TwoHeartsAttackMetadata.h"
#include "TwoHeartsPlayerAttackReceiverComponent.generated.h"

class AActor;

UENUM(BlueprintType)
enum class ETwoHeartsPlayerAttackSignalType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	AttackContact UMETA(DisplayName="AttackContact")
};

UENUM(BlueprintType)
enum class ETwoHeartsHostileHitResultType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	HitConfirmed UMETA(DisplayName="HitConfirmed"),
	SignalInvalid UMETA(DisplayName="SignalInvalid"),
	IgnoredNoHealth UMETA(DisplayName="IgnoredNoHealth")
};

UENUM(BlueprintType)
enum class ETwoHeartsHostileDamageResultType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	DamageApplied UMETA(DisplayName="DamageApplied"),
	IgnoredNoHealth UMETA(DisplayName="IgnoredNoHealth")
};

USTRUCT(BlueprintType)
struct FTwoHeartsPlayerAttackSignal
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	ETwoHeartsPlayerAttackSignalType SignalType = ETwoHeartsPlayerAttackSignalType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	FVector AttackDirection = FVector::ForwardVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	float TimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	bool bIsHitWindowActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	bool bHasContact = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	bool bWasDuplicateTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	FString Detail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Player Attack")
	FTwoHeartsAttackMetadata AttackMetadata;
};

USTRUCT(BlueprintType)
struct FTwoHeartsHostileHitResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	ETwoHeartsHostileHitResultType ResultType = ETwoHeartsHostileHitResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	bool bCanApplyDamage = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	float ResultTimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	FString Detail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Hit Result")
	FTwoHeartsAttackMetadata AttackMetadata;
};

USTRUCT(BlueprintType)
struct FTwoHeartsHostileDamageResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	ETwoHeartsHostileDamageResultType ResultType = ETwoHeartsHostileDamageResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	ETwoHeartsHostileHitResultType SourceHitResultType = ETwoHeartsHostileHitResultType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	float BaseDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	float AppliedDamage = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	float PreviousHealth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	float CurrentHealth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	bool bReachedZeroHealth = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	float ResultTimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	FString Detail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Hostile Damage Result")
	FTwoHeartsAttackMetadata AttackMetadata;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsPlayerAttackSignalDelegate, const FTwoHeartsPlayerAttackSignal&, Signal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsHostileHitResultDelegate, const FTwoHeartsHostileHitResult&, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsHostileDamageResultDelegate, const FTwoHeartsHostileDamageResult&, DamageResult);

UCLASS(ClassGroup=(Combat), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UTwoHeartsPlayerAttackReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTwoHeartsPlayerAttackReceiverComponent();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Combat|Player Attack")
	void ReceivePlayerAttackSignal(const FTwoHeartsPlayerAttackSignal& Signal);

	UFUNCTION(BlueprintCallable, Category="Combat|Player Attack")
	void ClearSignalHistory();

	UFUNCTION(BlueprintPure, Category="Combat|Player Attack")
	bool HasReceivedPlayerAttackSignal() const { return bHasReceivedSignal; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Attack")
	const FTwoHeartsPlayerAttackSignal& GetLastSignal() const { return LastSignal; }

	UFUNCTION(BlueprintPure, Category="Combat|Player Attack")
	const TArray<FTwoHeartsPlayerAttackSignal>& GetSignalHistory() const { return SignalHistory; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Damage Result")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Damage Result")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Damage Result")
	bool IsDefeated() const { return CurrentHealth <= KINDA_SMALL_NUMBER; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Hit Result")
	const FTwoHeartsHostileHitResult& GetLastHostileHitResult() const { return LastHostileHitResult; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Hit Result")
	const TArray<FTwoHeartsHostileHitResult>& GetHostileHitResultHistory() const { return HostileHitResultHistory; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Damage Result")
	const FTwoHeartsHostileDamageResult& GetLastHostileDamageResult() const { return LastHostileDamageResult; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Damage Result")
	const TArray<FTwoHeartsHostileDamageResult>& GetHostileDamageResultHistory() const { return HostileDamageResultHistory; }

	UPROPERTY(BlueprintAssignable, Category="Combat|Player Attack")
	FTwoHeartsPlayerAttackSignalDelegate OnPlayerAttackSignalReceived;

	UPROPERTY(BlueprintAssignable, Category="Combat|Hostile Hit Result")
	FTwoHeartsHostileHitResultDelegate OnHostileHitResultUpdated;

	UPROPERTY(BlueprintAssignable, Category="Combat|Hostile Damage Result")
	FTwoHeartsHostileDamageResultDelegate OnHostileDamageResultUpdated;

	UPROPERTY(BlueprintAssignable, Category="Combat|Hostile Damage Result")
	FTwoHeartsHostileDamageResultDelegate OnZeroHealthReached;

private:
	UPROPERTY(EditAnywhere, Category="Combat|Player Attack|Debug", meta=(ClampMin="1", UIMin="1"))
	int32 MaxSignalHistory = 8;

	UPROPERTY(EditAnywhere, Category="Combat|Hostile Hit Result|Debug", meta=(ClampMin="1", UIMin="1"))
	int32 MaxHostileHitResultHistory = 8;

	UPROPERTY(EditAnywhere, Category="Combat|Hostile Damage Result|Debug", meta=(ClampMin="1", UIMin="1"))
	int32 MaxHostileDamageResultHistory = 8;

	UPROPERTY(EditAnywhere, Category="Combat|Hostile Damage Result", meta=(ClampMin="0.0", UIMin="0.0"))
	float MaxHealth = 30.0f;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Player Attack|Debug")
	bool bHasReceivedSignal = false;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Player Attack|Debug")
	FTwoHeartsPlayerAttackSignal LastSignal;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Player Attack|Debug")
	TArray<FTwoHeartsPlayerAttackSignal> SignalHistory;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Hostile Hit Result|Debug")
	FTwoHeartsHostileHitResult LastHostileHitResult;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Hostile Hit Result|Debug")
	TArray<FTwoHeartsHostileHitResult> HostileHitResultHistory;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Hostile Damage Result|Debug")
	FTwoHeartsHostileDamageResult LastHostileDamageResult;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Hostile Damage Result|Debug")
	TArray<FTwoHeartsHostileDamageResult> HostileDamageResultHistory;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Hostile Damage Result|Debug")
	float CurrentHealth = 30.0f;
};
