#include "FEProjectManager.h"
#include "../../FEEditor.h"
#include "../../EditorWindows/PluginsWindow.h"
#include <shellapi.h>

namespace
{

	const float TopBarHeight = 64.0f;
	const float BottomBarHeight = 80.0f;
	const float DetailsPanelWidth = 560.0f;

	enum class ListButtonKind { Normal, Accent, Danger, DangerFilled, Segment };

	const std::pair<ImGuiCol, ImU32> ProjectListStyleColors[] = {
		{ ImGuiCol_Text, FEEditorStyle::Text },
		{ ImGuiCol_TextDisabled, FEEditorStyle::TextMuted },
		{ ImGuiCol_WindowBg, FEEditorStyle::Panel },
		{ ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0) },
		{ ImGuiCol_PopupBg, FEEditorStyle::Card },
		{ ImGuiCol_Border, FEEditorStyle::ButtonBorder },
		{ ImGuiCol_TitleBg, FEEditorStyle::Button },
		{ ImGuiCol_TitleBgActive, FEEditorStyle::Button },
		{ ImGuiCol_TitleBgCollapsed, FEEditorStyle::Button },
		{ ImGuiCol_FrameBg, FEEditorStyle::Background },
		{ ImGuiCol_FrameBgHovered, FEEditorStyle::Card },
		{ ImGuiCol_FrameBgActive, FEEditorStyle::Card },
		{ ImGuiCol_Button, FEEditorStyle::Button },
		{ ImGuiCol_ButtonHovered, FEEditorStyle::ButtonHovered },
		{ ImGuiCol_ButtonActive, FEEditorStyle::ButtonBorder },
		{ ImGuiCol_Header, FEEditorStyle::Card },
		{ ImGuiCol_HeaderHovered, FEEditorStyle::CardHovered },
		{ ImGuiCol_HeaderActive, FEEditorStyle::Button },
		{ ImGuiCol_Separator, FEEditorStyle::CardBorder },
		{ ImGuiCol_Tab, FEEditorStyle::Card },
		{ ImGuiCol_TabHovered, FEEditorStyle::ButtonHovered },
		{ ImGuiCol_TabSelected, FEEditorStyle::Button },
		{ ImGuiCol_TabSelectedOverline, FEEditorStyle::Accent },
		{ ImGuiCol_TabDimmed, FEEditorStyle::Card },
		{ ImGuiCol_TabDimmedSelected, FEEditorStyle::Button },
		{ ImGuiCol_TableHeaderBg, FEEditorStyle::Card },
		{ ImGuiCol_TableBorderStrong, FEEditorStyle::CardBorder },
		{ ImGuiCol_TableBorderLight, FEEditorStyle::CardBorder },
		{ ImGuiCol_TableRowBg, IM_COL32(0, 0, 0, 0) },
		{ ImGuiCol_TableRowBgAlt, IM_COL32(255, 255, 255, 6) },
		{ ImGuiCol_CheckMark, FEEditorStyle::Accent },
		{ ImGuiCol_ScrollbarBg, IM_COL32(0, 0, 0, 0) },
		{ ImGuiCol_ScrollbarGrab, FEEditorStyle::ButtonHovered },
		{ ImGuiCol_ScrollbarGrabHovered, FEEditorStyle::ButtonBorder },
		{ ImGuiCol_ScrollbarGrabActive, FEEditorStyle::ButtonBorder },
		{ ImGuiCol_ModalWindowDimBg, FEEditorStyle::Dim }
	};
}

// All text of the project list is drawn with these two fonts.
static ImFont* GetListFont()
{
	return ImGui::GetIO().Fonts->Fonts[0];
}

static ImFont* GetListBoldFont()
{
	return ImGui::GetIO().Fonts->Fonts[0];
}

void FEProjectManager::PushProjectListStyle()
{
	ImGui::PushFont(GetListFont(), 14.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 8.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.0f);
	for (const std::pair<ImGuiCol, ImU32>& StyleColor : ProjectListStyleColors)
		ImGui::PushStyleColor(StyleColor.first, StyleColor.second);
}

void FEProjectManager::PopProjectListStyle()
{
	ImGui::PopStyleColor(IM_ARRAYSIZE(ProjectListStyleColors));
	ImGui::PopStyleVar(9);
	ImGui::PopFont();
}

static float GetTextWidth(ImFont* Font, const float Size, const std::string& Text)
{
	return Font->CalcTextSizeA(Size, FLT_MAX, 0.0f, Text.c_str()).x;
}

static void DrawListText(ImFont* Font, const float Size, const ImVec2 Position, const ImU32 Color, const std::string& Text, const float WrapWidth = 0.0f)
{
	ImGui::GetWindowDrawList()->AddText(Font, Size, ImVec2(floorf(Position.x), floorf(Position.y)), Color, Text.c_str(), nullptr, WrapWidth);
}

static std::string CutWithEllipsis(ImFont* Font, const float Size, const std::string& Text, const float MaxWidth)
{
	if (GetTextWidth(Font, Size, Text) <= MaxWidth)
		return Text;

	size_t Low = 0;
	size_t High = Text.size();
	while (Low < High)
	{
		const size_t Middle = (Low + High + 1) / 2;
		if (GetTextWidth(Font, Size, Text.substr(0, Middle) + "...") <= MaxWidth)
		{
			Low = Middle;
		}
		else
		{
			High = Middle - 1;
		}
	}

	return Text.substr(0, Low) + "...";
}

// End of a path is kept, because it is the most informative part.
static std::string CutPathWithEllipsis(ImFont* Font, const float Size, const std::string& Path, const float MaxWidth)
{
	if (GetTextWidth(Font, Size, Path) <= MaxWidth)
		return Path;

	size_t Low = 0;
	size_t High = Path.size();
	while (Low < High)
	{
		const size_t Middle = (Low + High + 1) / 2;
		if (GetTextWidth(Font, Size, "..." + Path.substr(Path.size() - Middle)) <= MaxWidth)
		{
			Low = Middle;
		}
		else
		{
			High = Middle - 1;
		}
	}

	return "..." + Path.substr(Path.size() - Low);
}

static std::string ToLowerCase(std::string Text)
{
	std::transform(Text.begin(), Text.end(), Text.begin(), [](const unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
	return Text;
}

// Short form is "Today, 14:20", long form is "Opened today, 14:20".
static std::string GetOpenedTimeText(const uint64_t Time, const bool bLongForm)
{
	if (Time == 0)
		return bLongForm ? "Not opened yet" : "Never";

	const time_t OpenedTime = static_cast<time_t>(Time);
	tm LocalOpenedTime = {};
	localtime_s(&LocalOpenedTime, &OpenedTime);

	const time_t CurrentTime = static_cast<time_t>(TIME.GetTimeStamp(FE_TIME_RESOLUTION_SECONDS));
	tm StartOfToday = {};
	localtime_s(&StartOfToday, &CurrentTime);
	StartOfToday.tm_hour = 0;
	StartOfToday.tm_min = 0;
	StartOfToday.tm_sec = 0;
	const time_t StartOfTodayTime = mktime(&StartOfToday);

	char Buffer[64] = {};
	std::string Result;
	if (OpenedTime >= StartOfTodayTime)
	{
		strftime(Buffer, sizeof(Buffer), "%H:%M", &LocalOpenedTime);
		Result = std::string("Today, ") + Buffer;
	}
	else if (OpenedTime >= StartOfTodayTime - 24 * 60 * 60)
	{
		strftime(Buffer, sizeof(Buffer), "%H:%M", &LocalOpenedTime);
		Result = std::string("Yesterday, ") + Buffer;
	}
	else
	{
		strftime(Buffer, sizeof(Buffer), "%Y-%m-%d", &LocalOpenedTime);
		Result = Buffer;
	}

	if (bLongForm)
	{
		Result[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(Result[0])));
		Result = "Opened " + Result;
	}

	return Result;
}

static float GetListButtonWidth(const std::string& Label, const float Padding, const bool bBold = false)
{
	return floorf(GetTextWidth(bBold ? GetListBoldFont() : GetListFont(), 14.0f, Label) + Padding * 2.0f);
}

static bool ListButton(const char* Label, const ImVec2 Size, const ListButtonKind Kind = ListButtonKind::Normal, const bool bEnabled = true, const ImDrawFlags RoundFlags = ImDrawFlags_RoundCornersAll)
{
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + Size.x, Min.y + Size.y);
	const bool bPressed = ImGui::InvisibleButton(Label, Size) && bEnabled;
	const bool bHovered = ImGui::IsItemHovered() && bEnabled;

	ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
	ImU32 Border = FEEditorStyle::ButtonBorder;
	ImU32 TextColor = FEEditorStyle::Text;
	ImFont* Font = GetListFont();
	if (Kind == ListButtonKind::Accent && bEnabled)
	{
		Fill = bHovered ? FEEditorStyle::AccentHovered : FEEditorStyle::Accent;
		Border = 0;
		TextColor = FEEditorStyle::TextOnAccent;
		Font = GetListBoldFont();
	}
	else if (Kind == ListButtonKind::Danger)
	{
		TextColor = FEEditorStyle::Danger;
	}
	else if (Kind == ListButtonKind::DangerFilled)
	{
		Fill = bHovered ? FEEditorStyle::DangerButtonHovered : FEEditorStyle::DangerButton;
		Border = 0;
		TextColor = IM_COL32_WHITE;
		Font = GetListBoldFont();
	}
	else if (Kind == ListButtonKind::Segment)
	{
		Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Card;
	}

	if (!bEnabled)
		TextColor = FEEditorStyle::TextDisabled;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, Fill, 4.0f, RoundFlags);
	if (Border != 0)
		DrawList->AddRect(Min, Max, Border, 4.0f, 1.0f, RoundFlags);

	std::string Text = Label;
	if (Text.find("##") != std::string::npos)
		Text = Text.substr(0, Text.find("##"));

	const float TextWidth = GetTextWidth(Font, 14.0f, Text);
	DrawListText(Font, 14.0f, ImVec2(Min.x + (Size.x - TextWidth) / 2.0f, Min.y + (Size.y - 14.0f) / 2.0f), TextColor, Text);
	return bPressed;
}

static void PushInputStyle(const ImU32 Background)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 11.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, Background);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Background);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Background);
	ImGui::PushStyleColor(ImGuiCol_Border, FEEditorStyle::ButtonBorder);
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Text);
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, FEEditorStyle::TextMuted);
	ImGui::PushStyleColor(ImGuiCol_Button, FEEditorStyle::Button);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, FEEditorStyle::ButtonHovered);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, FEEditorStyle::ButtonHovered);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, FEEditorStyle::Card);
	ImGui::PushStyleColor(ImGuiCol_Header, FEEditorStyle::Button);
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, FEEditorStyle::ButtonHovered);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, FEEditorStyle::ButtonHovered);
}

static void PopInputStyle()
{
	ImGui::PopStyleColor(13);
	ImGui::PopStyleVar(6);
}

static void DrawThumbnail(FEProjectInfo* Project, const ImVec2 Min, const ImVec2 Max, const float Rounding, const bool bShowMissingText)
{
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	FETexture* Thumbnail = Project->GetThumbnail();
	if (Project->IsMissing() || Thumbnail == nullptr)
	{
		DrawList->AddRectFilled(Min, Max, FEEditorStyle::CardHovered, Rounding);
		if (Project->IsMissing() && bShowMissingText)
		{
			const std::string Text = "Folder not found";
			const float TextWidth = GetTextWidth(GetListBoldFont(), 13.0f, Text);
			DrawListText(GetListBoldFont(), 13.0f, ImVec2(Min.x + (Max.x - Min.x - TextWidth) / 2.0f, Min.y + (Max.y - Min.y - 13.0f) / 2.0f), FEEditorStyle::Warning, Text);
		}

		return;
	}

	DrawList->AddImageRounded(static_cast<ImTextureID>(Thumbnail->GetTextureID()), Min, Max, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), IM_COL32_WHITE, Rounding);
}

static bool MenuRow(const char* Label, const ImU32 TextColor = FEEditorStyle::Text, const bool bEnabled = true)
{
	const ImVec2 Size = ImVec2(208.0f, 32.0f);
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const bool bPressed = ImGui::InvisibleButton(Label, Size) && bEnabled;
	if (ImGui::IsItemHovered() && bEnabled)
		ImGui::GetWindowDrawList()->AddRectFilled(Min, ImVec2(Min.x + Size.x, Min.y + Size.y), FEEditorStyle::ButtonHovered, 3.0f);

	DrawListText(GetListFont(), 14.0f, ImVec2(Min.x + 10.0f, Min.y + 9.0f), bEnabled ? TextColor : FEEditorStyle::TextDisabled, Label);
	if (bPressed)
		ImGui::CloseCurrentPopup();

	return bPressed;
}

static void MenuSeparator()
{
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	ImGui::Dummy(ImVec2(208.0f, 13.0f));
	ImGui::GetWindowDrawList()->AddLine(ImVec2(Min.x + 4.0f, Min.y + 6.5f), ImVec2(Min.x + 204.0f, Min.y + 6.5f), FEEditorStyle::PopupBorder);
}

// Plugins window is shown over the project list.
static bool IsCoveredByWindow()
{
	return PLUGINS_WINDOW.IsVisible();
}

static void DimCurrentWindowIfCovered()
{
	if (!IsCoveredByWindow())
		return;

	const ImVec2 Position = ImGui::GetWindowPos();
	const ImVec2 Size = ImGui::GetWindowSize();
	ImGui::GetWindowDrawList()->AddRectFilled(Position, ImVec2(Position.x + Size.x, Position.y + Size.y), FEEditorStyle::Dim);
}

static void ShowInExplorer(const std::string& Folder)
{
	const HINSTANCE Result = ShellExecuteA(nullptr, "open", Folder.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	if (reinterpret_cast<INT_PTR>(Result) <= 32)
		LOG.Add("FEProjectManager: Failed to open \"" + Folder + "\" in file explorer", "FE_EDITOR", FE_LOG_WARNING);
}

void FEProjectManager::ShowPopup(const std::string& Name, FEProjectInfo* Project)
{
	RequestedPopup = Name;
	PopupProject = Project;
	PopupError = "";
	strncpy_s(PopupName, Project != nullptr ? Project->GetName().c_str() : "", _TRUNCATE);
	if (Name == "New Project")
		strcpy_s(NewProjectLocation, "Projects\\");
}

void FEProjectManager::ShowMessage(const std::string& Text)
{
	Message = Text;
	RequestedPopup = "Project List";
}

void FEProjectManager::LocateProjectWithDialog(FEProjectInfo* Project)
{
	FILE_SYSTEM.OpenFolderDialog([this, Project](const std::string& Path) {
		if (Path.empty() || !IsInProjectList(Project))
			return;

		if (!LocateProject(Project, Path))
			ShowMessage("\"" + Path + "\" does not contain project \"" + Project->GetName() + "\".");
	});
}

std::vector<FEProjectInfo*> FEProjectManager::GetVisibleProjects()
{
	std::vector<FEProjectInfo*> Result;
	const std::string LowerCaseFilter = ToLowerCase(Filter);
	for (size_t i = 0; i < List.size(); i++)
	{
		if (ToLowerCase(List[i]->GetName()).find(LowerCaseFilter) != std::string::npos)
			Result.push_back(List[i]);
	}

	if (Sorting == 1)
	{
		std::stable_sort(Result.begin(), Result.end(), [](const FEProjectInfo* First, const FEProjectInfo* Second) {
			return ToLowerCase(First->GetName()) < ToLowerCase(Second->GetName());
		});
	}
	else if (Sorting == 2)
	{
		std::stable_sort(Result.begin(), Result.end(), [](const FEProjectInfo* First, const FEProjectInfo* Second) {
			return First->GetAddedTime() > Second->GetAddedTime();
		});
	}
	else
	{
		std::stable_sort(Result.begin(), Result.end(), [](const FEProjectInfo* First, const FEProjectInfo* Second) {
			if (First->GetLastOpenedTime() != Second->GetLastOpenedTime())
				return First->GetLastOpenedTime() > Second->GetLastOpenedTime();

			return First->GetAddedTime() > Second->GetAddedTime();
		});
	}

	return Result;
}

// Should be called right after item of the project.
void FEProjectManager::DisplayProjectMenu(FEProjectInfo* Project)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, FEEditorStyle::CardHovered);
	ImGui::PushStyleColor(ImGuiCol_Border, FEEditorStyle::PopupBorder);

	if (ImGui::BeginPopupContextItem("##ProjectMenu"))
	{
		Selected = Project;
		const bool bCanBeRemoved = CanBeRemovedFromProjectList(Project);
		if (Project->IsMissing())
		{
			if (MenuRow("Locate folder..."))
				LocateProjectWithDialog(Project);
		}
		else
		{
			if (MenuRow("Open"))
				DeferredAction = [this, Project]() { OpenProjectFromList(Project); };

			if (MenuRow("Rename"))
				ShowPopup("Rename Project", Project);

			if (MenuRow("Show in Explorer"))
				ShowInExplorer(Project->GetFolder());

			MenuSeparator();
		}

		if (MenuRow("Remove from list", FEEditorStyle::Text, bCanBeRemoved))
			DeferredAction = [this, Project]() { RemoveFromProjectList(Project); };

		if (!bCanBeRemoved && ImGui::IsItemHovered())
			ImGui::SetTooltip("Projects of the default Projects folder are always in the list.");

		if (!Project->IsMissing() && MenuRow("Delete project", FEEditorStyle::Danger))
			ShowPopup("Delete Project", Project);

		ImGui::EndPopup();
	}

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(4);
}

void FEProjectManager::DisplayTopBar(const ImVec2 Origin, const float Width)
{
	const float Padding = bListView ? 24.0f : 32.0f;
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Origin, ImVec2(Origin.x + Width, Origin.y + TopBarHeight), FEEditorStyle::Panel);
	DrawList->AddLine(ImVec2(Origin.x, Origin.y + TopBarHeight - 0.5f), ImVec2(Origin.x + Width, Origin.y + TopBarHeight - 0.5f), FEEditorStyle::CardBorder);

	DrawListText(GetListBoldFont(), 20.0f, ImVec2(Origin.x + Padding, Origin.y + 21.0f), FEEditorStyle::Text, "Projects");
	const std::string CountText = List.size() == 1 ? "1 project" : std::to_string(List.size()) + " projects";
	DrawListText(GetListFont(), 13.0f, ImVec2(Origin.x + Padding + GetTextWidth(GetListBoldFont(), 20.0f, "Projects") + 16.0f, Origin.y + 27.0f), FEEditorStyle::TextMuted, CountText);

	const float ControlY = Origin.y + 14.0f;
	float X = Origin.x + Width - Padding;

	const float ListWidth = GetListButtonWidth("List", 16.0f, bListView);
	const float GridWidth = GetListButtonWidth("Grid", 16.0f, !bListView);
	const bool bWasListView = bListView;
	X -= ListWidth;
	ImGui::SetCursorScreenPos(ImVec2(X, ControlY));
	if (ListButton("List", ImVec2(ListWidth, 36.0f), bWasListView ? ListButtonKind::Accent : ListButtonKind::Segment, true, ImDrawFlags_RoundCornersRight))
		bListView = true;

	X -= GridWidth;
	ImGui::SetCursorScreenPos(ImVec2(X, ControlY));
	if (ListButton("Grid", ImVec2(GridWidth, 36.0f), bWasListView ? ListButtonKind::Segment : ListButtonKind::Accent, true, ImDrawFlags_RoundCornersLeft))
		bListView = false;

	if (bListView != bWasListView)
		SaveProjectList();

	const float SortingWidth = bWasListView ? 150.0f : 170.0f;
	X -= 16.0f + SortingWidth;
	ImGui::SetCursorScreenPos(ImVec2(X, ControlY));
	PushInputStyle(FEEditorStyle::Card);
	ImGui::SetNextItemWidth(SortingWidth);
	if (ImGui::Combo("##Sorting", &Sorting, "Last opened\0Name\0Date added\0"))
		SaveProjectList();
	PopInputStyle();
	X -= 8.0f + GetTextWidth(GetListFont(), 13.0f, "Sort by");
	DrawListText(GetListFont(), 13.0f, ImVec2(X, Origin.y + 26.0f), FEEditorStyle::TextMuted, "Sort by");

	const float FilterWidth = bWasListView ? 220.0f : 260.0f;
	X -= 16.0f + FilterWidth;
	ImGui::SetCursorScreenPos(ImVec2(X, ControlY));
	PushInputStyle(FEEditorStyle::Card);
	ImGui::SetNextItemWidth(FilterWidth);
	ImGui::InputTextWithHint("##Filter", "Project name", Filter, IM_ARRAYSIZE(Filter));
	PopInputStyle();
	X -= 8.0f + GetTextWidth(GetListFont(), 13.0f, "Filter");
	DrawListText(GetListFont(), 13.0f, ImVec2(X, Origin.y + 26.0f), FEEditorStyle::TextMuted, "Filter");
}

void FEProjectManager::DisplayGridView(const std::vector<FEProjectInfo*>& Projects, const ImVec2 Origin, const float Width, const float Height)
{
	ImGui::SetCursorScreenPos(ImVec2(Origin.x, Origin.y + TopBarHeight));
	ImGui::BeginChild("##ProjectGrid", ImVec2(Width, Height - TopBarHeight - BottomBarHeight), ImGuiChildFlags_None, IsCoveredByWindow() ? ImGuiWindowFlags_NoInputs : ImGuiWindowFlags_None);

	const float Padding = 32.0f;
	const float Gap = 24.0f;
	const float MinCardWidth = 340.0f;
	const float Available = Width - Padding * 2.0f;
	const int Columns = std::max(1, static_cast<int>((Available + Gap) / (MinCardWidth + Gap)));
	const float CardWidth = floorf((Available - Gap * static_cast<float>(Columns - 1)) / static_cast<float>(Columns));
	const float ThumbnailHeight = floorf((CardWidth - 16.0f) * 9.0f / 16.0f);
	const float CardHeight = ThumbnailHeight + 82.0f;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	for (size_t i = 0; i < Projects.size(); i++)
	{
		FEProjectInfo* Project = Projects[i];
		const int Column = static_cast<int>(i) % Columns;
		const int Row = static_cast<int>(i) / Columns;

		ImGui::PushID(Project);
		ImGui::SetCursorPos(ImVec2(Padding + static_cast<float>(Column) * (CardWidth + Gap), 24.0f + static_cast<float>(Row) * (CardHeight + Gap)));
		const ImVec2 Min = ImGui::GetCursorScreenPos();
		const ImVec2 Max = ImVec2(Min.x + CardWidth, Min.y + CardHeight);
		if (ImGui::InvisibleButton("##Card", ImVec2(CardWidth, CardHeight)))
			Selected = Project;

		const bool bHovered = ImGui::IsItemHovered();
		if (bHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			DeferredAction = [this, Project]() { OpenProjectFromList(Project); };

		DisplayProjectMenu(Project);

		ImU32 Fill = bHovered ? FEEditorStyle::CardHovered : FEEditorStyle::Card;
		if (Project->IsMissing())
			Fill = FEEditorStyle::Panel;

		DrawList->AddRectFilled(Min, Max, Fill, 6.0f);
		if (Selected == Project)
		{
			DrawList->AddRect(ImVec2(Min.x + 0.5f, Min.y + 0.5f), ImVec2(Max.x - 0.5f, Max.y - 0.5f), FEEditorStyle::Accent, 6.0f, 2.0f);
		}
		else
		{
			DrawList->AddRect(Min, Max, FEEditorStyle::CardBorder, 6.0f);
		}

		const ImVec2 ThumbnailMin = ImVec2(Min.x + 8.0f, Min.y + 8.0f);
		const ImVec2 ThumbnailMax = ImVec2(Max.x - 8.0f, Min.y + 8.0f + ThumbnailHeight);
		DrawThumbnail(Project, ThumbnailMin, ThumbnailMax, 3.0f, true);

		const float TextWidth = CardWidth - 16.0f;
		float TextY = ThumbnailMax.y + 10.0f;
		DrawListText(GetListBoldFont(), 15.0f, ImVec2(ThumbnailMin.x, TextY), Project->IsMissing() ? FEEditorStyle::TextMuted : FEEditorStyle::Text, CutWithEllipsis(GetListBoldFont(), 15.0f, Project->GetName(), TextWidth));
		TextY += 22.0f;
		DrawListText(GetListFont(), 12.0f, ImVec2(ThumbnailMin.x, TextY), FEEditorStyle::TextMuted, CutPathWithEllipsis(GetListFont(), 12.0f, Project->GetDisplayFolder(), TextWidth));
		TextY += 19.0f;
		DrawListText(GetListFont(), 12.0f, ImVec2(ThumbnailMin.x, TextY), FEEditorStyle::TextMuted, GetOpenedTimeText(Project->GetLastOpenedTime(), true));

		ImGui::PopID();
	}

	const int Rows = (static_cast<int>(Projects.size()) + Columns - 1) / Columns;
	ImGui::SetCursorPos(ImVec2(0.0f, 24.0f + static_cast<float>(Rows) * (CardHeight + Gap) - 1.0f));
	ImGui::Dummy(ImVec2(1.0f, 1.0f));
	DimCurrentWindowIfCovered();
	ImGui::EndChild();
}

void FEProjectManager::DisplayListView(const std::vector<FEProjectInfo*>& Projects, const ImVec2 Origin, const float Width, const float Height)
{
	ImGui::SetCursorScreenPos(ImVec2(Origin.x, Origin.y + TopBarHeight));
	ImGui::BeginChild("##ProjectRows", ImVec2(Width, Height - TopBarHeight - BottomBarHeight), ImGuiChildFlags_None, IsCoveredByWindow() ? ImGuiWindowFlags_NoInputs : ImGuiWindowFlags_None);

	const float Padding = 24.0f;
	const float RowWidth = Width - Padding * 2.0f;
	const float RowHeight = 76.0f;
	const float Gap = 8.0f;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	for (size_t i = 0; i < Projects.size(); i++)
	{
		FEProjectInfo* Project = Projects[i];

		ImGui::PushID(Project);
		ImGui::SetCursorPos(ImVec2(Padding, 16.0f + static_cast<float>(i) * (RowHeight + Gap)));
		const ImVec2 Min = ImGui::GetCursorScreenPos();
		const ImVec2 Max = ImVec2(Min.x + RowWidth, Min.y + RowHeight);
		if (ImGui::InvisibleButton("##Row", ImVec2(RowWidth, RowHeight)))
			Selected = Project;

		const bool bHovered = ImGui::IsItemHovered();
		if (bHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			DeferredAction = [this, Project]() { OpenProjectFromList(Project); };

		DisplayProjectMenu(Project);

		ImU32 Fill = bHovered ? FEEditorStyle::CardHovered : FEEditorStyle::Card;
		if (Selected == Project)
		{
			Fill = FEEditorStyle::SelectedRow;
		}
		else if (Project->IsMissing())
		{
			Fill = FEEditorStyle::Panel;
		}

		DrawList->AddRectFilled(Min, Max, Fill, 6.0f);
		DrawList->AddRect(Min, Max, Selected == Project ? FEEditorStyle::Accent : FEEditorStyle::CardBorder, 6.0f);

		const ImVec2 ThumbnailMin = ImVec2(Min.x + 12.0f, Min.y + 11.0f);
		DrawThumbnail(Project, ThumbnailMin, ImVec2(ThumbnailMin.x + 96.0f, ThumbnailMin.y + 54.0f), 3.0f, false);

		const std::string RightText = Project->IsMissing() ? "Folder not found" : GetOpenedTimeText(Project->GetLastOpenedTime(), false);
		ImFont* RightFont = Project->IsMissing() ? GetListBoldFont() : GetListFont();
		const float RightX = Max.x - 12.0f - GetTextWidth(RightFont, 12.0f, RightText);
		DrawListText(RightFont, 12.0f, ImVec2(RightX, Min.y + 31.0f), Project->IsMissing() ? FEEditorStyle::Warning : FEEditorStyle::TextMuted, RightText);

		const float TextX = ThumbnailMin.x + 96.0f + 16.0f;
		const float TextWidth = RightX - 16.0f - TextX;
		DrawListText(GetListBoldFont(), 15.0f, ImVec2(TextX, Min.y + 19.0f), Project->IsMissing() ? FEEditorStyle::TextMuted : FEEditorStyle::Text, CutWithEllipsis(GetListBoldFont(), 15.0f, Project->GetName(), TextWidth));
		DrawListText(GetListFont(), 12.0f, ImVec2(TextX, Min.y + 42.0f), FEEditorStyle::TextMuted, CutPathWithEllipsis(GetListFont(), 12.0f, Project->GetDisplayFolder(), TextWidth));

		ImGui::PopID();
	}

	ImGui::SetCursorPos(ImVec2(0.0f, 16.0f + static_cast<float>(Projects.size()) * (RowHeight + Gap) + 7.0f));
	ImGui::Dummy(ImVec2(1.0f, 1.0f));
	DimCurrentWindowIfCovered();
	ImGui::EndChild();
}

void FEProjectManager::DisplayDetailsPanel(const ImVec2 Origin, const float Height)
{
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Origin, ImVec2(Origin.x + DetailsPanelWidth, Origin.y + Height), FEEditorStyle::Panel);
	DrawList->AddLine(ImVec2(Origin.x + 0.5f, Origin.y), ImVec2(Origin.x + 0.5f, Origin.y + Height), FEEditorStyle::CardBorder);

	const float Padding = 24.0f;
	const float ContentWidth = DetailsPanelWidth - Padding * 2.0f;
	const float X = Origin.x + Padding;
	if (!IsInProjectList(Selected))
	{
		DrawListText(GetListFont(), 14.0f, ImVec2(X, Origin.y + Padding), FEEditorStyle::TextMuted, "Select a project to see its details.");
		return;
	}

	FEProjectInfo* Project = Selected;
	float Y = Origin.y + Padding;
	DrawThumbnail(Project, ImVec2(X, Y), ImVec2(X + 512.0f, Y + 288.0f), 4.0f, true);
	Y += 288.0f + 20.0f;

	DrawListText(GetListBoldFont(), 22.0f, ImVec2(X, Y), FEEditorStyle::Text, CutWithEllipsis(GetListBoldFont(), 22.0f, Project->GetName(), ContentWidth));
	Y += 27.0f + 20.0f;

	std::vector<std::pair<std::string, std::string>> Properties;
	Properties.push_back({ "Folder", Project->GetFolder() });
	Properties.push_back({ "Project ID", UNIQUE_ID.ToString(Project->GetID()) });
	Properties.push_back({ "Last opened", GetOpenedTimeText(Project->GetLastOpenedTime(), false) });
	if (!Project->IsMissing())
	{
		Properties.push_back({ "Created with", Project->GetCreatedWith().empty() ? "Unknown" : Project->GetCreatedWith() });
		Properties.push_back({ "Saved with", Project->GetSavedWith().empty() ? "Unknown" : Project->GetSavedWith() });

		std::string Plugins;
		for (size_t i = 0; i < Project->GetPluginNames().size(); i++)
			Plugins += (i == 0 ? "" : ", ") + Project->GetPluginNames()[i];

		Properties.push_back({ "Plugins", Plugins.empty() ? "None enabled" : Plugins });
	}

	const float ValueX = X + 136.0f;
	const float ValueWidth = ContentWidth - 136.0f;
	for (size_t i = 0; i < Properties.size(); i++)
	{
		DrawListText(GetListFont(), 13.0f, ImVec2(X, Y), FEEditorStyle::TextMuted, Properties[i].first);
		DrawListText(GetListFont(), 13.0f, ImVec2(ValueX, Y), FEEditorStyle::Text, Properties[i].second, ValueWidth);
		Y += std::max(16.0f, GetListFont()->CalcTextSizeA(13.0f, FLT_MAX, ValueWidth, Properties[i].second.c_str()).y + 3.0f) + 10.0f;
	}

	const bool bCanBeRemoved = CanBeRemovedFromProjectList(Project);
	const float HalfWidth = (ContentWidth - 12.0f) / 2.0f;
	const float BottomRowY = Origin.y + Height - Padding - 44.0f;
	if (Project->IsMissing())
	{
		ImGui::SetCursorScreenPos(ImVec2(X, BottomRowY - 48.0f - 20.0f));
		if (ListButton("Locate folder...", ImVec2(ContentWidth, 48.0f), ListButtonKind::Accent))
			LocateProjectWithDialog(Project);

		ImGui::SetCursorScreenPos(ImVec2(X, BottomRowY));
		if (ListButton("Remove from list", ImVec2(ContentWidth, 44.0f)))
			DeferredAction = [this, Project]() { RemoveFromProjectList(Project); };

		return;
	}

	ImGui::SetCursorScreenPos(ImVec2(X, BottomRowY - 44.0f - 12.0f - 48.0f - 20.0f));
	if (ListButton("Open Project##Details", ImVec2(ContentWidth, 48.0f), ListButtonKind::Accent))
		DeferredAction = [this, Project]() { OpenProjectFromList(Project); };

	ImGui::SetCursorScreenPos(ImVec2(X, BottomRowY - 44.0f - 12.0f));
	if (ListButton("Show in Explorer", ImVec2(HalfWidth, 44.0f)))
		ShowInExplorer(Project->GetFolder());

	ImGui::SetCursorScreenPos(ImVec2(X + HalfWidth + 12.0f, BottomRowY - 44.0f - 12.0f));
	if (ListButton("Rename", ImVec2(HalfWidth, 44.0f)))
		ShowPopup("Rename Project", Project);

	ImGui::SetCursorScreenPos(ImVec2(X, BottomRowY));
	if (ListButton("Remove from list", ImVec2(HalfWidth, 44.0f), ListButtonKind::Normal, bCanBeRemoved))
		DeferredAction = [this, Project]() { RemoveFromProjectList(Project); };

	if (!bCanBeRemoved && ImGui::IsItemHovered())
		ImGui::SetTooltip("Projects of the default Projects folder are always in the list.");

	ImGui::SetCursorScreenPos(ImVec2(X + HalfWidth + 12.0f, BottomRowY));
	if (ListButton("Delete project", ImVec2(HalfWidth, 44.0f), ListButtonKind::Danger))
		ShowPopup("Delete Project", Project);
}

void FEProjectManager::DisplayBottomBar(const ImVec2 Origin, const float Width, const float Height)
{
	const float Padding = bListView ? 24.0f : 32.0f;
	const float BarY = Origin.y + Height - BottomBarHeight;
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(ImVec2(Origin.x, BarY), ImVec2(Origin.x + Width, Origin.y + Height), FEEditorStyle::Panel);
	DrawList->AddLine(ImVec2(Origin.x, BarY + 0.5f), ImVec2(Origin.x + Width, BarY + 0.5f), FEEditorStyle::CardBorder);

	const float ButtonY = BarY + 16.0f;
	const float CreateWidth = GetListButtonWidth("Create New Project", 24.0f);
	const float AddWidth = GetListButtonWidth("Add project...", 24.0f);
	const float AddFolderWidth = GetListButtonWidth("Add projects from folder...", 24.0f);
	const float PluginsWidth = GetListButtonWidth("Plugins", 24.0f);

	// In the list view project is opened with the button of details panel.
	float X = Origin.x + Padding;
	if (!bListView)
	{
		const float OpenWidth = GetListButtonWidth("Open Project", 24.0f, true);
		ImGui::SetCursorScreenPos(ImVec2(X, ButtonY));
		if (ListButton("Open Project", ImVec2(OpenWidth, 48.0f), ListButtonKind::Accent, IsInProjectList(Selected) && !Selected->IsMissing()))
			DeferredAction = [this]() { OpenProjectFromList(Selected); };

		X += OpenWidth + 12.0f;
	}

	ImGui::SetCursorScreenPos(ImVec2(X, ButtonY));
	if (ListButton("Create New Project", ImVec2(CreateWidth, 48.0f)))
		ShowPopup("New Project", nullptr);

	X += CreateWidth + 12.0f;
	if (!bListView)
		X = Origin.x + Width - Padding - PluginsWidth - 12.0f - AddFolderWidth - 12.0f - AddWidth;

	ImGui::SetCursorScreenPos(ImVec2(X, ButtonY));
	if (ListButton("Add project...", ImVec2(AddWidth, 48.0f)))
	{
		FILE_SYSTEM.OpenFolderDialog([this](const std::string& Path) {
			if (Path.empty())
				return;

			if (!ContainProject(Path))
			{
				ShowMessage("\"" + Path + "\" is not a project folder.");
				return;
			}

			FEProjectInfo* AddedProject = AddToProjectList(Path);
			if (AddedProject != nullptr)
				Selected = AddedProject;
		});
	}

	X += AddWidth + 12.0f;
	ImGui::SetCursorScreenPos(ImVec2(X, ButtonY));
	if (ListButton("Add projects from folder...", ImVec2(AddFolderWidth, 48.0f)))
	{
		FILE_SYSTEM.OpenFolderDialog([this](const std::string& Path) {
			if (Path.empty())
				return;

			if (AddProjectsFromFolderToList(Path) == 0)
				ShowMessage("No projects were found in \"" + Path + "\".");
		});
	}

	ImGui::SetCursorScreenPos(ImVec2(Origin.x + Width - Padding - PluginsWidth, ButtonY));
	if (ListButton("Plugins", ImVec2(PluginsWidth, 48.0f)))
		PLUGINS_WINDOW.SetVisible(true);
}

static bool BeginListPopup(const char* Name)
{
	const ImVec2 Center = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f);
	ImGui::SetNextWindowPos(Center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 13.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 12.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, FEEditorStyle::Card);
	ImGui::PushStyleColor(ImGuiCol_Border, FEEditorStyle::PopupBorder);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, FEEditorStyle::Button);
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, FEEditorStyle::Button);
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, FEEditorStyle::Dim);
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Text);
	ImGui::PushFont(GetListBoldFont(), 14.0f);
	const bool bOpened = ImGui::BeginPopupModal(Name, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PopFont();
	if (bOpened)
		ImGui::PushFont(GetListFont(), 14.0f);

	return bOpened;
}

// It should be called even if popup was not opened.
static void EndListPopup(const bool bOpened)
{
	if (bOpened)
	{
		ImGui::PopFont();
		ImGui::EndPopup();
	}

	ImGui::PopStyleColor(6);
	ImGui::PopStyleVar(5);
}

static void PopupLabel(const std::string& Text, const float Size = 13.0f, const float WrapWidth = 0.0f, const ImU32 Color = FEEditorStyle::TextMuted)
{
	ImGui::PushFont(GetListFont(), Size);
	ImGui::PushStyleColor(ImGuiCol_Text, Color);
	if (WrapWidth > 0.0f)
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + WrapWidth);

	ImGui::TextUnformatted(Text.c_str());
	if (WrapWidth > 0.0f)
		ImGui::PopTextWrapPos();

	ImGui::PopStyleColor();
	ImGui::PopFont();
}

static bool PopupInput(const char* Label, const char* ID, char* Buffer, const size_t BufferSize, const float Width)
{
	PopupLabel(Label);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6.0f);
	PushInputStyle(FEEditorStyle::Background);
	ImGui::SetNextItemWidth(Width);
	const bool bResult = ImGui::InputText(ID, Buffer, BufferSize);
	PopInputStyle();
	return bResult;
}

// Returns true if confirm button was pressed, cancel button closes the popup.
static bool PopupButtons(const float Width, const char* ConfirmLabel, const float ConfirmWidth, const ListButtonKind ConfirmKind)
{
	ImGui::SetCursorPos(ImVec2(16.0f + Width - 120.0f - 12.0f - ConfirmWidth, ImGui::GetCursorPosY() + 8.0f));
	if (ListButton("Cancel", ImVec2(120.0f, 44.0f)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine(0.0f, 12.0f);
	return ListButton(ConfirmLabel, ImVec2(ConfirmWidth, 44.0f), ConfirmKind);
}

void FEProjectManager::DisplayPopups()
{
	if (!RequestedPopup.empty())
	{
		ImGui::OpenPopup(RequestedPopup.c_str());
		RequestedPopup.clear();
	}

	bool bOpened = BeginListPopup("New Project");
	if (bOpened)
	{
		const float Width = 368.0f;
		PopupInput("Name", "##Name", PopupName, IM_ARRAYSIZE(PopupName), Width);
		PopupInput("Location", "##Location", NewProjectLocation, IM_ARRAYSIZE(NewProjectLocation), Width - 44.0f - 8.0f);
		ImGui::SameLine(0.0f, 8.0f);
		if (ListButton("...", ImVec2(44.0f, 36.0f)))
		{
			FILE_SYSTEM.OpenFolderDialog([this](const std::string& Path) {
				if (!Path.empty())
					strncpy_s(NewProjectLocation, Path.c_str(), _TRUNCATE);
			});
		}

		const std::string Name = PopupName;
		const std::string Location = NewProjectLocation;
		const std::string Folder = FILE_SYSTEM.GetNormalizedPath(Location + "/" + Name);
		if (!PopupError.empty())
		{
			PopupLabel(PopupError, 12.0f, Width, FEEditorStyle::Danger);
		}
		else if (!Name.empty() && !Location.empty())
		{
			PopupLabel("The project is created in " + Folder, 12.0f, Width);
		}

		if (PopupButtons(Width, "Create", 120.0f, ListButtonKind::Accent))
		{
			PopupError = "";
			if (Name.empty() || Name.find_first_of("\\/:*?\"<>|") != std::string::npos)
			{
				PopupError = "Name is empty or has characters that a folder name can not have.";
			}
			else if (Location.empty())
			{
				PopupError = "Location is empty.";
			}
			else if (FILE_SYSTEM.DoesDirectoryExist(Folder))
			{
				PopupError = "Folder " + Folder + " already exists.";
			}
			else if (!FILE_SYSTEM.DoesDirectoryExist(Location) && !FILE_SYSTEM.MakeDirectory(Location))
			{
				PopupError = "Location " + Location + " could not be created.";
			}
			else if (!FILE_SYSTEM.MakeDirectory(Folder))
			{
				PopupError = "Folder " + Folder + " could not be created.";
			}

			if (PopupError.empty())
			{
				DeferredAction = [this, Name, Folder]() {
					if (!CreateNewProject(Name, Folder + "/"))
						return;

					FEProjectInfo* NewProject = AddToProjectList(Folder);
					if (NewProject != nullptr)
						Selected = NewProject;
				};
				ImGui::CloseCurrentPopup();
			}
		}
	}
	EndListPopup(bOpened);

	bOpened = BeginListPopup("Rename Project");
	if (bOpened)
	{
		const float Width = 328.0f;
		PopupInput("Name", "##Name", PopupName, IM_ARRAYSIZE(PopupName), Width);
		PopupLabel("The folder name is not changed.", 12.0f, Width);
		if (PopupButtons(Width, "Rename", 120.0f, ListButtonKind::Accent) || !IsInProjectList(PopupProject))
		{
			if (IsInProjectList(PopupProject) && strlen(PopupName) != 0 && !RenameProject(PopupProject, PopupName))
				ShowMessage("Project.txt of \"" + PopupProject->GetName() + "\" could not be changed.");

			ImGui::CloseCurrentPopup();
		}
	}
	EndListPopup(bOpened);

	bOpened = BeginListPopup("Delete Project");
	if (bOpened)
	{
		const float Width = 408.0f;
		const std::string Name = IsInProjectList(PopupProject) ? PopupProject->GetName() : "";
		const std::string Folder = IsInProjectList(PopupProject) ? PopupProject->GetFolder() : "";

		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + Width);
		ImGui::TextUnformatted(("Move project " + Name + " to the Recycle Bin?").c_str());
		ImGui::PopTextWrapPos();

		const ImVec2 Min = ImGui::GetCursorScreenPos();
		const float TextHeight = GetListFont()->CalcTextSizeA(13.0f, FLT_MAX, Width - 20.0f, Folder.c_str()).y;
		const ImVec2 Max = ImVec2(Min.x + Width, Min.y + TextHeight + 20.0f);
		ImGui::Dummy(ImVec2(Width, TextHeight + 20.0f));
		ImGui::GetWindowDrawList()->AddRectFilled(Min, Max, FEEditorStyle::Background, 4.0f);
		ImGui::GetWindowDrawList()->AddRect(Min, Max, FEEditorStyle::CardBorder, 4.0f);
		DrawListText(GetListFont(), 13.0f, ImVec2(Min.x + 10.0f, Min.y + 10.0f), FEEditorStyle::Text, Folder, Width - 20.0f);

		PopupLabel("The whole folder is moved, also files that the editor did not create.", 12.0f, Width);
		if (PopupButtons(Width, "Delete project", 140.0f, ListButtonKind::DangerFilled) || !IsInProjectList(PopupProject))
		{
			FEProjectInfo* Project = PopupProject;
			DeferredAction = [this, Project, Folder]() {
				if (IsInProjectList(Project) && !DeleteProject(Project))
					ShowMessage("\"" + Folder + "\" could not be moved to the Recycle Bin.");
			};
			ImGui::CloseCurrentPopup();
		}
	}
	EndListPopup(bOpened);

	bOpened = BeginListPopup("Project List");
	if (bOpened)
	{
		const float Width = 408.0f;
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + Width);
		ImGui::TextUnformatted(Message.c_str());
		ImGui::PopTextWrapPos();
		ImGui::Dummy(ImVec2(Width, 0.0f));

		ImGui::SetCursorPos(ImVec2(16.0f + Width - 120.0f, ImGui::GetCursorPosY() + 8.0f));
		if (ListButton("OK", ImVec2(120.0f, 44.0f), ListButtonKind::Accent))
			ImGui::CloseCurrentPopup();
	}
	EndListPopup(bOpened);
}

void FEProjectManager::DisplayOpeningMessage(const ImVec2 Origin, const float Width, const float Height)
{
	const std::string Text = "Opening " + ProjectToOpen->GetName() + "...";
	if (bOpeningMessageShown)
	{
		OpenRequestedProject();
		ProjectToOpen = nullptr;
	}
	bOpeningMessageShown = !bOpeningMessageShown;

	const float TextSize = 18.0f;
	DrawListText(GetListBoldFont(), TextSize, ImVec2(Origin.x + (Width - GetTextWidth(GetListBoldFont(), TextSize, Text)) / 2.0f, Origin.y + (Height - TextSize) / 2.0f), FEEditorStyle::Text, Text);
}

void FEProjectManager::DisplayProjectSelection()
{
	const std::vector<FEProjectInfo*> Projects = GetVisibleProjects();
	if (!IsInProjectList(Selected))
		Selected = Projects.empty() ? nullptr : Projects[0];

	const float Width = static_cast<float>(APPLICATION.GetMainWindow()->GetWidth());
	const float Height = static_cast<float>(APPLICATION.GetMainWindow()->GetHeight());
	const bool bDetailsPanel = bListView;
	const float ContentWidth = bDetailsPanel ? Width - DetailsPanelWidth : Width;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, FEEditorStyle::Background);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(Width, Height));
	ImGuiWindowFlags Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking;
	// Without input the list stays behind the window that covers it.
	if (IsCoveredByWindow())
		Flags |= ImGuiWindowFlags_NoInputs;

	ImGui::Begin("Project Browser", nullptr, Flags);
	ImGui::PushFont(GetListFont(), 14.0f);

	const ImVec2 Origin = ImGui::GetWindowPos();
	if (ProjectToOpen != nullptr)
	{
		DisplayOpeningMessage(Origin, Width, Height);
		ImGui::PopFont();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
		return;
	}

	DisplayTopBar(Origin, ContentWidth);
	if (bDetailsPanel)
	{
		DisplayListView(Projects, Origin, ContentWidth, Height);
		DisplayDetailsPanel(ImVec2(Origin.x + ContentWidth, Origin.y), Height);
	}
	else
	{
		DisplayGridView(Projects, Origin, ContentWidth, Height);
	}

	if (Projects.empty())
	{
		const std::string Text = List.empty() ? "No projects yet. Create a new project or add an existing one." : "No project matches the filter.";
		DrawListText(GetListFont(), 14.0f, ImVec2(Origin.x + (bDetailsPanel ? 24.0f : 32.0f), Origin.y + TopBarHeight + 24.0f), FEEditorStyle::TextMuted, Text);
	}

	DisplayBottomBar(Origin, ContentWidth, Height);
	DisplayPopups();

	// Area of the projects is dimmed by its own child window.
	if (IsCoveredByWindow())
	{
		ImDrawList* DrawList = ImGui::GetWindowDrawList();
		DrawList->AddRectFilled(Origin, ImVec2(Origin.x + ContentWidth, Origin.y + TopBarHeight), FEEditorStyle::Dim);
		DrawList->AddRectFilled(ImVec2(Origin.x, Origin.y + Height - BottomBarHeight), ImVec2(Origin.x + ContentWidth, Origin.y + Height), FEEditorStyle::Dim);
		if (bDetailsPanel)
			DrawList->AddRectFilled(ImVec2(Origin.x + ContentWidth, Origin.y), ImVec2(Origin.x + Width, Origin.y + Height), FEEditorStyle::Dim);
	}

	ImGui::PopFont();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);

	if (DeferredAction)
	{
		const std::function<void()> Action = DeferredAction;
		DeferredAction = nullptr;
		Action();
	}
}