#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TwoHearts/Combat/TwoHeartsAttackMetadata.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsHostileAttackReceiverComponent.h"
#include "TwoHearts/Combat/Hostile/TwoHeartsPlayerAttackReceiverComponent.h"
#include "TwoHeartsHostileAttackProbeCharacter.generated.h"


class UAnimationAsset;
class UPrimitiveComponent;
class USphereComponent;
class UTwoHeartsCombatActionContextComponent;
struct FHitResult;

UCLASS(BlueprintType, Blueprintable)
class ATwoHeartsHostileAttackProbeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATwoHeartsHostileAttackProbeCharacter();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category="Combat|Hostile Attack Probe")
	bool TriggerProbeAttack();

	UFUNCTION(BlueprintCallable, Category="Combat|Hostile Attack Probe")
	void ResetProbeToIdle();

	UFUNCTION(BlueprintPure, Category="Combat|Hostile Attack Probe")
	UTwoHeartsCombatActionContextComponent* GetCombatActionContextComponent() const { return CombatActionContextComponent; }

protected:
	UFUNCTION()
	void HandleTriggerSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTriggerSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	void UpdateHitVolumePlacement();
	void StartAttackStartup();
	void OpenHitWindow();
	void CloseHitWindow();
	void FinishAttack();
	void HandleRepeatAttackTimerElapsed();
	void RestoreIdleAnimation() const;
	void NotifyCurrentTarget(ETwoHeartsHostileAttackSignalType SignalType, const FString& Detail, bool bHasContact) const;
	void NotifyTargetActor(AActor* TargetActor, ETwoHeartsHostileAttackSignalType SignalType, const FString& Detail, bool bHasContact) const;
	void NotifyHitTargets();
	void InitializeCurrentAttackMetadata();
	void UpdateCurrentAttackMetadataTiming(ETwoHeartsAttackTimingPhase TimingPhase, FName TimingWindowName);
	FTwoHeartsAttackMetadata BuildCurrentAttackMetadataSnapshot() const;
	FTwoHeartsHostileAttackSignal BuildSignal(ETwoHeartsHostileAttackSignalType SignalType, AActor* TargetActor, const FString& Detail, bool bHasContact) const;
	void DrawDebugProbeState(const FColor& Color, const FString& Label) const;
	bool IsActorInsideTriggerSphere(const AActor* Actor) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTwoHeartsCombatActionContextComponent> CombatActionContextComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> TriggerSphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> HitSphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UTwoHeartsPlayerAttackReceiverComponent> PlayerAttackReceiverComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimationAsset> IdleAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimationAsset> AttackAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float TriggerRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float AttackReach = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float AttackRadius = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float StartupSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(ClampMin="0.01", UIMin="0.01", AllowPrivateAccess="true"))
	float HitWindowSeconds = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float RecoverySeconds = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Rule", meta=(AllowPrivateAccess="true"))
	bool bAttackCanBeGuarded = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Rule", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float GuardMaxDistance = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Rule", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float GuardMaxHeightDifference = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Rule", meta=(ClampMin="0.0", ClampMax="180.0", UIMin="0.0", UIMax="180.0", AllowPrivateAccess="true"))
	float GuardFacingHalfAngleDegrees = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Guard Outcome", meta=(AllowPrivateAccess="true"))
	ETwoHeartsGuardDisplacementResult GuardSuccessDisplacementResult = ETwoHeartsGuardDisplacementResult::AttackerPushedBack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Guard Outcome", meta=(AllowPrivateAccess="true"))
	ETwoHeartsGuardDamageResult GuardSuccessDamageResult = ETwoHeartsGuardDamageResult::FullyBlocked;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Guard Outcome", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0", AllowPrivateAccess="true"))
	float GuardPartialDamageMultiplier = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(ClampMin="0.0", UIMin="0.0", AllowPrivateAccess="true"))
	float RepeatCooldownSeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(AllowPrivateAccess="true"))
	bool bAutoTriggerWhenTargetEntersRange = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe", meta=(AllowPrivateAccess="true"))
	bool bLoopAttackWhileTargetStaysInRange = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Debug", meta=(AllowPrivateAccess="true"))
	bool bEnableScreenDebugOutput = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Debug", meta=(AllowPrivateAccess="true"))
	bool bDrawDebugShapes = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Debug", meta=(AllowPrivateAccess="true"))
	bool bAttackActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Debug", meta=(AllowPrivateAccess="true"))
	bool bHitWindowActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Debug", meta=(AllowPrivateAccess="true"))
	int32 AttackInstanceCounter = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Debug", meta=(AllowPrivateAccess="true"))
	FString CurrentAttackInstanceName = TEXT("None");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat|Hostile Attack Probe|Debug", meta=(AllowPrivateAccess="true"))
	FTwoHeartsAttackMetadata CurrentAttackMetadata;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentTargetActor = nullptr;


	UPROPERTY(Transient)
	TObjectPtr<AActor> AttackTargetActor = nullptr;

	FTimerHandle AttackPhaseTimerHandle;
	FTimerHandle RepeatAttackTimerHandle;
	TSet<TWeakObjectPtr<AActor>> ContactedTargetsThisAttack;
};
