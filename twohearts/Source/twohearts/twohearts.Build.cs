// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
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
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		string[] includeDirectories = new string[] {
			".",
			"Variant_Platforming",
			"Variant_Platforming/Animation",
			"Variant_Combat",
			"Variant_Combat/AI",
			"Variant_Combat/Animation",
			"Variant_Combat/Gameplay",
			"Variant_Combat/Interfaces",
			"Variant_Combat/UI",
			"Variant_SideScrolling",
			"Variant_SideScrolling/AI",
			"Variant_SideScrolling/Gameplay",
			"Variant_SideScrolling/Interfaces",
			"Variant_SideScrolling/UI"
		};

		foreach (string includeDirectory in includeDirectories)
		{
			string fullPath = Path.GetFullPath(Path.Combine(ModuleDirectory, includeDirectory));
			if (Directory.Exists(fullPath))
			{
				PublicIncludePaths.Add(fullPath);
			}
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
