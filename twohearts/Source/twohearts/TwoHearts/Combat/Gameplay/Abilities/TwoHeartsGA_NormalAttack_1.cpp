#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_1.h"

#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"

UTwoHeartsGA_NormalAttack_1::UTwoHeartsGA_NormalAttack_1()
{
	NormalAttackSegment = 1;
	AddDefaultAssetTag(TAG_TwoHearts_Ability_NormalAttack_Segment1);
	SegmentAbilityTag = TAG_TwoHearts_Ability_NormalAttack_Segment1;
	NextSegmentAbilityTag = TAG_TwoHearts_Ability_NormalAttack_Segment2;
}
