#include "FEDearImguiWrapper.h"

ImGuiModalPopup::ImGuiModalPopup()
{
	PopupCaption = "";
	bShouldOpen = false;
	bOpened = false;
	FE_IMGUI_WINDOW_MANAGER.RegisterPopup(this);
}

void ImGuiModalPopup::Show()
{
	bShouldOpen = true;
	bOpened = true;
}

void ImGuiModalPopup::Render()
{
	if (bShouldOpen)
	{
		ImGui::OpenPopup(PopupCaption.c_str());
		bShouldOpen = false;
	}
}

bool ImGuiModalPopup::IsOpened() const
{
	return bOpened;
}

void ImGuiModalPopup::Close()
{
	bOpened = false;
	ImGui::CloseCurrentPopup();
}

ImGuiButton::ImGuiButton(std::string Caption)
{
	if (Caption.size() + 1 > BUTTON_CAPTION_SIZE)
		Caption.erase(Caption.begin() + BUTTON_CAPTION_SIZE - 1, Caption.end() - (BUTTON_CAPTION_SIZE + 1));
	
	strcpy_s(this->Caption, Caption.size() + 1, Caption.c_str());
}

void ImGuiButton::SetCaption(std::string NewCaption)
{
	if (NewCaption.size() + 1 > BUTTON_CAPTION_SIZE)
		NewCaption.erase(NewCaption.begin() + BUTTON_CAPTION_SIZE - 1, NewCaption.end() - (BUTTON_CAPTION_SIZE + 1));

	strcpy_s(this->Caption, NewCaption.size() + 1, NewCaption.c_str());
}

ImVec2 ImGuiButton::GetPosition() const
{
	return Position;
}

void ImGuiButton::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;
}

ImVec2 ImGuiButton::GetSize() const
{
	return Size;
}

void ImGuiButton::SetSize(const ImVec2 NewSize)
{
	Size = NewSize;
}

ImVec4 ImGuiButton::GetDefaultColor() const
{
	return DefaultColor;
}

void ImGuiButton::SetDefaultColor(const ImVec4 NewDefaultColor)
{
	DefaultColor = NewDefaultColor;
}

ImVec4 ImGuiButton::GetHoveredColor() const
{
	return HoveredColor;
}

void ImGuiButton::SetHoveredColor(const ImVec4 NewHoveredColor)
{
	HoveredColor = NewHoveredColor;
}

ImVec4 ImGuiButton::GetActiveColor() const
{
	return ActiveColor;
}

void ImGuiButton::SetActiveColor(ImVec4 NewActiveColor)
{
	ActiveColor = NewActiveColor;
}

void ImGuiButton::RenderBegin()
{
	bWasClicked = false;

	ImGui::PushStyleColor(ImGuiCol_Button, DefaultColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ActiveColor);

	if (Position.x != -1.0f)
		ImGui::SetCursorPosX(Position.x);
	
	if (Position.y != -1.0f)
		ImGui::SetCursorPosY(Position.y);
	
	bHovered = false;
	
	ImGui::Button(Caption, Size);

	// flag important for drag and drop functionality
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		bHovered = true;

	if (ImGui::IsMouseClicked(0) && bHovered)
		bWasClicked = true;
}

bool ImGuiButton::IsClicked() const
{
	return bWasClicked;
}

void ImGuiButton::RenderEnd()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void ImGuiButton::Render()
{
	RenderBegin();
	RenderEnd();
}

ImGuiImageButton::ImGuiImageButton(FETexture* Texture)
{
	this->Texture = Texture;
	ID = UNIQUE_ID.GenerateID();
}

ImVec2 ImGuiImageButton::GetPosition() const
{
	return Position;
}

void ImGuiImageButton::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;
}

ImVec2 ImGuiImageButton::GetSize() const
{
	return Size;
}

void ImGuiImageButton::SetSize(const ImVec2 NewSize)
{
	Size = NewSize;
}

ImVec4 ImGuiImageButton::GetDefaultColor() const
{
	return DefaultColor;
}

void ImGuiImageButton::SetDefaultColor(const ImVec4 NewDefaultColor)
{
	DefaultColor = NewDefaultColor;
}

ImVec4 ImGuiImageButton::GetHoveredColor() const
{
	return HoveredColor;
}

void ImGuiImageButton::SetHoveredColor(const ImVec4 NewHoveredColor)
{
	HoveredColor = NewHoveredColor;
}

ImVec4 ImGuiImageButton::GetActiveColor() const
{
	return ActiveColor;
}

void ImGuiImageButton::SetActiveColor(const ImVec4 NewActiveColor)
{
	ActiveColor = NewActiveColor;
}

ImVec2 ImGuiImageButton::GetUV0() const
{
	return UV0;
}

void ImGuiImageButton::SetUV0(const ImVec2 NewValue)
{
	UV0 = NewValue;
}

ImVec2 ImGuiImageButton::GetUV1() const
{
	return UV1;
}

void ImGuiImageButton::SetUV1(const ImVec2 NewValue)
{
	UV1 = NewValue;
}

float ImGuiImageButton::GetFramePadding() const
{
	return FramePadding;
}

void ImGuiImageButton::SetFramePadding(float NewFramePadding)
{
	if (NewFramePadding < 0.0f)
		NewFramePadding = 0.0f;

	FramePadding = NewFramePadding;
}

ImVec4 ImGuiImageButton::GetBackgroundColor() const
{
	return BackgroundColor;
}

void ImGuiImageButton::SetBackgroundColor(const ImVec4 NewBackgroundColor)
{
	BackgroundColor = NewBackgroundColor;
}

ImVec4 ImGuiImageButton::GetTintColor() const
{
	return TintColor;
}

void ImGuiImageButton::SetTintColor(const ImVec4 NewTintColor)
{
	TintColor = NewTintColor;
}

FETexture* ImGuiImageButton::GetTexture() const
{
	return Texture;
}

void ImGuiImageButton::SetTexture(FETexture* NewTexture)
{
	Texture = NewTexture;
}

bool ImGuiImageButton::IsHovered() const
{
	return bHovered;
}

void ImGuiImageButton::RenderBegin()
{
	bWasClicked = false;

	ImGui::PushStyleColor(ImGuiCol_Button, DefaultColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ActiveColor);

	if (Position.x != -1.0f)
		ImGui::SetCursorPosX(Position.x);

	if (Position.y != -1.0f)
		ImGui::SetCursorPosY(Position.y);

	bHovered = false;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(FramePadding, FramePadding));
	ImGui::ImageButton(UNIQUE_ID.ToString(ID).c_str(), Texture->GetTextureID(), Size, UV0, UV1, BackgroundColor, TintColor);
	ImGui::PopStyleVar();

	// flag important for drag and drop functionality
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		bHovered = true;

	if (ImGui::IsMouseClicked(0) && bHovered)
		bWasClicked = true;
}

bool ImGuiImageButton::IsClicked() const
{
	return bWasClicked;
}

void ImGuiImageButton::RenderEnd()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void ImGuiImageButton::Render()
{
	RenderBegin();
	RenderEnd();
}

FEImGuiWindow::FEImGuiWindow()
{
	Position = ImVec2(0.0f, 0.0f);
	Size = ImVec2(100.0f, 100.0f);
	bVisible = false;
	FE_IMGUI_WINDOW_MANAGER.RegisterWindow(this);
}

FEImGuiWindow::~FEImGuiWindow()
{
	FE_IMGUI_WINDOW_MANAGER.UnRegisterWindow(this);
}

void FEImGuiWindow::Show()
{
	bVisible = true;
	bWasClosedLastFrame = true;

	bUserRequestedClose = false;
	bUserWantsWindowToBeOpen = true;
}

bool FEImGuiWindow::IsMouseHovered() const
{
	if (Window == nullptr)
		return false;

	if (ImGui::GetCurrentContext()->HoveredWindow != nullptr &&
		ImGui::GetCurrentContext()->HoveredWindow == Window)
		return true;

	return false;
}

ImGuiWindow* WindowsManager::GetCurrentWindowImpl()
{
	ImGuiContext* Context = ImGui::GetCurrentContext();
	if (Context == nullptr)
		return nullptr;

	return Context->CurrentWindow;
}

void FEImGuiWindow::Render()
{
	if (bVisible)
	{
		if (bWasClosedLastFrame)
		{
			ImGui::SetNextWindowPos(Position);
		}
		ImGui::SetNextWindowSize(Size);

		bWasClosedLastFrame = false;

		if (GetBorderSize() != 0.0f)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, GetBorderSize());

		if (GetPadding().x != 0.0f || GetPadding().y != 0.0f)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GetPadding().x, GetPadding().y));

		ImGui::Begin(Caption, (bHaveCloseButton ? &bUserWantsWindowToBeOpen : nullptr), Flags);
		Window = FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl();
	}
}

void FEImGuiWindow::OnRenderEnd()
{
	if (bVisible)
	{
		if (!Window->Collapsed)
			Size = ImGui::GetWindowSize();

		if (GetBorderSize() != 0.0f)
			ImGui::PopStyleVar();

		if (GetPadding().x != 0.0f || GetPadding().y != 0.0f)
			ImGui::PopStyleVar();

		ImGui::End();

		if (!bUserWantsWindowToBeOpen)
			bUserRequestedClose = true;
	}
}

bool FEImGuiWindow::GetUserRequestedClose() const
{
	return bUserRequestedClose;
}

bool FEImGuiWindow::IsVisible() const
{
	return bVisible;
}

void FEImGuiWindow::Close()
{
	if (bVisible)
	{
		bVisible = false;

		if (GetBorderSize() != 0.0f)
			ImGui::PopStyleVar();

		if (GetPadding().x != 0.0f || GetPadding().y != 0.0f)
			ImGui::PopStyleVar();

		ImGui::End();
	}
}

std::string FEImGuiWindow::GetCaption() const
{
	return std::string(Caption);
}

void FEImGuiWindow::SetCaption(const std::string NewCaption)
{
	strcpy_s(Caption, NewCaption.size() + 1, NewCaption.c_str());
}

ImGuiWindow* FEImGuiWindow::GetWindow() const
{
	return Window;
}

void FEImGuiWindow::SetVisible(bool NewValue)
{
	bVisible = NewValue;
}

float FEImGuiWindow::GetBorderSize() const
{
	return BorderSize;
}

void FEImGuiWindow::SetBorderSize(float NewValue)
{
	BorderSize = NewValue;
}

glm::vec2 FEImGuiWindow::GetPadding() const
{
	return Padding;
}

void FEImGuiWindow::SetPadding(glm::vec2 NewValue)
{
	Padding = NewValue;
}

WindowsManager::WindowsManager()
{
}

void WindowsManager::RegisterWindow(FEImGuiWindow* Window)
{
	Windows.push_back(Window);
}

void WindowsManager::UnRegisterWindow(FEImGuiWindow* Window)
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		if (Windows[i] == Window)
		{
			Windows.erase(Windows.begin() + i);
			break;
		}
	}
}

void WindowsManager::RegisterPopup(ImGuiModalPopup* Popup)
{
	PopUps.push_back(Popup);
}

void WindowsManager::CloseAllPopups() const
{
	for (size_t i = 0; i < PopUps.size(); i++)
	{
		PopUps[i]->Close();
	}
}

void WindowsManager::CloseAllWindows() const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		Windows[i]->bVisible = false;
	}
}

void WindowsManager::RenderAllWindows() const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		Windows[i]->Render();
	}
}

bool WindowsManager::IsRegisteredWindowWithCaption(const std::string Caption) const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		if (Windows[i]->Caption == Caption)
			return true;
	}

	return false;
}

FEImGuiWindow* WindowsManager::GetWindowByCaption(const std::string Caption) const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		if (Windows[i]->Caption == Caption)
			return Windows[i];
	}

	return nullptr;
}

void ShowToolTip(const char* Text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(Text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void RenderRowLabel(const char* Label, const float LabelWidth, const float LabelIndent)
{
	const float RowStartX = ImGui::GetCursorPosX();
	ImGui::AlignTextToFramePadding();
	ImGui::SetCursorPosX(RowStartX + LabelIndent);
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
	ImGui::TextUnformatted(Label);
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::SetCursorPosX(RowStartX + LabelWidth);
}

std::string GetNumberWithSeparators(const size_t Number)
{
	std::string Result = std::to_string(Number);
	for (int Position = static_cast<int>(Result.size()) - 3; Position > 0; Position -= 3)
		Result.insert(Position, ",");

	return Result;
}

std::string ToLowerCase(std::string Text)
{
	std::transform(Text.begin(), Text.end(), Text.begin(), [](const unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
	return Text;
}

std::string CutTextToWidth(std::string Text, const float Width)
{
	if (ImGui::CalcTextSize(Text.c_str()).x <= Width)
		return Text;

	while (Text.size() > 1 && ImGui::CalcTextSize((Text + "...").c_str()).x > Width)
		Text.pop_back();

	return Text + "...";
}

bool RenderSwitchButton(const char* Label, const bool bSelected, const ImDrawFlags RoundFlags)
{
	const ImVec2 Size = ImVec2(ImGui::CalcTextSize(Label).x + 24.0f, ImGui::GetFrameHeight());
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + Size.x, Min.y + Size.y);
	const bool bPressed = ImGui::InvisibleButton(Label, Size);
	const bool bHovered = ImGui::IsItemHovered();

	ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
	if (bSelected)
		Fill = FEEditorStyle::Accent;

	// GetColorU32 applies alpha of the style, so the button fades when it is disabled.
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, ImGui::GetColorU32(Fill), 4.0f, RoundFlags);
	if (!bSelected)
		DrawList->AddRect(Min, Max, ImGui::GetColorU32(FEEditorStyle::ButtonBorder), 4.0f, 1.0f, RoundFlags);

	DrawList->AddText(ImVec2(Min.x + 12.0f, Min.y + (Size.y - ImGui::GetFontSize()) / 2.0f), ImGui::GetColorU32(bSelected ? FEEditorStyle::TextOnAccent : FEEditorStyle::Text), Label);
	return bPressed;
}

static float StyledPopupWidth = 0.0f;

bool BeginStyledPopup(const char* ID, const FEPopupTone Tone, const float Width)
{
	const ImVec2 Center = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f);
	ImGui::SetNextWindowPos(Center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, FEEditorStyle::Card);
	ImGui::PushStyleColor(ImGuiCol_Border, FEEditorStyle::PopupBorder);
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, FEEditorStyle::Dim);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, FEEditorStyle::Background);

	const bool bOpened = ImGui::BeginPopupModal(ID, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	if (bOpened)
	{
		StyledPopupWidth = Width;

		// Line at the top says what kind of action the popup is about.
		const ImVec2 WindowMin = ImGui::GetWindowPos();
		const ImU32 ToneColor = Tone == FEPopupTone::Danger ? FEEditorStyle::DangerButton : FEEditorStyle::PrimaryButton;
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(WindowMin.x + 1.0f, WindowMin.y + 1.0f), ImVec2(WindowMin.x + ImGui::GetWindowWidth() - 1.0f, WindowMin.y + 5.0f), ToneColor, 6.0f, ImDrawFlags_RoundCornersTop);

		ImGui::Dummy(ImVec2(Width, 0.0f));
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
	}

	return bOpened;
}

void EndStyledPopup(const bool bOpened)
{
	if (bOpened)
		ImGui::EndPopup();

	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(4);
}

void RenderPopupHeading(const std::string& Text)
{
	RenderPopupText(Text, FEEditorStyle::Text, 24.0f);
}

void RenderPopupText(const std::string& Text, const ImU32 Color, const float FontSize)
{
	if (FontSize > 0.0f)
		ImGui::PushFont(nullptr, FontSize);

	ImGui::PushStyleColor(ImGuiCol_Text, Color);
	ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + StyledPopupWidth);
	ImGui::TextUnformatted(Text.c_str());
	ImGui::PopTextWrapPos();
	ImGui::PopStyleColor();
	if (FontSize > 0.0f)
		ImGui::PopFont();
}

void RenderPopupCard(FETexture* Preview, const std::string& Name, const std::vector<std::string>& Lines)
{
	const float Padding = 10.0f;
	const float PreviewSize = 72.0f;
	const float SmallFontSize = 16.0f;
	// Card without a picture is as high as its text.
	const float TextHeight = ImGui::GetFontSize() + 10.0f + static_cast<float>(Lines.size()) * (SmallFontSize + 4.0f);
	const float ContentHeight = Preview != nullptr ? PreviewSize : TextHeight;
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + StyledPopupWidth, Min.y + ContentHeight + Padding * 2.0f);
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, FEEditorStyle::Background, 4.0f);
	DrawList->AddRect(Min, Max, FEEditorStyle::CardBorder, 4.0f);

	float TextX = Min.x + Padding + 4.0f;
	if (Preview != nullptr)
	{
		DrawList->AddImage(Preview->GetTextureID(), ImVec2(Min.x + Padding, Min.y + Padding), ImVec2(Min.x + Padding + PreviewSize, Min.y + Padding + PreviewSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		TextX = Min.x + Padding + PreviewSize + 14.0f;
	}

	DrawList->PushClipRect(ImVec2(TextX, Min.y), ImVec2(Max.x - Padding, Max.y), true);
	DrawList->AddText(ImVec2(TextX, Min.y + Padding + 2.0f), FEEditorStyle::Text, Name.c_str());
	float LineY = Min.y + Padding + ImGui::GetFontSize() + 14.0f;
	ImGui::PushFont(nullptr, SmallFontSize);
	for (size_t i = 0; i < Lines.size(); i++)
	{
		DrawList->AddText(ImVec2(TextX, LineY), FEEditorStyle::TextMuted, Lines[i].c_str());
		LineY += SmallFontSize + 4.0f;
	}
	ImGui::PopFont();
	DrawList->PopClipRect();

	ImGui::Dummy(ImVec2(StyledPopupWidth, ContentHeight + Padding * 2.0f));
}

void RenderPopupNameList(const std::vector<std::string>& Names, const size_t MaxShown)
{
	const float SmallFontSize = 16.0f;
	const float NameIndent = 22.0f;
	ImGui::PushFont(nullptr, SmallFontSize);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 4.0f));
	for (size_t i = 0; i < Names.size() && i < MaxShown; i++)
	{
		const ImVec2 Position = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(Position.x + 8.0f, floorf(Position.y + SmallFontSize / 2.0f)), 3.0f, FEEditorStyle::TextMuted);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + NameIndent);
		ImGui::TextUnformatted(Names[i].c_str());
	}

	if (Names.size() > MaxShown)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + NameIndent);
		ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
		ImGui::TextUnformatted(("and " + GetNumberWithSeparators(Names.size() - MaxShown) + " more").c_str());
		ImGui::PopStyleColor();
	}
	ImGui::PopStyleVar();
	ImGui::PopFont();
}

int RenderPopupButtons(const std::vector<FEPopupButton>& Buttons)
{
	const float Spacing = 10.0f;
	std::vector<float> Widths;
	float TotalWidth = 0.0f;
	for (size_t i = 0; i < Buttons.size(); i++)
	{
		Widths.push_back(std::max(130.0f, ImGui::CalcTextSize(Buttons[i].Name).x + 40.0f));
		TotalWidth += Widths[i] + (i == 0 ? 0.0f : Spacing);
	}

	int Result = -1;
	const float Height = ImGui::GetFrameHeight() + 10.0f;
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + StyledPopupWidth - TotalWidth, ImGui::GetCursorPosY() + 12.0f));
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	for (size_t i = 0; i < Buttons.size(); i++)
	{
		const ImVec2 Min = ImGui::GetCursorScreenPos();
		const ImVec2 Max = ImVec2(Min.x + Widths[i], Min.y + Height);
		if (ImGui::InvisibleButton(Buttons[i].Name, ImVec2(Widths[i], Height)))
			Result = static_cast<int>(i);

		const bool bHovered = ImGui::IsItemHovered();
		ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
		ImU32 TextColor = FEEditorStyle::Text;
		if (Buttons[i].Kind == FEPopupButtonKind::Primary)
		{
			Fill = bHovered ? FEEditorStyle::PrimaryButtonHovered : FEEditorStyle::PrimaryButton;
		}
		else if (Buttons[i].Kind == FEPopupButtonKind::Danger)
		{
			Fill = bHovered ? FEEditorStyle::DangerButtonHovered : FEEditorStyle::DangerButton;
			TextColor = IM_COL32(255, 255, 255, 255);
		}
		else if (Buttons[i].Kind == FEPopupButtonKind::DangerText)
		{
			TextColor = FEEditorStyle::Danger;
		}

		DrawList->AddRectFilled(Min, Max, Fill, 4.0f);
		if (Buttons[i].Kind == FEPopupButtonKind::Normal || Buttons[i].Kind == FEPopupButtonKind::DangerText)
			DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f);

		const ImVec2 TextSize = ImGui::CalcTextSize(Buttons[i].Name);
		DrawList->AddText(ImVec2(floorf(Min.x + (Widths[i] - TextSize.x) / 2.0f), floorf(Min.y + (Height - TextSize.y) / 2.0f)), TextColor, Buttons[i].Name);
		if (i + 1 < Buttons.size())
			ImGui::SameLine(0.0f, Spacing);
	}

	if (Result == -1 && !Buttons.empty() && ImGui::IsKeyPressed(ImGuiKey_Escape))
		Result = 0;

	return Result;
}

void SetSelectedStyle(ImGuiImageButton* Button)
{
	Button->SetDefaultColor(SelectedStyle);
	Button->SetHoveredColor(SelectedStyle);
	Button->SetActiveColor(SelectedStyle);
}

void SetDefaultStyle(ImGuiImageButton* Button)
{
	Button->SetDefaultColor(DefaultColor);
	Button->SetHoveredColor(HoveredColor);
	Button->SetActiveColor(ActiveColor);
}

MessagePopUp::MessagePopUp() {};

void MessagePopUp::Show(const std::string NewWindowCaption, const std::string MessageToShow)
{
	bShouldOpen = true;
	Message = MessageToShow;
	PopupCaption = NewWindowCaption;
}

void MessagePopUp::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str());
	if (bOpened)
	{
		RenderPopupHeading(PopupCaption);
		RenderPopupText(Message, FEEditorStyle::Text, 0.0f);
		if (RenderPopupButtons({ { "OK", FEPopupButtonKind::Primary } }) != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}