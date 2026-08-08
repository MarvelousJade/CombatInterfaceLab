using UnrealBuildTool;
using System.Collections.Generic;

public class CombatInterfaceLabEditorTarget : TargetRules
{
    public CombatInterfaceLabEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("CombatInterfaceLab");
    }
}
