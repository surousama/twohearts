#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_Ability_NormalAttack, "Ability.NormalAttack", "Normal attack ability tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_Ability_Dodge, "Ability.Dodge", "Dodge ability tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_Ability_NormalAttack_Segment1, "Ability.NormalAttack.Segment1", "First normal attack segment ability tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_Ability_NormalAttack_Segment2, "Ability.NormalAttack.Segment2", "Second normal attack segment ability tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_Ability_NormalAttack_Segment3, "Ability.NormalAttack.Segment3", "Third normal attack segment ability tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_State_Action_NormalAttack, "State.Action.NormalAttack", "Normal attack action state tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_State_Action_Dodge, "State.Action.Dodge", "Dodge action state tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_State_Action_Guard, "State.Action.Guard", "Guard action state tag.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_State_CannotAttack, "State.CannotAttack", "Prevents attack abilities from activating.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_TwoHearts_State_CannotInput, "State.CannotInput", "Prevents combat ability input from activating.");

FGameplayTag FTwoHeartsGameplayTags::Ability_NormalAttack()
{
	return TAG_TwoHearts_Ability_NormalAttack;
}

FGameplayTag FTwoHeartsGameplayTags::Ability_Dodge()
{
	return TAG_TwoHearts_Ability_Dodge;
}

FGameplayTag FTwoHeartsGameplayTags::Ability_NormalAttack_Segment1()
{
	return TAG_TwoHearts_Ability_NormalAttack_Segment1;
}

FGameplayTag FTwoHeartsGameplayTags::Ability_NormalAttack_Segment2()
{
	return TAG_TwoHearts_Ability_NormalAttack_Segment2;
}

FGameplayTag FTwoHeartsGameplayTags::Ability_NormalAttack_Segment3()
{
	return TAG_TwoHearts_Ability_NormalAttack_Segment3;
}

FGameplayTag FTwoHeartsGameplayTags::State_Action_NormalAttack()
{
	return TAG_TwoHearts_State_Action_NormalAttack;
}

FGameplayTag FTwoHeartsGameplayTags::State_Action_Dodge()
{
	return TAG_TwoHearts_State_Action_Dodge;
}

FGameplayTag FTwoHeartsGameplayTags::State_Action_Guard()
{
	return TAG_TwoHearts_State_Action_Guard;
}

FGameplayTag FTwoHeartsGameplayTags::State_CannotAttack()
{
	return TAG_TwoHearts_State_CannotAttack;
}

FGameplayTag FTwoHeartsGameplayTags::State_CannotInput()
{
	return TAG_TwoHearts_State_CannotInput;
}
