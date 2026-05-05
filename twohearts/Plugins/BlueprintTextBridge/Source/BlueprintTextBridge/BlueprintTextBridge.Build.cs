using UnrealBuildTool;

public class BlueprintTextBridge : ModuleRules
{
	public BlueprintTextBridge(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
			});

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"ApplicationCore",
				"CoreUObject",
				"Engine",
				"GraphEditor",
				"Kismet",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"UnrealEd"
			});
	}
}
