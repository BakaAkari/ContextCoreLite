// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ContextCoreLite : ModuleRules
{
	public ContextCoreLite(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"BlueprintGraph",
				"Kismet",
				"GraphEditor",
				"AssetRegistry",
				"Json",
				"JsonUtilities",
				"AnimGraph",
				"ContentBrowser",
				"ToolMenus",
			}
		);
	}
}
