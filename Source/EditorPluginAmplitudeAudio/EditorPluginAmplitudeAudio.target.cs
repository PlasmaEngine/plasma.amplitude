using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class EditorPluginAmplitudeAudioTarget : TargetRules
{
    public EditorPluginAmplitudeAudioTarget(BuildContext context) : base(context)
    {
        Type = TargetType.SharedLibrary;
        OutputName = "plEditorPluginAmplitudeAudio";
        OutputDirectory = PlasmaPackageSdk.PackageBinaryDirectory(context);

        UsePCHFiles = true;
        UseUnityBuild = true;
        UseAdaptiveUnityBuild = true;
        UseIncrementalLinking = true;

        TargetDependencies.Add("AmplitudeAudioPlugin");

        ExtraModules.Add("EditorPluginAmplitudeAudioModule");
    }
}
