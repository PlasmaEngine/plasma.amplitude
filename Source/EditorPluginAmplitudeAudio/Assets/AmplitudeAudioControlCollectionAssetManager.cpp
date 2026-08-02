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

#include <EditorPluginAmplitudeAudio/Assets/AmplitudeAudioControlCollectionAsset.h>
#include <EditorPluginAmplitudeAudio/Assets/AmplitudeAudioControlCollectionAssetManager.h>
#include <EditorPluginAmplitudeAudio/Assets/AmplitudeAudioControlCollectionAssetWindow.moc.h>

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAmplitudeAudioControlCollectionAssetDocumentManager, 1, plRTTIDefaultAllocator<plAmplitudeAudioControlCollectionAssetDocumentManager>)
PL_END_DYNAMIC_REFLECTED_TYPE;

plAmplitudeAudioControlCollectionAssetDocumentManager::plAmplitudeAudioControlCollectionAssetDocumentManager()
{
  plDocumentManager::s_Events.AddEventHandler(plMakeDelegate(&plAmplitudeAudioControlCollectionAssetDocumentManager::OnDocumentManagerEvent, this));

  m_DocTypeDesc.m_sDocumentTypeName = "Audio Control Collection";
  m_DocTypeDesc.m_sFileExtension = "plAudioControlCollectionAsset";
  m_DocTypeDesc.m_sIcon = ":/AssetIcons/Audio_Control_Collection.png";
  m_DocTypeDesc.m_pDocumentType = plGetStaticRTTI<plAmplitudeAudioControlCollectionAssetDocument>();
  m_DocTypeDesc.m_pManager = this;
  m_DocTypeDesc.m_CompatibleTypes.PushBack("CompatibleAsset_AmplitudeAudio_Audio_Control_Collection");

  m_DocTypeDesc.m_sResourceFileExtension = "plAudioSystemControls";
  m_DocTypeDesc.m_AssetDocumentFlags = plAssetDocumentFlags::AutoTransformOnSave;

  plQtImageCache::GetSingleton()->RegisterTypeImage("Audio Control Collection", QPixmap(":/AssetIcons/Audio_Control_Collection.png"));
}

plAmplitudeAudioControlCollectionAssetDocumentManager::~plAmplitudeAudioControlCollectionAssetDocumentManager()
{
  plDocumentManager::s_Events.RemoveEventHandler(plMakeDelegate(&plAmplitudeAudioControlCollectionAssetDocumentManager::OnDocumentManagerEvent, this));
}

void plAmplitudeAudioControlCollectionAssetDocumentManager::OnDocumentManagerEvent(const plDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
    case plDocumentManager::Event::Type::DocumentWindowRequested:
    {
      if (e.m_pDocument->GetDynamicRTTI() == plGetStaticRTTI<plAmplitudeAudioControlCollectionAssetDocument>())
      {
        auto* pDocWnd = new plQtAmplitudeAudioControlCollectionAssetDocumentWindow(e.m_pDocument);
      }
    }
    break;

    default:
      break;
  }
}

void plAmplitudeAudioControlCollectionAssetDocumentManager::InternalCreateDocument(
  plStringView szDocumentTypeName, plStringView szPath, bool bCreateNewDocument, plDocument*& out_pDocument, const plDocumentObject* pOpenContext)
{
  out_pDocument = new plAmplitudeAudioControlCollectionAssetDocument(szPath);
}

void plAmplitudeAudioControlCollectionAssetDocumentManager::InternalGetSupportedDocumentTypes(plDynamicArray<const plDocumentTypeDescriptor*>& inout_DocumentTypes) const
{
  inout_DocumentTypes.PushBack(&m_DocTypeDesc);
}

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

PL_STATICLINK_FILE(EditorPluginAmplitudeAudio, EditorPluginAmplitudeAudio_Assets_AmplitudeAudioControlCollectionAssetManager);
