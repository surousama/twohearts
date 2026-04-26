// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "CombatCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UAbilitySystemComponent;
class UGameplayAbility;
class UAnimMontage;
class UTHCombatAbilitySystemComponent;
class UTHCombatAttributeSet;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogCombatCharacter, Log, All);

/**
 * Combat base character for the Phase 01 GAS-driven combat flow.
 */
UCLASS(abstract)
class ACombatCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Ability System Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTHCombatAbilitySystemComponent> AbilitySystemComponent;

	/** Attribute Set used by GAS */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTHCombatAttributeSet> AttributeSet;

protected:

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Normal attack input routed into the GAS normal attack ability */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* NormalAttackAction;

	/** Gameplay Abilities granted on initialization */
	UPROPERTY(EditAnywhere, Category="GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	/** Explicit normal attack ability granted to this character */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS|Abilities", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> NormalAttackAbilityClass;

	/** Montage used by the normal attack GAS ability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS|Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> NormalAttackMontage;

	/** Prevent startup abilities from being granted multiple times */
	bool bStartupAbilitiesGranted = false;

	/** Prevent the explicit normal attack ability from being granted multiple times */
	bool bNormalAttackAbilityGranted = false;

	/** Default camera boom length for the combat character */
	UPROPERTY(EditAnywhere, Category="Camera", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float DefaultCameraDistance = 300.0f;

public:

	/** Constructor */
	ACombatCharacter();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for normal attack input */
	void NormalAttackPressed();

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Routes normal attack input into the GAS normal attack ability */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual bool DoNormalAttackStart();

	/** Activates the configured normal attack GAS ability */
	UFUNCTION(BlueprintCallable, Category="GAS|Abilities")
	virtual bool TriggerNormalAttackAbility();

protected:

	/** Initializes Ability Actor Info for GAS */
	void InitializeAbilityActorInfo();

	/** Grants character startup abilities once authority is available */
	void GrantStartupAbilities();

	/** Grants the explicit normal attack ability once authority is available */
	void GrantNormalAttackAbility();

public:

	// ~begin IAbilitySystemInterface interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// ~end IAbilitySystemInterface interface

	/** Returns the montage used by the normal attack ability */
	UAnimMontage* GetNormalAttackMontage() const;

protected:

	/** Initialization */
	virtual void BeginPlay() override;

	/** Handles input bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Handles possessed initialization */
	virtual void PossessedBy(AController* NewController) override;

	/** Keeps Ability Actor Info in sync when the controller changes */
	virtual void NotifyControllerChanged() override;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
