#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Dialogs/AmplitudeAudioSettings.h>

// Common.h needs the Amplitude types from AmplitudeAudioSettings.h
#include <AmplitudeAudioPlugin/Core/Common.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <GuiFoundation/Dialogs/FileDialog.h>
#include <GuiFoundation/EngineUi/EnginePrompt.h>

namespace
{
  constexpr const char* s_szConfigFile = ":project/Audio/AudioSystemConfig.ddl";
} // namespace

void plAmplitudeAudioSettings::Load()
{
  plFileReader file;
  plOpenDdlReader ddl;
  if (file.Open(s_szConfigFile).Succeeded() && ddl.ParseDocument(file).Succeeded())
  {
    for (const plOpenDdlReaderElement* pChild = ddl.GetRootElement()->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
    {
      if (pChild->IsCustomType("Middleware") && pChild->HasName() && pChild->GetName().Compare(s_szAmplitudeMiddlewareName) == 0)
      {
        if (m_Configs.Load(*pChild).Failed())
          plLog::Error("Failed to load configuration for audio middleware: {0}.", pChild->GetName());

        break;
      }
    }
  }

  m_ConfigsOld = m_Configs;
  FindProjectFiles();
}

plString plAmplitudeAudioSettings::GetFirstPlatform() const
{
  if (m_Configs.m_AssetProfiles.IsEmpty())
    return {};

  return m_Configs.m_AssetProfiles.GetIterator().Key();
}

bool plAmplitudeAudioSettings::AskAndSave() const
{
  if (m_ConfigsOld.m_AssetProfiles == m_Configs.m_AssetProfiles && m_ConfigsOld.m_sProjectPath == m_Configs.m_sProjectPath)
    return true;

  if (plEditorUiServices::MessageBoxQuestion("Save the changes to the Amplitude Audio configuration?\nYou need to reload the project for the changes to take effect.", plEditorUiMessageButton::Yes | plEditorUiMessageButton::No, plEditorUiMessageButton::No) != plEditorUiMessageButton::Yes)
    return true;

  plEditorApp::GetSingleton()->AddReloadProjectRequiredReason("Amplitude Audio configuration was modified.");

  return Save().Succeeded();
}

plResult plAmplitudeAudioSettings::Save() const
{
  // the document goes to memory first, so a configuration that cannot be saved never cuts the file off
  plDefaultMemoryStreamStorage storage;
  plMemoryStreamWriter memory(&storage);

  plOpenDdlWriter ddl;
  ddl.SetOutputStream(&memory);

  ddl.BeginObject("Middleware", s_szAmplitudeMiddlewareName);
  const plResult result = m_Configs.Save(ddl);
  ddl.EndObject();

  plFileWriter file;
  if (result.Failed() || file.Open(s_szConfigFile).Failed() || storage.CopyToStream(file).Failed())
  {
    plEditorUiServices::MessageBoxWarning("Failed to save the Audio System configuration file\n'>project/Audio/AudioSystemConfig.ddl'");
    return PL_FAILURE;
  }

  return PL_SUCCESS;
}

plString plAmplitudeAudioSettings::AskAndAddPlatform()
{
  plStringBuilder sName = plEnginePrompt::GetText("Add Platform", "Platform Name:");
  sName.Trim(" \t\r\n");

  if (sName.IsEmpty())
    return {};

  const plString sPlatform = sName;

  if (!m_Configs.m_AssetProfiles.Contains(sPlatform))
    m_Configs.m_AssetProfiles.Insert(sPlatform, MakeNewPlatform());

  return sPlatform;
}

plAmplitudeConfiguration plAmplitudeAudioSettings::MakeNewPlatform() const
{
  plAmplitudeConfiguration config;

  if (!m_Banks.IsEmpty())
    config.m_sInitSoundBank = m_Banks[0];

  if (!m_ConfigFiles.IsEmpty())
    config.m_sEngineConfigFileName = m_ConfigFiles[0];

  return config;
}

bool plAmplitudeAudioSettings::PickProject(plString& out_sPath)
{
  plStringBuilder projectPath;
  if (plFileSystem::ResolvePath(":project/Audio/Amplitude/", &projectPath, nullptr).Failed())
  {
    projectPath = plOSFile::GetUserDocumentsFolder();
  }

  plStringBuilder sSelected = plFileDialog::GetOpenFileName("Select Amplitude Project", projectPath, "Amplitude Project (*.amproject)");

  if (sSelected.IsEmpty())
    return false;

  // the configuration stores the path with the platform's separators
  sSelected.MakePathSeparatorsNative();

  out_sPath = sSelected;
  return true;
}

void plAmplitudeAudioSettings::SetProjectPath(plStringView sPath)
{
  m_Configs.m_sProjectPath = sPath;
  FindProjectFiles();
}

void plAmplitudeAudioSettings::FindProjectFiles()
{
  m_Banks.Clear();
  m_ConfigFiles.Clear();

  if (m_Configs.m_sProjectPath.IsEmpty())
    return;

  plStringBuilder basePath(plPathUtils::GetFileDirectory(m_Configs.m_sProjectPath));
  basePath.AppendPath("build");

  plStringBuilder banksPath(basePath);
  banksPath.AppendPath(kSoundBanksFolder);

  plFileSystemIterator fsIt;
  for (fsIt.StartSearch(basePath, plFileSystemIteratorFlags::ReportFiles); fsIt.IsValid(); fsIt.Next())
  {
    plStringBuilder fileName = fsIt.GetStats().m_sName;
    if (fileName.HasExtension(".amconfig"))
    {
      m_ConfigFiles.PushBack(fileName);
    }
  }

  for (fsIt.StartSearch(banksPath, plFileSystemIteratorFlags::ReportFiles); fsIt.IsValid(); fsIt.Next())
  {
    plStringBuilder fileName = fsIt.GetStats().m_sName;
    if (fileName.HasExtension(".ambank"))
    {
      m_Banks.PushBack(fileName);
    }
  }
}
