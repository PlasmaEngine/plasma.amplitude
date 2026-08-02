// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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
#include <AmplitudeAudioPlugin/Resources/AmplitudeRoomMaterialResource.h>

#include <AudioSystem/Components/AudioSystemComponent.h>

#include <SparkyStudios/Audio/Amplitude/Core/Room.h>

typedef plAudioSystemComponentManager<class plAmplitudeRoomComponent> plAmplitudeRoomComponentManager;

/// \brief Component that applies environmental effects in a box shape.
class PL_AMPLITUDEAUDIOPLUGIN_DLL plAmplitudeRoomComponent : public plAmplitudeComponent
{
  PL_DECLARE_COMPONENT_TYPE(plAmplitudeRoomComponent, plAmplitudeComponent, plAmplitudeRoomComponentManager);

  // plComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void OnActivated() override;
  void OnDeactivated() override;
  void SerializeComponent(plWorldWriter& stream) const override;
  void DeserializeComponent(plWorldReader& stream) override;

  // plAmplitudeComponent

protected:
  void plAmplitudeComponentIsAbstract() override {}

  // plAmplitudeRoomComponent

public:
  plAmplitudeRoomComponent();

  /// \brief Gets the radius of the sphere that
  /// specifies the environment.
  PL_NODISCARD const plVec3& GetHalfExtends() const;

  /// \brief Sets the radius of the sphere that
  /// specifies the environment.
  void SetHalfExtends(const plVec3& vHalfExtends);

  /// \brief Gets the room ID.
  PL_NODISCARD SparkyStudios::Audio::Amplitude::AmRoomID GetRoomID() const;

  /// \brief Gets the gain applied to the room.
  PL_NODISCARD SparkyStudios::Audio::Amplitude::AmReal32 GetGain() const;

  /// \brief Sets the gain applied to the room.
  void SetGain(SparkyStudios::Audio::Amplitude::AmReal32 fGain);

  /// \brief Gets the material resource handle for the left wall.
  PL_NODISCARD plAmplitudeRoomMaterialResourceHandle GetLeftWallMaterial() const;

  /// \brief Gets the material resource handle for the right wall.
  PL_NODISCARD plAmplitudeRoomMaterialResourceHandle GetRightWallMaterial() const;

  /// \brief Gets the material resource handle for the front wall.
  PL_NODISCARD plAmplitudeRoomMaterialResourceHandle GetFrontWallMaterial() const;

  /// \brief Gets the material resource handle for the back wall.
  PL_NODISCARD plAmplitudeRoomMaterialResourceHandle GetBackWallMaterial() const;

  /// \brief Gets the material resource handle for the ceiling.
  PL_NODISCARD plAmplitudeRoomMaterialResourceHandle GetCeilingMaterial() const;

  /// \brief Gets the material resource handle for the floor.
  PL_NODISCARD plAmplitudeRoomMaterialResourceHandle GetFloorMaterial() const;

protected:
  void Update();

private:
  void InitializeRoom();

  plColor m_ShapeColor;

  plVec3 m_vHalfExtends;
  SparkyStudios::Audio::Amplitude::AmRoomID m_uiRoomID;
  SparkyStudios::Audio::Amplitude::AmReal32 m_fGain;

  plAmplitudeRoomMaterialResourceHandle m_hLeftWallMaterial;
  plAmplitudeRoomMaterialResourceHandle m_hRightWallMaterial;
  plAmplitudeRoomMaterialResourceHandle m_hFrontWallMaterial;
  plAmplitudeRoomMaterialResourceHandle m_hBackWallMaterial;
  plAmplitudeRoomMaterialResourceHandle m_hCeilingMaterial;
  plAmplitudeRoomMaterialResourceHandle m_hFloorMaterial;

  SparkyStudios::Audio::Amplitude::Room m_Room;
};
