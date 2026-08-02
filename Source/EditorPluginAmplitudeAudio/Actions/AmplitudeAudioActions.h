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

#pragma once

#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>

#include <Foundation/Configuration/CVar.h>

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class plPreferences;

class PL_EDITORPLUGINAMPLITUDEAUDIO_DLL plAmplitudeAudioActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapMenuActions(const char* szMapping);
  static void MapToolbarActions(const char* szMapping);

  static plActionDescriptorHandle s_hCategoryAudioSystem;
  static plActionDescriptorHandle s_hProjectSettings;
  static plActionDescriptorHandle s_hReloadControls;
};


class PL_EDITORPLUGINAMPLITUDEAUDIO_DLL plAmplitudeAudioAction : public plButtonAction
{
  PL_ADD_DYNAMIC_REFLECTION(plAmplitudeAudioAction, plButtonAction);

public:
  enum class ActionType
  {
    ProjectSettings,
    ReloadControls
  };

  plAmplitudeAudioAction(const plActionContext& context, const char* szName, ActionType type);
  ~plAmplitudeAudioAction() override;

  void Execute(const plVariant& value) override;

  ActionType m_Type;
};
