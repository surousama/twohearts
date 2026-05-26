#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_Dodge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_HostileAttackProbe);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack_Segment1);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack_Segment2);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack_Segment3);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Action_NormalAttack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Action_Dodge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Action_Guard);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Action_HostileAttackProbe);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_CannotAttack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_CannotDodge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_CannotInput);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Dodge_Invulnerable);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Cooldown_Dodge);

struct FTwoHeartsGameplayTags
{
	static FGameplayTag Ability_NormalAttack();
	static FGameplayTag Ability_Dodge();
	static FGameplayTag Ability_HostileAttackProbe();
	static FGameplayTag Ability_NormalAttack_Segment1();
	static FGameplayTag Ability_NormalAttack_Segment2();
	static FGameplayTag Ability_NormalAttack_Segment3();
	static FGameplayTag State_Action_NormalAttack();
	static FGameplayTag State_Action_Dodge();
	static FGameplayTag State_Action_Guard();
	static FGameplayTag State_Action_HostileAttackProbe();
	static FGameplayTag State_CannotAttack();
	static FGameplayTag State_CannotDodge();
	static FGameplayTag State_CannotInput();
	static FGameplayTag State_Dodge_Invulnerable();
	static FGameplayTag Cooldown_Dodge();
};
