#include "PluginsWindow.h"
#include "../SubSystems/ProjectManagement/FEProjectManager.h"
#include <shellapi.h>

static const ImVec4 LabelColor = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
static const ImVec4 LoadedColor = ImVec4(0.45f, 0.80f, 0.45f, 1.0f);
static const ImVec4 WarningColor = ImVec4(0.95f, 0.75f, 0.25f, 1.0f);
static const ImVec4 ErrorColor = ImVec4(0.95f, 0.5f, 0.5f, 1.0f);
static const ImVec4 EngineModuleColor = ImVec4(0.55f, 0.75f, 0.95f, 1.0f);
static const ImVec4 EditorModuleColor = ImVec4(0.80f, 0.60f, 0.95f, 1.0f);

FEEditorPluginsWindow::FEEditorPluginsWindow()
{
}

FEEditorPluginsWindow::~FEEditorPluginsWindow()
{
}

void FEEditorPluginsWindow::Render()
{
	if (!bVisible)
		return;

	// Padding is popped right after Begin, so tooltips keep default padding.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	const bool bWindowOpen = ImGui::Begin("Plugins", nullptr, ImGuiWindowFlags_None);
	ImGui::PopStyleVar();

	if (bWindowOpen)
	{
		const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
		size_t ModuleCount = 0;
		size_t LoadedModuleCount = 0;
		for (size_t i = 0; i < Plugins.size(); i++)
		{
			for (size_t j = 0; j < Plugins[i].Manifest.Modules.size(); j++)
			{
				ModuleCount++;
				if (EDITOR_PLUGIN_MANAGER.IsModuleLoaded(Plugins[i].Manifest.Modules[j]))
					LoadedModuleCount++;
			}
		}

		ImGui::TextColored(LabelColor, "%zu %s, %zu/%zu modules loaded", Plugins.size(), Plugins.size() == 1 ? "plugin" : "plugins", LoadedModuleCount, ModuleCount);

		const std::string PluginDirectory = GetDisplayPath(PLUGIN_MANAGER.GetPluginDirectory());
		const char* NewPluginButtonText = "New plugin...";
		const char* OpenButtonText = "Open plugins folder";
		const ImGuiStyle& Style = ImGui::GetStyle();
		const float ButtonsWidth = ImGui::CalcTextSize(NewPluginButtonText).x + ImGui::CalcTextSize(OpenButtonText).x + Style.FramePadding.x * 4.0f + Style.ItemSpacing.x;
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImMax(0.0f, ImGui::GetContentRegionAvail().x - ButtonsWidth));
		if (ImGui::SmallButton(NewPluginButtonText))
		{
			NewPluginName.clear();
			NewPluginDescription.clear();
			NewPluginCreationError.clear();
			CreatedPluginFolder.clear();
			bShouldOpenNewPluginPopup = true;
		}

		ImGui::SameLine();
		if (ImGui::SmallButton(OpenButtonText))
			OpenFolderInExplorer(PluginDirectory);
		ImGui::SetItemTooltip("%s", PluginDirectory.c_str());

		ImGui::Separator();
		ImGui::Spacing();

		if (PROJECT_MANAGER.GetCurrent() != nullptr)
			RenderMissingPlugins(PROJECT_MANAGER.GetCurrent());

		if (Plugins.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, LabelColor);
			ImGui::TextWrapped("No plugins were found, plugin folders should be placed in %s", PluginDirectory.c_str());
			ImGui::PopStyleColor();
		}

		for (size_t i = 0; i < Plugins.size(); i++)
			RenderPluginDetails(Plugins[i]);
	}
	ImGui::End();

	RenderNewPluginPopup();
}

void FEEditorPluginsWindow::RenderMissingPlugins(FEProject* Project)
{
	const std::vector<FEProjectPluginInfo> MissingPlugins = Project->GetMissingPlugins();
	if (MissingPlugins.empty())
		return;

	ImGui::TextColored(ErrorColor, "Missing plugins");
	ImGui::SameLine();
	ImGui::TextColored(LabelColor, "Project uses them, but they are not installed.");

	if (ImGui::BeginTable("MissingPlugins", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Plugin");
		ImGui::TableSetupColumn("Version");
		ImGui::TableSetupColumn("ID");
		ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableHeadersRow();

		for (size_t i = 0; i < MissingPlugins.size(); i++)
		{
			ImGui::PushID(static_cast<int>(i));
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			RenderStatusDot(ErrorColor);
			ImGui::TextUnformatted(MissingPlugins[i].Name.c_str());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(MissingPlugins[i].Version.c_str());

			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "%s...", UNIQUE_ID.ToString(MissingPlugins[i].ID).substr(0, 8).c_str());
			AddCopyIDBehavior(MissingPlugins[i].ID);

			ImGui::TableNextColumn();
			if (ImGui::SmallButton("Remove from project"))
				Project->RemoveMissingPlugin(MissingPlugins[i].ID);
			ImGui::SetItemTooltip("Project will no longer report this plugin as missing.");

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
}

void FEEditorPluginsWindow::RenderPluginDetails(const DiscoveredPlugin& Plugin)
{
	const PluginManifest& Manifest = Plugin.Manifest;
	size_t LoadedModuleCount = 0;
	for (size_t i = 0; i < Manifest.Modules.size(); i++)
	{
		if (EDITOR_PLUGIN_MANAGER.IsModuleLoaded(Manifest.Modules[i]))
			LoadedModuleCount++;
	}

	const bool bAllModulesLoaded = LoadedModuleCount == Manifest.Modules.size();
	ImVec4 StatusColor = WarningColor;
	if (bAllModulesLoaded)
		StatusColor = LoadedColor;
	else if (LoadedModuleCount == 0)
		StatusColor = ErrorColor;

	ImGui::PushID(&Plugin);

	// Plugins with problems are open, so problems are visible right away.
	if (!bAllModulesLoaded)
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);

	const ImVec2 HeaderPosition = ImGui::GetCursorScreenPos();
	const float HeaderRight = HeaderPosition.x + ImGui::GetContentRegionAvail().x;
	const bool bOpen = ImGui::CollapsingHeader("##Header", ImGuiTreeNodeFlags_AllowOverlap);

	// Header has no label, status dot, name and version are drawn over it, so they could have different colors.
	const ImGuiStyle& Style = ImGui::GetStyle();
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	const float FrameHeight = ImGui::GetFrameHeight();
	const float DotRadius = ImGui::GetFontSize() * 0.25f;
	const float TextY = HeaderPosition.y + Style.FramePadding.y;
	// Same position as ImGui uses for header label, after collapsing arrow.
	float TextX = HeaderPosition.x + ImGui::GetFontSize() + Style.FramePadding.x * 3.0f;

	DrawList->AddCircleFilled(ImVec2(TextX + DotRadius, HeaderPosition.y + FrameHeight * 0.5f), DotRadius, ImGui::GetColorU32(StatusColor));
	TextX += DotRadius * 2.0f + Style.ItemSpacing.x;
	DrawList->AddText(ImVec2(TextX, TextY), ImGui::GetColorU32(ImGuiCol_Text), Manifest.Name.c_str());
	TextX += ImGui::CalcTextSize(Manifest.Name.c_str()).x + Style.ItemSpacing.x;
	DrawList->AddText(ImVec2(TextX, TextY), ImGui::GetColorU32(LabelColor), Manifest.Version.c_str());

	const char* OpenFolderText = "Open folder";
	const float OpenFolderButtonX = HeaderRight - ImGui::CalcTextSize(OpenFolderText).x - Style.FramePadding.x * 2.0f;
	const std::string ModuleCountText = std::to_string(LoadedModuleCount) + "/" + std::to_string(Manifest.Modules.size()) + " loaded";
	const float ModuleCountX = OpenFolderButtonX - Style.ItemSpacing.x * 2.0f - ImGui::CalcTextSize(ModuleCountText.c_str()).x;
	DrawList->AddText(ImVec2(ModuleCountX, TextY), ImGui::GetColorU32(StatusColor), ModuleCountText.c_str());

	const std::string FolderPath = GetDisplayPath(Plugin.FolderPath);
	// SmallButton aligns itself to text baseline of the header line, so header top is enough to center it.
	ImGui::SameLine();
	ImGui::SetCursorScreenPos(ImVec2(OpenFolderButtonX, HeaderPosition.y));
	if (ImGui::SmallButton(OpenFolderText))
		OpenFolderInExplorer(FolderPath);
	ImGui::SetItemTooltip("%s", FolderPath.c_str());

	if (!bOpen)
	{
		ImGui::PopID();
		return;
	}

	ImGui::Indent();
	ImGui::Spacing();

	if (!Manifest.Description.empty())
	{
		ImGui::TextWrapped("%s", Manifest.Description.c_str());
		ImGui::Spacing();
	}

	if (ImGui::BeginTable("Details", 2, ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		std::string SupportedOS;
		for (size_t i = 0; i < Manifest.SupportedOS.size(); i++)
			SupportedOS += (i == 0 ? "" : ", ") + OSToString(Manifest.SupportedOS[i]);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextColored(LabelColor, "Supported OS");
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(SupportedOS.c_str());

		std::string Dependencies;
		for (size_t i = 0; i < Manifest.Dependencies.size(); i++)
			Dependencies += (i == 0 ? "" : ", ") + FindPluginName(Manifest.Dependencies[i]);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextColored(LabelColor, "Depends on");
		ImGui::TableNextColumn();
		if (Dependencies.empty())
			ImGui::TextColored(LabelColor, "-");
		else
			ImGui::TextUnformatted(Dependencies.c_str());

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextColored(LabelColor, "ID");
		ImGui::TableNextColumn();
		ImGui::TextColored(LabelColor, "%s...", UNIQUE_ID.ToString(Manifest.ID).substr(0, 8).c_str());
		AddCopyIDBehavior(Manifest.ID);

		ImGui::EndTable();
	}

	ImGui::Spacing();
	RenderModulesTable(Plugin);
	ImGui::Spacing();

	ImGui::Unindent();
	ImGui::PopID();
}

void FEEditorPluginsWindow::RenderModulesTable(const DiscoveredPlugin& Plugin)
{
	const PluginManifest& Manifest = Plugin.Manifest;
	if (!ImGui::BeginTable("Modules", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
		return;

	ImGui::TableSetupColumn("Module");
	ImGui::TableSetupColumn("Type");
	ImGui::TableSetupColumn("Status");
	ImGui::TableSetupColumn("Depends on");
	ImGui::TableHeadersRow();

	for (size_t i = 0; i < Manifest.Modules.size(); i++)
	{
		const PluginModule& Module = Manifest.Modules[i];
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Module.Name.c_str());
		AddCopyIDBehavior(Module.ID);

		ImGui::TableNextColumn();
		if (Module.Type == "Engine")
			ImGui::TextColored(EngineModuleColor, "%s", Module.Type.c_str());
		else if (Module.Type == "Editor")
			ImGui::TextColored(EditorModuleColor, "%s", Module.Type.c_str());
		else
			ImGui::TextUnformatted(Module.Type.c_str());

		ImGui::TableNextColumn();
		switch (GetModuleStatus(Module))
		{
			case ModuleStatus::Loaded:
				RenderStatusDot(LoadedColor);
				ImGui::TextColored(LoadedColor, "Loaded");
				break;

			case ModuleStatus::Skipped:
				RenderStatusDot(WarningColor);
				ImGui::TextColored(WarningColor, "Skipped");
				ImGui::SetItemTooltip("Module that it depends on is not loaded.");
				break;

			case ModuleStatus::Failed:
				RenderStatusDot(ErrorColor);
				ImGui::TextColored(ErrorColor, "Failed");
				ImGui::SetItemTooltip("Reason is in the log.");
				break;
		}

		ImGui::TableNextColumn();
		std::string ModuleDependencies;
		for (size_t j = 0; j < Module.Dependencies.size(); j++)
			ModuleDependencies += (j == 0 ? "" : ", ") + FindModuleName(Module.Dependencies[j], Manifest.ID);

		if (ModuleDependencies.empty())
			ImGui::TextColored(LabelColor, "-");
		else
			ImGui::TextUnformatted(ModuleDependencies.c_str());
	}

	ImGui::EndTable();
}

void FEEditorPluginsWindow::RenderStatusDot(const ImVec4& Color)
{
	const float Radius = ImGui::GetFontSize() * 0.25f;
	const ImVec2 Position = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(Position.x + Radius, Position.y + ImGui::GetTextLineHeight() * 0.5f), Radius, ImGui::GetColorU32(Color));
	ImGui::Dummy(ImVec2(Radius * 2.0f, ImGui::GetTextLineHeight()));
	ImGui::SameLine();
}

void FEEditorPluginsWindow::AddCopyIDBehavior(const FEUUID& ID)
{
	if (!ImGui::IsItemHovered())
		return;

	const std::string IDString = UNIQUE_ID.ToString(ID);
	ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	ImGui::SetTooltip("ID: %s\nClick to copy", IDString.c_str());
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		ImGui::SetClipboardText(IDString.c_str());
}

void FEEditorPluginsWindow::OpenFolderInExplorer(const std::string& Path)
{
	HINSTANCE Result = ShellExecuteA(nullptr, "open", Path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	if (reinterpret_cast<INT_PTR>(Result) <= 32)
		LOG.Add("FEEditorPluginsWindow::OpenFolderInExplorer: Failed to open \"" + Path + "\" in file explorer", "FE_EDITOR", FE_LOG_WARNING);
}

FEEditorPluginsWindow::ModuleStatus FEEditorPluginsWindow::GetModuleStatus(const PluginModule& Module) const
{
	if (EDITOR_PLUGIN_MANAGER.IsModuleLoaded(Module))
		return ModuleStatus::Loaded;

	// Engine modules are loaded regardless of their dependencies, only editor plugin manager skips modules with not loaded dependencies.
	if (Module.Type == "Editor")
	{
		for (size_t i = 0; i < Module.Dependencies.size(); i++)
		{
			const PluginModule* Dependency = FindModule(Module.Dependencies[i]);
			if (Dependency == nullptr || !EDITOR_PLUGIN_MANAGER.IsModuleLoaded(*Dependency))
				return ModuleStatus::Skipped;
		}
	}

	return ModuleStatus::Failed;
}

std::string FEEditorPluginsWindow::GetDisplayPath(const std::string& Path) const
{
	std::string Result = FILE_SYSTEM.GetAbsolutePath(Path);
	std::replace(Result.begin(), Result.end(), '/', '\\');
	return Result;
}

std::string FEEditorPluginsWindow::FindPluginName(const FEUUID& PluginID) const
{
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		if (Plugins[i].Manifest.ID == PluginID)
			return Plugins[i].Manifest.Name;
	}

	return UNIQUE_ID.ToString(PluginID) + " (not found)";
}

const PluginModule* FEEditorPluginsWindow::FindModule(const FEUUID& ModuleID) const
{
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const PluginManifest& Manifest = Plugins[i].Manifest;
		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			if (Manifest.Modules[j].ID == ModuleID)
				return &Manifest.Modules[j];
		}
	}

	return nullptr;
}

std::string FEEditorPluginsWindow::FindModuleName(const FEUUID& ModuleID, const FEUUID& CurrentPluginID) const
{
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const PluginManifest& Manifest = Plugins[i].Manifest;
		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			if (Manifest.Modules[j].ID != ModuleID)
				continue;

			if (Manifest.ID == CurrentPluginID)
				return Manifest.Modules[j].Name;

			return Manifest.Name + "/" + Manifest.Modules[j].Name;
		}
	}

	return UNIQUE_ID.ToString(ModuleID) + " (not found)";
}

void FEEditorPluginsWindow::RenderNewPluginPopup()
{
	// Popup is opened and rendered outside of plugins window, so both use the same ID stack.
	if (bShouldOpenNewPluginPopup)
	{
		ImGui::OpenPopup("New plugin");
		bShouldOpenNewPluginPopup = false;
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetFontSize() * 40.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	const bool bPopupOpen = ImGui::BeginPopupModal("New plugin", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PopStyleVar();
	if (!bPopupOpen)
		return;

	if (CreatedPluginFolder.empty())
		RenderNewPluginForm();
	else
		RenderNewPluginCreatedMessage();

	ImGui::EndPopup();
}

void FEEditorPluginsWindow::RenderNewPluginForm()
{
	const ImGuiStyle& Style = ImGui::GetStyle();
	if (ImGui::BeginTable("NewPluginForm", 2, ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(LabelColor, "Name");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();
		ImGui::InputText("##Name", &NewPluginName);

		const char* BrowseButtonText = "Browse...";
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(LabelColor, "Location");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(BrowseButtonText).x - Style.FramePadding.x * 2.0f - Style.ItemSpacing.x);
		ImGui::InputText("##Location", &NewPluginLocation);
		ImGui::SameLine();
		if (ImGui::Button(BrowseButtonText))
		{
			FILE_SYSTEM.OpenFolderDialog([this](const std::string& Path) {
				if (!Path.empty())
					NewPluginLocation = Path;
			});
		}

		const std::string NewPluginFolder = GetNewPluginFolder();
		if (!NewPluginFolder.empty())
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "Project folder: %s", NewPluginFolder.c_str());
		}

		// Descriptions of types are aligned in one column.
		const float TypeDescriptionOffset = ImGui::GetFrameHeight() + Style.ItemInnerSpacing.x + ImGui::CalcTextSize("Engine + Editor").x + Style.ItemSpacing.x * 3.0f;
		auto RenderTypeRow = [&](const char* Label, const FEPluginProjectType Type, const char* Description) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (Type == FEPluginProjectType::Engine)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextColored(LabelColor, "Type");
			}

			ImGui::TableNextColumn();
			const float CellX = ImGui::GetCursorPosX();
			if (ImGui::RadioButton(Label, NewPluginType == Type))
				NewPluginType = Type;

			ImGui::SameLine();
			ImGui::SetCursorPosX(CellX + TypeDescriptionOffset);
			ImGui::TextColored(LabelColor, "%s", Description);
		};

		RenderTypeRow("Engine", FEPluginProjectType::Engine, "Runs in the editor and in games.");
		RenderTypeRow("Engine + Editor", FEPluginProjectType::EngineAndEditor, "Engine module and editor UI that uses it.");
		RenderTypeRow("Editor", FEPluginProjectType::Editor, "Editor UI only, not included in games.");

		// Option rows are shown for every type, so form does not change its layout, not applicable options are disabled.
		auto RenderOptionRow = [&](const char* Label, const char* CheckboxID, bool* bValue, const bool bEditable, const bool bShownValue, const char* Description) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::TextColored(LabelColor, "%s", Label);
			ImGui::TableNextColumn();
			if (bEditable)
			{
				ImGui::Checkbox(CheckboxID, bValue);
			}
			else
			{
				bool bDisabledValue = bShownValue;
				ImGui::BeginDisabled();
				ImGui::Checkbox(CheckboxID, &bDisabledValue);
				ImGui::EndDisabled();
			}

			ImGui::SameLine();
			ImGui::TextColored(LabelColor, "%s", Description);
		};

		if (NewPluginType == FEPluginProjectType::Engine)
			RenderOptionRow("Public interface", "##PublicInterface", &bNewPluginPublicInterface, true, false, "Other plugins and native scripts could call engine module API.");
		else if (NewPluginType == FEPluginProjectType::EngineAndEditor)
			RenderOptionRow("Public interface", "##PublicInterface", &bNewPluginPublicInterface, false, true, "Always added, editor module uses it.");
		else
			RenderOptionRow("Public interface", "##PublicInterface", &bNewPluginPublicInterface, false, false, "Plugin has no engine module.");

		if (NewPluginType == FEPluginProjectType::Engine)
			RenderOptionRow("Editor interface", "##EditorPublicInterface", &bNewPluginEditorPublicInterface, false, false, "Plugin has no editor module.");
		else
			RenderOptionRow("Editor interface", "##EditorPublicInterface", &bNewPluginEditorPublicInterface, true, false, "Editor modules of other plugins could call editor module API.");

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(LabelColor, "Description");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputText("##Description", &NewPluginDescription);

		ImGui::EndTable();
	}

	ImGui::Spacing();
	const std::string ValidationError = ValidateNewPlugin();
	if (!ValidationError.empty())
	{
		ImGui::TextColored(WarningColor, "%s", ValidationError.c_str());
	}
	else
	{
		const std::string NewPluginFolder = GetNewPluginFolder();
		if (FILE_SYSTEM.DoesDirectoryExist(NewPluginFolder) && (!FILE_SYSTEM.GetFileList(NewPluginFolder).empty() || !FILE_SYSTEM.GetDirectoryList(NewPluginFolder).empty()))
			ImGui::TextColored(WarningColor, "Project folder is not empty, files with the same names will be overwritten.");
	}

	if (!NewPluginCreationError.empty())
		ImGui::TextColored(ErrorColor, "%s", NewPluginCreationError.c_str());

	ImGui::PushStyleColor(ImGuiCol_Text, LabelColor);
	ImGui::TextWrapped("Engine is added to the project as git submodule and built, it can take several minutes, editor does not respond until it is done.");
	ImGui::PopStyleColor();

	ImGui::Spacing();
	const ImVec2 ButtonSize = ImVec2(ImGui::GetFontSize() * 6.0f, 0.0f);
	ImGui::BeginDisabled(!ValidationError.empty());
	if (ImGui::Button("Create", ButtonSize))
	{
		FEEditorPluginProjectSettings Settings;
		Settings.Type = NewPluginType;
		Settings.bAddPublicInterface = bNewPluginPublicInterface;
		Settings.bAddEditorPublicInterface = bNewPluginEditorPublicInterface;
		Settings.Description = NewPluginDescription;

		std::string PluginName = NewPluginName;
		const std::string NewPluginFolder = GetNewPluginFolder();
		NewPluginCreationError.clear();
		if (EDITOR_PLUGIN_MANAGER.CreatePluginProject(PluginName, NewPluginFolder, Settings))
			CreatedPluginFolder = NewPluginFolder;
		else
			NewPluginCreationError = "Plugin project was not created, reason is in the log.";
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Cancel", ButtonSize))
		ImGui::CloseCurrentPopup();
}

void FEEditorPluginsWindow::RenderNewPluginCreatedMessage()
{
	const std::string PluginDirectory = GetDisplayPath(PLUGIN_MANAGER.GetPluginDirectory());
	ImGui::TextColored(LoadedColor, "Plugin project was created and opened in Visual Studio.");
	ImGui::TextUnformatted(GetDisplayPath(CreatedPluginFolder).c_str());

	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_Text, LabelColor);
	ImGui::TextWrapped("To use plugin in the editor, build it, copy content of its Build folder to %s\\%s and restart the editor.", PluginDirectory.c_str(), NewPluginName.c_str());
	ImGui::PopStyleColor();

	ImGui::Spacing();
	if (ImGui::Button("Open plugins folder"))
		OpenFolderInExplorer(PluginDirectory);

	ImGui::SameLine();
	if (ImGui::Button("Close", ImVec2(ImGui::GetFontSize() * 6.0f, 0.0f)))
		ImGui::CloseCurrentPopup();
}

std::string FEEditorPluginsWindow::GetNewPluginFolder() const
{
	if (NewPluginLocation.empty() || NewPluginName.empty())
		return "";

	std::string Folder = NewPluginLocation;
	if (Folder.back() != '/' && Folder.back() != '\\')
		Folder += '\\';

	return Folder + NewPluginName;
}

std::string FEEditorPluginsWindow::ValidateNewPlugin() const
{
	if (NewPluginName.empty())
		return "Enter plugin name.";

	// Name is used as C++ class name, CMake target and DLL name.
	for (size_t i = 0; i < NewPluginName.size(); i++)
	{
		const char Character = NewPluginName[i];
		const bool bLetter = (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || Character == '_';
		const bool bDigit = Character >= '0' && Character <= '9';
		if (!bLetter && !(bDigit && i > 0))
			return "Name can contain only English letters, digits and underscores, and can not start with a digit.";
	}

	auto ToLower = [](std::string Text) {
		for (char& Character : Text)
		{
			if (Character >= 'A' && Character <= 'Z')
				Character = static_cast<char>(Character - 'A' + 'a');
		}
		return Text;
	};

	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		if (ToLower(Plugins[i].Manifest.Name) == ToLower(NewPluginName))
			return "Plugin with this name is already installed.";
	}

	if (NewPluginLocation.empty())
		return "Choose location of plugin project.";

	if (!FILE_SYSTEM.DoesDirectoryExist(NewPluginLocation))
		return "Location does not exist.";

	return "";
}