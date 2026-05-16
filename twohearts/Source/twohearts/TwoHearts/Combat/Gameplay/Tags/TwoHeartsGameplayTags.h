#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack_Segment1);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack_Segment2);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_Ability_NormalAttack_Segment3);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Action_NormalAttack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Action_Dodge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_Action_Guard);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_CannotAttack);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_TwoHearts_State_CannotInput);

struct FTwoHeartsGameplayTags
{
	static FGameplayTag Ability_NormalAttack();
	static FGameplayTag Ability_NormalAttack_Segment1();
	static FGameplayTag Ability_NormalAttack_Segment2();
	static FGameplayTag Ability_NormalAttack_Segment3();
	static FGameplayTag State_Action_NormalAttack();
	static FGameplayTag State_Action_Dodge();
	static FGameplayTag State_Action_Guard();
	static FGameplayTag State_CannotAttack();
	static FGameplayTag State_CannotInput();
};
