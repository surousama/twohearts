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
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsAbilityGrant.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"
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

	default:
		return FString::Printf(TEXT("Input_%d"), static_cast<int32>(InputID));
	}
}

FString GetCombatInputEvaluationResultName(ETwoHeartsCombatInputEvaluationResult Result)
{
	const UEnum* ResultEnum = StaticEnum<ETwoHeartsCombatInputEvaluationResult>();
	return ResultEnum ? ResultEnum->GetNameStringByValue(static_cast<int64>(Result)) : TEXT("Unknown");
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

void AtwoheartsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

	const int32 NumericInputID = static_cast<int32>(InputID);
	const bool bIsNormalAttackInput = InputID == ETwoHeartsAbilityInputID::NormalAttack;
	const bool bIsDodgeInput = InputID == ETwoHeartsAbilityInputID::Dodge;
	const FString InputName = GetCombatInputName(InputID);
	const ETwoHeartsCombatActionType IncomingActionType = GetCombatActionTypeForInputID(InputID);
	bool bForwardedToActiveAbility = false;
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

	auto RecordInputDebugEvent = [this, bIsDodgeInput](const TCHAR* EventName, const FString& Detail, bool bVerboseOnly = false)
	{
		if (bIsDodgeInput)
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

		RecordNormalAttackDebugEvent(EventName, Detail, bVerboseOnly);
	};

	auto RecordInputFailure = [this, bIsDodgeInput, &RecordInputDebugEvent](const TCHAR* EventName, const FString& Detail)
	{
		if (bIsDodgeInput)
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

		RecordNormalAttackFailure(EventName, Detail);
	};

	FTwoHeartsCombatInputEvaluation InputEvaluation;
	if (CombatActionContextComponent)
	{
		InputEvaluation = CombatActionContextComponent->EvaluateInputForAction(IncomingActionType);
	}
	else
	{
		InputEvaluation.Result = ETwoHeartsCombatInputEvaluationResult::ExecuteNow;
		InputEvaluation.IncomingActionType = IncomingActionType;
		InputEvaluation.Reason = TEXT("No combat action context component was available; falling back to immediate activation.");
	}

	if (InputEvaluation.Result == ETwoHeartsCombatInputEvaluationResult::Reject)
	{
		const FString RejectDetail = FString::Printf(TEXT("%s Input=%s."), *InputEvaluation.Reason, *InputName);
		RecordInputFailure(TEXT("InputRejected"), RejectDetail);
		RecordCombatInputDebugEvent(InputName, GetCombatInputEvaluationResultName(InputEvaluation.Result), RejectDetail);
		return false;
	}

	if (InputEvaluation.Result == ETwoHeartsCombatInputEvaluationResult::BufferInput)
	{
		if (InputEvaluation.bShouldForwardToActiveAbility)
		{
			for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
			{
				if (AbilitySpec.InputID != NumericInputID || !AbilitySpec.IsActive())
				{
					continue;
				}

				bForwardedToActiveAbility = true;
				AbilitySystemComponent->AbilitySpecInputPressed(AbilitySpec);
				RecordInputDebugEvent(
					TEXT("ForwardInputToActiveAbility"),
					FString::Printf(TEXT("Forwarded buffered input to active ability %s."), *GetNameSafe(AbilitySpec.Ability)),
					true);
			}
		}

		const FString BufferDetail = bForwardedToActiveAbility
			? FString::Printf(TEXT("%s Input=%s was forwarded to the active ability buffer path."), *InputEvaluation.Reason, *InputName)
			: FString::Printf(TEXT("%s Input=%s is accepted by the formal preinput hook and reserved for a later consumer."), *InputEvaluation.Reason, *InputName);
		RecordInputDebugEvent(TEXT("InputBuffered"), BufferDetail);
		RecordCombatInputDebugEvent(InputName, GetCombatInputEvaluationResultName(InputEvaluation.Result), BufferDetail);
		return true;
	}

	const FGameplayTag StarterAbilityTag = TAG_TwoHearts_Ability_NormalAttack_Segment1;

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
			RecordInputFailure(
				TEXT("ActivateFailed"),
				FString::Printf(TEXT("Failed to activate %s. %s"), *GetNameSafe(AbilitySpec.Ability), *BuildFailureReason(FailureTags)));
			continue;
		}

		AbilitySystemComponent->AbilitySpecInputPressed(AbilitySpec);
		if (AbilitySystemComponent->TryActivateAbility(AbilitySpec.Handle))
		{
			bActivatedAbility = true;
			ActivatedAbilityName = GetNameSafe(AbilitySpec.Ability);
			RecordInputDebugEvent(
				TEXT("ActivateAbility"),
				FString::Printf(TEXT("Activated ability %s for input %d after ExecuteNow evaluation."), *ActivatedAbilityName, NumericInputID));
			break;
		}

		RecordInputFailure(
			TEXT("ActivateFailed"),
			FString::Printf(TEXT("TryActivateAbility failed for %s after CanActivateAbility succeeded."), *GetNameSafe(AbilitySpec.Ability)));
	}

	if (!MatchingAbilityNames.IsEmpty())
	{
		RecordInputDebugEvent(
			TEXT("InputScan"),
			FString::Printf(TEXT("Input %d scanned abilities: %s."), NumericInputID, *FString::Join(MatchingAbilityNames, TEXT(", "))),
			true);
	}

	if (bAttemptedActivation)
	{
		const FString ExecuteDetail = bActivatedAbility
			? FString::Printf(TEXT("%s Input=%s executed immediately via %s."), *InputEvaluation.Reason, *InputName, *ActivatedAbilityName)
			: FString::Printf(TEXT("%s Input=%s was evaluated as ExecuteNow, but ability activation still failed."), *InputEvaluation.Reason, *InputName);
		RecordCombatInputDebugEvent(InputName, GetCombatInputEvaluationResultName(InputEvaluation.Result), ExecuteDetail);
		return bActivatedAbility;
	}

	if (bIsNormalAttackInput)
	{
		RecordInputFailure(
			TEXT("ActivateFailed"),
			TEXT("NormalAttack input found no valid starter ability. First input must enter Segment1."));
	}
	else if (bIsDodgeInput)
	{
		RecordInputFailure(
			TEXT("ActivateFailed"),
			MatchingAbilityNames.IsEmpty()
				? TEXT("Dodge input found no bound dodge ability.")
				: TEXT("Dodge input matched abilities but none were eligible for activation."));
	}
	else if (!MatchingAbilityNames.IsEmpty())
	{
		RecordInputDebugEvent(
			TEXT("ActivateFailed"),
			FString::Printf(TEXT("Input %d matched abilities but none were eligible for activation."), NumericInputID));
	}

	RecordCombatInputDebugEvent(
		InputName,
		GetCombatInputEvaluationResultName(InputEvaluation.Result),
		FString::Printf(TEXT("%s Input=%s found no eligible ability instance to execute immediately."), *InputEvaluation.Reason, *InputName));
	return false;
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

void AtwoheartsCharacter::RecordCombatInputDebugEvent(const FString& InputName, const FString& ResultName, const FString& Detail)
{
	FTwoHeartsCombatInputDebugEvent Event;
	Event.TimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Event.InputName = InputName;
	Event.ResultName = ResultName;
	Event.Detail = Detail;

	CombatInputDebugEvents.Add(MoveTemp(Event));
	const int32 ExcessEvents = CombatInputDebugEvents.Num() - FMath::Max(1, CombatInputDebugMaxEvents);
	if (ExcessEvents > 0)
	{
		CombatInputDebugEvents.RemoveAt(0, ExcessEvents);
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[CombatInputEval] time=%.3f actor=%s input=%s result=%s detail=\"%s\""),
		CombatInputDebugEvents.Last().TimestampSeconds,
		*GetNameSafe(this),
		*CombatInputDebugEvents.Last().InputName,
		*CombatInputDebugEvents.Last().ResultName,
		*CombatInputDebugEvents.Last().Detail);
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
