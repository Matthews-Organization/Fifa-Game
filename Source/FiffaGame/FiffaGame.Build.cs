// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FiffaGame : ModuleRules
{
	public FiffaGame(ReadOnlyTargetRules Target) : base(Target)
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
			"OpenCV",
			"OpenCVHelper"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FiffaGame",
			"FiffaGame/Variant_Horror",
			"FiffaGame/Variant_Horror/UI",
			"FiffaGame/Variant_Shooter",
			"FiffaGame/Variant_Shooter/AI",
			"FiffaGame/Variant_Shooter/UI",
			"FiffaGame/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
