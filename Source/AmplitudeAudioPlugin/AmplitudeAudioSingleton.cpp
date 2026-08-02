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

#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>
#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>
#include <AmplitudeAudioPlugin/Core/Common.h>

#include <AudioSystem/Core/AudioSystemAllocator.h>

#include <Core/ResourceManager/ResourceManager.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>

#include <GameEngine/GameApplication/GameApplication.h>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/Core/Common/Constants.h>

using namespace SparkyStudios::Audio;

namespace Log
{
  static class Impl final : public Amplitude::Logger
  {
  protected:
    void Log(Amplitude::eLogMessageLevel level, const char* file, int line, const Amplitude::AmString& message) override
    {
      switch (level)
      {
        case Amplitude::eLogMessageLevel_Debug:
          plLog::Debug("[Amplitude] {0}", message.c_str());
          break;
        case Amplitude::eLogMessageLevel_Info:
          plLog::Info("[Amplitude] {0}", message.c_str());
          break;
        case Amplitude::eLogMessageLevel_Warning:
          plLog::Warning("[Amplitude] {0}", message.c_str());
          break;
        case Amplitude::eLogMessageLevel_Critical:
        case Amplitude::eLogMessageLevel_Error:
          plLog::Error("[Amplitude] {0}", message.c_str());
          break;
        case Amplitude::eLogMessageLevel_Success:
          plLog::Success("[Amplitude] {0}", message.c_str());
          break;
      }
    }
  } g_Logger;

  static void DeviceNotification(Amplitude::DeviceNotification notification, const Amplitude::DeviceDescription& device, Amplitude::Driver* driver)
  {
    switch (notification)
    {
      case Amplitude::DeviceNotification::Opened:
        plLog::Info("[Amplitude] Device opened: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Started:
        plLog::Info("[Amplitude] Device started: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Stopped:
        plLog::Info("[Amplitude] Device stopped: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Rerouted:
        plLog::Info("[Amplitude] Device rerouted: {0}", device.mDeviceName.c_str());
        break;
      case Amplitude::DeviceNotification::Closed:
        plLog::Info("[Amplitude] Device closed: {0}", device.mDeviceName.c_str());
        break;
    }
  }
} // namespace Log

namespace Memory
{
  class Impl final : public Amplitude::MemoryAllocator
  {
    Amplitude::AmVoidPtr Malloc([[maybe_unused]] Amplitude::eMemoryPoolKind pool, Amplitude::AmSize size) override
    {
      return plAudioMiddlewareAllocatorWrapper::GetAllocator()->Allocate(size, PL_AUDIOSYSTEM_MEMORY_ALIGNMENT);
    }

    Amplitude::AmVoidPtr Malign([[maybe_unused]] Amplitude::eMemoryPoolKind pool, Amplitude::AmSize size, Amplitude::AmUInt32 alignment) override
    {
      return plAudioMiddlewareAllocatorWrapper::GetAllocator()->Allocate(size, alignment);
    }

    Amplitude::AmVoidPtr Realloc(Amplitude::eMemoryPoolKind pool, Amplitude::AmVoidPtr address, Amplitude::AmSize size) override
    {
      if (address == nullptr)
        return Malloc(pool, size);

      return plAudioMiddlewareAllocatorWrapper::GetAllocator()->Reallocate(address, plAudioMiddlewareAllocatorWrapper::GetAllocator()->AllocatedSize(address), size, PL_AUDIOSYSTEM_MEMORY_ALIGNMENT);
    }

    Amplitude::AmVoidPtr Realign(Amplitude::eMemoryPoolKind pool, Amplitude::AmVoidPtr address, Amplitude::AmSize size, Amplitude::AmUInt32 alignment) override
    {
      if (address == nullptr)
        return Malign(pool, size, alignment);

      return plAudioMiddlewareAllocatorWrapper::GetAllocator()->Reallocate(address, plAudioMiddlewareAllocatorWrapper::GetAllocator()->AllocatedSize(address), size, alignment);
    }

    void Free([[maybe_unused]] Amplitude::eMemoryPoolKind pool, Amplitude::AmVoidPtr address) override
    {
      plAudioMiddlewareAllocatorWrapper::GetAllocator()->Deallocate(address);
    }

    Amplitude::AmSize SizeOf([[maybe_unused]] Amplitude::eMemoryPoolKind pool, Amplitude::AmVoidPtr address) override
    {
      return plAudioMiddlewareAllocatorWrapper::GetAllocator()->AllocatedSize(address);
    }
  };
} // namespace Memory

namespace Utils
{
  AmVec3 plVec3ToAmVec3(const plVec3& vec)
  {
    const Amplitude::CartesianCoordinateSystem pl(Amplitude::CartesianCoordinateSystem::Axis::PositiveY, Amplitude::CartesianCoordinateSystem::Axis::PositiveX, Amplitude::CartesianCoordinateSystem::Axis::PositiveZ);
    return Amplitude::CartesianCoordinateSystem::ConvertToDefault(AM_V3(vec.x, vec.y, vec.z), pl);
  }
} // namespace Utils

PL_IMPLEMENT_SINGLETON(plAmplitude);

void plAmplitudeConfiguration::Save(plOpenDdlWriter& ddl) const
{
  plOpenDdlUtils::StoreString(ddl, m_sInitSoundBank, s_szAmplitudeConfigKeyInitBank);
  plOpenDdlUtils::StoreString(ddl, m_sEngineConfigFileName, s_szAmplitudeConfigKeyEngineConfigFileName);
}

void plAmplitudeConfiguration::Load(const plOpenDdlReaderElement& ddl)
{
  if (const plOpenDdlReaderElement* pElement = ddl.FindChildOfType(plOpenDdlPrimitiveType::String, s_szAmplitudeConfigKeyInitBank))
    m_sInitSoundBank = pElement->GetPrimitivesString()[0];

  if (const plOpenDdlReaderElement* pElement = ddl.FindChildOfType(plOpenDdlPrimitiveType::String, s_szAmplitudeConfigKeyEngineConfigFileName))
    m_sEngineConfigFileName = pElement->GetPrimitivesString()[0];
}

bool plAmplitudeConfiguration::operator==(const plAmplitudeConfiguration& rhs) const
{
  if (m_sInitSoundBank != rhs.m_sInitSoundBank)
    return false;

  if (m_sEngineConfigFileName != rhs.m_sEngineConfigFileName)
    return false;

  return true;
}

plResult plAmplitudeAssetProfiles::Save(plOpenDdlWriter& writer) const
{
  if (m_AssetProfiles.IsEmpty())
    return PL_FAILURE;

  plOpenDdlUtils::StoreString(writer, m_sProjectPath, "ProjectPath");

  for (auto it = m_AssetProfiles.GetIterator(); it.IsValid(); ++it)
  {
    if (!it.Key().IsEmpty())
    {
      writer.BeginObject("Platform", it.Key());
      {
        it.Value().Save(writer);
      }
      writer.EndObject();
    }
  }

  return PL_SUCCESS;
}

plResult plAmplitudeAssetProfiles::Load(const plOpenDdlReaderElement& reader)
{
  if (const plOpenDdlReaderElement* pElement = reader.FindChildOfType(plOpenDdlPrimitiveType::String, "ProjectPath"))
    m_sProjectPath = pElement->GetPrimitivesString()[0];

  m_AssetProfiles.Clear();

  const plOpenDdlReaderElement* pChild = reader.GetFirstChild();

  while (pChild)
  {
    if (pChild->IsCustomType("Platform") && pChild->HasName())
    {
      auto& cfg = m_AssetProfiles[pChild->GetName()];

      cfg.Load(*pChild);
    }

    pChild = pChild->GetSibling();
  }

  return PL_SUCCESS;
}


plAmplitude::plAmplitude()
  : m_SingletonRegistrar(this)
  , m_dCurrentTime(0.0)
  , m_bInitialized(false)
{
  m_pData = PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), Data);
}

plAmplitude::~plAmplitude()
{
  // Shutdown().IgnoreResult();
}

plResult plAmplitude::LoadConfiguration(const plOpenDdlReaderElement& reader)
{
  return m_pData->m_Configs.Load(reader);
}

plResult plAmplitude::Startup()
{
  if (m_bInitialized)
    return PL_SUCCESS;

  // Initialize the engine
  Amplitude::Logger::SetLogger(&Log::g_Logger);
  Amplitude::RegisterDeviceNotificationCallback(Log::DeviceNotification);

  Amplitude::MemoryManager::Initialize(std::make_unique<Memory::Impl>());
  PL_ASSERT_DEBUG(Amplitude::MemoryManager::IsInitialized(), "Amplitude memory manager not initialized.");

  DetectPlatform();

  if (!m_pData->m_Configs.m_AssetProfiles.Contains(m_pData->m_sPlatform))
  {
    plLog::Error("Amplitude configuration for platform '{0}' not available. Amplitude will be deactivated.", m_pData->m_sPlatform);
    return PL_FAILURE;
  }

  const auto& config = m_pData->m_Configs.m_AssetProfiles[m_pData->m_sPlatform];

  plStringBuilder assetsPath;

  if (plFileSystem::ResolvePath(":project/Audio/Amplitude/" AMPLITUDE_ASSETS_DIR_NAME, &assetsPath, nullptr).Failed())
  {
    plLog::Error("No Amplitude assets directory available in the project. Amplitude will be deactivated.", m_pData->m_sPlatform);
    return PL_FAILURE;
  }

  // Register default plugins
  Amplitude::Engine::RegisterDefaultPlugins();

  PL_ASSERT_DEBUG(amEngine != nullptr, "Amplitude engine not available.");

  m_Loader.SetBasePath(AM_STRING_TO_OS_STRING(assetsPath.GetData()));
  amEngine->SetFileSystem(&m_Loader);

  // Wait for the file system to complete loading
  amEngine->StartOpenFileSystem();
  while (!amEngine->TryFinalizeOpenFileSystem())
    Amplitude::Thread::Sleep(1);

  const auto sPluginsPath = m_Loader.ResolvePath(AM_OS_STRING("plugins"));
  Amplitude::Engine::AddPluginSearchPath(sPluginsPath);

  // Auto load available plugins
  {
    plFileSystemIterator fsIt;
    for (fsIt.StartSearch(AM_OS_STRING_TO_STRING(sPluginsPath), plFileSystemIteratorFlags::ReportFiles); fsIt.IsValid(); fsIt.Next())
    {
      plStringBuilder fileName = fsIt.GetStats().m_sName;
#if PL_ENABLED(PL_COMPILE_FOR_DEVELOPMENT)
      if (fileName.EndsWith_NoCase("_d.dll"))
#else
      if (fileName.EndsWith_NoCase(".dll"))
#endif
      {
        plStringBuilder sTemp;
        Amplitude::Engine::LoadPlugin(AM_STRING_TO_OS_STRING(fileName.GetFileName().GetData(sTemp)));
      }
    }
  }

  if (!amEngine->Initialize(AM_STRING_TO_OS_STRING(config.m_sEngineConfigFileName.GetData())))
  {
    plLog::Error("Amplitude engine initialization failed with the config file '{0}'.", config.m_sEngineConfigFileName);
    return PL_FAILURE;
  }

  Amplitude::AmBankID uiInitBankId = Amplitude::kAmInvalidObjectId;

  if (!amEngine->LoadSoundBank(AM_STRING_TO_OS_STRING(config.m_sInitSoundBank.GetData()), uiInitBankId))
  {
    plLog::Error("Amplitude engine initialization failed. Could not load initial sound bank '{0}'.", config.m_sInitSoundBank);
    return PL_FAILURE;
  }

  // Wait for the sound files to complete loading
  amEngine->StartLoadSoundFiles();
  while (!amEngine->TryFinalizeLoadSoundFiles())
    Amplitude::Thread::Sleep(1);

  m_bInitialized = true;

  plLog::Success("Amplitude Audio initialized.");
  return PL_SUCCESS;
}

plResult plAmplitude::Shutdown()
{
  if (m_bInitialized)
  {
    m_bInitialized = false;

    if (amEngine != nullptr)
    {
      amEngine->Deinitialize();

      // Wait for the file system to complete closing
      amEngine->StartCloseFileSystem();
      while (!amEngine->TryFinalizeCloseFileSystem())
        Amplitude::Thread::Sleep(1);

      Amplitude::Engine::DestroyInstance();
    }

    Amplitude::Engine::UnregisterDefaultPlugins();

    // Terminate the Memory Manager
    if (Amplitude::MemoryManager::IsInitialized())
    {
      Amplitude::MemoryManager::Deinitialize();
    }

    // Finally delete all data
    m_pData.Clear();

    plLog::Success("Amplitude Audio deinitialized.");
  }

  return PL_SUCCESS;
}

plResult plAmplitude::Release()
{
  return PL_SUCCESS;
}

plResult plAmplitude::StopAllSounds()
{
  if (m_bInitialized)
    amEngine->StopAll();

  return PL_SUCCESS;
}

plResult plAmplitude::AddEntity(plAudioSystemEntityData* pEntityData, const char* szEntityName)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const Amplitude::Entity& entity = amEngine->AddEntity(pAmplitudeEntity->m_uiAmId);

  return entity.Valid() ? PL_SUCCESS : PL_FAILURE;
}

plResult plAmplitude::ResetEntity(plAudioSystemEntityData* pEntityData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  // Nothing to reset for now...

  return PL_SUCCESS;
}

plResult plAmplitude::UpdateEntity(plAudioSystemEntityData* pEntityData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  // Nothing to update for now... The real entity update is scheduled in the Amplitude's Update function.

  return PL_SUCCESS;
}

plResult plAmplitude::RemoveEntity(plAudioSystemEntityData* pEntityData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  amEngine->RemoveEntity(pAmplitudeEntity->m_uiAmId);
  const Amplitude::Entity& entity = amEngine->GetEntity(pAmplitudeEntity->m_uiAmId);

  return entity.Valid() ? PL_FAILURE : PL_SUCCESS;
}

plResult plAmplitude::SetEntityTransform(plAudioSystemEntityData* pEntityData, const plAudioSystemTransform& Transform)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  if (const Amplitude::Entity& entity = amEngine->GetEntity(pAmplitudeEntity->m_uiAmId); entity.Valid())
  {
    entity.SetLocation(Utils::plVec3ToAmVec3(Transform.m_vPosition));
    entity.SetOrientation(Amplitude::Orientation(Utils::plVec3ToAmVec3(Transform.m_vForward), Utils::plVec3ToAmVec3(Transform.m_vUp)));
  }

  return PL_SUCCESS;
}

plResult plAmplitude::LoadTrigger(plAudioSystemEntityData* pEntityData, const plAudioSystemTriggerData* pTriggerData, plAudioSystemEventData* pEventData)
{
  // Amplitude doesn't support loading triggers

  return PL_SUCCESS;
}

plResult plAmplitude::ActivateTrigger(plAudioSystemEntityData* pEntityData, const plAudioSystemTriggerData* pTriggerData, plAudioSystemEventData* pEventData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const auto* const pAmplitudeTrigger = plDynamicCast<const plAmplitudeAudioTriggerData*>(pTriggerData);
  if (pAmplitudeTrigger == nullptr)
    return PL_FAILURE;

  auto* pAmplitudeEvent = plDynamicCast<plAmplitudeAudioEventData*>(pEventData);
  if (pAmplitudeEvent == nullptr)
    return PL_FAILURE;

  plResult result = PL_FAILURE;

  Amplitude::AmEntityID entityId = 0;

  // if (amplitudeEntity->m_bHasPosition)
  // {
  // Positioned entities
  entityId = pAmplitudeEntity->m_uiAmId;
  // }

  const Amplitude::Entity& entity = amEngine->GetEntity(entityId);

#if PL_ENABLED(PL_COMPILE_FOR_DEVELOPMENT)
  if (entityId != Amplitude::kAmInvalidObjectId && !entity.Valid())
  {
    plLog::Error("Unable to find entity with ID: {0}", entityId);
  }
#endif

  if (Amplitude::EventHandle const event = amEngine->GetEventHandle(pAmplitudeTrigger->m_uiAmId))
  {
    if (const Amplitude::EventCanceler& canceler = amEngine->Trigger(event, entity); canceler.Valid())
    {
      pAmplitudeEvent->m_eState = plAudioSystemEventState::Playing;
      pAmplitudeEvent->m_EventCanceler = canceler;
      result = PL_SUCCESS;
    }
  }
  else
  {
    plLog::Error("[Amplitude] Unable to activate a trigger, the associated Amplitude event with ID {0} has not been found in loaded banks.", pAmplitudeTrigger->m_uiAmId);
  }

  return result;
}

plResult plAmplitude::UnloadTrigger(plAudioSystemEntityData* pEntityData, const plAudioSystemTriggerData* pTriggerData)
{
  // Amplitude doesn't support unloading triggers

  return PL_SUCCESS;
}

plResult plAmplitude::StopEvent(plAudioSystemEntityData* pEntityData, const plAudioSystemEventData* pEventData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const auto* const pAmplitudeEvent = plDynamicCast<const plAmplitudeAudioEventData*>(pEventData);
  if (pAmplitudeEvent == nullptr)
    return PL_FAILURE;

  plResult result = PL_FAILURE;

  switch (pAmplitudeEvent->m_eState)
  {
    case plAudioSystemEventState::Playing:
    {
      if (pAmplitudeEvent->m_EventCanceler.Valid())
      {
        pAmplitudeEvent->m_EventCanceler.Cancel();
        result = PL_SUCCESS;
        plLog::Success("[Amplitude] Successfully stopped the event.");
      }
      else
      {
        plLog::Error("[Amplitude] Encountered a running event without a valid canceler.");
      }
      break;
    }

    default:
    {
      plLog::Warning("[Amplitude] Stopping an event of this type is not supported yet");
      break;
    }
  }

  return result;
}

plResult plAmplitude::StopAllEvents(plAudioSystemEntityData* pEntityData)
{
  // TODO: Stop all events
  return PL_SUCCESS;
}

plResult plAmplitude::SetRtpc(plAudioSystemEntityData* pEntityData, const plAudioSystemRtpcData* pRtpcData, float fValue)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const auto* const pAmplitudeRtpc = plDynamicCast<const plAmplitudeAudioRtpcData*>(pRtpcData);
  if (pAmplitudeRtpc == nullptr)
    return PL_FAILURE;

  Amplitude::RtpcHandle const rtpc = amEngine->GetRtpcHandle(pAmplitudeRtpc->m_uiAmId);

  if (rtpc == nullptr)
    return PL_FAILURE;

  rtpc->SetValue(fValue);

  return PL_SUCCESS;
}

plResult plAmplitude::ResetRtpc(plAudioSystemEntityData* pEntityData, const plAudioSystemRtpcData* pRtpcData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const auto* const pAmplitudeRtpc = plDynamicCast<const plAmplitudeAudioRtpcData*>(pRtpcData);
  if (pAmplitudeRtpc == nullptr)
    return PL_FAILURE;

  Amplitude::RtpcHandle const rtpc = amEngine->GetRtpcHandle(pAmplitudeRtpc->m_uiAmId);

  if (rtpc == nullptr)
    return PL_FAILURE;

  rtpc->Reset();

  return PL_SUCCESS;
}

plResult plAmplitude::SetSwitchState(plAudioSystemEntityData* pEntityData, const plAudioSystemSwitchStateData* pSwitchStateData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const auto* const pAmplitudeSwitch = plDynamicCast<const plAmplitudeAudioSwitchStateData*>(pSwitchStateData);
  if (pAmplitudeSwitch == nullptr)
    return PL_FAILURE;

  Amplitude::SwitchHandle const _switch = amEngine->GetSwitchHandle(pAmplitudeSwitch->m_uiSwitchId);

  if (_switch == nullptr)
    return PL_FAILURE;

  _switch->SetState(pAmplitudeSwitch->m_uiSwitchStateId);

  return PL_SUCCESS;
}

plResult plAmplitude::SetObstructionAndOcclusion(plAudioSystemEntityData* pEntityData, float fObstruction, float fOcclusion)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const Amplitude::Entity& entity = amEngine->GetEntity(pAmplitudeEntity->m_uiAmId);

  if (!entity.Valid())
    return PL_FAILURE;

  entity.SetObstruction(fObstruction);
  entity.SetOcclusion(fOcclusion);

  return PL_SUCCESS;
}

plResult plAmplitude::SetEnvironmentAmount(plAudioSystemEntityData* pEntityData, const plAudioSystemEnvironmentData* pEnvironmentData, float fAmount)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeEntity = plDynamicCast<plAmplitudeAudioEntityData*>(pEntityData);
  if (pAmplitudeEntity == nullptr)
    return PL_FAILURE;

  const auto* const pAmplitudeEnvironment = plDynamicCast<const plAmplitudeAudioEnvironmentData*>(pEnvironmentData);
  if (pAmplitudeEnvironment == nullptr)
    return PL_FAILURE;

  const Amplitude::Environment& environment = amEngine->AddEnvironment(pAmplitudeEnvironment->m_uiAmId);
  environment.SetEffect(pAmplitudeEnvironment->m_uiEffectId);

  if (!environment.Valid())
    return PL_FAILURE;

  const Amplitude::Entity& entity = amEngine->GetEntity(pAmplitudeEntity->m_uiAmId);

  if (!entity.Valid())
    return PL_FAILURE;

  entity.SetEnvironmentFactor(environment.GetId(), fAmount);

  return PL_SUCCESS;
}

plResult plAmplitude::AddListener(plAudioSystemListenerData* pListenerData, const char* szListenerName, bool bIsDefault)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeListener = plDynamicCast<plAmplitudeAudioListenerData*>(pListenerData);
  if (pAmplitudeListener == nullptr)
    return PL_FAILURE;

  const Amplitude::Listener& listener = amEngine->AddListener(pAmplitudeListener->m_uiAmId);

  if (listener.Valid() && bIsDefault)
    amEngine->SetDefaultListener(listener.GetId());

  return listener.Valid() ? PL_SUCCESS : PL_FAILURE;
}

plResult plAmplitude::ResetListener(plAudioSystemListenerData* pListenerData)
{
  // Nothing to reset for now
  return PL_SUCCESS;
}

plResult plAmplitude::RemoveListener(plAudioSystemListenerData* pListenerData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeListener = plDynamicCast<plAmplitudeAudioListenerData*>(pListenerData);
  if (pAmplitudeListener == nullptr)
    return PL_FAILURE;

  amEngine->RemoveListener(pAmplitudeListener->m_uiAmId);
  const Amplitude::Listener& listener = amEngine->GetListener(pAmplitudeListener->m_uiAmId);

  return listener.Valid() ? PL_FAILURE : PL_SUCCESS;
}

plResult plAmplitude::SetListenerTransform(plAudioSystemListenerData* pListenerData, const plAudioSystemTransform& Transform)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeListener = plDynamicCast<plAmplitudeAudioListenerData*>(pListenerData);
  if (pAmplitudeListener == nullptr)
    return PL_FAILURE;

  if (const Amplitude::Listener& listener = amEngine->GetListener(pAmplitudeListener->m_uiAmId); listener.Valid())
  {
    listener.SetLocation(Utils::plVec3ToAmVec3(Transform.m_vPosition));
    listener.SetOrientation(Amplitude::Orientation(Utils::plVec3ToAmVec3(Transform.m_vForward), Utils::plVec3ToAmVec3(Transform.m_vUp)));
  }

  return PL_SUCCESS;
}

plResult plAmplitude::LoadBank(plAudioSystemBankData* pBankData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeBank = plDynamicCast<plAmplitudeAudioSoundBankData*>(pBankData);
  if (pAmplitudeBank == nullptr)
    return PL_FAILURE;

  if (!amEngine->LoadSoundBank(AM_STRING_TO_OS_STRING(pAmplitudeBank->m_sFileName.GetData())))
  {
    plLog::Error("[Amplitude] Could not load sound bank '{0}'.", pAmplitudeBank->m_sFileName);
    return PL_FAILURE;
  }

  return PL_SUCCESS;
}

plResult plAmplitude::UnloadBank(plAudioSystemBankData* pBankData)
{
  if (!m_bInitialized)
    return PL_FAILURE;

  const auto* const pAmplitudeBank = plDynamicCast<plAmplitudeAudioSoundBankData*>(pBankData);
  if (pAmplitudeBank == nullptr)
    return PL_FAILURE;

  if (pAmplitudeBank->m_uiAmId == Amplitude::kAmInvalidObjectId)
    return PL_FAILURE;

  amEngine->UnloadSoundBank(pAmplitudeBank->m_uiAmId);

  return PL_SUCCESS;
}

plAudioSystemEntityData* plAmplitude::CreateWorldEntity(plAudioSystemDataID uiEntityId)
{
  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioEntityData, uiEntityId, false);
}

plAudioSystemEntityData* plAmplitude::CreateEntityData(plAudioSystemDataID uiEntityId)
{
  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioEntityData, uiEntityId);
}

plResult plAmplitude::DestroyEntityData(plAudioSystemEntityData* pEntityData)
{
  if (pEntityData == nullptr || !pEntityData->IsInstanceOf<plAmplitudeAudioEntityData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pEntityData);
  return PL_SUCCESS;
}

plAudioSystemListenerData* plAmplitude::CreateListenerData(plAudioSystemDataID uiListenerId)
{
  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioListenerData, uiListenerId);
}

plResult plAmplitude::DestroyListenerData(plAudioSystemListenerData* pListenerData)
{
  if (pListenerData == nullptr || !pListenerData->IsInstanceOf<plAmplitudeAudioListenerData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pListenerData);
  return PL_SUCCESS;
}

plAudioSystemEventData* plAmplitude::CreateEventData(plAudioSystemDataID uiEventId)
{
  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioEventData, uiEventId);
}

plResult plAmplitude::ResetEventData(plAudioSystemEventData* pEventData)
{
  if (pEventData == nullptr || !pEventData->IsInstanceOf<plAmplitudeAudioEventData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pEventData);
  return PL_SUCCESS;
}

plResult plAmplitude::DestroyEventData(plAudioSystemEventData* pEventData)
{
  if (pEventData == nullptr || !pEventData->IsInstanceOf<plAmplitudeAudioEventData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pEventData);
  return PL_SUCCESS;
}

plAudioSystemBankData* plAmplitude::DeserializeBankEntry(plStreamReader* pBankEntry)
{
  if (pBankEntry == nullptr)
    return nullptr;

  Amplitude::AmBankID uiAmId;
  *pBankEntry >> uiAmId;

  plString sFileName;
  *pBankEntry >> sFileName;

  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioSoundBankData, uiAmId, sFileName);
}

plResult plAmplitude::DestroyBank(plAudioSystemBankData* pBankData)
{
  if (pBankData == nullptr || !pBankData->IsInstanceOf<plAmplitudeAudioSoundBankData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pBankData);
  return PL_SUCCESS;
}

plAudioSystemTriggerData* plAmplitude::DeserializeTriggerEntry(plStreamReader* pTriggerEntry) const
{
  if (pTriggerEntry == nullptr)
    return nullptr;

  Amplitude::AmEventID uiAmId;
  *pTriggerEntry >> uiAmId;

  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioTriggerData, uiAmId);
}

plResult plAmplitude::DestroyTriggerData(plAudioSystemTriggerData* pTriggerData)
{
  if (pTriggerData == nullptr || !pTriggerData->IsInstanceOf<plAmplitudeAudioTriggerData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pTriggerData);
  return PL_SUCCESS;
}

plAudioSystemRtpcData* plAmplitude::DeserializeRtpcEntry(plStreamReader* pRtpcEntry) const
{
  if (pRtpcEntry == nullptr)
    return nullptr;

  Amplitude::AmEventID uiAmId;
  *pRtpcEntry >> uiAmId;

  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioRtpcData, uiAmId);
}

plResult plAmplitude::DestroyRtpcData(plAudioSystemRtpcData* pRtpcData)
{
  if (pRtpcData == nullptr || !pRtpcData->IsInstanceOf<plAmplitudeAudioRtpcData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pRtpcData);
  return PL_SUCCESS;
}

plAudioSystemSwitchStateData* plAmplitude::DeserializeSwitchStateEntry(plStreamReader* pSwitchStateEntry) const
{
  if (pSwitchStateEntry == nullptr)
    return nullptr;

  Amplitude::AmSwitchID uiSwitchId;
  *pSwitchStateEntry >> uiSwitchId;

  Amplitude::AmObjectID uiSwitchStateId;
  *pSwitchStateEntry >> uiSwitchStateId;

  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioSwitchStateData, uiSwitchId, uiSwitchStateId);
}

plResult plAmplitude::DestroySwitchStateData(plAudioSystemSwitchStateData* pSwitchStateData)
{
  if (pSwitchStateData == nullptr || !pSwitchStateData->IsInstanceOf<plAmplitudeAudioSwitchStateData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pSwitchStateData);
  return PL_SUCCESS;
}

plAudioSystemEnvironmentData* plAmplitude::DeserializeEnvironmentEntry(plStreamReader* pEnvironmentEntry) const
{
  if (pEnvironmentEntry == nullptr)
    return nullptr;

  Amplitude::AmEnvironmentID uiEnvironmentId;
  *pEnvironmentEntry >> uiEnvironmentId;

  Amplitude::AmEffectID uiEffectId;
  *pEnvironmentEntry >> uiEffectId;

  return PL_NEW(plAudioMiddlewareAllocatorWrapper::GetAllocator(), plAmplitudeAudioEnvironmentData, uiEnvironmentId, uiEffectId);
}

plResult plAmplitude::DestroyEnvironmentData(plAudioSystemEnvironmentData* pEnvironmentData)
{
  if (pEnvironmentData == nullptr || !pEnvironmentData->IsInstanceOf<plAmplitudeAudioEnvironmentData>())
    return PL_FAILURE;

  PL_DELETE(plAudioMiddlewareAllocatorWrapper::GetAllocator(), pEnvironmentData);
  return PL_SUCCESS;
}

plResult plAmplitude::SetLanguage(const char* szLanguage)
{
  return PL_SUCCESS;
}

const char* plAmplitude::GetMiddlewareName() const
{
  return s_szAmplitudeMiddlewareName;
}

float plAmplitude::GetMasterGain() const
{
  return amEngine->GetMasterGain();
}

bool plAmplitude::GetMute() const
{
  return amEngine->IsMuted();
}

void plAmplitude::OnMasterGainChange(float fGain)
{
  // Master Volume
  amEngine->SetMasterGain(fGain);
}

void plAmplitude::OnMuteChange(bool bMute)
{
  // Mute
  amEngine->SetMute(bMute);
}

void plAmplitude::OnLoseFocus()
{
  amEngine->Pause(true);
}

void plAmplitude::OnGainFocus()
{
  amEngine->Pause(false);
}

void plAmplitude::GameApplicationEventHandler(const plGameApplicationExecutionEvent& e)
{
  // TODO
}

void plAmplitude::Update(plTime delta)
{
  if (amEngine == nullptr)
    return;

  PL_ASSERT_DEV(m_pData != nullptr, "UpdateSound() should not be called at this time.");

  amEngine->AdvanceFrame(delta.AsFloatInSeconds() * Amplitude::kAmSecond);
}

void plAmplitude::DetectPlatform() const
{
  if (!m_pData->m_sPlatform.IsEmpty())
    return;

#if PL_ENABLED(PL_PLATFORM_WINDOWS_DESKTOP)
  m_pData->m_sPlatform = "Desktop";

#elif PL_ENABLED(PL_PLATFORM_WINDOWS_UWP)
  m_pData->m_sPlatform = "Desktop"; /// \todo Need to detect mobile device mode

#elif PL_ENABLED(PL_PLATFORM_LINUX)
  m_pData->m_sPlatform = "Desktop"; /// \todo Need to detect mobile device mode (Android)

#elif PL_ENABLED(PL_PLATFORM_OSX)
  m_pData->m_sPlatform = "Desktop";

#elif PL_ENABLED(PL_PLATFORM_IOS)
  m_pData->m_sPlatform = "Mobile";

#elif PL_ENABLED(PL_PLATFORM_ANDROID)
  m_pData->m_sPlatform = "Mobile";

#elif
#  error "Unknown Platform"

#endif
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

PL_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_AmplitudeAudioSingleton);
