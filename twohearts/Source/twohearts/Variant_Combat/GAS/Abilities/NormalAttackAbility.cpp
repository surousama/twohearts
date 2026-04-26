// Copyright Epic Games, Inc. All Rights Reserved.

#include "NormalAttackAbility.h"
#include "CombatCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UNormalAttackAbility::UNormalAttackAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UNormalAttackAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ActiveHandle = Handle;
	ActiveActivationInfo = ActivationInfo;
	bHasCommittedAbility = false;

	if (ActorInfo == nullptr || ActorInfo->AvatarActor.Get() == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACombatCharacter* CombatCharacter = Cast<ACombatCharacter>(ActorInfo->AvatarActor.Get());
	if (CombatCharacter == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* NormalAttackMontage = CombatCharacter->GetNormalAttackMontage();
	if (NormalAttackMontage == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bHasCommittedAbility = true;

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("NormalAttackMontage"),
		NormalAttackMontage,
		1.0f);

	if (PlayMontageTask == nullptr)
	{
		FinishAbility(true);
		return;
	}

	PlayMontageTask->OnCompleted.AddDynamic(this, &UNormalAttackAbility::OnMontageCompleted);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UNormalAttackAbility::OnMontageCompleted);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UNormalAttackAbility::OnMontageInterrupted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UNormalAttackAbility::OnMontageCancelled);
	PlayMontageTask->ReadyForActivation();
}

void UNormalAttackAbility::OnMontageCompleted()
{
	FinishAbility(false);
}

void UNormalAttackAbility::OnMontageInterrupted()
{
	FinishAbility(true);
}

void UNormalAttackAbility::OnMontageCancelled()
{
	FinishAbility(true);
}

void UNormalAttackAbility::FinishAbility(bool bWasCancelled)
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (ActorInfo == nullptr)
	{
		return;
	}

	EndAbility(ActiveHandle, ActorInfo, ActiveActivationInfo, true, bWasCancelled || !bHasCommittedAbility);
}
