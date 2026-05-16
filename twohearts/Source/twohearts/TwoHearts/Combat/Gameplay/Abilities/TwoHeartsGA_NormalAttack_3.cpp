#include "TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_NormalAttack_3.h"

#include "TwoHearts/Combat/Gameplay/Tags/TwoHeartsGameplayTags.h"

UTwoHeartsGA_NormalAttack_3::UTwoHeartsGA_NormalAttack_3()
{
	NormalAttackSegment = 3;
	AddDefaultAssetTag(TAG_TwoHearts_Ability_NormalAttack_Segment3);
	SegmentAbilityTag = TAG_TwoHearts_Ability_NormalAttack_Segment3;
}
