// Copyright Epic Games, Inc. All Rights Reserved.

#include "twoheartsCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsAbilityGrant.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h"
#include "TwoHearts/Combat/Gameplay/Input/TwoHeartsAbilityInputID.h"
#include "twohearts.h"

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

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	NormalAttackSectionNames = {
		TEXT("Attack_1"),
		TEXT("Attack_2"),
		TEXT("Attack_3")
	};

	if (DefaultCombatAbilities.IsEmpty())
	{
		FTwoHeartsAbilityGrant NormalAttackSegment1Grant;
		NormalAttackSegment1Grant.AbilityClass = UTwoHeartsGA_NormalAttack_1::StaticClass();
		NormalAttackSegment1Grant.InputID = ETwoHeartsAbilityInputID::NormalAttack;
		DefaultCombatAbilities.Add(NormalAttackSegment1Grant);

		FTwoHeartsAbilityGrant NormalAttackSegment2Grant;
		NormalAttackSegment2Grant.AbilityClass = UTwoHeartsGA_NormalAttack_2::StaticClass();
		NormalAttackSegment2Grant.InputID = ETwoHeartsAbilityInputID::NormalAttack;
		DefaultCombatAbilities.Add(NormalAttackSegment2Grant);

		FTwoHeartsAbilityGrant NormalAttackSegment3Grant;
		NormalAttackSegment3Grant.AbilityClass = UTwoHeartsGA_NormalAttack_3::StaticClass();
		NormalAttackSegment3Grant.InputID = ETwoHeartsAbilityInputID::NormalAttack;
		DefaultCombatAbilities.Add(NormalAttackSegment3Grant);
	}
}

UAbilitySystemComponent* AtwoheartsCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AtwoheartsCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem();
	GrantDefaultCombatAbilities();
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
	if (HandleAbilityInputPressed(ETwoHeartsAbilityInputID::NormalAttack))
	{
		return;
	}

	UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[AbilityInput] NormalAttack input did not match any granted combat ability on %s."), *GetNameSafe(this));
}

void AtwoheartsCharacter::InitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogtwoheartsCombatTest, Error, TEXT("[AbilitySystem] %s is missing an AbilitySystemComponent."), *GetNameSafe(this));
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	UE_LOG(LogtwoheartsCombatTest, Display, TEXT("[AbilitySystem] Initialized ActorInfo for %s."), *GetNameSafe(this));
}

void AtwoheartsCharacter::GrantDefaultCombatAbilities()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	for (const FTwoHeartsAbilityGrant& AbilityGrant : DefaultCombatAbilities)
	{
		if (!AbilityGrant.AbilityClass)
		{
			continue;
		}

		bool bAlreadyGranted = false;
		for (const FGameplayAbilitySpec& ExistingSpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (ExistingSpec.Ability && ExistingSpec.Ability->GetClass() == AbilityGrant.AbilityClass)
			{
				bAlreadyGranted = true;
				break;
			}
		}

		if (bAlreadyGranted)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityGrant.AbilityClass, AbilityGrant.AbilityLevel, static_cast<int32>(AbilityGrant.InputID), this);
		AbilitySystemComponent->GiveAbility(AbilitySpec);

		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[AbilitySystem] Granted ability %s to %s with InputID=%d."),
			*GetNameSafe(AbilityGrant.AbilityClass),
			*GetNameSafe(this),
			static_cast<int32>(AbilityGrant.InputID));
	}
}

bool AtwoheartsCharacter::HandleAbilityInputPressed(ETwoHeartsAbilityInputID InputID)
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	bool bHandledInput = false;
	const int32 NumericInputID = static_cast<int32>(InputID);

	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID != NumericInputID || !AbilitySpec.IsActive())
		{
			continue;
		}

		bHandledInput = true;
		AbilitySystemComponent->AbilitySpecInputPressed(AbilitySpec);
	}

	if (bHandledInput)
	{
		return true;
	}

	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID != NumericInputID)
		{
			continue;
		}

		bHandledInput = true;
		AbilitySystemComponent->AbilitySpecInputPressed(AbilitySpec);

		if (!AbilitySpec.IsActive() && AbilitySystemComponent->TryActivateAbility(AbilitySpec.Handle))
		{
			return true;
		}
	}

	return bHandledInput;
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

bool AtwoheartsCharacter::IsValidNormalAttackSegment(int32 Segment) const
{
	return Segment >= 1 && Segment <= 3;
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

void AtwoheartsCharacter::SetNormalAttackDebugPanelEnabled(bool bEnabled)
{
	bShowNormalAttackDebugPanel = bEnabled;
	RecordNormalAttackDebugEvent(TEXT("DebugPanelToggled"), FString::Printf(TEXT("Panel visibility set to %s."), bEnabled ? TEXT("true") : TEXT("false")));
}

void AtwoheartsCharacter::SetNormalAttackDebugLoggingEnabled(bool bEnabled)
{
	bEnableNormalAttackDebugLogging = bEnabled;
	RecordNormalAttackDebugEvent(TEXT("DebugLoggingToggled"), FString::Printf(TEXT("Structured logging set to %s."), bEnabled ? TEXT("true") : TEXT("false")));
}

void AtwoheartsCharacter::SetNormalAttackVerboseLoggingEnabled(bool bEnabled)
{
	bEnableNormalAttackVerboseLogging = bEnabled;
	RecordNormalAttackDebugEvent(TEXT("VerboseLoggingToggled"), FString::Printf(TEXT("Verbose logging set to %s."), bEnabled ? TEXT("true") : TEXT("false")));
}

void AtwoheartsCharacter::ClearNormalAttackDebugEvents()
{
	NormalAttackDebugEvents.Reset();
	LastNormalAttackDebugFailureReason.Reset();
	SetNormalAttackDebugRuntimeState(false, 0, false, TEXT("None"));
	RecordNormalAttackDebugEvent(TEXT("DebugEventsCleared"), TEXT("Cleared normal attack debug event history."));
}

void AtwoheartsCharacter::SetNormalAttackDebugRuntimeState(bool bIsActive, int32 Segment, bool bHasQueuedNextSegment, const FString& SectionName)
{
	bIsNormalAttackAbilityActive = bIsActive;
	CurrentNormalAttackAbilitySegment = Segment;
	bHasQueuedNextNormalAttackAbilitySegment = bHasQueuedNextSegment;
	CurrentNormalAttackAbilitySectionName = SectionName;
}

void AtwoheartsCharacter::SetLastNormalAttackDebugFailureReason(const FString& FailureReason)
{
	LastNormalAttackDebugFailureReason = FailureReason;
}

void AtwoheartsCharacter::PushNormalAttackDebugEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly)
{
	RecordNormalAttackDebugEvent(EventName, Detail, bVerboseOnly);
}

void AtwoheartsCharacter::PushNormalAttackDebugFailure(const TCHAR* EventName, const FString& Detail)
{
	RecordNormalAttackFailure(EventName, Detail);
}

void AtwoheartsCharacter::RecordNormalAttackDebugEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly)
{
	FNormalAttackDebugEvent Event;
	Event.TimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Event.EventName = EventName;
	Event.Detail = Detail;
	Event.Segment = CurrentNormalAttackAbilitySegment;
	Event.bIsAttacking = bIsNormalAttackAbilityActive;
	Event.bHasQueuedNextSegment = bHasQueuedNextNormalAttackAbilitySegment;
	Event.SectionName = CurrentNormalAttackAbilitySectionName.IsEmpty() ? TEXT("None") : CurrentNormalAttackAbilitySectionName;

	NormalAttackDebugEvents.Add(MoveTemp(Event));
	const int32 ExcessEvents = NormalAttackDebugEvents.Num() - FMath::Max(1, NormalAttackDebugMaxEvents);
	if (ExcessEvents > 0)
	{
		NormalAttackDebugEvents.RemoveAt(0, ExcessEvents);
	}

	if (!bEnableNormalAttackDebugLogging)
	{
		return;
	}

	if (bVerboseOnly && !bEnableNormalAttackVerboseLogging)
	{
		return;
	}

	const FNormalAttackDebugEvent& LatestEvent = NormalAttackDebugEvents.Last();
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[NormalAttack] time=%.3f event=%s actor=%s segment=%d attacking=%s queued_next=%s section=%s detail=\"%s\""),
		LatestEvent.TimestampSeconds,
		*LatestEvent.EventName,
		*GetNameSafe(this),
		LatestEvent.Segment,
		LatestEvent.bIsAttacking ? TEXT("true") : TEXT("false"),
		LatestEvent.bHasQueuedNextSegment ? TEXT("true") : TEXT("false"),
		*LatestEvent.SectionName,
		*LatestEvent.Detail);
}

void AtwoheartsCharacter::RecordNormalAttackFailure(const TCHAR* EventName, const FString& Detail)
{
	LastNormalAttackDebugFailureReason = Detail;
	RecordNormalAttackDebugEvent(EventName, Detail);

	if (bEnableNormalAttackDebugLogging)
	{
		UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[NormalAttackFailure] actor=%s detail=\"%s\""), *GetNameSafe(this), *Detail);
	}
}
