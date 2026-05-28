#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHeartsCombatActionContextComponent.generated.h"

UENUM(BlueprintType)
enum class ETwoHeartsCombatActionType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	NormalAttack UMETA(DisplayName="NormalAttack"),
	Dodge UMETA(DisplayName="Dodge"),
	Guard UMETA(DisplayName="Guard"),
	HitReaction UMETA(DisplayName="HitReaction"),
	HostileAttackProbe UMETA(DisplayName="HostileAttackProbe")
};

UENUM(BlueprintType)
enum class ETwoHeartsCombatActionEndReason : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Completed UMETA(DisplayName="Completed"),
	Cancelled UMETA(DisplayName="Cancelled"),
	Interrupted UMETA(DisplayName="Interrupted"),
	Failed UMETA(DisplayName="Failed")
};

UENUM(BlueprintType)
enum class ETwoHeartsCombatInputEvaluationResult : uint8
{
	ExecuteNow = 0 UMETA(DisplayName="ExecuteNow"),
	BufferInput UMETA(DisplayName="BufferInput"),
	Reject UMETA(DisplayName="Reject")
};

UENUM(BlueprintType)
enum class ETwoHeartsCombatInputConsumptionRoute : uint8
{
	None = 0 UMETA(DisplayName="None"),
	ActivateMatchingAbility UMETA(DisplayName="ActivateMatchingAbility"),
	ForwardToActiveAbility UMETA(DisplayName="ForwardToActiveAbility"),
	ReserveForFutureBufferConsumer UMETA(DisplayName="ReserveForFutureBufferConsumer")
};

USTRUCT(BlueprintType)
struct FTwoHeartsCombatActionRegistration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Action Context")
	ETwoHeartsCombatActionType ActionType = ETwoHeartsCombatActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Action Context")
	ETwoHeartsCombatPhase InitialPhase = ETwoHeartsCombatPhase::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Action Context")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Action Context")
	FGameplayTag ActionStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Action Context")
	FString ActionInstanceName = TEXT("None");
};

USTRUCT(BlueprintType)
struct FTwoHeartsBufferedCombatInput
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	bool bIsSet = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	ETwoHeartsCombatActionType IncomingActionType = ETwoHeartsCombatActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	ETwoHeartsCombatInputConsumptionRoute ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	FString Reason = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	float BufferedTimeSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FTwoHeartsCombatActionContextSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	bool bIsActionActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	bool bHasLogicEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	ETwoHeartsCombatActionType ActionType = ETwoHeartsCombatActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	ETwoHeartsCombatPhase ActionPhase = ETwoHeartsCombatPhase::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	ETwoHeartsCombatActionEndReason LastEndReason = ETwoHeartsCombatActionEndReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	FGameplayTag AbilityTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	FGameplayTag ActionStateTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	FString ActionInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	FString LastReason = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	float ActionStartTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Action Context")
	float LastUpdateTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	bool bHasBufferedInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	ETwoHeartsCombatActionType BufferedInputActionType = ETwoHeartsCombatActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	ETwoHeartsCombatInputConsumptionRoute BufferedInputRoute = ETwoHeartsCombatInputConsumptionRoute::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	FString BufferedInputReason = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	float BufferedInputTimeSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	FString BufferedInputLastEventName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	FString BufferedInputLastEventDetail = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Buffer")
	float BufferedInputLastEventTimeSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FTwoHeartsCombatInputEvaluation
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	ETwoHeartsCombatInputEvaluationResult Result = ETwoHeartsCombatInputEvaluationResult::ExecuteNow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	ETwoHeartsCombatActionType IncomingActionType = ETwoHeartsCombatActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	ETwoHeartsCombatInputConsumptionRoute ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	bool bHasActiveAction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	ETwoHeartsCombatActionType ActiveActionType = ETwoHeartsCombatActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	ETwoHeartsCombatPhase ActiveActionPhase = ETwoHeartsCombatPhase::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	bool bShouldForwardToActiveAbility = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input Evaluation")
	FString Reason = TEXT("None");
};

UCLASS(ClassGroup=(Combat), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UTwoHeartsCombatActionContextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTwoHeartsCombatActionContextComponent();

	UFUNCTION(BlueprintCallable, Category="Combat|Action Context")
	void BeginAction(const FTwoHeartsCombatActionRegistration& Registration, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category="Combat|Action Context")
	void TransitionToPhase(ETwoHeartsCombatPhase NewPhase, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category="Combat|Action Context")
	void MarkLogicEnded(const FString& Reason);

	UFUNCTION(BlueprintCallable, Category="Combat|Action Context")
	void FinishAction(ETwoHeartsCombatActionEndReason EndReason, const FString& Reason);

	const FTwoHeartsCombatActionContextSnapshot& GetCurrentContext() const { return CurrentContext; }

	UFUNCTION(BlueprintPure, Category="Combat|Action Context")
	FTwoHeartsCombatActionContextSnapshot GetCurrentContextCopy() const { return CurrentContext; }

	UFUNCTION(BlueprintPure, Category="Combat|Action Context")
	bool HasActiveAction() const { return CurrentContext.bIsActionActive; }

	UFUNCTION(BlueprintPure, Category="Combat|Action Context")
	bool CanCurrentActionBeInterruptedBy(ETwoHeartsCombatActionType IncomingActionType) const;

	UFUNCTION(BlueprintPure, Category="Combat|Input Evaluation")
	FTwoHeartsCombatInputEvaluation EvaluateInputForAction(ETwoHeartsCombatActionType IncomingActionType) const;

	UFUNCTION(BlueprintCallable, Category="Combat|Input Buffer")
	void BufferInput(ETwoHeartsCombatActionType IncomingActionType, ETwoHeartsCombatInputConsumptionRoute ConsumptionRoute, const FString& Reason);

	UFUNCTION(BlueprintPure, Category="Combat|Input Buffer")
	bool HasBufferedInput() const { return BufferedInput.bIsSet; }

	UFUNCTION(BlueprintPure, Category="Combat|Input Buffer")
	FTwoHeartsBufferedCombatInput GetBufferedInputCopy() const { return BufferedInput; }

	UFUNCTION(BlueprintCallable, Category="Combat|Input Buffer")
	bool ConsumeBufferedInput(FTwoHeartsBufferedCombatInput& OutBufferedInput, const FString& ConsumerName);

	UFUNCTION(BlueprintCallable, Category="Combat|Input Buffer")
	bool RestoreBufferedInput(const FTwoHeartsBufferedCombatInput& InputToRestore, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category="Combat|Input Buffer")
	void ClearBufferedInput(const FString& Reason);

	UFUNCTION(BlueprintPure, Category="Combat|Action Context")
	FString BuildCurrentContextDebugString() const;

private:
	float GetWorldTimeSecondsSafe() const;
	void RecordContextEvent(const TCHAR* EventName, const FString& Detail) const;
	void RecordBufferedInputStateEvent(const TCHAR* EventName, const FString& Detail);
	void SyncBufferedInputToSnapshot();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Action Context", meta=(AllowPrivateAccess="true"))
	FTwoHeartsCombatActionContextSnapshot CurrentContext;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Input Buffer", meta=(AllowPrivateAccess="true"))
	FTwoHeartsBufferedCombatInput BufferedInput;
};
