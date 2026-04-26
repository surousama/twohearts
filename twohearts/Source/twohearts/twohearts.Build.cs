// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class twohearts : ModuleRules
{
	public twohearts(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"twohearts",
			"twohearts/Variant_Platforming",
			"twohearts/Variant_Platforming/Animation",
			"twohearts/Variant_Combat",
			"twohearts/Variant_Combat/AI",
			"twohearts/Variant_Combat/Animation",
			"twohearts/Variant_Combat/GAS",
			"twohearts/Variant_Combat/GAS/Abilities",
			"twohearts/Variant_Combat/Gameplay",
			"twohearts/Variant_Combat/Interfaces",
			"twohearts/Variant_Combat/UI",
			"twohearts/Variant_SideScrolling",
			"twohearts/Variant_SideScrolling/AI",
			"twohearts/Variant_SideScrolling/Gameplay",
			"twohearts/Variant_SideScrolling/Interfaces",
			"twohearts/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
