using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class EditorPluginAmplitudeAudioModule : ModuleRules
{
    public EditorPluginAmplitudeAudioModule(BuildContext context) : base(context)
    {
        PlasmaPackageSdk.ConfigurePackageModule(this, context, "Source/EditorPluginAmplitudeAudio",
            "EditorPluginAmplitudeAudioPCH.h", "BUILDSYSTEM_BUILDING_EDITORPLUGINAMPLITUDEAUDIO_LIB");

        PlasmaPackageSdk.ConfigureEditorSdkConsumer(this, context);

        // Includes and defines only. The runtime plugin already ships Amplitude.dll, so copying it
        // a second time here would just race two identical files into the same directory.
        PlasmaPackageAmplitude.ConfigureConsumer(this, context, bCopyRuntime: false);

        PlasmaPackageSdk.AddPackagePluginImport(this, context, "Source/AmplitudeAudioPlugin",
            "plAmplitudeAudioPlugin");

        PlasmaPackageQt.ConfigureQtModule(this, context, "EditorPluginAmplitudeAudioModule",
            "Source/EditorPluginAmplitudeAudio", "Core", "Gui", "Widgets");
    }
}
