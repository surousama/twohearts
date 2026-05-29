#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackProbeCharacter.h"

#include "Animation/AnimationAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsPlayerAttackReceiverComponent.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "twohearts.h"

namespace
{
	const TCHAR* LexToString(const ETwoHeartsHostileAttackSignalType SignalType)
	{
		switch (SignalType)
		{
		case ETwoHeartsHostileAttackSignalType::AttackStarted:
			return TEXT("AttackStarted");
		case ETwoHeartsHostileAttackSignalType::HitWindowOpened:
			return TEXT("HitWindowOpened");
		case ETwoHeartsHostileAttackSignalType::HitWindowClosed:
			return TEXT("HitWindowClosed");
		case ETwoHeartsHostileAttackSignalType::AttackContact:
			return TEXT("AttackContact");
		case ETwoHeartsHostileAttackSignalType::AttackFinished:
			return TEXT("AttackFinished");
		case ETwoHeartsHostileAttackSignalType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexProbeHitReactionTypeToString(const ETwoHeartsHitReactionType HitReactionType)
	{
		switch (HitReactionType)
		{
		case ETwoHeartsHitReactionType::Light:
			return TEXT("Light");
		case ETwoHeartsHitReactionType::Heavy:
			return TEXT("Heavy");
		case ETwoHeartsHitReactionType::GuardBreak:
			return TEXT("GuardBreak");
		case ETwoHeartsHitReactionType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexProbeAttackTimingPhaseToString(const ETwoHeartsAttackTimingPhase TimingPhase)
	{
		switch (TimingPhase)
		{
		case ETwoHeartsAttackTimingPhase::Startup:
			return TEXT("Startup");
		case ETwoHeartsAttackTimingPhase::HitWindow:
			return TEXT("HitWindow");
		case ETwoHeartsAttackTimingPhase::Recovery:
			return TEXT("Recovery");
		case ETwoHeartsAttackTimingPhase::Finished:
			return TEXT("Finished");
		case ETwoHeartsAttackTimingPhase::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexProbeHostileHitResultTypeToString(const ETwoHeartsHostileHitResultType ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsHostileHitResultType::HitConfirmed:
			return TEXT("HitConfirmed");
		case ETwoHeartsHostileHitResultType::SignalInvalid:
			return TEXT("SignalInvalid");
		case ETwoHeartsHostileHitResultType::IgnoredNoHealth:
			return TEXT("IgnoredNoHealth");
		case ETwoHeartsHostileHitResultType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* LexProbeHostileDamageResultTypeToString(const ETwoHeartsHostileDamageResultType ResultType)
	{
		switch (ResultType)
		{
		case ETwoHeartsHostileDamageResultType::DamageApplied:
			return TEXT("DamageApplied");
		case ETwoHeartsHostileDamageResultType::IgnoredNoHealth:
			return TEXT("IgnoredNoHealth");
		case ETwoHeartsHostileDamageResultType::None:
		default:
			return TEXT("None");
		}
	}

	FString BuildProbeAttackMetadataDebugString(const FTwoHeartsAttackMetadata& AttackMetadata)
	{
		const FString DamageMechanicTags = AttackMetadata.DamageMechanicTags.IsEmpty()
			? TEXT("None")
			: AttackMetadata.DamageMechanicTags.ToStringSimple();
		const FString TimingWindowName = AttackMetadata.TimingWindowName.IsNone()
			? TEXT("None")
			: AttackMetadata.TimingWindowName.ToString();
		return FString::Printf(
			TEXT("reaction=%s damage=%.2f tags=%s guard=%s dist=%.1f height=%.1f angle=%.1f settlement=%s/%s chip=%.2f dodge=%s timing=%s/%s"),
			LexProbeHitReactionTypeToString(AttackMetadata.HitReactionType),
			AttackMetadata.BaseDamage,
			*DamageMechanicTags,
			AttackMetadata.bCanBeGuarded ? TEXT("true") : TEXT("false"),
			AttackMetadata.GuardMaxDistance,
			AttackMetadata.GuardMaxHeightDifference,
			AttackMetadata.GuardFacingHalfAngleDegrees,
			*StaticEnum<ETwoHeartsGuardDisplacementResult>()->GetNameStringByValue(static_cast<int64>(AttackMetadata.GuardSuccessDisplacementResult)),
			*StaticEnum<ETwoHeartsGuardDamageResult>()->GetNameStringByValue(static_cast<int64>(AttackMetadata.GuardSuccessDamageResult)),
			AttackMetadata.GuardPartialDamageMultiplier,
			AttackMetadata.bCanBeDodged ? TEXT("true") : TEXT("false"),
			LexProbeAttackTimingPhaseToString(AttackMetadata.TimingPhase),
			*TimingWindowName);
	}
}

ATwoHeartsHostileAttackProbeCharacter::ATwoHeartsHostileAttackProbeCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	CombatActionContextComponent = CreateDefaultSubobject<UTwoHeartsCombatActionContextComponent>(TEXT("CombatActionContextComponent"));
	TriggerSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphereComponent"));
	TriggerSphereComponent->SetupAttachment(RootComponent);
	TriggerSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphereComponent->SetGenerateOverlapEvents(true);

	HitSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("HitSphereComponent"));
	HitSphereComponent->SetupAttachment(RootComponent);
	HitSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HitSphereComponent->SetGenerateOverlapEvents(true);
	PlayerAttackReceiverComponent = CreateDefaultSubobject<UTwoHeartsPlayerAttackReceiverComponent>(TEXT("PlayerAttackReceiverComponent"));

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMesh(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (MannyMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MannyMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimationAsset> IdleAnimationFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
	if (IdleAnimationFinder.Succeeded())
	{
		IdleAnimation = IdleAnimationFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimationAsset> AttackAnimationFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01"));
	if (AttackAnimationFinder.Succeeded())
	{
		AttackAnimation = AttackAnimationFinder.Object;
	}

	UpdateHitVolumePlacement();
}

void ATwoHeartsHostileAttackProbeCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHitVolumePlacement();
	RestoreIdleAnimation();

	if (TriggerSphereComponent)
	{
		TriggerSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ATwoHeartsHostileAttackProbeCharacter::HandleTriggerSphereBeginOverlap);
		TriggerSphereComponent->OnComponentEndOverlap.AddDynamic(this, &ATwoHeartsHostileAttackProbeCharacter::HandleTriggerSphereEndOverlap);
	}

	if (PlayerAttackReceiverComponent)
	{
		PlayerAttackReceiverComponent->OnPlayerAttackSignalReceived.AddDynamic(this, &ATwoHeartsHostileAttackProbeCharacter::HandlePlayerAttackSignalReceived);
		PlayerAttackReceiverComponent->OnHostileHitResultUpdated.AddDynamic(this, &ATwoHeartsHostileAttackProbeCharacter::HandleHostileHitResultUpdated);
		PlayerAttackReceiverComponent->OnHostileDamageResultUpdated.AddDynamic(this, &ATwoHeartsHostileAttackProbeCharacter::HandleHostileDamageResultUpdated);
		PlayerAttackReceiverComponent->OnZeroHealthReached.AddDynamic(this, &ATwoHeartsHostileAttackProbeCharacter::HandleZeroHealthReached);

		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[HostileAttackProbe] actor=%s event=HealthInitialized current=%.2f max=%.2f defeated=%s"),
			*GetNameSafe(this),
			PlayerAttackReceiverComponent->GetCurrentHealth(),
			PlayerAttackReceiverComponent->GetMaxHealth(),
			PlayerAttackReceiverComponent->IsDefeated() ? TEXT("true") : TEXT("false"));
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackProbe] actor=%s event=BeginPlay trigger_radius=%.1f attack_reach=%.1f attack_radius=%.1f startup=%.2f hit_window=%.2f recovery=%.2f repeat_cooldown=%.2f auto_trigger=%s loop=%s"),
		*GetNameSafe(this),
		TriggerRadius,
		AttackReach,
		AttackRadius,
		StartupSeconds,
		HitWindowSeconds,
		RecoverySeconds,
		RepeatCooldownSeconds,
		bAutoTriggerWhenTargetEntersRange ? TEXT("true") : TEXT("false"),
		bLoopAttackWhileTargetStaysInRange ? TEXT("true") : TEXT("false"));
}

void ATwoHeartsHostileAttackProbeCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateHitVolumePlacement();
}

bool ATwoHeartsHostileAttackProbeCharacter::TriggerProbeAttack()
{
	if (IsDefeated())
	{
		UE_LOG(LogtwoheartsCombatTest, Display, TEXT("[HostileAttackProbe] actor=%s event=TriggerIgnored reason=health_depleted"), *GetNameSafe(this));
		return false;
	}

	if (bAttackActive)
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=TriggerIgnored reason=attack_already_active attack=%s"), *GetNameSafe(this), *CurrentAttackInstanceName);
		return false;
	}

	if (!AttackAnimation)
	{
		UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[HostileAttackProbe] actor=%s event=TriggerFailed reason=missing_attack_animation"), *GetNameSafe(this));
		return false;
	}

	GetWorldTimerManager().ClearTimer(RepeatAttackTimerHandle);
	UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=TriggerAccepted target=%s"), *GetNameSafe(this), *GetNameSafe(CurrentTargetActor));
	StartAttackStartup();
	return true;
}

void ATwoHeartsHostileAttackProbeCharacter::ResetProbeToIdle()
{
	GetWorldTimerManager().ClearTimer(AttackPhaseTimerHandle);
	GetWorldTimerManager().ClearTimer(RepeatAttackTimerHandle);

	bAttackActive = false;
	bHitWindowActive = false;
	CurrentAttackInstanceName = TEXT("None");
	CurrentAttackMetadata = FTwoHeartsAttackMetadata();
	AttackTargetActor = nullptr;
	ContactedTargetsThisAttack.Reset();

	if (CombatActionContextComponent && CombatActionContextComponent->HasActiveAction())
	{
		CombatActionContextComponent->FinishAction(ETwoHeartsCombatActionEndReason::Cancelled, TEXT("ProbeResetToIdle"));
	}

	RestoreIdleAnimation();

	UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=ResetToIdle"), *GetNameSafe(this));
}

void ATwoHeartsHostileAttackProbeCharacter::HandlePlayerAttackSignalReceived(const FTwoHeartsPlayerAttackSignal& Signal)
{
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackProbe] actor=%s event=PlayerAttackSignalReceived attack=%s source=%s base_damage=%.2f hit_window=%s contact=%s"),
		*GetNameSafe(this),
		*Signal.AttackInstanceName,
		*GetNameSafe(Signal.SourceActor),
		Signal.AttackMetadata.BaseDamage,
		Signal.bIsHitWindowActive ? TEXT("true") : TEXT("false"),
		Signal.bHasContact ? TEXT("true") : TEXT("false"));
}

void ATwoHeartsHostileAttackProbeCharacter::HandleHostileHitResultUpdated(const FTwoHeartsHostileHitResult& HitResult)
{
	const float RemainingHealth = PlayerAttackReceiverComponent ? PlayerAttackReceiverComponent->GetCurrentHealth() : 0.0f;
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackProbe] actor=%s event=PlayerHitResolved result=%s attack=%s remaining_health=%.2f detail=\"%s\""),
		*GetNameSafe(this),
		LexProbeHostileHitResultTypeToString(HitResult.ResultType),
		*HitResult.AttackInstanceName,
		RemainingHealth,
		*HitResult.Detail);
}

void ATwoHeartsHostileAttackProbeCharacter::HandleHostileDamageResultUpdated(const FTwoHeartsHostileDamageResult& DamageResult)
{
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackProbe] actor=%s event=PlayerDamageResolved result=%s attack=%s damage=%.2f health=%.2f->%.2f zero=%s"),
		*GetNameSafe(this),
		LexProbeHostileDamageResultTypeToString(DamageResult.ResultType),
		*DamageResult.AttackInstanceName,
		DamageResult.AppliedDamage,
		DamageResult.PreviousHealth,
		DamageResult.CurrentHealth,
		DamageResult.bReachedZeroHealth ? TEXT("true") : TEXT("false"));

	if (bEnableScreenDebugOutput && GEngine)
	{
		const FString Label = DamageResult.ResultType == ETwoHeartsHostileDamageResultType::IgnoredNoHealth
			? FString::Printf(TEXT("[Probe Hurt] ignored at 0 hp by %s"), *DamageResult.AttackInstanceName)
			: FString::Printf(TEXT("[Probe Hurt] %s -%.0f => %.0f"), *DamageResult.AttackInstanceName, DamageResult.AppliedDamage, DamageResult.CurrentHealth);
		const FColor Color = DamageResult.ResultType == ETwoHeartsHostileDamageResultType::IgnoredNoHealth ? FColor::Silver : FColor::Red;
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()) + 17ULL, 1.5f, Color, Label);
	}
}

void ATwoHeartsHostileAttackProbeCharacter::HandleZeroHealthReached(const FTwoHeartsHostileDamageResult& DamageResult)
{
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackProbe] actor=%s event=ZeroHealthShutdown attack=%s remaining_health=%.2f will_ignore_new_hits=true"),
		*GetNameSafe(this),
		*DamageResult.AttackInstanceName,
		DamageResult.CurrentHealth);

	if (bEnableScreenDebugOutput && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()) + 18ULL,
			2.5f,
			FColor::Red,
			FString::Printf(TEXT("[Probe] defeated by %s"), *DamageResult.AttackInstanceName));
	}

	ResetProbeToIdle();
	CurrentTargetActor = nullptr;
	AttackTargetActor = nullptr;

	if (TriggerSphereComponent)
	{
		TriggerSphereComponent->SetGenerateOverlapEvents(false);
		TriggerSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (HitSphereComponent)
	{
		HitSphereComponent->SetGenerateOverlapEvents(false);
		HitSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ATwoHeartsHostileAttackProbeCharacter::HandleTriggerSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (IsDefeated())
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=TriggerEnterIgnored other=%s reason=health_depleted"), *GetNameSafe(this), *GetNameSafe(OtherActor));
		return;
	}

	if (!OtherActor->FindComponentByClass<UTwoHeartsHostileAttackReceiverComponent>())
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=TriggerEnterIgnored other=%s reason=missing_receiver_component"), *GetNameSafe(this), *GetNameSafe(OtherActor));
		return;
	}

	CurrentTargetActor = OtherActor;
	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[HostileAttackProbe] actor=%s event=TriggerEnter target=%s distance=%.1f auto_trigger=%s"),
		*GetNameSafe(this),
		*GetNameSafe(CurrentTargetActor),
		FVector::Distance(GetActorLocation(), CurrentTargetActor->GetActorLocation()),
		bAutoTriggerWhenTargetEntersRange ? TEXT("true") : TEXT("false"));

	if (bAutoTriggerWhenTargetEntersRange)
	{
		TriggerProbeAttack();
	}
}

void ATwoHeartsHostileAttackProbeCharacter::HandleTriggerSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == CurrentTargetActor)
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=TriggerExit target=%s"), *GetNameSafe(this), *GetNameSafe(CurrentTargetActor));
		CurrentTargetActor = nullptr;
	}
}

void ATwoHeartsHostileAttackProbeCharacter::UpdateHitVolumePlacement()
{
	if (TriggerSphereComponent)
	{
		TriggerSphereComponent->SetSphereRadius(TriggerRadius);
	}

	if (HitSphereComponent)
	{
		HitSphereComponent->SetSphereRadius(AttackRadius);
		HitSphereComponent->SetRelativeLocation(FVector(AttackReach, 0.0f, 70.0f));
	}
}

void ATwoHeartsHostileAttackProbeCharacter::InitializeCurrentAttackMetadata()
{
	CurrentAttackMetadata = FTwoHeartsAttackMetadata();
	CurrentAttackMetadata.AttackInstanceName = CurrentAttackInstanceName;
	CurrentAttackMetadata.SourceActor = this;
	CurrentAttackMetadata.SourceLocation = GetActorLocation();
	CurrentAttackMetadata.AttackDirection = GetActorForwardVector().GetSafeNormal2D();
	CurrentAttackMetadata.HitReactionType = HitReactionType;
	CurrentAttackMetadata.BaseDamage = BaseDamage;
	CurrentAttackMetadata.DamageMechanicTags.AddTag(FTwoHeartsGameplayTags::Attack_Mechanic_Physical());
	CurrentAttackMetadata.DamageMechanicTags.AddTag(FTwoHeartsGameplayTags::Attack_Mechanic_HostileProbe());
	CurrentAttackMetadata.bCanBeGuarded = bAttackCanBeGuarded;

	CurrentAttackMetadata.GuardMaxDistance = GuardMaxDistance;
	CurrentAttackMetadata.GuardMaxHeightDifference = GuardMaxHeightDifference;
	CurrentAttackMetadata.GuardFacingHalfAngleDegrees = GuardFacingHalfAngleDegrees;
	CurrentAttackMetadata.GuardSuccessDisplacementResult = GuardSuccessDisplacementResult;
	CurrentAttackMetadata.GuardSuccessDamageResult = GuardSuccessDamageResult;
	CurrentAttackMetadata.GuardPartialDamageMultiplier = GuardPartialDamageMultiplier;
	CurrentAttackMetadata.bCanBeDodged = bAttackCanBeDodged;
	CurrentAttackMetadata.TimingPhase = ETwoHeartsAttackTimingPhase::Startup;

	CurrentAttackMetadata.TimingWindowName = TEXT("Startup");
}

void ATwoHeartsHostileAttackProbeCharacter::UpdateCurrentAttackMetadataTiming(
	ETwoHeartsAttackTimingPhase TimingPhase,
	FName TimingWindowName)
{
	CurrentAttackMetadata.AttackInstanceName = CurrentAttackInstanceName;
	CurrentAttackMetadata.SourceActor = this;
	CurrentAttackMetadata.SourceLocation = GetActorLocation();
	CurrentAttackMetadata.AttackDirection = GetActorForwardVector().GetSafeNormal2D();
	CurrentAttackMetadata.TimingPhase = TimingPhase;
	CurrentAttackMetadata.TimingWindowName = TimingWindowName;
}

FTwoHeartsAttackMetadata ATwoHeartsHostileAttackProbeCharacter::BuildCurrentAttackMetadataSnapshot() const
{
	FTwoHeartsAttackMetadata Metadata = CurrentAttackMetadata;
	Metadata.AttackInstanceName = CurrentAttackInstanceName;
	Metadata.SourceActor = const_cast<ATwoHeartsHostileAttackProbeCharacter*>(this);
	Metadata.SourceLocation = GetActorLocation();
	Metadata.AttackDirection = GetActorForwardVector().GetSafeNormal2D();
	return Metadata;
}

void ATwoHeartsHostileAttackProbeCharacter::StartAttackStartup()

{
	bAttackActive = true;
	bHitWindowActive = false;
	AttackTargetActor = CurrentTargetActor;
	ContactedTargetsThisAttack.Reset();
	CurrentAttackInstanceName = FString::Printf(TEXT("HostileProbe_%d"), ++AttackInstanceCounter);
	InitializeCurrentAttackMetadata();

	if (CombatActionContextComponent)
	{
		FTwoHeartsCombatActionRegistration Registration;
		Registration.ActionType = ETwoHeartsCombatActionType::HostileAttackProbe;
		Registration.InitialPhase = ETwoHeartsCombatPhase::Startup;
		Registration.AbilityTag = FTwoHeartsGameplayTags::Ability_HostileAttackProbe();
		Registration.ActionStateTag = FTwoHeartsGameplayTags::State_Action_HostileAttackProbe();
		Registration.ActionInstanceName = CurrentAttackInstanceName;
		CombatActionContextComponent->BeginAction(Registration, TEXT("HostileAttackProbeStarted"));
	}

	GetMesh()->PlayAnimation(AttackAnimation, false);
	NotifyCurrentTarget(ETwoHeartsHostileAttackSignalType::AttackStarted, TEXT("Hostile attack startup began."), false);
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackProbe] actor=%s event=AttackStarted attack=%s target=%s startup=%.2f hit_window=%.2f recovery=%.2f metadata=\"%s\""),
		*GetNameSafe(this),
		*CurrentAttackInstanceName,
		*GetNameSafe(AttackTargetActor),
		StartupSeconds,
		HitWindowSeconds,
		RecoverySeconds,
		*BuildProbeAttackMetadataDebugString(CurrentAttackMetadata));
	DrawDebugProbeState(FColor::Yellow, TEXT("Startup"));

	if (bEnableScreenDebugOutput && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()),
			1.0f,
			FColor::Yellow,
			FString::Printf(TEXT("[Probe] %s startup"), *CurrentAttackInstanceName));
	}

	GetWorldTimerManager().SetTimer(AttackPhaseTimerHandle, this, &ATwoHeartsHostileAttackProbeCharacter::OpenHitWindow, StartupSeconds, false);
}

void ATwoHeartsHostileAttackProbeCharacter::OpenHitWindow()
{
	if (!bAttackActive)
	{
		return;
	}

	bHitWindowActive = true;
	UpdateCurrentAttackMetadataTiming(ETwoHeartsAttackTimingPhase::HitWindow, TEXT("PrimaryHitWindow"));
	if (CombatActionContextComponent)
	{
		CombatActionContextComponent->TransitionToPhase(ETwoHeartsCombatPhase::Active, TEXT("HostileAttackProbeHitWindowOpened"));
	}


	NotifyTargetActor(AttackTargetActor, ETwoHeartsHostileAttackSignalType::HitWindowOpened, TEXT("Hostile attack hit window is now active."), false);
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[HostileAttackProbe] actor=%s event=HitWindowOpened attack=%s target=%s attack_radius=%.1f attack_reach=%.1f metadata=\"%s\""),
		*GetNameSafe(this),
		*CurrentAttackInstanceName,
		*GetNameSafe(AttackTargetActor),
		AttackRadius,
		AttackReach,
		*BuildProbeAttackMetadataDebugString(CurrentAttackMetadata));
	NotifyHitTargets();
	DrawDebugProbeState(FColor::Red, TEXT("HitWindow"));

	GetWorldTimerManager().SetTimer(AttackPhaseTimerHandle, this, &ATwoHeartsHostileAttackProbeCharacter::CloseHitWindow, HitWindowSeconds, false);
}

void ATwoHeartsHostileAttackProbeCharacter::CloseHitWindow()
{
	if (!bAttackActive)
	{
		return;
	}

	bHitWindowActive = false;
	UpdateCurrentAttackMetadataTiming(ETwoHeartsAttackTimingPhase::Recovery, TEXT("Recovery"));
	if (CombatActionContextComponent)
	{
		CombatActionContextComponent->TransitionToPhase(ETwoHeartsCombatPhase::Recovery, TEXT("HostileAttackProbeHitWindowClosed"));
	}


	NotifyTargetActor(AttackTargetActor, ETwoHeartsHostileAttackSignalType::HitWindowClosed, TEXT("Hostile attack hit window closed."), false);
	UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=HitWindowClosed attack=%s target=%s"), *GetNameSafe(this), *CurrentAttackInstanceName, *GetNameSafe(AttackTargetActor));
	DrawDebugProbeState(FColor::Blue, TEXT("Recovery"));

	GetWorldTimerManager().SetTimer(AttackPhaseTimerHandle, this, &ATwoHeartsHostileAttackProbeCharacter::FinishAttack, RecoverySeconds, false);
}

void ATwoHeartsHostileAttackProbeCharacter::FinishAttack()
{
	if (!bAttackActive)
	{
		return;
	}

	bAttackActive = false;
	bHitWindowActive = false;
	UpdateCurrentAttackMetadataTiming(ETwoHeartsAttackTimingPhase::Finished, TEXT("Finished"));

	if (CombatActionContextComponent)

	{
		CombatActionContextComponent->MarkLogicEnded(TEXT("HostileAttackProbeLogicEnded"));
		CombatActionContextComponent->FinishAction(ETwoHeartsCombatActionEndReason::Completed, TEXT("HostileAttackProbeFinished"));
	}

	NotifyTargetActor(AttackTargetActor, ETwoHeartsHostileAttackSignalType::AttackFinished, TEXT("Hostile attack finished and probe returned to idle."), false);
	UE_LOG(LogtwoheartsCombatTest, Display, TEXT("[HostileAttackProbe] actor=%s event=AttackFinished attack=%s target=%s contacted_targets=%d metadata=\"%s\""), *GetNameSafe(this), *CurrentAttackInstanceName, *GetNameSafe(AttackTargetActor), ContactedTargetsThisAttack.Num(), *BuildProbeAttackMetadataDebugString(CurrentAttackMetadata));
	DrawDebugProbeState(FColor::Green, TEXT("Finished"));
	RestoreIdleAnimation();
	ContactedTargetsThisAttack.Reset();

	if (bLoopAttackWhileTargetStaysInRange && CurrentTargetActor && IsActorInsideTriggerSphere(CurrentTargetActor))
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=LoopQueued attack=%s next_in=%.2f target=%s"), *GetNameSafe(this), *CurrentAttackInstanceName, RepeatCooldownSeconds, *GetNameSafe(CurrentTargetActor));
		GetWorldTimerManager().SetTimer(
			RepeatAttackTimerHandle,
			this,
			&ATwoHeartsHostileAttackProbeCharacter::HandleRepeatAttackTimerElapsed,
			RepeatCooldownSeconds,
			false);
	}

	AttackTargetActor = nullptr;
}

void ATwoHeartsHostileAttackProbeCharacter::HandleRepeatAttackTimerElapsed()
{
	if (IsDefeated())
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=LoopTriggerIgnored reason=health_depleted"), *GetNameSafe(this));
		return;
	}

	UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=LoopTriggerElapsed target=%s"), *GetNameSafe(this), *GetNameSafe(CurrentTargetActor));
	TriggerProbeAttack();
}

void ATwoHeartsHostileAttackProbeCharacter::RestoreIdleAnimation() const
{
	if (IdleAnimation)
	{
		GetMesh()->PlayAnimation(IdleAnimation, true);
	}
}

void ATwoHeartsHostileAttackProbeCharacter::NotifyCurrentTarget(ETwoHeartsHostileAttackSignalType SignalType, const FString& Detail, bool bHasContact) const
{
	NotifyTargetActor(CurrentTargetActor, SignalType, Detail, bHasContact);
}

void ATwoHeartsHostileAttackProbeCharacter::NotifyTargetActor(
	AActor* TargetActor,
	ETwoHeartsHostileAttackSignalType SignalType,
	const FString& Detail,
	bool bHasContact) const
{
	if (!TargetActor)
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=SignalSkipped type=%s reason=no_target_actor"), *GetNameSafe(this), LexToString(SignalType));
		return;
	}

	if (UTwoHeartsHostileAttackReceiverComponent* Receiver = TargetActor->FindComponentByClass<UTwoHeartsHostileAttackReceiverComponent>())
	{
		UE_LOG(
			LogtwoheartsCombatTest,
			Verbose,
			TEXT("[HostileAttackProbe] actor=%s event=SignalSent type=%s attack=%s target=%s contact=%s metadata=\"%s\""),
			*GetNameSafe(this),
			LexToString(SignalType),
			*CurrentAttackInstanceName,
			*GetNameSafe(TargetActor),
			bHasContact ? TEXT("true") : TEXT("false"),
			*BuildProbeAttackMetadataDebugString(BuildCurrentAttackMetadataSnapshot()));
		Receiver->ReceiveHostileAttackSignal(BuildSignal(SignalType, TargetActor, Detail, bHasContact));
	}
}

void ATwoHeartsHostileAttackProbeCharacter::NotifyHitTargets()
{
	if (!HitSphereComponent)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	HitSphereComponent->GetOverlappingActors(OverlappingActors, AActor::StaticClass());
	UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] actor=%s event=ContactScan attack=%s overlap_count=%d"), *GetNameSafe(this), *CurrentAttackInstanceName, OverlappingActors.Num());

	bool bContactSent = false;
	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (!OverlappingActor || OverlappingActor == this)
		{
			continue;
		}

		if (ContactedTargetsThisAttack.Contains(OverlappingActor))
		{
			continue;
		}

		UTwoHeartsHostileAttackReceiverComponent* Receiver = OverlappingActor->FindComponentByClass<UTwoHeartsHostileAttackReceiverComponent>();
		if (!Receiver)
		{
			continue;
		}

		Receiver->ReceiveHostileAttackSignal(
			BuildSignal(
				ETwoHeartsHostileAttackSignalType::AttackContact,
				OverlappingActor,
				TEXT("Probe attack overlap/contact occurred during hit window."),
				true));
		ContactedTargetsThisAttack.Add(OverlappingActor);
		bContactSent = true;

		UE_LOG(
			LogtwoheartsCombatTest,
			Display,
			TEXT("[HostileAttackProbe] actor=%s event=ContactSent attack=%s target=%s contacted_count=%d"),
			*GetNameSafe(this),
			*CurrentAttackInstanceName,
			*GetNameSafe(OverlappingActor),
			ContactedTargetsThisAttack.Num());
	}

	if (!bContactSent)
	{
		UE_LOG(LogtwoheartsCombatTest, Display, TEXT("[HostileAttackProbe] actor=%s event=ContactMiss attack=%s reason=no_valid_overlap_target"), *GetNameSafe(this), *CurrentAttackInstanceName);
	}
}

FTwoHeartsHostileAttackSignal ATwoHeartsHostileAttackProbeCharacter::BuildSignal(
	ETwoHeartsHostileAttackSignalType SignalType,
	AActor* TargetActor,
	const FString& Detail,
	bool bHasContact) const
{
	FTwoHeartsHostileAttackSignal Signal;
	Signal.SignalType = SignalType;
	Signal.AttackInstanceName = CurrentAttackInstanceName;
	Signal.SourceActor = const_cast<ATwoHeartsHostileAttackProbeCharacter*>(this);
	Signal.TargetActor = TargetActor;
	Signal.SourceLocation = GetActorLocation();
	Signal.AttackDirection = GetActorForwardVector().GetSafeNormal2D();
	Signal.TimestampSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Signal.bIsHitWindowActive = bHitWindowActive;
	Signal.bHasContact = bHasContact;
	Signal.Detail = Detail;
	Signal.AttackMetadata = BuildCurrentAttackMetadataSnapshot();
	return Signal;

}

void ATwoHeartsHostileAttackProbeCharacter::DrawDebugProbeState(const FColor& Color, const FString& Label) const
{
	if (!bDrawDebugShapes || !GetWorld())
	{
		return;
	}

	const FVector ForwardCenter = GetActorLocation() + (GetActorForwardVector().GetSafeNormal2D() * AttackReach) + FVector(0.0f, 0.0f, 70.0f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), TriggerRadius, 24, FColor::Cyan, false, 1.0f, 0, 1.2f);
	DrawDebugSphere(GetWorld(), ForwardCenter, AttackRadius, 20, Color, false, 1.0f, 0, 1.6f);
	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), ForwardCenter, 40.0f, Color, false, 1.0f, 0, 2.0f);

	UE_LOG(
		LogtwoheartsCombatTest,
		Verbose,
		TEXT("[HostileAttackProbe] actor=%s phase=%s attack=%s target=%s hit_window=%s"),
		*GetNameSafe(this),
		*Label,
		*CurrentAttackInstanceName,
		*GetNameSafe(CurrentTargetActor),
		bHitWindowActive ? TEXT("true") : TEXT("false"));

}

bool ATwoHeartsHostileAttackProbeCharacter::IsActorInsideTriggerSphere(const AActor* Actor) const
{
	return Actor
		&& TriggerSphereComponent
		&& FVector::DistSquared(Actor->GetActorLocation(), TriggerSphereComponent->GetComponentLocation()) <= FMath::Square(TriggerRadius);
}

bool ATwoHeartsHostileAttackProbeCharacter::IsDefeated() const
{
	return PlayerAttackReceiverComponent && PlayerAttackReceiverComponent->IsDefeated();
}
