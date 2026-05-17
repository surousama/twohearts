#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttackBase.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"
#include "twohearts.h"

UTwoHeartsGA_Dodge::UTwoHeartsGA_Dodge()
{
	AddDefaultAssetTag(TAG_TwoHearts_Ability_Dodge);
	ActivationOwnedTags.AddTag(TAG_TwoHearts_State_Action_Dodge);
	ActivationBlockedTags.AddTag(TAG_TwoHearts_State_CannotInput);
}

void UTwoHeartsGA_Dodge::ActivateAbility(
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

	if (UTwoHeartsGA_NormalAttackBase* ActiveNormalAttack = FindActiveNormalAttackAbility())
	{
		if (!ActiveNormalAttack->TryInterruptByDodge())
		{
			UE_LOG(
				LogtwoheartsCombatTest,
				Display,
				TEXT("[GameplayAbility] ability=%s owner=%s avatar=%s message=\"Dodge activation rejected because the active normal attack phase is not interruptible.\""),
				*GetNameSafe(GetClass()),
				*GetNameSafe(GetAbilityOwnerActor()),
				*GetNameSafe(GetAbilityAvatarActor()));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}

	UE_LOG(
		LogtwoheartsCombatTest,
		Display,
		TEXT("[GameplayAbility] ability=%s owner=%s avatar=%s message=\"Minimal dodge ability activated.\""),
		*GetNameSafe(GetClass()),
		*GetNameSafe(GetAbilityOwnerActor()),
		*GetNameSafe(GetAbilityAvatarActor()));

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UTwoHeartsGA_NormalAttackBase* UTwoHeartsGA_Dodge::FindActiveNormalAttackAbility() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetTwoHeartsAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		if (UTwoHeartsGA_NormalAttackBase* ActiveNormalAttack = Cast<UTwoHeartsGA_NormalAttackBase>(AbilitySpec.GetPrimaryInstance()))
		{
			return ActiveNormalAttack;
		}
	}

	return nullptr;
}
