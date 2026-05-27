// Copyright Epic Games, Inc. All Rights Reserved.

#include "twoheartsCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsAbilityGrant.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Guard.h"
#include "TwoHearts/Combat/Gameplay/Input/TwoHeartsAbilityInputID.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"

namespace
{
ETwoHeartsCombatActionType GetCombatActionTypeForInputID(ETwoHeartsAbilityInputID InputID)
{
	switch (InputID)
	{
	case ETwoHeartsAbilityInputID::NormalAttack:
		return ETwoHeartsCombatActionType::NormalAttack;

	case ETwoHeartsAbilityInputID::Dodge:
		return ETwoHeartsCombatActionType::Dodge;

	case ETwoHeartsAbilityInputID::Guard:
		return ETwoHeartsCombatActionType::Guard;

	default:
		return ETwoHeartsCombatActionType::None;
	}
}

FString GetCombatInputName(ETwoHeartsAbilityInputID InputID)
{
	switch (InputID)
	{
	case ETwoHeartsAbilityInputID::NormalAttack:
		return TEXT("NormalAttack");

	case ETwoHeartsAbilityInputID::Dodge:
		return TEXT("Dodge");

	case ETwoHeartsAbilityInputID::Guard:
		return TEXT("Guard");

	default:
		return FString::Printf(TEXT("Input_%d"), static_cast<int32>(InputID));
	}
}

bool TryGetCombatInputIDForActionType(ETwoHeartsCombatActionType ActionType, ETwoHeartsAbilityInputID& OutInputID)
{
	switch (ActionType)
	{
	case ETwoHeartsCombatActionType::NormalAttack:
		OutInputID = ETwoHeartsAbilityInputID::NormalAttack;
		return true;

	case ETwoHeartsCombatActionType::Dodge:
		OutInputID = ETwoHeartsAbilityInputID::Dodge;
		return true;

	case ETwoHeartsCombatActionType::Guard:
		OutInputID = ETwoHeartsAbilityInputID::Guard;
		return true;

	default:
		return false;
	}
}

FString GetCombatInputEvaluationResultName(ETwoHeartsCombatInputEvaluationResult Result)
{
	const UEnum* ResultEnum = StaticEnum<ETwoHeartsCombatInputEvaluationResult>();
	return ResultEnum ? ResultEnum->GetNameStringByValue(static_cast<int64>(Result)) : TEXT("Unknown");
}

FString GetCombatInputConsumptionRouteName(ETwoHeartsCombatInputConsumptionRoute Route)
{
	const UEnum* RouteEnum = StaticEnum<ETwoHeartsCombatInputConsumptionRoute>();
	return RouteEnum ? RouteEnum->GetNameStringByValue(static_cast<int64>(Route)) : TEXT("Unknown");
}
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

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	CombatActionContextComponent = CreateDefaultSubobject<UTwoHeartsCombatActionContextComponent>(TEXT("CombatActionContextComponent"));
	WeaponVisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponVisualComponent"));
	HostileAttackReceiverComponent = CreateDefaultSubobject<UTwoHeartsHostileAttackReceiverComponent>(TEXT("HostileAttackReceiverComponent"));
	WeaponVisualComponent->SetupAttachment(GetMesh());
	WeaponVisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponVisualComponent->SetGenerateOverlapEvents(false);
	WeaponVisualComponent->SetCanEverAffectNavigation(false);
	WeaponVisualComponent->SetCastShadow(true);
	WeaponVisualComponent->SetHiddenInGame(true);
	PrimaryActorTick.bCanEverTick = true;

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

		FTwoHeartsAbilityGrant DodgeGrant;
		DodgeGrant.AbilityClass = UTwoHeartsGA_Dodge::StaticClass();
		DodgeGrant.InputID = ETwoHeartsAbilityInputID::Dodge;
		DefaultCombatAbilities.Add(DodgeGrant);

		FTwoHeartsAbilityGrant GuardGrant;
		GuardGrant.AbilityClass = UTwoHeartsGA_Guard::StaticClass();
		GuardGrant.InputID = ETwoHeartsAbilityInputID::Guard;
		DefaultCombatAbilities.Add(GuardGrant);
	}
}

UAbilitySystemComponent* AtwoheartsCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AtwoheartsCharacter::BeginPlay()
{
	Super::BeginPlay();

	ResetCombatDebugLogFile();
	InitializeAbilitySystem();
	GrantDefaultCombatAbilities();
	RefreshWeaponVisualState();
}

void AtwoheartsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RefreshWeaponVisualState();
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AtwoheartsCharacter::ClearMoveInput);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AtwoheartsCharacter::ClearMoveInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AtwoheartsCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AtwoheartsCharacter::Look);

		// Normal Attack
		if (NormalAttackAction)
		{
			EnhancedInputComponent->BindAction(NormalAttackAction, ETriggerEvent::Started, this, &AtwoheartsCharacter::NormalAttack);
		}

		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AtwoheartsCharacter::Dodge);
		}

		if (GuardAction)
		{
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &AtwoheartsCharacter::Guard);
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, this, &AtwoheartsCharacter::GuardReleased);
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Canceled, this, &AtwoheartsCharacter::GuardReleased);
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

void AtwoheartsCharacter::ClearMoveInput(const FInputActionValue& Value)
{
	CachedMoveInput = FVector2D::ZeroVector;
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

	UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[AbilityInput] NormalAttack input did not activate any ability on %s."), *GetNameSafe(this));
}

void AtwoheartsCharacter::Dodge(const FInputActionValue& Value)
{
	if (HandleAbilityInputPressed(ETwoHeartsAbilityInputID::Dodge))
	{
		return;
	}

	UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[AbilityInput] Dodge input did not activate any ability on %s."), *GetNameSafe(this));
}

void AtwoheartsCharacter::Guard(const FInputActionValue& Value)
{
	if (HandleAbilityInputPressed(ETwoHeartsAbilityInputID::Guard))
	{
		return;
	}

	UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[AbilityInput] Guard input did not activate any ability on %s."), *GetNameSafe(this));
}

void AtwoheartsCharacter::GuardReleased(const FInputActionValue& Value)
{
	HandleAbilityInputReleased(ETwoHeartsAbilityInputID::Guard);
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

	const FString InputName = GetCombatInputName(InputID);
	const ETwoHeartsCombatActionType IncomingActionType = GetCombatActionTypeForInputID(InputID);

	FTwoHeartsCombatInputEvaluation InputEvaluation;
	if (CombatActionContextComponent)
	{
		InputEvaluation = CombatActionContextComponent->EvaluateInputForAction(IncomingActionType);
	}
	else
	{
		InputEvaluation.Result = ETwoHeartsCombatInputEvaluationResult::ExecuteNow;
		InputEvaluation.IncomingActionType = IncomingActionType;
		InputEvaluation.ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::ActivateMatchingAbility;
		InputEvaluation.Reason = TEXT("No combat action context component was available; falling back to immediate activation.");
	}

	if (InputEvaluation.Result == ETwoHeartsCombatInputEvaluationResult::Reject)
	{
		const FString RejectDetail = FString::Printf(TEXT("%s Input=%s."), *InputEvaluation.Reason, *InputName);
		RecordAbilityInputFailure(InputID, TEXT("InputRejected"), RejectDetail);
		RecordCombatInputDebugEvent(
			InputName,
			GetCombatInputEvaluationResultName(InputEvaluation.Result),
			GetCombatInputConsumptionRouteName(InputEvaluation.ConsumptionRoute),
			RejectDetail);
		return false;
	}

	if (InputEvaluation.Result == ETwoHeartsCombatInputEvaluationResult::BufferInput)
	{
		return HandleBufferedCombatInput(InputID, InputName, InputEvaluation);
	}

	if (InputEvaluation.ConsumptionRoute != ETwoHeartsCombatInputConsumptionRoute::ActivateMatchingAbility)
	{
		const FString UnexpectedRouteDetail = FString::Printf(
			TEXT("%s Input=%s evaluated as ExecuteNow without an immediate activation route."),
			*InputEvaluation.Reason,
			*InputName);
		RecordAbilityInputFailure(InputID, TEXT("InputRouteInvalid"), UnexpectedRouteDetail);
		RecordCombatInputDebugEvent(
			InputName,
			GetCombatInputEvaluationResultName(InputEvaluation.Result),
			GetCombatInputConsumptionRouteName(InputEvaluation.ConsumptionRoute),
			UnexpectedRouteDetail);
		return false;
	}

	return TryExecuteCombatInputNow(InputID, InputName, InputEvaluation);
}

void AtwoheartsCharacter::HandleAbilityInputReleased(ETwoHeartsAbilityInputID InputID)
{
	if (InputID != ETwoHeartsAbilityInputID::Guard)
	{
		return;
	}

	const bool bHoldReserved = GuardConfig.InputMode == ETwoHeartsGuardInputMode::HoldReserved;
	const FString Detail = bHoldReserved
		? TEXT("Guard release was received while HoldReserved mode is configured for future extension.")
		: TEXT("Guard release was received, but current basic guard uses tap-only timing.");
	PushGuardDebugEvent(TEXT("GuardInputReleased"), Detail);
	RecordCombatInputDebugEvent(TEXT("GuardRelease"), TEXT("Observed"), TEXT("ReleaseOnly"), Detail);
}

void AtwoheartsCharacter::DoMove(float Right, float Forward)
{
	CachedMoveInput = FVector2D(Right, Forward);

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

FVector AtwoheartsCharacter::GetDesiredDodgeDirectionWorld() const
{
	const FVector ActorForward = GetActorForwardVector().GetSafeNormal2D();
	const FVector ActorRight = GetActorRightVector().GetSafeNormal2D();
	const FVector RelativeDodgeDirection = (ActorForward * CachedMoveInput.Y) + (ActorRight * CachedMoveInput.X);
	if (!RelativeDodgeDirection.IsNearlyZero())
	{
		return RelativeDodgeDirection.GetSafeNormal2D();
	}

	return ActorForward.IsNearlyZero() ? FVector::ForwardVector : ActorForward;
}

FString AtwoheartsCharacter::GetDesiredDodgeDirectionName() const
{
	const FVector Direction = GetDesiredDodgeDirectionWorld();
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = GetActorRightVector().GetSafeNormal2D();

	const float ForwardDot = FVector::DotProduct(Direction, Forward);
	const float RightDot = FVector::DotProduct(Direction, Right);
	const float AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

	if (AngleDegrees >= -22.5f && AngleDegrees < 22.5f)
	{
		return TEXT("Forward");
	}

	if (AngleDegrees >= 22.5f && AngleDegrees < 67.5f)
	{
		return TEXT("ForwardRight");
	}

	if (AngleDegrees >= 67.5f && AngleDegrees < 112.5f)
	{
		return TEXT("Right");
	}

	if (AngleDegrees >= 112.5f && AngleDegrees < 157.5f)
	{
		return TEXT("BackwardRight");
	}

	if (AngleDegrees >= 157.5f || AngleDegrees < -157.5f)
	{
		return TEXT("Backward");
	}

	if (AngleDegrees >= -157.5f && AngleDegrees < -112.5f)
	{
		return TEXT("BackwardLeft");
	}

	if (AngleDegrees >= -112.5f && AngleDegrees < -67.5f)
	{
		return TEXT("Left");
	}

	return TEXT("ForwardLeft");
}

void AtwoheartsCharacter::RefreshWeaponVisualState()
{
	if (!WeaponVisualComponent)
	{
		return;
	}

	if (!WeaponVisualConfig.WeaponMesh)
	{
		WeaponVisualComponent->SetStaticMesh(nullptr);
		ApplyWeaponVisualState(ETwoHeartsWeaponVisualState::Hidden);
		return;
	}

	if (WeaponVisualComponent->GetStaticMesh() != WeaponVisualConfig.WeaponMesh)
	{
		WeaponVisualComponent->SetStaticMesh(WeaponVisualConfig.WeaponMesh);
	}

	const ETwoHeartsWeaponVisualState DesiredState = ShouldShowWeaponAsEquipped()
		? ETwoHeartsWeaponVisualState::Equipped
		: ETwoHeartsWeaponVisualState::Unequipped;
	ApplyWeaponVisualState(DesiredState);
}

bool AtwoheartsCharacter::ShouldShowWeaponAsEquipped() const
{
	if (CombatActionContextComponent)
	{
		const FTwoHeartsCombatActionContextSnapshot& CombatContext = CombatActionContextComponent->GetCurrentContext();
		if (CombatContext.bIsActionActive && CombatContext.ActionType == ETwoHeartsCombatActionType::NormalAttack)
		{
			return true;
		}
	}

	return !IsCharacterInMovingWeaponState();
}

bool AtwoheartsCharacter::IsCharacterInMovingWeaponState() const
{
	const float HorizontalSpeed = GetVelocity().Size2D();
	const bool bHasMoveIntent = !CachedMoveInput.IsNearlyZero();
	return bHasMoveIntent || HorizontalSpeed > WeaponVisualConfig.MovementSpeedThreshold;
}

void AtwoheartsCharacter::ApplyWeaponVisualState(ETwoHeartsWeaponVisualState NewState, bool bForceRefresh)
{
	if (!WeaponVisualComponent)
	{
		return;
	}

	if (!bForceRefresh && CurrentWeaponVisualState == NewState)
	{
		return;
	}

	CurrentWeaponVisualState = NewState;

	if (NewState == ETwoHeartsWeaponVisualState::Hidden)
	{
		WeaponVisualComponent->SetHiddenInGame(true);
		return;
	}

	if (NewState == ETwoHeartsWeaponVisualState::Equipped)
	{
		if (WeaponVisualConfig.EquippedSocketName.IsNone())
		{
			WeaponVisualComponent->SetHiddenInGame(true);
			return;
		}

		AttachWeaponVisualToSocket(WeaponVisualConfig.EquippedSocketName, WeaponVisualConfig.EquippedRelativeTransform);
		WeaponVisualComponent->SetHiddenInGame(false);
		return;
	}

	if (WeaponVisualConfig.UnequippedSocketName.IsNone())
	{
		WeaponVisualComponent->SetHiddenInGame(WeaponVisualConfig.bHideWhenUnequippedSocketMissing);
		if (!WeaponVisualConfig.bHideWhenUnequippedSocketMissing)
		{
			AttachWeaponVisualToSocket(WeaponVisualConfig.EquippedSocketName, WeaponVisualConfig.EquippedRelativeTransform);
		}
		return;
	}

	AttachWeaponVisualToSocket(WeaponVisualConfig.UnequippedSocketName, WeaponVisualConfig.UnequippedRelativeTransform);
	WeaponVisualComponent->SetHiddenInGame(false);
}

void AtwoheartsCharacter::AttachWeaponVisualToSocket(FName SocketName, const FTransform& RelativeTransform)
{
	if (!WeaponVisualComponent || !GetMesh() || SocketName.IsNone())
	{
		return;
	}

	WeaponVisualComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	WeaponVisualComponent->SetRelativeLocation(RelativeTransform.GetLocation());
	WeaponVisualComponent->SetRelativeRotation(RelativeTransform.GetRotation().Rotator());
	WeaponVisualComponent->SetRelativeScale3D(RelativeTransform.GetScale3D());
}

UAnimMontage* AtwoheartsCharacter::GetDodgeMontageForDirection(const FString& DirectionName) const
{
	if (DirectionName == TEXT("Forward"))
	{
		return DodgeConfig.DodgeMontageForward ? DodgeConfig.DodgeMontageForward : DodgeConfig.DodgeMontageFallback;
	}

	if (DirectionName == TEXT("ForwardRight"))
	{
		return DodgeConfig.DodgeMontageForwardRight ? DodgeConfig.DodgeMontageForwardRight : DodgeConfig.DodgeMontageFallback;
	}

	if (DirectionName == TEXT("Right"))
	{
		return DodgeConfig.DodgeMontageRight ? DodgeConfig.DodgeMontageRight : DodgeConfig.DodgeMontageFallback;
	}

	if (DirectionName == TEXT("BackwardRight"))
	{
		return DodgeConfig.DodgeMontageBackwardRight ? DodgeConfig.DodgeMontageBackwardRight : DodgeConfig.DodgeMontageFallback;
	}

	if (DirectionName == TEXT("Backward"))
	{
		return DodgeConfig.DodgeMontageBackward ? DodgeConfig.DodgeMontageBackward : DodgeConfig.DodgeMontageFallback;
	}

	if (DirectionName == TEXT("BackwardLeft"))
	{
		return DodgeConfig.DodgeMontageBackwardLeft ? DodgeConfig.DodgeMontageBackwardLeft : DodgeConfig.DodgeMontageFallback;
	}

	if (DirectionName == TEXT("Left"))
	{
		return DodgeConfig.DodgeMontageLeft ? DodgeConfig.DodgeMontageLeft : DodgeConfig.DodgeMontageFallback;
	}

	if (DirectionName == TEXT("ForwardLeft"))
	{
		return DodgeConfig.DodgeMontageForwardLeft ? DodgeConfig.DodgeMontageForwardLeft : DodgeConfig.DodgeMontageFallback;
	}

	return DodgeConfig.DodgeMontageFallback;
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
	SetNormalAttackDebugRuntimeState(false, 0, false, TEXT("None"), ETwoHeartsCombatPhase::None, false, false);
	RecordNormalAttackDebugEvent(TEXT("DebugEventsCleared"), TEXT("Cleared normal attack debug event history."));
}

void AtwoheartsCharacter::SetNormalAttackDebugRuntimeState(bool bIsActive, int32 Segment, bool bHasQueuedNextSegment, const FString& SectionName, ETwoHeartsCombatPhase CombatPhase, bool bInterruptibleByDodge, bool bLogicEnded)
{
	bIsNormalAttackAbilityActive = bIsActive;
	CurrentNormalAttackAbilitySegment = Segment;
	bHasQueuedNextNormalAttackAbilitySegment = bHasQueuedNextSegment;
	CurrentNormalAttackAbilitySectionName = SectionName;
	CurrentNormalAttackCombatPhase = CombatPhase;
	bIsNormalAttackInterruptibleByDodge = bInterruptibleByDodge;
	bIsNormalAttackLogicEnded = bLogicEnded;
}

void AtwoheartsCharacter::SetLastNormalAttackDebugFailureReason(const FString& FailureReason)
{
	LastNormalAttackDebugFailureReason = FailureReason;
}

void AtwoheartsCharacter::SetDodgeDebugRuntimeState(bool bIsActive, bool bIsInvulnerable, bool bIsCooldownReady, const FString& DirectionName)
{
	bIsDodgeAbilityActive = bIsActive;
	bIsDodgeInvulnerable = bIsInvulnerable;
	bIsDodgeCooldownReady = bIsCooldownReady;
	CurrentDodgeDirectionName = DirectionName;
}

void AtwoheartsCharacter::SetGuardDebugRuntimeState(bool bIsActive, bool bIsWindowActive, const FString& PhaseName, bool bHoldReserved)
{
	bIsGuardAbilityActive = bIsActive;
	bIsGuardWindowActive = bIsWindowActive;
	CurrentGuardPhaseName = PhaseName;
	bGuardHoldInputReserved = bHoldReserved;
}

void AtwoheartsCharacter::PushNormalAttackDebugEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly)
{
	RecordNormalAttackDebugEvent(EventName, Detail, bVerboseOnly);
}

void AtwoheartsCharacter::PushNormalAttackDebugFailure(const TCHAR* EventName, const FString& Detail)
{
	RecordNormalAttackFailure(EventName, Detail);
}

void AtwoheartsCharacter::PushDodgeDebugEvent(const TCHAR* EventName, const FString& Detail)
{
	LastDodgeDebugEventName = EventName;
	LastDodgeDebugDetail = Detail;
	LastDodgeEventTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void AtwoheartsCharacter::PushGuardDebugEvent(const TCHAR* EventName, const FString& Detail)
{
	LastGuardDebugEventName = EventName;
	LastGuardDebugDetail = Detail;
	LastGuardEventTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	AppendCombatDebugLogLine(
		FString::Printf(
			TEXT("[Guard] time=%.3f event=%s active=%s window=%s phase=%s hold_reserved=%s detail=\"%s\""),
			LastGuardEventTimeSeconds,
			EventName,
			bIsGuardAbilityActive ? TEXT("true") : TEXT("false"),
			bIsGuardWindowActive ? TEXT("true") : TEXT("false"),
			*CurrentGuardPhaseName,
			bGuardHoldInputReserved ? TEXT("true") : TEXT("false"),
			*Detail));
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
	Event.PhaseName = GetCombatPhaseDebugName(CurrentNormalAttackCombatPhase);
	Event.bInterruptibleByDodge = bIsNormalAttackInterruptibleByDodge;
	Event.bLogicEnded = bIsNormalAttackLogicEnded;
	Event.bVerboseOnly = bVerboseOnly;

	NormalAttackDebugEvents.Add(MoveTemp(Event));
	const int32 ExcessEvents = NormalAttackDebugEvents.Num() - FMath::Max(1, NormalAttackDebugMaxEvents);
	if (ExcessEvents > 0)
	{
		NormalAttackDebugEvents.RemoveAt(0, ExcessEvents);
	}

	const FNormalAttackDebugEvent& LoggedEvent = NormalAttackDebugEvents.Last();

	AppendCombatDebugLogLine(
		FString::Printf(
			TEXT("[NormalAttack] time=%.3f event=%s segment=%d phase=%s attacking=%s queued_next=%s interruptible_by_dodge=%s logic_ended=%s section=%s verbose=%s detail=\"%s\""),
			LoggedEvent.TimestampSeconds,
			*LoggedEvent.EventName,
			LoggedEvent.Segment,
			*LoggedEvent.PhaseName,
			LoggedEvent.bIsAttacking ? TEXT("true") : TEXT("false"),
			LoggedEvent.bHasQueuedNextSegment ? TEXT("true") : TEXT("false"),
			LoggedEvent.bInterruptibleByDodge ? TEXT("true") : TEXT("false"),
			LoggedEvent.bLogicEnded ? TEXT("true") : TEXT("false"),
			*LoggedEvent.SectionName,
			LoggedEvent.bVerboseOnly ? TEXT("true") : TEXT("false"),
			*LoggedEvent.Detail));

	if (!bEnableNormalAttackDebugLogging)
	{
		return;
	}

	if (!ShouldEmitNormalAttackDebugLog(EventName, bVerboseOnly))
	{
		return;
	}

	const FNormalAttackDebugEvent& LatestEvent = NormalAttackDebugEvents.Last();
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[NormalAttack] time=%.3f event=%s actor=%s segment=%d phase=%s attacking=%s queued_next=%s interruptible_by_dodge=%s logic_ended=%s section=%s detail=\"%s\""),
		LatestEvent.TimestampSeconds,
		*LatestEvent.EventName,
		*GetNameSafe(this),
		LatestEvent.Segment,
		*LatestEvent.PhaseName,
		LatestEvent.bIsAttacking ? TEXT("true") : TEXT("false"),
		LatestEvent.bHasQueuedNextSegment ? TEXT("true") : TEXT("false"),
		LatestEvent.bInterruptibleByDodge ? TEXT("true") : TEXT("false"),
		LatestEvent.bLogicEnded ? TEXT("true") : TEXT("false"),
		*LatestEvent.SectionName,
		*LatestEvent.Detail);
}

bool AtwoheartsCharacter::ShouldEmitNormalAttackDebugLog(const TCHAR* EventName, bool bVerboseOnly) const
{
	if (bVerboseOnly)
	{
		return bEnableNormalAttackVerboseLogging;
	}

	const FString EventNameString = EventName;
	return
		EventNameString == TEXT("PlaySegment") ||
		EventNameString == TEXT("MontageNotifyBegin") ||
		EventNameString == TEXT("MontageNotify") ||
		EventNameString == TEXT("AdvanceWindowOpened") ||
		EventNameString == TEXT("AdvanceSegmentReady") ||
		EventNameString == TEXT("AdvanceSegmentAttempt") ||
		EventNameString == TEXT("EnterPhase") ||
		EventNameString == TEXT("LogicEnded") ||
		EventNameString == TEXT("InterruptCheck") ||
		EventNameString == TEXT("InterruptedByAction") ||
		EventNameString == TEXT("InterruptedByDodge") ||
		EventNameString == TEXT("SegmentFinished") ||
		EventNameString == TEXT("ActivateFailed") ||
		EventNameString == TEXT("SegmentInterrupted") ||
		EventNameString.Contains(TEXT("Failure"));
}

bool AtwoheartsCharacter::ShouldDisplayNormalAttackDebugEvent(const FNormalAttackDebugEvent& Event) const
{
	if (Event.bVerboseOnly && !bEnableNormalAttackVerboseLogging)
	{
		return false;
	}

	return
		Event.EventName == TEXT("PlaySegment") ||
		Event.EventName == TEXT("MontageNotifyBegin") ||
		Event.EventName == TEXT("MontageNotify") ||
		Event.EventName == TEXT("AdvanceWindowOpened") ||
		Event.EventName == TEXT("AdvanceSegmentReady") ||
		Event.EventName == TEXT("AdvanceSegmentAttempt") ||
		Event.EventName == TEXT("EnterPhase") ||
		Event.EventName == TEXT("LogicEnded") ||
		Event.EventName == TEXT("InterruptCheck") ||
		Event.EventName == TEXT("InterruptedByAction") ||
		Event.EventName == TEXT("InterruptedByDodge") ||
		Event.EventName == TEXT("SegmentFinished") ||
		Event.EventName == TEXT("ActivateFailed") ||
		Event.EventName == TEXT("SegmentInterrupted") ||
		Event.EventName.Contains(TEXT("Failure"));
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

void AtwoheartsCharacter::RecordAbilityInputDebugEvent(ETwoHeartsAbilityInputID InputID, const TCHAR* EventName, const FString& Detail, bool bVerboseOnly)
{
	if (InputID == ETwoHeartsAbilityInputID::Dodge)
	{
		PushDodgeDebugEvent(EventName, Detail);
		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[DodgeInput] actor=%s event=%s detail=\"%s\""),
			*GetNameSafe(this),
			EventName,
			*Detail);
		return;
	}

	if (InputID == ETwoHeartsAbilityInputID::Guard)
	{
		PushGuardDebugEvent(EventName, Detail);
		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[GuardInput] actor=%s event=%s detail=\"%s\""),
			*GetNameSafe(this),
			EventName,
			*Detail);
		return;
	}

	RecordNormalAttackDebugEvent(EventName, Detail, bVerboseOnly);
}

void AtwoheartsCharacter::RecordAbilityInputFailure(ETwoHeartsAbilityInputID InputID, const TCHAR* EventName, const FString& Detail)
{
	if (InputID == ETwoHeartsAbilityInputID::Dodge)
	{
		PushDodgeDebugEvent(EventName, Detail);
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[DodgeInputFailure] actor=%s detail=\"%s\""),
			*GetNameSafe(this),
			*Detail);
		return;
	}

	if (InputID == ETwoHeartsAbilityInputID::Guard)
	{
		PushGuardDebugEvent(EventName, Detail);
		UE_LOG(
			LogtwoheartsCombatTest,
			Warning,
			TEXT("[GuardInputFailure] actor=%s detail=\"%s\""),
			*GetNameSafe(this),
			*Detail);
		return;
	}

	RecordNormalAttackFailure(EventName, Detail);
}

bool AtwoheartsCharacter::HandleBufferedCombatInput(ETwoHeartsAbilityInputID InputID, const FString& InputName, const FTwoHeartsCombatInputEvaluation& InputEvaluation)
{
	const int32 NumericInputID = static_cast<int32>(InputID);
	bool bForwardedToActiveAbility = false;
	FGameplayTag ForwardTargetAbilityTag;
	FString ForwardTargetInstanceName = TEXT("None");

	if (InputEvaluation.ConsumptionRoute == ETwoHeartsCombatInputConsumptionRoute::ForwardToActiveAbility
		&& CombatActionContextComponent)
	{
		const FTwoHeartsCombatActionContextSnapshot& CurrentContext = CombatActionContextComponent->GetCurrentContext();
		ForwardTargetAbilityTag = CurrentContext.AbilityTag;
		ForwardTargetInstanceName = CurrentContext.ActionInstanceName;
	}

	if (InputEvaluation.ConsumptionRoute == ETwoHeartsCombatInputConsumptionRoute::ForwardToActiveAbility)
	{
		for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (AbilitySpec.InputID != NumericInputID || !AbilitySpec.IsActive())
			{
				continue;
			}

			if (ForwardTargetAbilityTag.IsValid()
				&& (!AbilitySpec.Ability || !AbilitySpec.Ability->GetAssetTags().HasTagExact(ForwardTargetAbilityTag)))
			{
				RecordAbilityInputDebugEvent(
					InputID,
					TEXT("SkipForwardToNonContextAbility"),
					FString::Printf(
						TEXT("Skipped forwarding buffered input to active ability %s because current action context targets %s (%s)."),
						*GetNameSafe(AbilitySpec.Ability),
						*ForwardTargetInstanceName,
						*ForwardTargetAbilityTag.ToString()),
					true);
				continue;
			}

			bForwardedToActiveAbility = true;
			AbilitySystemComponent->AbilitySpecInputPressed(AbilitySpec);
			RecordAbilityInputDebugEvent(
				InputID,
				TEXT("ForwardInputToActiveAbility"),
				FString::Printf(
					TEXT("Forwarded buffered input to active ability %s. ContextTarget=%s (%s)."),
					*GetNameSafe(AbilitySpec.Ability),
					*ForwardTargetInstanceName,
					ForwardTargetAbilityTag.IsValid() ? *ForwardTargetAbilityTag.ToString() : TEXT("NoAbilityTag")),
				true);
			break;
		}
	}

	if (InputEvaluation.ConsumptionRoute == ETwoHeartsCombatInputConsumptionRoute::ForwardToActiveAbility
		&& !bForwardedToActiveAbility)
	{
		const FString ForwardFailureDetail = FString::Printf(
			TEXT("%s Input=%s expected forwarding to the active ability, but no matching active ability instance was found."),
			*InputEvaluation.Reason,
			*InputName);
		RecordAbilityInputFailure(InputID, TEXT("ForwardToActiveAbilityFailed"), ForwardFailureDetail);
		RecordCombatInputDebugEvent(
			InputName,
			GetCombatInputEvaluationResultName(InputEvaluation.Result),
			GetCombatInputConsumptionRouteName(InputEvaluation.ConsumptionRoute),
			ForwardFailureDetail);
		return false;
	}

	if (InputEvaluation.ConsumptionRoute == ETwoHeartsCombatInputConsumptionRoute::ReserveForFutureBufferConsumer
		&& CombatActionContextComponent)
	{
		CombatActionContextComponent->BufferInput(
			InputEvaluation.IncomingActionType,
			InputEvaluation.ConsumptionRoute,
			InputEvaluation.Reason);
	}

	const FString BufferDetail = bForwardedToActiveAbility
		? FString::Printf(TEXT("%s Input=%s was forwarded to the active ability buffer path."), *InputEvaluation.Reason, *InputName)
		: FString::Printf(TEXT("%s Input=%s is now stored in the formal buffered follow-up slot for a later consumer."), *InputEvaluation.Reason, *InputName);
	RecordAbilityInputDebugEvent(InputID, TEXT("InputBuffered"), BufferDetail);
	RecordCombatInputDebugEvent(
		InputName,
		GetCombatInputEvaluationResultName(InputEvaluation.Result),
		GetCombatInputConsumptionRouteName(InputEvaluation.ConsumptionRoute),
		BufferDetail);
	return true;
}

bool AtwoheartsCharacter::TryExecuteCombatInputNow(ETwoHeartsAbilityInputID InputID, const FString& InputName, const FTwoHeartsCombatInputEvaluation& InputEvaluation)
{
	const int32 NumericInputID = static_cast<int32>(InputID);
	const bool bIsNormalAttackInput = InputID == ETwoHeartsAbilityInputID::NormalAttack;
	const bool bIsDodgeInput = InputID == ETwoHeartsAbilityInputID::Dodge;
	const bool bIsGuardInput = InputID == ETwoHeartsAbilityInputID::Guard;
	const FGameplayTag StarterAbilityTag = TAG_TwoHearts_Ability_NormalAttack_Segment1;
	bool bAttemptedActivation = false;
	bool bActivatedAbility = false;
	FString ActivatedAbilityName;
	TArray<FString> MatchingAbilityNames;

	auto BuildFailureReason = [](const FGameplayTagContainer& FailureTags) -> FString
	{
		return FailureTags.IsEmpty()
			? TEXT("CanActivateAbility returned false with no failure tags.")
			: FString::Printf(TEXT("Blocked by tags: %s"), *FailureTags.ToStringSimple());
	};

	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID != NumericInputID)
		{
			continue;
		}

		if (!AbilitySpec.Ability)
		{
			continue;
		}

		MatchingAbilityNames.Add(GetNameSafe(AbilitySpec.Ability));

		if (bIsNormalAttackInput && !AbilitySpec.Ability->GetAssetTags().HasTagExact(StarterAbilityTag))
		{
			continue;
		}

		bAttemptedActivation = true;

		FGameplayTagContainer FailureTags;
		const bool bCanActivate = AbilitySpec.Ability->CanActivateAbility(
			AbilitySpec.Handle,
			AbilitySystemComponent->AbilityActorInfo.Get(),
			nullptr,
			nullptr,
			&FailureTags);

		if (!bCanActivate)
		{
			RecordAbilityInputFailure(
				InputID,
				TEXT("ActivateFailed"),
				FString::Printf(TEXT("Failed to activate %s. %s"), *GetNameSafe(AbilitySpec.Ability), *BuildFailureReason(FailureTags)));
			continue;
		}

		AbilitySystemComponent->AbilitySpecInputPressed(AbilitySpec);
		if (AbilitySystemComponent->TryActivateAbility(AbilitySpec.Handle))
		{
			bActivatedAbility = true;
			ActivatedAbilityName = GetNameSafe(AbilitySpec.Ability);
			RecordAbilityInputDebugEvent(
				InputID,
				TEXT("ActivateAbility"),
				FString::Printf(TEXT("Activated ability %s for input %d after ExecuteNow evaluation."), *ActivatedAbilityName, NumericInputID));
			break;
		}

		RecordAbilityInputFailure(
			InputID,
			TEXT("ActivateFailed"),
			FString::Printf(TEXT("TryActivateAbility failed for %s after CanActivateAbility succeeded."), *GetNameSafe(AbilitySpec.Ability)));
	}

	if (!MatchingAbilityNames.IsEmpty())
	{
		RecordAbilityInputDebugEvent(
			InputID,
			TEXT("InputScan"),
			FString::Printf(TEXT("Input %d scanned abilities: %s."), NumericInputID, *FString::Join(MatchingAbilityNames, TEXT(", "))),
			true);
	}

	if (bAttemptedActivation)
	{
		const FString ExecuteDetail = bActivatedAbility
			? FString::Printf(TEXT("%s Input=%s executed immediately via %s."), *InputEvaluation.Reason, *InputName, *ActivatedAbilityName)
			: FString::Printf(TEXT("%s Input=%s was evaluated as ExecuteNow, but ability activation still failed."), *InputEvaluation.Reason, *InputName);
		RecordCombatInputDebugEvent(
			InputName,
			GetCombatInputEvaluationResultName(InputEvaluation.Result),
			GetCombatInputConsumptionRouteName(InputEvaluation.ConsumptionRoute),
			ExecuteDetail);
		return bActivatedAbility;
	}

	if (bIsNormalAttackInput)
	{
		RecordAbilityInputFailure(
			InputID,
			TEXT("ActivateFailed"),
			TEXT("NormalAttack input found no valid starter ability. First input must enter Segment1."));
	}
	else if (bIsDodgeInput)
	{
		RecordAbilityInputFailure(
			InputID,
			TEXT("ActivateFailed"),
			MatchingAbilityNames.IsEmpty()
				? TEXT("Dodge input found no bound dodge ability.")
				: TEXT("Dodge input matched abilities but none were eligible for activation."));
	}
	else if (bIsGuardInput)
	{
		RecordAbilityInputFailure(
			InputID,
			TEXT("ActivateFailed"),
			MatchingAbilityNames.IsEmpty()
				? TEXT("Guard input found no bound guard ability.")
				: TEXT("Guard input matched abilities but none were eligible for activation."));
	}
	else if (!MatchingAbilityNames.IsEmpty())
	{
		RecordAbilityInputDebugEvent(
			InputID,
			TEXT("ActivateFailed"),
			FString::Printf(TEXT("Input %d matched abilities but none were eligible for activation."), NumericInputID));
	}

	RecordCombatInputDebugEvent(
		InputName,
		GetCombatInputEvaluationResultName(InputEvaluation.Result),
		GetCombatInputConsumptionRouteName(InputEvaluation.ConsumptionRoute),
		FString::Printf(TEXT("%s Input=%s found no eligible ability instance to execute immediately."), *InputEvaluation.Reason, *InputName));
	return false;
}

void AtwoheartsCharacter::PushCombatInputDebugEvent(const FString& InputName, const FString& ResultName, const FString& RouteName, const FString& Detail)
{
	RecordCombatInputDebugEvent(InputName, ResultName, RouteName, Detail);
}

bool AtwoheartsCharacter::TryConsumeReservedCombatInput(const FString& ConsumerName)
{
	if (!CombatActionContextComponent)
	{
		return false;
	}

	FTwoHeartsBufferedCombatInput BufferedInput;
	if (!CombatActionContextComponent->ConsumeBufferedInput(BufferedInput, ConsumerName))
	{
		return false;
	}

	ETwoHeartsAbilityInputID InputID = ETwoHeartsAbilityInputID::None;
	if (!TryGetCombatInputIDForActionType(BufferedInput.IncomingActionType, InputID))
	{
		const bool bRestoredUnsupportedInput = CombatActionContextComponent->RestoreBufferedInput(
			BufferedInput,
			FString::Printf(TEXT("%s.UnsupportedActionType"), *ConsumerName));
		RecordCombatInputDebugEvent(
			TEXT("Unknown"),
			TEXT("BufferedConsumeFailed"),
			GetCombatInputConsumptionRouteName(BufferedInput.ConsumptionRoute),
			FString::Printf(
				TEXT("Buffered input consumer %s found unsupported action type %s. Restored=%s."),
				*ConsumerName,
				*StaticEnum<ETwoHeartsCombatActionType>()->GetNameStringByValue(static_cast<int64>(BufferedInput.IncomingActionType)),
				bRestoredUnsupportedInput ? TEXT("true") : TEXT("false")));
		return false;
	}

	const FString InputName = GetCombatInputName(InputID);
	FTwoHeartsCombatInputEvaluation ConsumedEvaluation;
	ConsumedEvaluation.Result = ETwoHeartsCombatInputEvaluationResult::BufferInput;
	ConsumedEvaluation.IncomingActionType = BufferedInput.IncomingActionType;
	ConsumedEvaluation.ConsumptionRoute = ETwoHeartsCombatInputConsumptionRoute::ActivateMatchingAbility;
	ConsumedEvaluation.Reason = FString::Printf(TEXT("Buffered input was consumed by %s. OriginalReason=%s"), *ConsumerName, *BufferedInput.Reason);

	const bool bConsumed = TryExecuteCombatInputNow(InputID, InputName, ConsumedEvaluation);
	const bool bRestoredBufferedInput = !bConsumed
		&& CombatActionContextComponent->RestoreBufferedInput(
			BufferedInput,
			FString::Printf(TEXT("%s.ActivationFailed"), *ConsumerName));
	RecordCombatInputDebugEvent(
		InputName,
		bConsumed ? TEXT("BufferedConsumed") : TEXT("BufferedConsumeFailed"),
		GetCombatInputConsumptionRouteName(ConsumedEvaluation.ConsumptionRoute),
		bConsumed
			? FString::Printf(TEXT("Buffered input was consumed successfully by %s."), *ConsumerName)
			: FString::Printf(
				TEXT("Buffered input consumption by %s failed to activate a follow-up action. Restored=%s."),
				*ConsumerName,
				bRestoredBufferedInput ? TEXT("true") : TEXT("false")));
	return bConsumed;
}

void AtwoheartsCharacter::RecordCombatInputDebugEvent(const FString& InputName, const FString& ResultName, const FString& RouteName, const FString& Detail)
{
	FTwoHeartsCombatInputDebugEvent Event;
	Event.TimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Event.InputName = InputName;
	Event.ResultName = ResultName;
	Event.RouteName = RouteName;
	Event.Detail = Detail;

	CombatInputDebugEvents.Add(MoveTemp(Event));
	const int32 ExcessEvents = CombatInputDebugEvents.Num() - FMath::Max(1, CombatInputDebugMaxEvents);
	if (ExcessEvents > 0)
	{
		CombatInputDebugEvents.RemoveAt(0, ExcessEvents);
	}

	const FTwoHeartsCombatInputDebugEvent& LoggedEvent = CombatInputDebugEvents.Last();

	AppendCombatDebugLogLine(
		FString::Printf(
			TEXT("[CombatInputEval] time=%.3f input=%s result=%s route=%s detail=\"%s\""),
			LoggedEvent.TimestampSeconds,
			*LoggedEvent.InputName,
			*LoggedEvent.ResultName,
			*LoggedEvent.RouteName,
			*LoggedEvent.Detail));

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[CombatInputEval] time=%.3f actor=%s input=%s result=%s route=%s detail=\"%s\""),
		CombatInputDebugEvents.Last().TimestampSeconds,
		*GetNameSafe(this),
		*CombatInputDebugEvents.Last().InputName,
		*CombatInputDebugEvents.Last().ResultName,
		*CombatInputDebugEvents.Last().RouteName,
		*CombatInputDebugEvents.Last().Detail);
}

FString AtwoheartsCharacter::GetCombatDebugLogFilePath() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CombatDebug"), TEXT("normal-attack-debug.log"));
}

void AtwoheartsCharacter::ResetCombatDebugLogFile()
{
	const FString LogFilePath = GetCombatDebugLogFilePath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(LogFilePath), true);

	const FString Header = FString::Printf(
		TEXT("=== Combat Debug Session Start actor=%s utc=%s ===\n"),
		*GetNameSafe(this),
		*FDateTime::UtcNow().ToString());
	FFileHelper::SaveStringToFile(Header, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect);
}

void AtwoheartsCharacter::AppendCombatDebugLogLine(const FString& Line) const
{
	const FString LogFilePath = GetCombatDebugLogFilePath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(LogFilePath), true);
	FFileHelper::SaveStringToFile(
		Line + LINE_TERMINATOR,
		*LogFilePath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(),
		FILEWRITE_Append);
}

void AtwoheartsCharacter::DrawNormalAttackDebugOverlay() const
{
	// The debug HUD already renders a cleaner combat panel. Keep the legacy
	// on-screen debug overlay disabled to avoid duplicate noise during tuning.
}

FString AtwoheartsCharacter::GetCombatPhaseDebugName(ETwoHeartsCombatPhase CombatPhase) const
{
	const UEnum* Enum = StaticEnum<ETwoHeartsCombatPhase>();
	return Enum ? Enum->GetNameStringByValue(static_cast<int64>(CombatPhase)) : TEXT("Unknown");
}
