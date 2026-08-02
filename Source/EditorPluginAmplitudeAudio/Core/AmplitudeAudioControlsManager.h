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

#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>
#include <AudioSystem/Core/AudioSystemData.h>

#include <Foundation/Configuration/Singleton.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio;

class plJSONReader;

class PL_EDITORPLUGINAMPLITUDEAUDIO_DLL plAmplitudeAudioControlsManager
{
  PL_DECLARE_SINGLETON(plAmplitudeAudioControlsManager);

public:
  plAmplitudeAudioControlsManager();
  ~plAmplitudeAudioControlsManager() = default;

  /// \brief Create controls assets needed for Amplitude Audio.
  /// \return PL_SUCCESS on success, otherwise PL_FAILURE.
  plResult ReloadControls();

private:
  plResult SerializeTriggerControl(plStreamWriter* pStream, const plAudioSystemTriggerData* pControlData);
  plResult SerializeRtpcControl(plStreamWriter* pStream, const plAudioSystemRtpcData* pControlData);
  plResult SerializeSwitchStateControl(plStreamWriter* pStream, const plAudioSystemSwitchStateData* pControlData);
  plResult SerializeEnvironmentControl(plStreamWriter* pStream, const plAudioSystemEnvironmentData* pControlData);
  plResult SerializeSoundBankControl(plStreamWriter* pStream, const plAudioSystemBankData* pControlData);

  plResult CreateTriggerControl(const char* szControlName, const plAudioSystemTriggerData* pControlData);
  plResult CreateRtpcControl(const char* szControlName, const plAudioSystemRtpcData* pControlData);
  plResult CreateSwitchStateControl(const char* szControlName, const plAudioSystemSwitchStateData* pControlData);
  plResult CreateEnvironmentControl(const char* szControlName, const plAudioSystemEnvironmentData* pControlData);
  plResult CreateSoundBankControl(const char* szControlName, const plAudioSystemBankData* pControlData);

  plResult LoadSoundBanks(const char* szRootFolder, const char* szSubPath);
  //  void LoadBuses(const char* sRootFolder);
  plResult LoadControlsInFolder(const char* szFolderPath, const plEnum<plAmplitudeAudioControlType>& eType);
  plResult LoadControl(const plVariantDictionary& json, const plEnum<plAmplitudeAudioControlType>& eType);
};
