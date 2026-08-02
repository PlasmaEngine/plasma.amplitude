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

#include <AmplitudeAudioPlugin/Core/AmplitudeAudioData.h>

// clang-format off
PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioEntityData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioListenerData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioTriggerData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioRtpcData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioSwitchStateData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioEnvironmentData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioEventData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioSoundBankData, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_STATIC_REFLECTED_ENUM(plAmplitudeAudioControlType, 1)
  PL_ENUM_CONSTANT(plAmplitudeAudioControlType::Invalid),
  PL_ENUM_CONSTANT(plAmplitudeAudioControlType::Trigger),
  PL_ENUM_CONSTANT(plAmplitudeAudioControlType::Rtpc),
  PL_ENUM_CONSTANT(plAmplitudeAudioControlType::SoundBank),
  PL_ENUM_CONSTANT(plAmplitudeAudioControlType::Switch),
  PL_ENUM_CONSTANT(plAmplitudeAudioControlType::SwitchState),
  PL_ENUM_CONSTANT(plAmplitudeAudioControlType::Environment),
PL_END_STATIC_REFLECTED_ENUM;
// clang-format on
