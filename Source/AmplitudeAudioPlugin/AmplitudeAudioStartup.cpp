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
#include <AmplitudeAudioPlugin/Components/AmplitudeRoomComponent.h>
#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>

#include <AudioSystem/Core/AudioSystem.h>

#include <Core/World/World.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/Archive/ArchiveUtils.h>
#include <GameEngine/GameApplication/GameApplication.h>

static plAmplitude* s_pAmplitudeSingleton = nullptr;

PL_BEGIN_SUBSYSTEM_DECLARATION(SparkyStudios, AmplitudeAudioPlugin)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core",
    "AudioSystem"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    s_pAmplitudeSingleton = PL_AUDIOSYSTEM_NEW(plAmplitude);

    // Audio Control Collection Resources
    {
      plResourceManager::RegisterResourceForAssetType("Audio Control Collection", plGetStaticRTTI<plAmplitudeAudioControlCollectionResource>());
      plResourceManager::RegisterResourceForAssetType("Amplitude Room Material", plGetStaticRTTI<plAmplitudeRoomMaterialResource>());

      {
        plAmplitudeAudioControlCollectionResourceDescriptor desc;
        const auto hResource = plResourceManager::CreateResource<plAmplitudeAudioControlCollectionResource>("AudioControlCollectionMissing", std::move(desc), "Fallback for missing audio control collections.");
        plResourceManager::SetResourceTypeMissingFallback<plAmplitudeAudioControlCollectionResource>(hResource);
      }
    }

    plAudioSystem::GetSingleton()->Startup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    plAudioSystem::GetSingleton()->Shutdown();

    plAmplitudeAudioControlCollectionResource::CleanupDynamicPluginReferences();

    PL_AUDIOSYSTEM_DELETE(s_pAmplitudeSingleton);
  }

PL_END_SUBSYSTEM_DECLARATION;

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

PL_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_AmplitudeAudioPluginStartup);
