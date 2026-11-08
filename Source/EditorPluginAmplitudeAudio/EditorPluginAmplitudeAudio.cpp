// Copyright (c) 2022-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Actions/AmplitudeAudioActions.h>

#include <EditorFramework/Actions/AssetActions.h>
#include <EditorFramework/Actions/ProjectActions.h>

#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/StandardMenus.h>

#include <EditorUi/Widgets.h>
#include <Foundation/Configuration/Plugin.h>

namespace
{
  /// The package's icon folder: the icons under their ids (":/AssetIcons/Audio_Control_Collection.svg" is
  /// "EditorIcons/AssetIcons/Audio_Control_Collection.svg"). The plugin binary lives in <package>/Bin/<configuration>.
  plString s_sIconFolder;

  void RegisterIconFolder()
  {
    plStringBuilder sPlugin, sCopy;
    plPlugin::GetPluginPaths("plEditorPluginAmplitudeAudio", sPlugin, sCopy, 0);

    plStringBuilder sFolder = sPlugin.GetFileDirectory();
    sFolder.AppendPath("..", "..", "EditorIcons");
    sFolder.MakeCleanPath();

    s_sIconFolder = sFolder;
    plEditorUi::AddIconFolder(s_sIconFolder);
  }

  void UnregisterIconFolder()
  {
    plEditorUi::RemoveIconFolder(s_sIconFolder);
    s_sIconFolder.Clear();
  }
} // namespace

// BEGIN-DOCS-CODE-SNIPPET: plugin-setup
PL_PLUGIN_ON_LOADED()
{
  RegisterIconFolder();

  plAmplitudeAudioActions::RegisterActions();

  // Control Collection
  {
    // Menu Bar
    plActionMapManager::RegisterActionMap("AudioControlCollectionAssetMenuBar").IgnoreResult();
    plStandardMenus::MapActions("AudioControlCollectionAssetMenuBar", plStandardMenuTypes::Default | plStandardMenuTypes::Edit);
    plProjectActions::MapActions("AudioControlCollectionAssetMenuBar");
    plDocumentActions::MapMenuActions("AudioControlCollectionAssetMenuBar");
    plCommandHistoryActions::MapActions("AudioControlCollectionAssetMenuBar");

    // Tool Bar
    {
      plActionMapManager::RegisterActionMap("AudioControlCollectionAssetToolBar").IgnoreResult();
      plDocumentActions::MapToolbarActions("AudioControlCollectionAssetToolBar");
      plCommandHistoryActions::MapActions("AudioControlCollectionAssetToolBar", "");
      plAssetActions::MapToolBarActions("AudioControlCollectionAssetToolBar", true);
      plAmplitudeAudioActions::MapToolbarActions("AudioControlCollectionAssetToolBar");
    }
  }

  // Scene
  {
    // Menu Bar
    {
      plAmplitudeAudioActions::MapMenuActions("EditorPluginScene_DocumentMenuBar");
      plAmplitudeAudioActions::MapMenuActions("EditorPluginScene_Scene2MenuBar");
      plAmplitudeAudioActions::MapToolbarActions("EditorPluginScene_DocumentToolBar");
      plAmplitudeAudioActions::MapToolbarActions("EditorPluginScene_Scene2ToolBar");
    }
  }
}

PL_PLUGIN_ON_UNLOADED()
{
  plAmplitudeAudioActions::UnregisterActions();

  UnregisterIconFolder();
}
// END-DOCS-CODE-SNIPPET
