// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TwoHearts/Combat/TwoHeartsCombatPhase.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsAbilityGrant.h"
#include "twoheartsCharacter.generated.h"

class UAbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAnimMontage;
enum class ETwoHeartsAbilityInputID : uint8;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

USTRUCT(BlueprintType)
struct FNormalAttackDebugEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	float TimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	FString EventName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	FString Detail;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	int32 Segment = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	bool bHasQueuedNextSegment = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	FString SectionName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	FString PhaseName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	bool bInterruptibleByDodge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	bool bLogicEnded = false;
};

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AtwoheartsCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Normal attack Input Action */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* NormalAttackAction;

	/** Dodge Input Action */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Ability System", meta=(AllowPrivateAccess="true"))
	TArray<FTwoHeartsAbilityGrant> DefaultCombatAbilities;

	/** Montage that contains the three minimum normal attack sections */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimMontage> NormalAttackMontage;

	/** Section names used by the minimum 1-2-3 normal attack combo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	TArray<FName> NormalAttackSectionNames;

	/** Debug snapshot for the currently running normal attack Ability state. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	bool bIsNormalAttackAbilityActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	int32 CurrentNormalAttackAbilitySegment = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	bool bHasQueuedNextNormalAttackAbilitySegment = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	FString CurrentNormalAttackAbilitySectionName;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	ETwoHeartsCombatPhase CurrentNormalAttackCombatPhase = ETwoHeartsCombatPhase::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	bool bIsNormalAttackInterruptibleByDodge = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack", meta=(AllowPrivateAccess="true"))
	bool bIsNormalAttackLogicEnded = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	bool bEnableNormalAttackDebugLogging = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	bool bEnableNormalAttackVerboseLogging = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	bool bShowNormalAttackDebugPanel = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="50", UIMax="50", AllowPrivateAccess="true"))
	int32 NormalAttackDebugMaxEvents = 12;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	TArray<FNormalAttackDebugEvent> NormalAttackDebugEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	FString LastNormalAttackDebugFailureReason;

public:

	/** Constructor */
	AtwoheartsCharacter();	

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for normal attack input */
	void NormalAttack(const FInputActionValue& Value);

	/** Called for dodge input */
	void Dodge(const FInputActionValue& Value);

	void InitializeAbilitySystem();
	void GrantDefaultCombatAbilities();
	bool HandleAbilityInputPressed(ETwoHeartsAbilityInputID InputID);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintPure, Category="Combat|Normal Attack")
	UAnimMontage* GetNormalAttackMontage() const { return NormalAttackMontage; }

	UFUNCTION(BlueprintPure, Category="Combat|Normal Attack")
	bool IsValidNormalAttackSegment(int32 Segment) const;

	UFUNCTION(BlueprintPure, Category="Combat|Normal Attack")
	FName GetNormalAttackSectionName(int32 Segment) const;

	UFUNCTION(BlueprintPure, Category="Combat|Normal Attack")
	float GetNormalAttackSectionLength(int32 Segment) const;

	UFUNCTION(BlueprintPure, Category="Combat|Normal Attack|Debug")
	bool IsNormalAttackDebugPanelEnabled() const { return bShowNormalAttackDebugPanel; }

	UFUNCTION(BlueprintPure, Category="Combat|Normal Attack|Debug")
	bool IsNormalAttackDebugLoggingEnabled() const { return bEnableNormalAttackDebugLogging; }

	UFUNCTION(BlueprintPure, Category="Combat|Normal Attack|Debug")
	bool IsNormalAttackVerboseLoggingEnabled() const { return bEnableNormalAttackVerboseLogging; }

	UFUNCTION(BlueprintCallable, Category="Combat|Normal Attack|Debug")
	void SetNormalAttackDebugPanelEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="Combat|Normal Attack|Debug")
	void SetNormalAttackDebugLoggingEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="Combat|Normal Attack|Debug")
	void SetNormalAttackVerboseLoggingEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="Combat|Normal Attack|Debug")
	void ClearNormalAttackDebugEvents();

	void SetNormalAttackDebugRuntimeState(bool bIsActive, int32 Segment, bool bHasQueuedNextSegment, const FString& SectionName, ETwoHeartsCombatPhase CombatPhase, bool bInterruptibleByDodge, bool bLogicEnded);
	void SetLastNormalAttackDebugFailureReason(const FString& FailureReason);
	void PushNormalAttackDebugEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false);
	void PushNormalAttackDebugFailure(const TCHAR* EventName, const FString& Detail);

	bool IsNormalAttackingDebugState() const { return bIsNormalAttackAbilityActive; }
	int32 GetCurrentNormalAttackSegmentDebugState() const { return CurrentNormalAttackAbilitySegment; }
	bool HasQueuedNextNormalAttackSegmentDebugState() const { return bHasQueuedNextNormalAttackAbilitySegment; }
	const FString& GetCurrentNormalAttackSectionDebugState() const { return CurrentNormalAttackAbilitySectionName; }
	ETwoHeartsCombatPhase GetCurrentNormalAttackCombatPhaseDebugState() const { return CurrentNormalAttackCombatPhase; }
	bool IsNormalAttackInterruptibleByDodgeDebugState() const { return bIsNormalAttackInterruptibleByDodge; }
	bool IsNormalAttackLogicEndedDebugState() const { return bIsNormalAttackLogicEnded; }
	const TArray<FNormalAttackDebugEvent>& GetNormalAttackDebugEvents() const { return NormalAttackDebugEvents; }
	const FString& GetLastNormalAttackDebugFailureReason() const { return LastNormalAttackDebugFailureReason; }
	FString GetCombatPhaseDebugName(ETwoHeartsCombatPhase CombatPhase) const;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	void RecordNormalAttackDebugEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false);
	void RecordNormalAttackFailure(const TCHAR* EventName, const FString& Detail);
	void DrawNormalAttackDebugOverlay() const;
};

