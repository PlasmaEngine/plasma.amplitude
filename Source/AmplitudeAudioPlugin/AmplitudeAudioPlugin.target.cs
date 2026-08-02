using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class AmplitudeAudioPluginTarget : TargetRules
{
    public AmplitudeAudioPluginTarget(BuildContext context) : base(context)
    {
        Type = TargetType.SharedLibrary;
        OutputName = "plAmplitudeAudioPlugin";
        OutputDirectory = PlasmaPackageSdk.PackageBinaryDirectory(context);

        UsePCHFiles = true;
        UseUnityBuild = true;
        UseAdaptiveUnityBuild = true;
        UseIncrementalLinking = true;

        ExtraModules.Add("AmplitudeAudioPluginModule");
    }
}
