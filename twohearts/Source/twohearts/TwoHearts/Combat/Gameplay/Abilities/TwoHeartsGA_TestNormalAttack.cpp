#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_TestNormalAttack.h"

#include "GameplayTagContainer.h"
#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"

UTwoHeartsGA_TestNormalAttack::UTwoHeartsGA_TestNormalAttack()
{
	SetDefaultAssetTag(TAG_TwoHearts_Ability_NormalAttack);
	AddDefaultActivationOwnedTag(TAG_TwoHearts_State_Action_NormalAttack);
	AddDefaultActivationBlockedTag(TAG_TwoHearts_State_CannotAttack);
	AddDefaultActivationBlockedTag(TAG_TwoHearts_State_CannotInput);
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
