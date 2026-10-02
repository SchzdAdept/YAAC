// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YAAC : ModuleRules
{
	public YAAC(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"YAAC",
			"YAAC/Variant_Platforming",
			"YAAC/Variant_Platforming/Animation",
			"YAAC/Variant_Combat",
			"YAAC/Variant_Combat/AI",
			"YAAC/Variant_Combat/Animation",
			"YAAC/Variant_Combat/Gameplay",
			"YAAC/Variant_Combat/Interfaces",
			"YAAC/Variant_Combat/UI",
			"YAAC/Variant_SideScrolling",
			"YAAC/Variant_SideScrolling/AI",
			"YAAC/Variant_SideScrolling/Gameplay",
			"YAAC/Variant_SideScrolling/Interfaces",
			"YAAC/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
