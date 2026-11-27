#pragma once

#include "../FEngine.h"
#include "../ThirdParty/textEditor/TextEditor.h"
#include "../FEEditorStyle.h"
using namespace FocalEngine;

//silly windows manager
class ImGuiModalPopup;
class FEImGuiWindow;

class WindowsManager
{
public:
	SINGLETON_PUBLIC_PART(WindowsManager)

	void RegisterWindow(FEImGuiWindow* Window);
	void RegisterPopup(ImGuiModalPopup* Popup);

	bool IsRegisteredWindowWithCaption(std::string Caption) const;
	FEImGuiWindow* GetWindowByCaption(std::string Caption) const;

	void CloseAllPopups() const;
	void CloseAllWindows() const;

	void RenderAllWindows() const;

	void UnRegisterWindow(FEImGuiWindow* Window);

	// FIXME: Currently using internal ImGui functions. Need to find an alternative approach.
	ImGuiWindow* GetCurrentWindowImpl();
private:
	SINGLETON_PRIVATE_PART(WindowsManager)

	std::vector<ImGuiModalPopup*> PopUps;
	std::vector<FEImGuiWindow*> Windows;
};

#define FE_IMGUI_WINDOW_MANAGER WindowsManager::GetInstance()

class ImGuiModalPopup
{
	friend WindowsManager;
protected:
	bool bShouldOpen;
	bool bOpened;
	std::string PopupCaption;
	ImGuiModalPopup();
public:
	virtual void Show();
	virtual void Close();
	virtual void Render();
	bool IsOpened() const;
};

class ImGuiButton
{
	static const int BUTTON_CAPTION_SIZE = 512;
protected:
	char Caption[BUTTON_CAPTION_SIZE];
	ImVec2 Position = ImVec2(-1.0f, -1.0f);
	ImVec2 Size = ImVec2(120.0f, 0.0f);

	ImVec4 DefaultColor = FEEditorStyle::ToVec4(FEEditorStyle::Button);
	ImVec4 HoveredColor = FEEditorStyle::ToVec4(FEEditorStyle::ButtonHovered);
	ImVec4 ActiveColor = FEEditorStyle::ToVec4(FEEditorStyle::Accent);

	bool bHovered = false;
	bool bWasClicked = false;
	void RenderBegin();
	void RenderEnd();
public:
	ImGuiButton(std::string Caption);
	void Render();

	void SetCaption(std::string NewCaption);

	ImVec4 GetDefaultColor() const;
	void SetDefaultColor(ImVec4 NewDefaultColor);

	ImVec4 GetHoveredColor() const;
	void SetHoveredColor(ImVec4 NewHoveredColor);

	ImVec4 GetActiveColor() const;
	void SetActiveColor(ImVec4 NewActiveColor);

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewSize);

	bool IsClicked() const;
};

class ImGuiImageButton
{
protected:
	FEUUID ID;
	ImVec2 Position = ImVec2(-1.0f, -1.0f);
	ImVec2 Size = ImVec2(32.0f, 32.0f);
	ImVec2 UV0 = ImVec2(0.0f, 0.0f);
	ImVec2 UV1 = ImVec2(1.0f, 1.0f);
	float FramePadding = 4.0f;
	ImVec4 BackgroundColor = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 TintColor = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec4 DefaultColor = FEEditorStyle::ToVec4(FEEditorStyle::Button);
	ImVec4 HoveredColor = FEEditorStyle::ToVec4(FEEditorStyle::ButtonHovered);
	ImVec4 ActiveColor = FEEditorStyle::ToVec4(FEEditorStyle::Accent);

	FETexture* Texture = nullptr;

	bool bHovered = false;
	bool bWasClicked = false;
	void RenderBegin();
	void RenderEnd();
public:
	ImGuiImageButton(FETexture* Texture);
	void Render();

	ImVec4 GetDefaultColor() const;
	void SetDefaultColor(ImVec4 NewDefaultColor);

	ImVec4 GetHoveredColor() const;
	void SetHoveredColor(ImVec4 NewHoveredColor);

	ImVec4 GetActiveColor() const;
	void SetActiveColor(ImVec4 NewActiveColor);

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewSize);

	ImVec2 GetUV0() const;
	void SetUV0(ImVec2 NewValue);

	ImVec2 GetUV1() const;
	void SetUV1(ImVec2 NewValue);
	
	float GetFramePadding() const;
	void SetFramePadding(float NewFramePadding);

	ImVec4 GetBackgroundColor() const;
	void SetBackgroundColor(ImVec4 NewBackgroundColor);

	ImVec4 GetTintColor() const;
	void SetTintColor(ImVec4 NewTintColor);

	FETexture* GetTexture() const;
	void SetTexture(FETexture* NewTexture);

	bool IsHovered() const;

	bool IsClicked() const;
};

class FEImGuiWindow
{
	friend WindowsManager;
protected:
	bool bVisible;
	char Caption[512];
	ImVec2 Position;
	ImVec2 Size;
	int Flags = ImGuiWindowFlags_None;
	bool bWasClosedLastFrame = false;
	FEImGuiWindow();
	ImGuiWindow* Window = nullptr;

	bool bHaveCloseButton = false;
	bool bUserWantsWindowToBeOpen = true;
	bool bUserRequestedClose = false;

	float BorderSize = 2.0f;
	glm::vec2 Padding = glm::vec2(15.0f, 15.0f);
public:
	virtual ~FEImGuiWindow();
	virtual void Show();
	virtual void Close();
	virtual void Render();
	virtual void OnRenderEnd();

	bool IsVisible() const;
	void SetVisible(bool NewValue);

	bool GetUserRequestedClose() const;

	bool IsMouseHovered() const;

	virtual void SetCaption(std::string NewCaption);
	std::string GetCaption() const;

	ImGuiWindow* GetWindow() const;

	float GetBorderSize() const;
	void SetBorderSize(float NewValue);

	glm::vec2 GetPadding() const;
	void SetPadding(glm::vec2 NewValue);
};

void ShowToolTip(const char* Text);
// Label of a row, what is rendered after it starts at LabelWidth from the start of the row.
void RenderRowLabel(const char* Label, float LabelWidth, float LabelIndent = 0.0f);
std::string GetNumberWithSeparators(size_t Number);
std::string ToLowerCase(std::string Text);
// Text that is wider than Width in the current font is cut and gets "..." at the end.
std::string CutTextToWidth(std::string Text, float Width);
// One button of a switch, RoundFlags says which corners are round.
bool RenderSwitchButton(const char* Label, bool bSelected, ImDrawFlags RoundFlags);

enum class FEPopupTone { Neutral, Danger };
enum class FEPopupButtonKind { Normal, Primary, Danger, DangerText };

struct FEPopupButton
{
	const char* Name;
	FEPopupButtonKind Kind = FEPopupButtonKind::Normal;
};

// Popup in the center of the editor, EndStyledPopup should be called even if it was not opened.
bool BeginStyledPopup(const char* ID, FEPopupTone Tone = FEPopupTone::Neutral, float Width = 620.0f);
void EndStyledPopup(bool bOpened);
void RenderPopupHeading(const std::string& Text);
// FontSize 0 is the size of the editor font.
void RenderPopupText(const std::string& Text, ImU32 Color = FEEditorStyle::TextMuted, float FontSize = 16.0f);
void RenderPopupCard(FETexture* Preview, const std::string& Name, const std::vector<std::string>& Lines);
void RenderPopupNameList(const std::vector<std::string>& Names, size_t MaxShown = 4);
// Returns index of the pressed button or -1, the last button is the main action and Escape presses the first one.
int RenderPopupButtons(const std::vector<FEPopupButton>& Buttons);
static ImVec4 DefaultColor = FEEditorStyle::ToVec4(FEEditorStyle::Button);
static ImVec4 HoveredColor = FEEditorStyle::ToVec4(FEEditorStyle::ButtonHovered);
static ImVec4 ActiveColor = FEEditorStyle::ToVec4(FEEditorStyle::Accent);
static ImVec4 SelectedStyle = FEEditorStyle::ToVec4(FEEditorStyle::Accent);
void SetSelectedStyle(ImGuiImageButton* Button);
void SetDefaultStyle(ImGuiImageButton* Button);

class JustTextWindow : public FEImGuiWindow
{
	TextEditor Editor;
	ImGuiButton* OkButton = nullptr;
public:
	JustTextWindow()
	{
		Flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
		Editor.SetShowWhitespaces(false);
		Editor.SetReadOnly(true);
		Size = ImVec2(800, 600);
		OkButton = new ImGuiButton("OK");
		Editor.SetPalette(TextEditor::GetLightPalette());
	}

	~JustTextWindow()
	{
		delete OkButton;
	}

	void Show(const std::string Text, std::string Caption)
	{
		Editor.SetText(Text);

		if (Caption.empty())
			Caption = "Text view";

		strcpy_s(this->Caption, Caption.size() + 1, Caption.c_str());
		FEImGuiWindow::Show();
	}

	void Render() override
	{
		FEImGuiWindow::Render();

		if (!IsVisible())
			return;

		OkButton->Render();
		if (OkButton->IsClicked())
		{
			FEImGuiWindow::Close();
		}

		Editor.Render("TextEditor");
		FEImGuiWindow::OnRenderEnd();
	}
};
inline JustTextWindow JustTextWindowObject;

class MessagePopUp : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(MessagePopUp)
	std::string Message;
public:
	SINGLETON_PUBLIC_PART(MessagePopUp)

	void Show(std::string NewWindowCaption, std::string MessageToShow);
	void Render() override;
};