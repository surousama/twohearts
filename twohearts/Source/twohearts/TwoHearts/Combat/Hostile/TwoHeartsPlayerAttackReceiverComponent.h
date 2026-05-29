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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTwoHeartsPlayerAttackSignalDelegate, const FTwoHeartsPlayerAttackSignal&, Signal);

UCLASS(ClassGroup=(Combat), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UTwoHeartsPlayerAttackReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTwoHeartsPlayerAttackReceiverComponent();

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

	UPROPERTY(BlueprintAssignable, Category="Combat|Player Attack")
	FTwoHeartsPlayerAttackSignalDelegate OnPlayerAttackSignalReceived;

private:
	UPROPERTY(EditAnywhere, Category="Combat|Player Attack|Debug", meta=(ClampMin="1", UIMin="1"))
	int32 MaxSignalHistory = 8;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Player Attack|Debug")
	bool bHasReceivedSignal = false;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Player Attack|Debug")
	FTwoHeartsPlayerAttackSignal LastSignal;

	UPROPERTY(VisibleInstanceOnly, Category="Combat|Player Attack|Debug")
	TArray<FTwoHeartsPlayerAttackSignal> SignalHistory;
};
