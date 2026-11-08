#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioPCH.h>

#include <EditorPluginAmplitudeAudio/Dialogs/AmplitudeAudioSettingsEngineDlg.h>
#include <EditorUi/Menu.h>
#include <EditorUi/Surface.h>
#include <GameUiPlugin/Components/UiCanvasComponent.h>

namespace
{
  const char* s_szOk = "Ok";
  const char* s_szCancel = "Cancel";

  constexpr float s_fWidth = 600.0f;
  constexpr float s_fListWidth = 170.0f;
  constexpr float s_fListHeight = 180.0f;
  constexpr float s_fStripHeight = 30.0f;
  constexpr float s_fStripButton = 22.0f;
  constexpr float s_fColumnGap = 12.0f;
  constexpr float s_fRowGap = 12.0f;
  constexpr float s_fProjectLabelWidth = 90.0f;
  constexpr float s_fLabelWidth = 120.0f;

  const char* s_szProjectField = "Amp.Project";
  const char* s_szProjectText = "Amp.Project.Text";

  const char* s_szBankTip = "Sound banks in <project>/build/soundbanks";
  const char* s_szConfigTip = "Must be identical to what has been configured in Amplitude Studio for this platform.";
} // namespace

plAmplitudeAudioSettingsEngineDlg::plAmplitudeAudioSettingsEngineDlg()
  : plEngineDialog("AmplitudeAudioSettings", "Amplitude Audio Settings")
{
  m_Settings.Load();
  m_sCurrentPlatform = m_Settings.GetFirstPlatform();

  if (!IsReady())
    return;

  plEditorUiDialogButton buttons[2];
  buttons[0].m_sId = s_szOk;
  buttons[0].m_sLabel = "OK";
  buttons[0].m_Role = plEditorUiDialogButton::Role::Accept;
  buttons[1].m_sId = s_szCancel;
  buttons[1].m_sLabel = "Cancel";
  buttons[1].m_Role = plEditorUiDialogButton::Role::Reject;
  GetHost().SetButtons(buttons);

  m_pList = PL_DEFAULT_NEW(plEditorUiListEditor, *GetSurface(), "Amp.Platforms");
  m_pList->SetRenamable(false);
  m_pList->SetReorderable(false);
  m_ListSubscription = m_pList->m_Events.AddEventHandler(plMakeDelegate(&plAmplitudeAudioSettingsEngineDlg::OnListEvent, this));

  FillList();
  WriteProjectField(m_Settings.m_Configs.m_sProjectPath);
}

plAmplitudeAudioSettingsEngineDlg::~plAmplitudeAudioSettingsEngineDlg()
{
  if (m_pList != nullptr)
    m_pList->m_Events.RemoveEventHandler(m_ListSubscription);
}

void plAmplitudeAudioSettingsEngineDlg::Open()
{
  if (!plEngineDialog::IsEngineEnabled())
  {
    plLog::Error("The Amplitude Audio settings dialog needs the editor UI renderer.");
    return;
  }

  plAmplitudeAudioSettingsEngineDlg dlg;
  dlg.Exec();
}

plVec2 plAmplitudeAudioSettingsEngineDlg::GetPreferredSize() const
{
  const float fPad = plEditorUiDialogHost::Padding;
  return plVec2(s_fWidth, fPad + plEditorUi::ControlHeight + s_fRowGap + s_fListHeight + s_fStripHeight + fPad + plEditorUiDialogHost::FooterHeight);
}

plAmplitudeConfiguration* plAmplitudeAudioSettingsEngineDlg::GetCurrentConfig()
{
  auto it = m_Settings.m_Configs.m_AssetProfiles.Find(m_sCurrentPlatform);
  return it.IsValid() ? &it.Value() : nullptr;
}

void plAmplitudeAudioSettingsEngineDlg::FillList()
{
  m_Platforms.Clear();

  plDynamicArray<plEditorUiListItem> items;
  for (auto it = m_Settings.m_Configs.m_AssetProfiles.GetIterator(); it.IsValid(); ++it)
  {
    m_Platforms.PushBack(it.Key());
    items.ExpandAndGetRef().m_sText = it.Key();
  }

  m_pList->SetItems(items);

  const plUInt32 uiIndex = m_Platforms.IndexOf(m_sCurrentPlatform);
  m_pList->SetSelectedIndex(uiIndex == plInvalidIndex ? -1 : static_cast<plInt32>(uiIndex));
}

void plAmplitudeAudioSettingsEngineDlg::SelectPlatform(plStringView sPlatform)
{
  m_sCurrentPlatform = sPlatform;
  Rebuild();
}

void plAmplitudeAudioSettingsEngineDlg::CommitProjectField()
{
  const plVariant value = GetSurface()->GetBlackboard().GetEntryValue(plTempHashedString(s_szProjectText));
  if (!value.IsValid())
    return;

  const plString sPath = value.ConvertTo<plString>();
  if (sPath != m_Settings.m_Configs.m_sProjectPath)
    m_Settings.SetProjectPath(sPath);
}

void plAmplitudeAudioSettingsEngineDlg::WriteProjectField(plStringView sPath)
{
  GetSurface()->GetCanvas().SetTextBinding(s_szProjectText, sPath);
  GetSurface()->GetBlackboard().SetEntryValue(s_szProjectText, plString(sPath));
}

void plAmplitudeAudioSettingsEngineDlg::OnUiUpdate()
{
  if (GetSurface()->GetCanvas().GetFocusedElementName() != s_szProjectField)
    CommitProjectField();
}

void plAmplitudeAudioSettingsEngineDlg::BuildBody(plUiElement* pBody, const plVec2& vSize)
{
  using namespace plEditorUi;

  const float fPad = plEditorUiDialogHost::Padding;

  BuildProjectRow(pBody, plRectFloat(fPad, fPad, plMath::Max(vSize.x - 2.0f * fPad, 0.0f), ControlHeight));

  const float y = fPad + ControlHeight + s_fRowGap;
  const float fHeight = plMath::Max(vSize.y - y - fPad, s_fStripHeight + 40.0f);

  const plRectFloat list(fPad, y, s_fListWidth, fHeight);
  BuildPlatformList(pBody, list);

  const float fFormX = list.Right() + s_fColumnGap;
  const plRectFloat form(fFormX, y, plMath::Max(vSize.x - fFormX - fPad, 0.0f), fHeight);

  if (GetCurrentConfig() != nullptr)
  {
    BuildPlatformForm(pBody, form);
    return;
  }

  plUiPanel* pEmpty = Panel(pBody);
  SetRect(pEmpty, form);
  pEmpty->m_Color = plColorGammaUB(0, 0, 0, 0);
  EmptyState(pEmpty, Icon::Volume, "No platform selected.", {});
}

void plAmplitudeAudioSettingsEngineDlg::BuildProjectRow(plUiElement* pBody, const plRectFloat& rect)
{
  using namespace plEditorUi;

  plBlackboard& board = GetSurface()->GetBlackboard();

  plEditorUiForm form(pBody, rect, s_fProjectLabelWidth);
  const plRectFloat row = form.Row("Project");

  // the project file: a path field and the file picker
  const float fBrowseWidth = PushButtonWidth("Browse...");
  TextField(pBody, plRectFloat(row.x, row.y, row.width - fBrowseWidth - 4.0f, row.height), s_szProjectField, s_szProjectText, {}, {}, true);
  PushButton(pBody, board, plRectFloat(row.Right() - fBrowseWidth, row.y, fBrowseWidth, row.height), "Browse...", "Amp.Browse");
  SetToolTip(board, "Amp.Browse", "Select Amplitude Project (*.amproject)");
}

void plAmplitudeAudioSettingsEngineDlg::BuildPlatformList(plUiElement* pBody, const plRectFloat& rect)
{
  using namespace plEditorUi;

  plBlackboard& board = GetSurface()->GetBlackboard();
  const float fListHeight = rect.height - s_fStripHeight;

  plUiPanel* pFrame = Panel(pBody);
  SetRect(pFrame, rect);
  pFrame->m_Color = Field();
  pFrame->m_fBorderWidth = 1.0f;
  pFrame->m_BorderColor = Line();
  pFrame->m_vCornerRadius = plVec4(4);

  plUiPanel* pListArea = Panel(pFrame);
  SetRect(pListArea, plRectFloat(1.0f, 1.0f, rect.width - 2.0f, fListHeight - 1.0f));
  pListArea->m_Color = plColorGammaUB(0, 0, 0, 0);
  m_pList->Build(pListArea);

  // the strip under the list: its caption, Add and Remove
  plUiPanel* pRule = Panel(pFrame);
  SetRect(pRule, plRectFloat(0.0f, fListHeight, rect.width, 1.0f));
  pRule->m_Color = Line();

  plUiPanel* pStrip = Panel(pFrame);
  SetRect(pStrip, plRectFloat(1.0f, fListHeight + 1.0f, rect.width - 2.0f, s_fStripHeight - 2.0f));
  pStrip->m_Color = Surface();
  pStrip->m_vCornerRadius = plVec4(0, 0, 3, 3);

  TextAt(pFrame, "Platform", plRectFloat(8.0f, fListHeight, 80.0f, s_fStripHeight), Dim(), FontSize - 2);

  const float fButtonY = fListHeight + (s_fStripHeight - s_fStripButton) * 0.5f;
  IconPushButton(pFrame, board, plRectFloat(rect.width - 2.0f * s_fStripButton - 6.0f, fButtonY, s_fStripButton, s_fStripButton), IconPath(Icon::Add), "Amp.Add");
  IconPushButton(pFrame, board, plRectFloat(rect.width - s_fStripButton - 4.0f, fButtonY, s_fStripButton, s_fStripButton), IconPath(Icon::Remove), "Amp.Remove", GetCurrentConfig() != nullptr);
  SetToolTip(board, "Amp.Add", "Add Platform");
  SetToolTip(board, "Amp.Remove", "Remove Platform");
}

void plAmplitudeAudioSettingsEngineDlg::BuildPlatformForm(plUiElement* pBody, const plRectFloat& rect)
{
  using namespace plEditorUi;

  plBlackboard& board = GetSurface()->GetBlackboard();
  const plAmplitudeConfiguration* pConfig = GetCurrentConfig();

  plEditorUiForm form(pBody, rect, s_fLabelWidth);

  m_BankRect = form.Row("Init Bank", ControlHeight, s_szBankTip);
  Dropdown(pBody, board, m_BankRect, pConfig->m_sInitSoundBank, "Amp.InitBank");
  SetToolTip(board, "Amp.InitBank", s_szBankTip);

  m_ConfigRect = form.Row("Amplitude Config", ControlHeight, s_szConfigTip);
  Dropdown(pBody, board, m_ConfigRect, pConfig->m_sEngineConfigFileName, "Amp.Config");
  SetToolTip(board, "Amp.Config", s_szConfigTip);

  AddBodyToolTips(form.GetToolTips());
}

void plAmplitudeAudioSettingsEngineDlg::AddPlatform()
{
  const plString sPlatform = m_Settings.AskAndAddPlatform();
  if (sPlatform.IsEmpty())
    return;

  m_sCurrentPlatform = sPlatform;

  FillList();
  Rebuild();
}

void plAmplitudeAudioSettingsEngineDlg::RemovePlatform()
{
  if (GetCurrentConfig() == nullptr)
    return;

  if (plEditorUiServices::MessageBoxQuestion(plFmt("Remove the platform '{}'?", m_sCurrentPlatform), plEditorUiMessageButton::Yes | plEditorUiMessageButton::No, plEditorUiMessageButton::No) != plEditorUiMessageButton::Yes)
    return;

  m_Settings.m_Configs.m_AssetProfiles.Remove(m_sCurrentPlatform);

  // the first platform in the list takes the selection
  m_sCurrentPlatform = m_Settings.GetFirstPlatform();

  FillList();
  Rebuild();
}

void plAmplitudeAudioSettingsEngineDlg::BrowseProject()
{
  plString sPath;
  if (!plAmplitudeAudioSettings::PickProject(sPath))
    return;

  WriteProjectField(sPath);
  m_Settings.SetProjectPath(sPath);
}

void plAmplitudeAudioSettingsEngineDlg::ShowChoiceMenu(Choice choice)
{
  const plAmplitudeConfiguration* pConfig = GetCurrentConfig();
  if (pConfig == nullptr)
    return;

  const auto& files = choice == Choice::InitBank ? m_Settings.m_Banks : m_Settings.m_ConfigFiles;
  const plString& sCurrent = choice == Choice::InitBank ? pConfig->m_sInitSoundBank : pConfig->m_sEngineConfigFileName;

  plEditorUiMenu menu;
  for (plUInt32 i = 0; i < files.GetCount(); ++i)
  {
    plEditorUiMenuItem& item = menu.AddAction(files[i], i);
    item.m_bCheckable = true;
    item.m_bChecked = files[i] == sCurrent;
  }

  if (files.IsEmpty())
    menu.AddAction("Nothing found in the project's build folder", 0).m_bEnabled = false;

  // the pick stores the file the menu showed, even when the project's files are read again while it is open
  const plRectFloat& field = choice == Choice::InitBank ? m_BankRect : m_ConfigRect;
  const plVec2 vPos(GetBodyRect().x + field.x, GetBodyRect().y + field.y + field.height);
  plEditorUiMenuView::Show(*GetSurface(), vPos, menu, [this, choice, shown = files](plUInt32 uiPicked)
    {
      plAmplitudeConfiguration* pPicked = GetCurrentConfig();
      if (pPicked == nullptr || uiPicked >= shown.GetCount())
        return;

      plString& sValue = choice == Choice::InitBank ? pPicked->m_sInitSoundBank : pPicked->m_sEngineConfigFileName;
      sValue = shown[uiPicked];
      Rebuild(); });
}

void plAmplitudeAudioSettingsEngineDlg::OnListEvent(const plEditorUiListEditorEvent& e)
{
  if (e.m_Type == plEditorUiListEditorEvent::Type::SelectionChanged)
  {
    const plInt32 iIndex = m_pList->GetSelectedIndex();
    const plString sPlatform = iIndex >= 0 ? m_Platforms[iIndex] : plString();

    if (sPlatform != m_sCurrentPlatform)
      SelectPlatform(sPlatform);
  }
  else if (e.m_Type == plEditorUiListEditorEvent::Type::RemoveRequested)
  {
    if (e.m_uiIndex < m_Platforms.GetCount() && m_Platforms[e.m_uiIndex] != m_sCurrentPlatform)
      SelectPlatform(m_Platforms[e.m_uiIndex]);

    RemovePlatform();
  }
}

void plAmplitudeAudioSettingsEngineDlg::OnUiEvent(const plMsgUiEvent& event)
{
  // a click arrives before the frame that sees the project field lose focus: the typed path counts first, so the
  // menus and a new platform use its files
  CommitProjectField();

  const plStringView sId = event.m_sIdentifier.GetView();

  if (sId == "Amp.Add")
    AddPlatform();
  else if (sId == "Amp.Remove")
    RemovePlatform();
  else if (sId == "Amp.Browse")
    BrowseProject();
  else if (sId == "Amp.InitBank")
    ShowChoiceMenu(Choice::InitBank);
  else if (sId == "Amp.Config")
    ShowChoiceMenu(Choice::Config);
}

void plAmplitudeAudioSettingsEngineDlg::OnButton(plStringView sId)
{
  if (sId == s_szOk)
  {
    CommitProjectField();

    if (!m_Settings.AskAndSave())
      return;
  }

  plEngineDialog::OnButton(sId);
}
