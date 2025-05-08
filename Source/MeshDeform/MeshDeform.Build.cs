// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MeshDeform : ModuleRules
{
	public MeshDeform(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GeometryCore", "GeometryScriptingCore", "DynamicMesh", "GeometryFramework" });
	}
}
