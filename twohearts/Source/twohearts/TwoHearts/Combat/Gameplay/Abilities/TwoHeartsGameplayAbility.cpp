#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "twohearts.h"

UTwoHeartsGameplayAbility::UTwoHeartsGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AActor* UTwoHeartsGameplayAbility::GetAbilityOwnerActor() const
{
	return GetCurrentActorInfo() ? GetCurrentActorInfo()->OwnerActor.Get() : nullptr;
}

AActor* UTwoHeartsGameplayAbility::GetAbilityAvatarActor() const
{
	return GetCurrentActorInfo() ? GetCurrentActorInfo()->AvatarActor.Get() : nullptr;
}

ACharacter* UTwoHeartsGameplayAbility::GetAbilityCharacter() const
{
	return Cast<ACharacter>(GetAbilityAvatarActor());
}

UAbilitySystemComponent* UTwoHeartsGameplayAbility::GetTwoHeartsAbilitySystemComponent() const
{
	return GetAbilitySystemComponentFromActorInfo();
}

void UTwoHeartsGameplayAbility::SetDefaultAssetTag(FGameplayTag Tag)
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(Tag);
	SetAssetTags(AssetTags);
}

void UTwoHeartsGameplayAbility::AddDefaultActivationOwnedTag(FGameplayTag Tag)
{
	ActivationOwnedTags.AddTag(Tag);
}

void UTwoHeartsGameplayAbility::AddDefaultActivationBlockedTag(FGameplayTag Tag)
{
	ActivationBlockedTags.AddTag(Tag);
}

void UTwoHeartsGameplayAbility::LogAbilityMessage(const FString& Message) const
{
	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[GameplayAbility] ability=%s owner=%s avatar=%s message=\"%s\""),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetAbilityOwnerActor()),
		*GetNameSafe(GetAbilityAvatarActor()),
		*Message);
}
