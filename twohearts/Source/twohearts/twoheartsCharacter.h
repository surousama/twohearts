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
class UStaticMesh;
class UStaticMeshComponent;
class UTwoHeartsCombatActionContextComponent;
class UTwoHeartsHostileAttackReceiverComponent;
enum class ETwoHeartsAbilityInputID : uint8;
struct FInputActionValue;
struct FTwoHeartsCombatInputEvaluation;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug")
	bool bVerboseOnly = false;
};

USTRUCT(BlueprintType)
struct FTwoHeartsDodgeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontageFallback = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageForward = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageForwardRight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageRight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageBackwardRight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageBackward = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageBackwardLeft = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageLeft = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge|Directional")
	TObjectPtr<UAnimMontage> DodgeMontageForwardLeft = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge", meta=(ClampMin="0.05", UIMin="0.05"))
	float DodgeDurationSeconds = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge", meta=(ClampMin="0.0", UIMin="0.0"))
	float DodgeDistance = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge", meta=(ClampMin="0.0", UIMin="0.0"))
	float DodgeCooldownSeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge", meta=(ClampMin="0.0", UIMin="0.0"))
	float DodgeInvulnerableStartSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge", meta=(ClampMin="0.0", UIMin="0.0"))
	float DodgeInvulnerableDurationSeconds = 0.22f;
};

UENUM(BlueprintType)
enum class ETwoHeartsGuardInputMode : uint8
{
	TapWindowOnly = 0 UMETA(DisplayName="TapWindowOnly"),
	HoldReserved UMETA(DisplayName="HoldReserved")
};

USTRUCT(BlueprintType)
struct FTwoHeartsGuardConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Guard")
	ETwoHeartsGuardInputMode InputMode = ETwoHeartsGuardInputMode::TapWindowOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Guard", meta=(ClampMin="0.0", UIMin="0.0"))
	float GuardStartupSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Guard", meta=(ClampMin="0.01", UIMin="0.01"))
	float GuardWindowSeconds = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Guard", meta=(ClampMin="0.0", UIMin="0.0"))
	float GuardRecoverySeconds = 0.10f;
};

USTRUCT(BlueprintType)
struct FTwoHeartsCombatInputDebugEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input|Debug")
	float TimestampSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input|Debug")
	FString InputName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input|Debug")
	FString ResultName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input|Debug")
	FString RouteName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Input|Debug")
	FString Detail;
};

USTRUCT(BlueprintType)
struct FTwoHeartsWeaponVisualConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
	TObjectPtr<UStaticMesh> WeaponMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
	FName EquippedSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
	FTransform EquippedRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
	FName UnequippedSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
	FTransform UnequippedRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon", meta=(ClampMin="0.0", UIMin="0.0"))
	float MovementSpeedThreshold = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
	bool bHideWhenUnequippedSocketMissing = true;
};

UENUM(BlueprintType)
enum class ETwoHeartsWeaponVisualState : uint8
{
	Hidden = 0 UMETA(DisplayName="Hidden"),
	Equipped UMETA(DisplayName="Equipped"),
	Unequipped UMETA(DisplayName="Unequipped")
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTwoHeartsCombatActionContextComponent> CombatActionContextComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> WeaponVisualComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTwoHeartsHostileAttackReceiverComponent> HostileAttackReceiverComponent;
	
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

	/** Guard Input Action */
	UPROPERTY(EditAnywhere, Category="Input|Combat")
	UInputAction* GuardAction;

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
	bool bEnableNormalAttackVerboseLogging = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	bool bShowNormalAttackDebugPanel = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="50", UIMax="50", AllowPrivateAccess="true"))
	int32 NormalAttackDebugMaxEvents = 6;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	TArray<FNormalAttackDebugEvent> NormalAttackDebugEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Normal Attack|Debug", meta=(AllowPrivateAccess="true"))
	FString LastNormalAttackDebugFailureReason;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Dodge", meta=(AllowPrivateAccess="true"))
	FTwoHeartsDodgeConfig DodgeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Guard", meta=(AllowPrivateAccess="true"))
	FTwoHeartsGuardConfig GuardConfig;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Dodge|Debug", meta=(AllowPrivateAccess="true"))
	bool bIsDodgeAbilityActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Dodge|Debug", meta=(AllowPrivateAccess="true"))
	bool bIsDodgeInvulnerable = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Dodge|Debug", meta=(AllowPrivateAccess="true"))
	bool bIsDodgeCooldownReady = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Dodge|Debug", meta=(AllowPrivateAccess="true"))
	FString CurrentDodgeDirectionName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Dodge|Debug", meta=(AllowPrivateAccess="true"))
	FString LastDodgeDebugEventName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Dodge|Debug", meta=(AllowPrivateAccess="true"))
	FString LastDodgeDebugDetail;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Dodge|Debug", meta=(AllowPrivateAccess="true"))
	float LastDodgeEventTimeSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard|Debug", meta=(AllowPrivateAccess="true"))
	bool bIsGuardAbilityActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard|Debug", meta=(AllowPrivateAccess="true"))
	bool bIsGuardWindowActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard|Debug", meta=(AllowPrivateAccess="true"))
	bool bGuardHoldInputReserved = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard|Debug", meta=(AllowPrivateAccess="true"))
	FString CurrentGuardPhaseName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard|Debug", meta=(AllowPrivateAccess="true"))
	FString LastGuardDebugEventName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard|Debug", meta=(AllowPrivateAccess="true"))
	FString LastGuardDebugDetail;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Guard|Debug", meta=(AllowPrivateAccess="true"))
	float LastGuardEventTimeSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Input", meta=(AllowPrivateAccess="true"))
	FVector2D CachedMoveInput = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Input|Debug", meta=(ClampMin="1", UIMin="1", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	int32 CombatInputDebugMaxEvents = 6;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Input|Debug", meta=(AllowPrivateAccess="true"))
	TArray<FTwoHeartsCombatInputDebugEvent> CombatInputDebugEvents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon", meta=(AllowPrivateAccess="true"))
	FTwoHeartsWeaponVisualConfig WeaponVisualConfig;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Weapon", meta=(AllowPrivateAccess="true"))
	ETwoHeartsWeaponVisualState CurrentWeaponVisualState = ETwoHeartsWeaponVisualState::Hidden;

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
	void ClearMoveInput(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for normal attack input */
	void NormalAttack(const FInputActionValue& Value);

	/** Called for dodge input */
	void Dodge(const FInputActionValue& Value);

	/** Called for guard input */
	void Guard(const FInputActionValue& Value);
	void GuardReleased(const FInputActionValue& Value);

	void InitializeAbilitySystem();
	void GrantDefaultCombatAbilities();
	bool HandleAbilityInputPressed(ETwoHeartsAbilityInputID InputID);
	void HandleAbilityInputReleased(ETwoHeartsAbilityInputID InputID);

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

	UFUNCTION(BlueprintPure, Category="Combat|Dodge")
	const FTwoHeartsDodgeConfig& GetDodgeConfig() const { return DodgeConfig; }

	UFUNCTION(BlueprintPure, Category="Combat|Guard")
	const FTwoHeartsGuardConfig& GetGuardConfig() const { return GuardConfig; }

	UFUNCTION(BlueprintPure, Category="Combat|Dodge")
	FVector GetDesiredDodgeDirectionWorld() const;

	UFUNCTION(BlueprintPure, Category="Combat|Dodge")
	FString GetDesiredDodgeDirectionName() const;

	UFUNCTION(BlueprintPure, Category="Combat|Dodge")
	UAnimMontage* GetDodgeMontageForDirection(const FString& DirectionName) const;

	UFUNCTION(BlueprintPure, Category="Combat|Action Context")
	UTwoHeartsCombatActionContextComponent* GetCombatActionContextComponent() const { return CombatActionContextComponent; }

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Attack")
	UTwoHeartsHostileAttackReceiverComponent* GetHostileAttackReceiverComponent() const { return HostileAttackReceiverComponent; }

	UFUNCTION(BlueprintPure, Category="Combat|Weapon")
	UStaticMeshComponent* GetWeaponVisualComponent() const { return WeaponVisualComponent; }

	UFUNCTION(BlueprintPure, Category="Combat|Weapon")
	const FTwoHeartsWeaponVisualConfig& GetWeaponVisualConfig() const { return WeaponVisualConfig; }

	UFUNCTION(BlueprintPure, Category="Combat|Weapon")
	ETwoHeartsWeaponVisualState GetCurrentWeaponVisualState() const { return CurrentWeaponVisualState; }

	UFUNCTION(BlueprintPure, Category="Combat|Weapon")
	bool IsWeaponShownAsEquipped() const { return CurrentWeaponVisualState == ETwoHeartsWeaponVisualState::Equipped; }

	UFUNCTION(BlueprintCallable, Category="Combat|Weapon")
	void RefreshWeaponVisualState();

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
	void SetDodgeDebugRuntimeState(bool bIsActive, bool bIsInvulnerable, bool bIsCooldownReady, const FString& DirectionName);
	void PushDodgeDebugEvent(const TCHAR* EventName, const FString& Detail);
	void SetGuardDebugRuntimeState(bool bIsActive, bool bIsWindowActive, const FString& PhaseName, bool bHoldReserved);
	void PushGuardDebugEvent(const TCHAR* EventName, const FString& Detail);

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
	bool ShouldDisplayNormalAttackDebugEvent(const FNormalAttackDebugEvent& Event) const;
	bool IsDodgingDebugState() const { return bIsDodgeAbilityActive; }
	bool IsDodgeInvulnerableDebugState() const { return bIsDodgeInvulnerable; }
	bool IsDodgeCooldownReadyDebugState() const { return bIsDodgeCooldownReady; }
	const FString& GetCurrentDodgeDirectionDebugState() const { return CurrentDodgeDirectionName; }
	const FString& GetLastDodgeDebugEventName() const { return LastDodgeDebugEventName; }
	const FString& GetLastDodgeDebugDetail() const { return LastDodgeDebugDetail; }
	float GetLastDodgeEventTimeSeconds() const { return LastDodgeEventTimeSeconds; }
	bool IsGuardingDebugState() const { return bIsGuardAbilityActive; }
	bool IsGuardWindowActiveDebugState() const { return bIsGuardWindowActive; }
	bool IsGuardHoldInputReservedDebugState() const { return bGuardHoldInputReserved; }
	const FString& GetCurrentGuardPhaseDebugState() const { return CurrentGuardPhaseName; }
	const FString& GetLastGuardDebugEventName() const { return LastGuardDebugEventName; }
	const FString& GetLastGuardDebugDetail() const { return LastGuardDebugDetail; }
	float GetLastGuardEventTimeSeconds() const { return LastGuardEventTimeSeconds; }
	const TArray<FTwoHeartsCombatInputDebugEvent>& GetCombatInputDebugEvents() const { return CombatInputDebugEvents; }
	void PushCombatInputDebugEvent(const FString& InputName, const FString& ResultName, const FString& RouteName, const FString& Detail);
	bool TryConsumeReservedCombatInput(const FString& ConsumerName);

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	void RecordNormalAttackDebugEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false);
	void RecordNormalAttackFailure(const TCHAR* EventName, const FString& Detail);
	void RecordAbilityInputDebugEvent(ETwoHeartsAbilityInputID InputID, const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false);
	void RecordAbilityInputFailure(ETwoHeartsAbilityInputID InputID, const TCHAR* EventName, const FString& Detail);
	bool HandleBufferedCombatInput(ETwoHeartsAbilityInputID InputID, const FString& InputName, const FTwoHeartsCombatInputEvaluation& InputEvaluation);
	bool TryExecuteCombatInputNow(ETwoHeartsAbilityInputID InputID, const FString& InputName, const FTwoHeartsCombatInputEvaluation& InputEvaluation);
	void RecordCombatInputDebugEvent(const FString& InputName, const FString& ResultName, const FString& RouteName, const FString& Detail);
	bool ShouldEmitNormalAttackDebugLog(const TCHAR* EventName, bool bVerboseOnly) const;
	FString GetCombatDebugLogFilePath() const;
	void ResetCombatDebugLogFile();
	void AppendCombatDebugLogLine(const FString& Line) const;
	void DrawNormalAttackDebugOverlay() const;
	bool ShouldShowWeaponAsEquipped() const;
	bool IsCharacterInMovingWeaponState() const;
	void ApplyWeaponVisualState(ETwoHeartsWeaponVisualState NewState, bool bForceRefresh = false);
	void AttachWeaponVisualToSocket(FName SocketName, const FTransform& RelativeTransform);
};

