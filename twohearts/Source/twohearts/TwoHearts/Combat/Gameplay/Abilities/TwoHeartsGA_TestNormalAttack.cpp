#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_TestNormalAttack.h"

#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"

UTwoHeartsGA_TestNormalAttack::UTwoHeartsGA_TestNormalAttack()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(FTwoHeartsGameplayTags::Ability_NormalAttack());
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(FTwoHeartsGameplayTags::State_Action_NormalAttack());
	ActivationBlockedTags.AddTag(FTwoHeartsGameplayTags::State_CannotAttack());
	ActivationBlockedTags.AddTag(FTwoHeartsGameplayTags::State_CannotInput());
}

void UTwoHeartsGA_TestNormalAttack::ActivateAbility(
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

	LogAbilityMessage(TEXT("Test normal attack ability activated."));
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
