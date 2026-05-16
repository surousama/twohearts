#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_2.h"

#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"

UTwoHeartsGA_NormalAttack_2::UTwoHeartsGA_NormalAttack_2()
{
	NormalAttackSegment = 2;
	AddDefaultAssetTag(TAG_TwoHearts_Ability_NormalAttack_Segment2);
	SegmentAbilityTag = TAG_TwoHearts_Ability_NormalAttack_Segment2;
	NextSegmentAbilityTag = TAG_TwoHearts_Ability_NormalAttack_Segment3;
}
