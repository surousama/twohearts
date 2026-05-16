#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"
#include "twoheartsCharacter.h"

UTwoHeartsGA_NormalAttackBase::UTwoHeartsGA_NormalAttackBase()
{
	AddDefaultAssetTag(TAG_TwoHearts_Ability_NormalAttack);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_Action_NormalAttack);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_State_CannotAttack);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_State_CannotInput);
}

void UTwoHeartsGA_NormalAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bHasQueuedNextSegment = false;
	bHasFinishedSegment = false;

	if (!StartSegmentPlayback())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void UTwoHeartsGA_NormalAttackBase::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (!CanQueueNextSegment())
	{
		RecordAbilityEvent(TEXT("InputIgnored"), FString::Printf(TEXT("Segment %d cannot queue a next segment."), NormalAttackSegment), true);
		return;
	}

	if (bHasQueuedNextSegment)
	{
		RecordAbilityEvent(TEXT("InputIgnored"), FString::Printf(TEXT("Segment %d already has a queued next segment."), NormalAttackSegment), true);
		return;
	}

	bHasQueuedNextSegment = true;
	UpdateDebugState(true);
	RecordAbilityEvent(TEXT("QueueNextSegment"), FString::Printf(TEXT("Queued next segment after %d."), NormalAttackSegment), true);
}

void UTwoHeartsGA_NormalAttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UpdateDebugState(false);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageCompleted()
{
	FinishSegment(false);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageInterrupted()
{
	FinishSegment(true);
}

void UTwoHeartsGA_NormalAttackBase::HandleMontageCancelled()
{
	FinishSegment(true);
}

bool UTwoHeartsGA_NormalAttackBase::CanQueueNextSegment() const
{
	return NormalAttackSegment < 3 && NextSegmentAbilityTag.IsValid();
}

bool UTwoHeartsGA_NormalAttackBase::StartSegmentPlayback()
{
	AtwoheartsCharacter* Character = GetTwoHeartsCharacter();
	if (!Character)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), TEXT("Normal attack Ability could not find a valid twohearts character."));
		return false;
	}

	UAnimMontage* Montage = Character->GetNormalAttackMontage();
	if (!Montage)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("NormalAttackMontage is not configured on %s."), *GetNameSafe(Character)));
		return false;
	}

	if (!Character->GetMesh() || !Character->GetMesh()->GetAnimInstance())
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("No AnimInstance found for normal attack on %s."), *GetNameSafe(Character)));
		return false;
	}

	const FName SectionName = Character->GetNormalAttackSectionName(NormalAttackSegment);
	if (SectionName.IsNone() || Montage->GetSectionIndex(SectionName) == INDEX_NONE)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("Normal attack section %s is missing on %s."), *SectionName.ToString(), *GetNameSafe(Montage)));
		return false;
	}

	UpdateDebugState(true);
	Character->SetLastNormalAttackDebugFailureReason(TEXT(""));
	RecordAbilityEvent(
		TEXT("PlaySegment"),
		FString::Printf(TEXT("Started segment %d with section %s."), NormalAttackSegment, *SectionName.ToString()));

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.0f,
		SectionName,
		false,
		1.0f);

	if (!MontageTask)
	{
		RecordAbilityFailure(TEXT("ActivateFailed"), FString::Printf(TEXT("Failed to create montage task for segment %d."), NormalAttackSegment));
		return false;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UTwoHeartsGA_NormalAttackBase::HandleMontageCancelled);
	MontageTask->ReadyForActivation();

	return true;
}

void UTwoHeartsGA_NormalAttackBase::FinishSegment(bool bWasCancelled)
{
	if (bHasFinishedSegment)
	{
		return;
	}

	bHasFinishedSegment = true;

	const FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();

	const bool bShouldActivateNext = !bWasCancelled && bHasQueuedNextSegment && CanQueueNextSegment() && AbilitySystemComponent;

	if (bWasCancelled)
	{
		RecordAbilityFailure(TEXT("SegmentInterrupted"), FString::Printf(TEXT("Segment %d was interrupted before completion."), NormalAttackSegment));
	}
	else
	{
		RecordAbilityEvent(
			TEXT("SegmentFinished"),
			FString::Printf(TEXT("Segment %d finished. QueuedNext=%s."), NormalAttackSegment, bHasQueuedNextSegment ? TEXT("true") : TEXT("false")));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, bWasCancelled);

	if (!bShouldActivateNext)
	{
		return;
	}

	FGameplayTagContainer NextAbilityTags;
	NextAbilityTags.AddTag(NextSegmentAbilityTag);
	if (!AbilitySystemComponent->TryActivateAbilitiesByTag(NextAbilityTags, true))
	{
		RecordAbilityFailure(
			TEXT("AdvanceSegmentFailed"),
			FString::Printf(TEXT("Failed to activate next normal attack segment from segment %d."), NormalAttackSegment));
		return;
	}

	RecordAbilityEvent(
		TEXT("AdvanceSegment"),
		FString::Printf(TEXT("Advancing from segment %d to next segment tag %s."), NormalAttackSegment, *NextSegmentAbilityTag.ToString()),
		true);
}

void UTwoHeartsGA_NormalAttackBase::UpdateDebugState(bool bShouldBeActive) const
{
	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		const FString SectionName = bShouldBeActive ? Character->GetNormalAttackSectionName(NormalAttackSegment).ToString() : TEXT("None");
		Character->SetNormalAttackDebugRuntimeState(bShouldBeActive, bShouldBeActive ? NormalAttackSegment : 0, bShouldBeActive ? bHasQueuedNextSegment : false, SectionName);
	}
}

void UTwoHeartsGA_NormalAttackBase::RecordAbilityEvent(const TCHAR* EventName, const FString& Detail, bool bVerboseOnly) const
{
	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		Character->PushNormalAttackDebugEvent(EventName, Detail, bVerboseOnly);
	}

	LogAbilityMessage(Detail);
}

void UTwoHeartsGA_NormalAttackBase::RecordAbilityFailure(const TCHAR* EventName, const FString& Detail) const
{
	if (AtwoheartsCharacter* Character = GetTwoHeartsCharacter())
	{
		Character->PushNormalAttackDebugFailure(EventName, Detail);
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Warning,
		TEXT("[GameplayAbility] ability=%s owner=%s avatar=%s detail=\"%s\""),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetAbilityOwnerActor()),
		*GetNameSafe(GetAbilityAvatarActor()),
		*Detail);
}

AtwoheartsCharacter* UTwoHeartsGA_NormalAttackBase::GetTwoHeartsCharacter() const
{
	return Cast<AtwoheartsCharacter>(GetAbilityCharacter());
}
