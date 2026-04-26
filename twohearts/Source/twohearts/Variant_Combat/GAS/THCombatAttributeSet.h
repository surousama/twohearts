// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "THCombatAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Minimal combat attribute set for future GAS-driven combat data.
 */
UCLASS()
class TWOHEARTS_API UTHCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UTHCombatAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UTHCombatAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category="Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UTHCombatAttributeSet, MaxHealth)
};
