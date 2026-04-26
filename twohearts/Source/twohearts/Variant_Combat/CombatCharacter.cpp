// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "THCombatAbilitySystemComponent.h"
#include "THCombatAttributeSet.h"

DEFINE_LOG_CATEGORY(LogCombatCharacter);

ACombatCharacter::ACombatCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DefaultCameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AbilitySystemComponent = CreateDefaultSubobject<UTHCombatAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UTHCombatAttributeSet>(TEXT("AttributeSet"));

	Tags.Add(FName("Player"));
}

void ACombatCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ACombatCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACombatCharacter::NormalAttackPressed()
{
	DoNormalAttackStart();
}

void ACombatCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ACombatCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

bool ACombatCharacter::DoNormalAttackStart()
{
	return TriggerNormalAttackAbility();
}

bool ACombatCharacter::TriggerNormalAttackAbility()
{
	if (AbilitySystemComponent == nullptr || NormalAttackAbilityClass == nullptr)
	{
		return false;
	}

	return AbilitySystemComponent->TryActivateAbilityByClass(NormalAttackAbilityClass);
}

void ACombatCharacter::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void ACombatCharacter::GrantStartupAbilities()
{
	if (!HasAuthority() || bStartupAbilitiesGranted || AbilitySystemComponent == nullptr)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		if (AbilityClass != nullptr)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}

	bStartupAbilitiesGranted = true;
}

void ACombatCharacter::GrantNormalAttackAbility()
{
	if (!HasAuthority() || bNormalAttackAbilityGranted || AbilitySystemComponent == nullptr || NormalAttackAbilityClass == nullptr)
	{
		return;
	}

	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(NormalAttackAbilityClass, 1));
	bNormalAttackAbilityGranted = true;
}

UAbilitySystemComponent* ACombatCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ACombatCharacter::GetNormalAttackMontage() const
{
	return NormalAttackMontage;
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilityActorInfo();
	GrantStartupAbilities();
	GrantNormalAttackAbility();

	GetCameraBoom()->TargetArmLength = DefaultCameraDistance;
}

void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction != nullptr)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Move);
		}

		if (LookAction != nullptr)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Look);
		}

		if (MouseLookAction != nullptr)
		{
			EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Look);
		}

		if (NormalAttackAction != nullptr)
		{
			EnhancedInputComponent->BindAction(NormalAttackAction, ETriggerEvent::Started, this, &ACombatCharacter::NormalAttackPressed);
		}
	}
}

void ACombatCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeAbilityActorInfo();
	GrantStartupAbilities();
	GrantNormalAttackAbility();
}

void ACombatCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	InitializeAbilityActorInfo();
}
