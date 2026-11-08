#pragma once

#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>
#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>

/// \brief What the Amplitude Audio Settings dialog edits: the Amplitude project and the platform
/// configurations in Audio/AudioSystemConfig.ddl, the init banks and engine configs found in the project's build
/// folder, and the prompt, the file picker and the save question around them.
class plAmplitudeAudioSettings
{
public:
  /// Reads the Amplitude part of the configuration file and the files of its project.
  void Load();

  /// The first platform in name order; empty without platforms.
  plString GetFirstPlatform() const;

  /// OK: when the project or a configuration changed, asks whether to save them ("No" closes without saving) and
  /// saves on "Yes". Returns false when saving failed, so the dialog stays open.
  bool AskAndSave() const;

  /// "Add Platform": asks for a name. A new platform starts with the first init bank and the first engine config of the
  /// project's build folder (empty when there are none); an existing one is only returned. Returns the name, or an
  /// empty string when cancelled.
  plString AskAndAddPlatform();

  /// "Browse...": picks an Amplitude project file. Returns false when cancelled.
  static bool PickProject(plString& out_sPath);

  /// Sets the project and reads the init banks and engine configs of its build folder.
  void SetProjectPath(plStringView sPath);

  plAmplitudeAssetProfiles m_Configs;

  /// The init banks (<project>/build/soundbanks/*.ambank) and the engine configs (<project>/build/*.amconfig).
  plHybridArray<plString, 16> m_Banks;
  plHybridArray<plString, 16> m_ConfigFiles;

private:
  /// Writes the file. A configuration that cannot be written (no platforms) leaves the file as it was.
  plResult Save() const;
  void FindProjectFiles();
  plAmplitudeConfiguration MakeNewPlatform() const;

  plAmplitudeAssetProfiles m_ConfigsOld;
};
