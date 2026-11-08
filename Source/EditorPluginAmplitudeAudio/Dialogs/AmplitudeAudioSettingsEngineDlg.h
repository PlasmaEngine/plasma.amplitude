#pragma once

#include <EditorPluginAmplitudeAudio/Dialogs/AmplitudeAudioSettings.h>
#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>
#include <EditorUi/ListEditor.h>
#include <GuiFoundation/EngineUi/EngineDialog.h>

/// \brief Amplitude Audio Settings drawn by the engine: the Amplitude project (typed, or picked with Browse...), the
/// platforms in a list with Add ("Add Platform" prompt) and Remove (asks first), and for the selected platform the
/// init bank and the Amplitude config, picked from the files in the project's build folder. OK asks before saving
/// Audio/AudioSystemConfig.ddl when something changed.
class plAmplitudeAudioSettingsEngineDlg : public plEngineDialog
{
public:
  plAmplitudeAudioSettingsEngineDlg();
  ~plAmplitudeAudioSettingsEngineDlg();

  /// Opens the engine dialog; without a UI renderer it logs an error.
  static void Open();

protected:
  void BuildBody(plUiElement* pBody, const plVec2& vSize) override;
  void OnButton(plStringView sId) override;
  void OnUiEvent(const plMsgUiEvent& event) override;
  void OnUiUpdate() override;
  plVec2 GetPreferredSize() const override;

private:
  enum class Choice : plUInt8
  {
    InitBank,
    Config,
  };

  void BuildProjectRow(plUiElement* pBody, const plRectFloat& rect);
  void BuildPlatformList(plUiElement* pBody, const plRectFloat& rect);
  void BuildPlatformForm(plUiElement* pBody, const plRectFloat& rect);

  plAmplitudeConfiguration* GetCurrentConfig();

  void FillList();
  void SelectPlatform(plStringView sPlatform);

  /// A typed project path counts once the field loses focus, before a control of the dialog acts, on Enter and on OK.
  void CommitProjectField();
  void WriteProjectField(plStringView sPath);

  void AddPlatform();
  void RemovePlatform();
  void BrowseProject();
  void ShowChoiceMenu(Choice choice);
  void OnListEvent(const plEditorUiListEditorEvent& e);

  plAmplitudeAudioSettings m_Settings;
  plString m_sCurrentPlatform;
  plDynamicArray<plString> m_Platforms;
  plUniquePtr<plEditorUiListEditor> m_pList;
  plEventSubscriptionID m_ListSubscription = 0;
  plRectFloat m_BankRect = plRectFloat(0, 0, 0, 0);
  plRectFloat m_ConfigRect = plRectFloat(0, 0, 0, 0);
};
