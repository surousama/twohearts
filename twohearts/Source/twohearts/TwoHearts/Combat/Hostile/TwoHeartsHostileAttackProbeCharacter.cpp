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
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "TwoHearts/Combat/TwoHeartsCombatActionContextComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "twohearts.h"

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
}

void ATwoHeartsHostileAttackProbeCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateHitVolumePlacement();
}

bool ATwoHeartsHostileAttackProbeCharacter::TriggerProbeAttack()
{
	if (bAttackActive)
	{
		UE_LOG(LogtwoheartsCombatTest, Verbose, TEXT("[HostileAttackProbe] %s ignored trigger because an attack is already active."), *GetNameSafe(this));
		return false;
	}

	if (!AttackAnimation)
	{
		UE_LOG(LogtwoheartsCombatTest, Warning, TEXT("[HostileAttackProbe] %s cannot start because AttackAnimation is not configured."), *GetNameSafe(this));
		return false;
	}

	GetWorldTimerManager().ClearTimer(RepeatAttackTimerHandle);
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
	ContactedTargetsThisAttack.Reset();

	if (CombatActionContextComponent && CombatActionContextComponent->HasActiveAction())
	{
		CombatActionContextComponent->FinishAction(ETwoHeartsCombatActionEndReason::Cancelled, TEXT("ProbeResetToIdle"));
	}

	RestoreIdleAnimation();
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

	if (!OtherActor->FindComponentByClass<UTwoHeartsHostileAttackReceiverComponent>())
	{
		return;
	}

	CurrentTargetActor = OtherActor;

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

void ATwoHeartsHostileAttackProbeCharacter::StartAttackStartup()
{
	bAttackActive = true;
	bHitWindowActive = false;
	ContactedTargetsThisAttack.Reset();
	CurrentAttackInstanceName = FString::Printf(TEXT("HostileProbe_%d"), ++AttackInstanceCounter);

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
	if (CombatActionContextComponent)
	{
		CombatActionContextComponent->TransitionToPhase(ETwoHeartsCombatPhase::Active, TEXT("HostileAttackProbeHitWindowOpened"));
	}

	NotifyCurrentTarget(ETwoHeartsHostileAttackSignalType::HitWindowOpened, TEXT("Hostile attack hit window is now active."), false);
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
	if (CombatActionContextComponent)
	{
		CombatActionContextComponent->TransitionToPhase(ETwoHeartsCombatPhase::Recovery, TEXT("HostileAttackProbeHitWindowClosed"));
	}

	NotifyCurrentTarget(ETwoHeartsHostileAttackSignalType::HitWindowClosed, TEXT("Hostile attack hit window closed."), false);
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

	if (CombatActionContextComponent)
	{
		CombatActionContextComponent->MarkLogicEnded(TEXT("HostileAttackProbeLogicEnded"));
		CombatActionContextComponent->FinishAction(ETwoHeartsCombatActionEndReason::Completed, TEXT("HostileAttackProbeFinished"));
	}

	NotifyCurrentTarget(ETwoHeartsHostileAttackSignalType::AttackFinished, TEXT("Hostile attack finished and probe returned to idle."), false);
	DrawDebugProbeState(FColor::Green, TEXT("Finished"));
	RestoreIdleAnimation();
	ContactedTargetsThisAttack.Reset();

	if (bLoopAttackWhileTargetStaysInRange && CurrentTargetActor && IsActorInsideTriggerSphere(CurrentTargetActor))
	{
		GetWorldTimerManager().SetTimer(
			RepeatAttackTimerHandle,
			this,
			&ATwoHeartsHostileAttackProbeCharacter::HandleRepeatAttackTimerElapsed,
			RepeatCooldownSeconds,
			false);
	}
}

void ATwoHeartsHostileAttackProbeCharacter::HandleRepeatAttackTimerElapsed()
{
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
	if (!CurrentTargetActor)
	{
		return;
	}

	if (UTwoHeartsHostileAttackReceiverComponent* Receiver = CurrentTargetActor->FindComponentByClass<UTwoHeartsHostileAttackReceiverComponent>())
	{
		Receiver->ReceiveHostileAttackSignal(BuildSignal(SignalType, CurrentTargetActor, Detail, bHasContact));
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
		Display,
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
