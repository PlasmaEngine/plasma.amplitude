using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class AmplitudeAudioPluginModule : ModuleRules
{
    public AmplitudeAudioPluginModule(BuildContext context) : base(context)
    {
        PlasmaPackageSdk.ConfigurePackageModule(this, context, "Source/AmplitudeAudioPlugin",
            "AmplitudeAudioPluginPCH.h", "BUILDSYSTEM_BUILDING_AMPLITUDEAUDIOPLUGIN_LIB");

        // Copies Amplitude.dll next to the plugin - this is the module that ships it.
        PlasmaPackageAmplitude.ConfigureConsumer(this, context, bCopyRuntime: true);
    }
}
