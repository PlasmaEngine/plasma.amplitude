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

#include <AmplitudeAudioPlugin/Components/AmplitudeComponent.h>

// clang-format off
PL_BEGIN_ABSTRACT_COMPONENT_TYPE(plAmplitudeComponent, 1)
{
  PL_BEGIN_ATTRIBUTES
  {
    new plCategoryAttribute("Audio/Amplitude"),
  }
  PL_END_ATTRIBUTES;
}
PL_END_ABSTRACT_COMPONENT_TYPE;
// clang-format on

PL_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_Components_AmplitudeComponent);
