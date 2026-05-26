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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsHostileAttackSignalDelegate, const FTwoHeartsHostileAttackSignal&, Signal);

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

	UFUNCTION(BlueprintCallable, Category="Combat|Hostile Attack")
	void ClearSignalHistory();

	UPROPERTY(BlueprintAssignable, Category="Combat|Hostile Attack")
	FTwoHeartsHostileAttackSignalDelegate OnHostileAttackSignalReceived;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 MaxSignalHistory = 8;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack", meta=(AllowPrivateAccess="true"))
	bool bHasReceivedSignal = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack", meta=(AllowPrivateAccess="true"))
	FTwoHeartsHostileAttackSignal LastSignal;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack", meta=(AllowPrivateAccess="true"))
	TArray<FTwoHeartsHostileAttackSignal> SignalHistory;
};
