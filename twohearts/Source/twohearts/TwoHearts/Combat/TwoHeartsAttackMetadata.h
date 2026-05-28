#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TwoHeartsAttackMetadata.generated.h"

class AActor;

UENUM(BlueprintType)
enum class ETwoHeartsHitReactionType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Light UMETA(DisplayName="Light"),
	Heavy UMETA(DisplayName="Heavy"),
	GuardBreak UMETA(DisplayName="GuardBreak")
};

UENUM(BlueprintType)
enum class ETwoHeartsAttackTimingPhase : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Startup UMETA(DisplayName="Startup"),
	HitWindow UMETA(DisplayName="HitWindow"),
	Recovery UMETA(DisplayName="Recovery"),
	Finished UMETA(DisplayName="Finished")
};

USTRUCT(BlueprintType)
struct FTwoHeartsAttackMetadata
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	FString AttackInstanceName = TEXT("None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	FVector AttackDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	ETwoHeartsHitReactionType HitReactionType = ETwoHeartsHitReactionType::Light;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata", meta=(ClampMin="0.0", UIMin="0.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	FGameplayTagContainer DamageMechanicTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	bool bCanBeGuarded = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata|Guard Rule", meta=(ClampMin="0.0", UIMin="0.0"))
	float GuardMaxDistance = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata|Guard Rule", meta=(ClampMin="0.0", UIMin="0.0"))
	float GuardMaxHeightDifference = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata|Guard Rule", meta=(ClampMin="0.0", ClampMax="180.0", UIMin="0.0", UIMax="180.0"))
	float GuardFacingHalfAngleDegrees = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	bool bCanBeDodged = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	ETwoHeartsAttackTimingPhase TimingPhase = ETwoHeartsAttackTimingPhase::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Attack Metadata")
	FName TimingWindowName = NAME_None;
};
