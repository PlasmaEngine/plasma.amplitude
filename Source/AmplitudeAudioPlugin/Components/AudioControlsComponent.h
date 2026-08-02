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

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginDLL.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeComponent.h>
#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>
#include <AudioSystem/Core/AudioSystemData.h>

typedef plComponentManager<class plAudioControlsComponent, plBlockStorageType::FreeList> plAudioControlsComponentManager;

/// \brief Component used to load and unload a set of audio controls.
///
/// The audio controls are provided by the selected audio control collection. Loaded audio
/// controls can be automatically registered to the audio system at component activation.
class PL_AMPLITUDEAUDIOPLUGIN_DLL plAudioControlsComponent : public plAmplitudeComponent
{
  PL_DECLARE_COMPONENT_TYPE(plAudioControlsComponent, plAmplitudeComponent, plAudioControlsComponentManager);

  // plComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void SerializeComponent(plWorldWriter& stream) const override;
  void DeserializeComponent(plWorldReader& stream) override;

  // plAmplitudeComponent

private:
  void plAmplitudeComponentIsAbstract() override {}

  // plAudioControlsComponent

public:
  plAudioControlsComponent();
  ~plAudioControlsComponent() override;

  /// \brief Load the audio controls from the given collection.
  /// This is automatically called on component initialization when
  /// the AutoLoad property is set to true.
  bool Load();

  /// \brief Unloads the audio controls.
  bool Unload();

private:
  plString m_sControlsAsset;
  bool m_bAutoLoad;

  bool m_bLoaded;
  plAudioControlCollectionResourceHandle m_hControlsResource;
};
