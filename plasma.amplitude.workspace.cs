// #pl-version 1

using PlasmaBuild.Core.Configuration;
using PlasmaBuild.Core.Rules;

public class PlasmaAmplitudeWorkspace : WorkspaceRules
{
    public PlasmaAmplitudeWorkspace(BuildContext context) : base(context)
    {
        TargetNames.Add("AmplitudeAudioPlugin");
        TargetNames.Add("EditorPluginAmplitudeAudio");
    }
}
