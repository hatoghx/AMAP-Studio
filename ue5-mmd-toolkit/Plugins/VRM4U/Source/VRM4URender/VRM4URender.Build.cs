
using UnrealBuildTool;

public class VRM4URender : ModuleRules
{
	public VRM4URender(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.Combine(GetModuleDirectory("Renderer"), "Private"), //required for FPostProcessMaterialInputs
				System.IO.Path.Combine(GetModuleDirectory("Renderer"), "Internal"), //required for FPostProcessMaterialInputs
			});



		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
                "Engine",
				"Projects",
				"RenderCore",

                "RHI",
                //"ShaderCore",
                "Renderer",
				"VRM4U",
				// AMAP5 build fix: VrmCustomStencilNotification.cpp uses
				// FSlateNotificationManager (Slate). Listing Slate/SlateCore only
				// under bBuildEditor left Slate.lib off this project's link line.
				"Slate",
				"SlateCore",
			});
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
				});
		}
	}
}
