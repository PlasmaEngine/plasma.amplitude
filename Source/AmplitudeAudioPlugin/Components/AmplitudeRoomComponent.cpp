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

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/Components/AmplitudeRoomComponent.h>
#include <AmplitudeAudioPlugin/Core/Common.h>
#include <AudioSystem/Core/AudioSystemRequests.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

constexpr plTypeVersion kVersion_AmplitudeRoomComponent = 1;

// clang-format off
PL_BEGIN_COMPONENT_TYPE(plAmplitudeRoomComponent, kVersion_AmplitudeRoomComponent, plComponentMode::Static)
{
  PL_BEGIN_ATTRIBUTES
  {
    new plBoxManipulatorAttribute("HalfExtends", 2.0f, true),
    new plBoxVisualizerAttribute("HalfExtends", 2.0f, plColor::White, "Color"),
  }
  PL_END_ATTRIBUTES;

  PL_BEGIN_PROPERTIES
  {
    PL_MEMBER_PROPERTY("Color", m_ShapeColor),
    PL_ACCESSOR_PROPERTY("HalfExtends", GetHalfExtends, SetHalfExtends)->AddAttributes(new plDefaultValueAttribute(plVec3(0.5f)), new plClampValueAttribute(0.1f, plVariant()), new plSuffixAttribute(" m")),
    PL_MEMBER_PROPERTY("RoomID", m_uiRoomID)->AddAttributes(new plClampValueAttribute(1, {})),
    PL_ACCESSOR_PROPERTY("Gain", GetGain, SetGain)->AddAttributes(new plDefaultValueAttribute(1.0f), new plClampValueAttribute(0.0f, 1.0f)),

    PL_RESOURCE_MEMBER_PROPERTY("LeftWallMaterial", m_hLeftWallMaterial)->AddAttributes(new plAssetBrowserAttribute("CompatibleAsset_CustomData", "plAmplitudeRoomMaterial")),
    PL_RESOURCE_MEMBER_PROPERTY("RightWallMaterial", m_hRightWallMaterial)->AddAttributes(new plAssetBrowserAttribute("CompatibleAsset_CustomData", "plAmplitudeRoomMaterial")),
    PL_RESOURCE_MEMBER_PROPERTY("FrontWallMaterial", m_hFrontWallMaterial)->AddAttributes(new plAssetBrowserAttribute("CompatibleAsset_CustomData", "plAmplitudeRoomMaterial")),
    PL_RESOURCE_MEMBER_PROPERTY("BackWallMaterial", m_hBackWallMaterial)->AddAttributes(new plAssetBrowserAttribute("CompatibleAsset_CustomData", "plAmplitudeRoomMaterial")),
    PL_RESOURCE_MEMBER_PROPERTY("CeilingMaterial", m_hCeilingMaterial)->AddAttributes(new plAssetBrowserAttribute("CompatibleAsset_CustomData", "plAmplitudeRoomMaterial")),
    PL_RESOURCE_MEMBER_PROPERTY("FloorMaterial", m_hFloorMaterial)->AddAttributes(new plAssetBrowserAttribute("CompatibleAsset_CustomData", "plAmplitudeRoomMaterial")),
  }
  PL_END_PROPERTIES;
}
PL_END_COMPONENT_TYPE;
// clang-format on

void plAmplitudeRoomComponent::Initialize()
{
  SUPER::Initialize();
}

void plAmplitudeRoomComponent::Deinitialize()
{
  SUPER::Deinitialize();
}

void plAmplitudeRoomComponent::OnActivated()
{
  SUPER::OnActivated();

  if (amEngine->IsInitialized())
    InitializeRoom();
  else
    amEngine->OnNextFrame([this](double)
      { InitializeRoom(); });
}

void plAmplitudeRoomComponent::OnDeactivated()
{
  if (m_Room.Valid())
    amEngine->RemoveRoom(m_uiRoomID);

  plAmplitudeComponent::OnDeactivated();
}

void plAmplitudeRoomComponent::SerializeComponent(plWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  s.WriteVersion(kVersion_AmplitudeRoomComponent);

  s << m_uiRoomID;
  s << m_vHalfExtends;

  s << m_hLeftWallMaterial;
  s << m_hRightWallMaterial;
  s << m_hFrontWallMaterial;
  s << m_hBackWallMaterial;
  s << m_hCeilingMaterial;
  s << m_hFloorMaterial;
}

void plAmplitudeRoomComponent::DeserializeComponent(plWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();

  s.ReadVersion(kVersion_AmplitudeRoomComponent);

  s >> m_uiRoomID;
  s >> m_vHalfExtends;

  s >> m_hLeftWallMaterial;
  s >> m_hRightWallMaterial;
  s >> m_hFrontWallMaterial;
  s >> m_hBackWallMaterial;
  s >> m_hCeilingMaterial;
  s >> m_hFloorMaterial;

  if (m_uiRoomID == kAmInvalidObjectId)
  {
    plRandom r;
    r.InitializeFromCurrentTime();
    m_uiRoomID = r.UInt();
  }
}

plAmplitudeRoomComponent::plAmplitudeRoomComponent()
  : m_uiRoomID(kAmInvalidObjectId)
  , m_fGain(1.0f)
{
}

const plVec3& plAmplitudeRoomComponent::GetHalfExtends() const
{
  return m_vHalfExtends;
}

void plAmplitudeRoomComponent::SetHalfExtends(const plVec3& vHalfExtends)
{
  m_vHalfExtends = vHalfExtends;
}

AmRoomID plAmplitudeRoomComponent::GetRoomID() const
{
  return m_uiRoomID;
}

AmReal32 plAmplitudeRoomComponent::GetGain() const
{
  return m_fGain;
}

void plAmplitudeRoomComponent::SetGain(AmReal32 fGain)
{
  m_fGain = fGain;
}

plAmplitudeRoomMaterialResourceHandle plAmplitudeRoomComponent::GetLeftWallMaterial() const
{
  return m_hLeftWallMaterial;
}

plAmplitudeRoomMaterialResourceHandle plAmplitudeRoomComponent::GetRightWallMaterial() const
{
  return m_hRightWallMaterial;
}

plAmplitudeRoomMaterialResourceHandle plAmplitudeRoomComponent::GetFrontWallMaterial() const
{
  return m_hFrontWallMaterial;
}

plAmplitudeRoomMaterialResourceHandle plAmplitudeRoomComponent::GetBackWallMaterial() const
{
  return m_hBackWallMaterial;
}

plAmplitudeRoomMaterialResourceHandle plAmplitudeRoomComponent::GetCeilingMaterial() const
{
  return m_hCeilingMaterial;
}

plAmplitudeRoomMaterialResourceHandle plAmplitudeRoomComponent::GetFloorMaterial() const
{
  return m_hFloorMaterial;
}

void plAmplitudeRoomComponent::Update()
{
  if (!m_Room.Valid())
    return;

  const plGameObject* pOwner = GetOwner();

  m_Room.SetDimensions(Utils::plVec3ToAmVec3(m_vHalfExtends * 2.0f));
  m_Room.SetGain(m_fGain);
  m_Room.SetLocation(Utils::plVec3ToAmVec3(pOwner->GetGlobalPosition()));
  m_Room.SetOrientation(Orientation(Utils::plVec3ToAmVec3(pOwner->GetGlobalDirForwards()), Utils::plVec3ToAmVec3(pOwner->GetGlobalDirUp())));

  const auto updateMaterial = [this](RoomWall eWall, plAmplitudeRoomMaterialResourceHandle hMaterial)
  {
    plResourceLock pWallMaterial(hMaterial, plResourceAcquireMode::AllowLoadingFallback_NeverFail);
    if (pWallMaterial.GetAcquireResult() == plResourceAcquireResult::Final)
    {
      const plAmplitudeRoomMaterial* pMaterial = pWallMaterial->GetData();

      RoomMaterial material;
      plMemoryUtils::Copy(material.m_absorptionCoefficients, pMaterial->m_absorptionCoefficients.GetData(), 9);

      m_Room.SetWallMaterial(eWall, material);
    }
  };

  updateMaterial(RoomWall::Left, m_hLeftWallMaterial);
  updateMaterial(RoomWall::Right, m_hRightWallMaterial);
  updateMaterial(RoomWall::Front, m_hFrontWallMaterial);
  updateMaterial(RoomWall::Back, m_hBackWallMaterial);
  updateMaterial(RoomWall::Ceiling, m_hCeilingMaterial);
  updateMaterial(RoomWall::Floor, m_hFloorMaterial);
}

void plAmplitudeRoomComponent::InitializeRoom()
{
  m_Room = amEngine->AddRoom(m_uiRoomID);

  Update();
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

PL_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Components_AmplitudeRoomComponent);
