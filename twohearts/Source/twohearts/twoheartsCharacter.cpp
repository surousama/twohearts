// Copyright Epic Games, Inc. All Rights Reserved.

#include "twoheartsCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "twohearts.h"

namespace
{
	constexpr int32 FirstNormalAttackSegment = 1;
	constexpr int32 LastNormalAttackSegment = 3;
}

AtwoheartsCharacter::AtwoheartsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	NormalAttackSectionNames = {
		TEXT("Attack_1"),
		TEXT("Attack_2"),
		TEXT("Attack_3")
	};
}

void AtwoheartsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AtwoheartsCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AtwoheartsCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AtwoheartsCharacter::Look);

		// Normal Attack
		if (NormalAttackAction)
		{
			EnhancedInputComponent->BindAction(NormalAttackAction, ETriggerEvent::Started, this, &AtwoheartsCharacter::NormalAttack);
		}
	}
	else
	{
		UE_LOG(Logtwohearts, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AtwoheartsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AtwoheartsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AtwoheartsCharacter::NormalAttack(const FInputActionValue& Value)
{
	TryStartNormalAttack();
}

void AtwoheartsCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AtwoheartsCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AtwoheartsCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AtwoheartsCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

bool AtwoheartsCharacter::TryStartNormalAttack()
{
	if (!bIsNormalAttacking)
	{
		PlayNormalAttackSegment(FirstNormalAttackSegment);
		return bIsNormalAttacking;
	}

	if (CurrentNormalAttackSegment >= LastNormalAttackSegment)
	{
		return false;
	}

	bHasQueuedNextNormalAttackSegment = true;
	return true;
}

void AtwoheartsCharacter::PlayNormalAttackSegment(int32 Segment)
{
	if (!IsValidNormalAttackSegment(Segment))
	{
		UE_LOG(Logtwohearts, Warning, TEXT("Invalid normal attack segment: %d."), Segment);
		ResetNormalAttackCombo();
		return;
	}

	if (!NormalAttackMontage)
	{
		UE_LOG(Logtwohearts, Warning, TEXT("NormalAttackMontage is not configured on %s."), *GetNameSafe(this));
		ResetNormalAttackCombo();
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(Logtwohearts, Warning, TEXT("No AnimInstance found for normal attack on %s."), *GetNameSafe(this));
		ResetNormalAttackCombo();
		return;
	}

	const FName SectionName = GetNormalAttackSectionName(Segment);
	if (SectionName.IsNone() || NormalAttackMontage->GetSectionIndex(SectionName) == INDEX_NONE)
	{
		UE_LOG(Logtwohearts, Warning, TEXT("Normal attack section %s is missing on %s."), *SectionName.ToString(), *GetNameSafe(NormalAttackMontage));
		ResetNormalAttackCombo();
		return;
	}

	const float SectionLength = GetNormalAttackSectionLength(Segment);
	if (SectionLength <= 0.0f)
	{
		UE_LOG(Logtwohearts, Warning, TEXT("Normal attack section %s has invalid length."), *SectionName.ToString());
		ResetNormalAttackCombo();
		return;
	}

	bIsNormalAttacking = true;
	CurrentNormalAttackSegment = Segment;
	bHasQueuedNextNormalAttackSegment = false;

	const float PlayedDuration = PlayAnimMontage(NormalAttackMontage, 1.0f, SectionName);
	if (PlayedDuration <= 0.0f)
	{
		UE_LOG(Logtwohearts, Warning, TEXT("Failed to play normal attack section %s on %s."), *SectionName.ToString(), *GetNameSafe(this));
		ResetNormalAttackCombo();
		return;
	}

	GetWorldTimerManager().ClearTimer(NormalAttackSegmentTimerHandle);
	GetWorldTimerManager().SetTimer(
		NormalAttackSegmentTimerHandle,
		this,
		&AtwoheartsCharacter::HandleNormalAttackSegmentFinished,
		SectionLength,
		false);
}

void AtwoheartsCharacter::HandleNormalAttackSegmentFinished()
{
	GetWorldTimerManager().ClearTimer(NormalAttackSegmentTimerHandle);

	if (!bIsNormalAttacking)
	{
		return;
	}

	const int32 FinishedSegment = CurrentNormalAttackSegment;
	if (bHasQueuedNextNormalAttackSegment && FinishedSegment < LastNormalAttackSegment)
	{
		PlayNormalAttackSegment(FinishedSegment + 1);
		return;
	}

	ResetNormalAttackCombo();
}

void AtwoheartsCharacter::ResetNormalAttackCombo()
{
	GetWorldTimerManager().ClearTimer(NormalAttackSegmentTimerHandle);

	if (NormalAttackMontage)
	{
		StopAnimMontage(NormalAttackMontage);
	}

	bIsNormalAttacking = false;
	CurrentNormalAttackSegment = 0;
	bHasQueuedNextNormalAttackSegment = false;
}

bool AtwoheartsCharacter::IsValidNormalAttackSegment(int32 Segment) const
{
	return Segment >= FirstNormalAttackSegment && Segment <= LastNormalAttackSegment;
}

FName AtwoheartsCharacter::GetNormalAttackSectionName(int32 Segment) const
{
	const int32 SectionIndex = Segment - 1;
	return NormalAttackSectionNames.IsValidIndex(SectionIndex) ? NormalAttackSectionNames[SectionIndex] : NAME_None;
}

float AtwoheartsCharacter::GetNormalAttackSectionLength(int32 Segment) const
{
	if (!NormalAttackMontage)
	{
		return 0.0f;
	}

	const int32 SectionIndex = NormalAttackMontage->GetSectionIndex(GetNormalAttackSectionName(Segment));
	return SectionIndex != INDEX_NONE ? NormalAttackMontage->GetSectionLength(SectionIndex) : 0.0f;
}
