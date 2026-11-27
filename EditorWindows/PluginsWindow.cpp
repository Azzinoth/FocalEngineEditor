#include "PluginsWindow.h"
#include "../SubSystems/ProjectManagement/FEProjectManager.h"
#include "../FEEditor.h"
#include <shellapi.h>

static const ImVec4 LabelColor = FEEditorStyle::ToVec4(FEEditorStyle::TextMuted);
static const ImVec4 LoadedColor = FEEditorStyle::ToVec4(FEEditorStyle::Success);
static const ImVec4 WarningColor = FEEditorStyle::ToVec4(FEEditorStyle::Warning);
static const ImVec4 ErrorColor = FEEditorStyle::ToVec4(FEEditorStyle::Danger);
static const ImVec4 EngineModuleColor = ImVec4(0.55f, 0.75f, 0.95f, 1.0f);
static const ImVec4 EditorModuleColor = ImVec4(0.80f, 0.60f, 0.95f, 1.0f);

static const std::vector<FEFileSystem::FileDialogFilter> PluginPackageFilter =
{
	{ "Plugin package (*.feplugin)", { "feplugin" } }
};

static const std::vector<FEFileSystem::FileDialogFilter> InstallPluginFilter =
{
	{ "Plugin package or archive (*.feplugin, *.zip)", { "feplugin", "zip" } }
};

static std::string GetGraphicsModesText(const PluginModule& Module)
{
	std::string Result;
	for (size_t i = 0; i < Module.SupportedGraphicsModes.size(); i++)
		Result += (i == 0 ? "" : ", ") + GraphicsModeToString(Module.SupportedGraphicsModes[i]);

	return Result;
}

FEEditorPluginsWindow::FEEditorPluginsWindow()
{
}

FEEditorPluginsWindow::~FEEditorPluginsWindow()
{
}

void FEEditorPluginsWindow::Render()
{
	EnablePluginsInstalledFromBrowser();
	if (!bVisible)
		return;

	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	// Without project window is shown over the project list and looks like it.
	const bool bProjectListStyle = CurrentProject == nullptr;
	if (bProjectListStyle)
		PROJECT_MANAGER.PushProjectListStyle();

	// Padding is popped right after Begin, so tooltips keep default padding.
	bool bWindowOpen = false;
	if (CurrentProject != nullptr)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		bWindowOpen = ImGui::Begin("Plugins", &bVisible, ImGuiWindowFlags_None);
		ImGui::PopStyleVar();
	}
	else
	{
		// It has its own ID, so docking of the usual window is not changed.
		const float MainWindowWidth = static_cast<float>(APPLICATION.GetMainWindow()->GetWidth());
		const float MainWindowHeight = static_cast<float>(APPLICATION.GetMainWindow()->GetHeight());
		ImGui::SetNextWindowPos(ImVec2(MainWindowWidth / 2.0f, MainWindowHeight / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(ImMin(MainWindowWidth - 160.0f, 1600.0f), ImMin(MainWindowHeight - 160.0f, 1100.0f)));

		// Frame padding sets height of the title bar.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 13.0f));
		bWindowOpen = ImGui::Begin("Plugins###PluginsWithoutProject", &bVisible, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
		ImGui::PopStyleVar(2);
	}

	if (bWindowOpen)
	{
		const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
		size_t EnabledPluginCount = 0;
		size_t ModuleCount = 0;
		size_t LoadedModuleCount = 0;
		for (size_t i = 0; i < Plugins.size(); i++)
		{
			if (CurrentProject != nullptr && CurrentProject->UsesPlugin(Plugins[i].Manifest.ID))
				EnabledPluginCount++;

			// Modules of plugins that were not requested are not expected to be loaded.
			if (!EDITOR_PLUGIN_MANAGER.IsPluginRequested(Plugins[i]))
				continue;

			for (size_t j = 0; j < Plugins[i].Manifest.Modules.size(); j++)
			{
				ModuleCount++;
				if (EDITOR_PLUGIN_MANAGER.IsModuleLoaded(Plugins[i].Manifest.Modules[j]))
					LoadedModuleCount++;
			}
		}

		if (CurrentProject != nullptr)
		{
			ImGui::TextColored(LabelColor, "%zu %s, %zu enabled in project, %zu/%zu modules loaded", Plugins.size(), Plugins.size() == 1 ? "plugin" : "plugins", EnabledPluginCount, LoadedModuleCount, ModuleCount);
		}
		else
		{
			ImGui::TextColored(LabelColor, "%zu %s, %zu/%zu modules loaded", Plugins.size(), Plugins.size() == 1 ? "plugin" : "plugins", LoadedModuleCount, ModuleCount);
		}

		const std::string PluginDirectory = GetDisplayPath(PLUGIN_MANAGER.GetPluginDirectory());
		const char* NewPluginButtonText = "New plugin...";
		const char* InstallButtonText = "Install plugin...";
		const char* OpenButtonText = "Open plugins folder";
		const ImGuiStyle& Style = ImGui::GetStyle();
		const float ButtonsWidth = ImGui::CalcTextSize(NewPluginButtonText).x + ImGui::CalcTextSize(InstallButtonText).x + ImGui::CalcTextSize(OpenButtonText).x + Style.FramePadding.x * 6.0f + Style.ItemSpacing.x * 2.0f;
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
		if (ImGui::SmallButton(InstallButtonText))
		{
			FILE_SYSTEM.OpenFileDialog(InstallPluginFilter, [this](const std::string& FilePath) {
				if (FilePath.empty())
					return;

				InstallPackagePath = FilePath;
				InstallPluginError.clear();
				bPluginInstalled = false;
				bInstallPackageRead = PLUGIN_MANAGER.ReadPluginPackageManifest(FilePath, InstallPackageManifest, InstallPluginError);
				bShouldOpenInstallPluginPopup = true;
			});
		}

		ImGui::SameLine();
		if (ImGui::SmallButton(OpenButtonText))
			OpenFolderInExplorer(PluginDirectory);
		ImGui::SetItemTooltip("%s", PluginDirectory.c_str());

		ImGui::Separator();
		ImGui::Spacing();

		std::vector<std::string> NamesToLoad = InstalledPluginNames;
		for (const std::string& Name : PLUGIN_BROWSER.GetInstalledPluginNames())
		{
			if (std::find(NamesToLoad.begin(), NamesToLoad.end(), Name) == NamesToLoad.end())
				NamesToLoad.push_back(Name);
		}

		if (!NamesToLoad.empty())
		{
			std::string Names;
			for (size_t i = 0; i < NamesToLoad.size(); i++)
				Names += (i == 0 ? "" : ", ") + NamesToLoad[i];

			ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
			// Without project only plugins of the editor are loaded, other plugins are loaded with projects that enable them.
			if (CurrentProject != nullptr)
			{
				ImGui::TextWrapped("Restart the editor to load installed plugins: %s", Names.c_str());
			}
			else
			{
				ImGui::TextWrapped("Restart the editor to add installed plugins to the list: %s", Names.c_str());
			}
			ImGui::PopStyleColor();
			ImGui::Spacing();
		}

		std::string NamesToRemove;
		for (size_t i = 0; i < Plugins.size(); i++)
		{
			if (PLUGIN_MANAGER.IsUninstallPending(Plugins[i].Manifest.ID))
				NamesToRemove += (NamesToRemove.empty() ? "" : ", ") + Plugins[i].Manifest.Name;
		}

		if (!NamesToRemove.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
			ImGui::TextWrapped("Restart the editor to remove uninstalled plugins: %s", NamesToRemove.c_str());
			ImGui::PopStyleColor();
			ImGui::Spacing();
		}

		std::string NamesToUpdate;
		for (size_t i = 0; i < Plugins.size(); i++)
		{
			const std::string PendingUpdateVersion = PLUGIN_MANAGER.GetPendingUpdateVersion(Plugins[i].Manifest.ID);
			if (!PendingUpdateVersion.empty())
				NamesToUpdate += (NamesToUpdate.empty() ? "" : ", ") + Plugins[i].Manifest.Name + " " + PendingUpdateVersion;
		}

		if (!NamesToUpdate.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
			ImGui::TextWrapped("Restart the editor to update plugins: %s", NamesToUpdate.c_str());
			ImGui::PopStyleColor();
			ImGui::Spacing();
		}

		const std::string PluginChanges = GetPendingPluginChangesText();
		if (!PluginChanges.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
			ImGui::TextWrapped("Restart the editor to apply plugin changes: %s", PluginChanges.c_str());
			ImGui::PopStyleColor();
			ImGui::Spacing();
		}

		if (!NamesToLoad.empty() || !NamesToRemove.empty() || !NamesToUpdate.empty() || !PluginChanges.empty())
		{
			if (CurrentProject != nullptr)
			{
				if (ImGui::SmallButton("Save project and restart"))
					EDITOR.RequestSaveProjectAndRestart();
			}
			else if (ImGui::SmallButton("Restart"))
			{
				EDITOR.RestartWithProject("");
			}
			ImGui::Spacing();
		}

		if (ImGui::BeginTabBar("PluginTabs"))
		{
			// Counts are not part of tab ID, so tab stays selected when they change.
			const size_t UpdateCount = FindAvailableUpdates().size();
			std::string InstalledTabLabel = "Installed (" + std::to_string(Plugins.size());
			if (UpdateCount > 0)
				InstalledTabLabel += ", " + std::to_string(UpdateCount) + (UpdateCount == 1 ? " update" : " updates");
			InstalledTabLabel += ")###Installed";

			if (ImGui::BeginTabItem(InstalledTabLabel.c_str()))
			{
				ImGui::Spacing();
				RenderUpdateAllLine();
				if (CurrentProject != nullptr)
					RenderMissingPlugins(CurrentProject);

				if (Plugins.empty())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, LabelColor);
					ImGui::TextWrapped("No plugins were found, plugin folders should be placed in %s", PluginDirectory.c_str());
					ImGui::PopStyleColor();
				}

				for (size_t i = 0; i < Plugins.size(); i++)
					RenderPluginDetails(Plugins[i]);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Browse"))
			{
				ImGui::Spacing();
				RenderBrowseTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	RenderNewPluginPopup();
	RenderInstallPluginPopup();
	RenderInstallDependenciesPopup();
	RenderEngineCommitPopup();
	RenderUninstallPopup();

	if (bProjectListStyle)
		PROJECT_MANAGER.PopProjectListStyle();
}

void FEEditorPluginsWindow::RenderMissingPlugins(FEProject* Project)
{
	// Plugin installed during this session is not known to plugin manager until restart.
	std::vector<FEProjectPluginInfo> MissingPlugins;
	for (const FEProjectPluginInfo& Plugin : Project->GetMissingPlugins())
	{
		if (!IsInstalledThisSession(Plugin.Name))
			MissingPlugins.push_back(Plugin);
	}

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
	const bool bUninstallPending = PLUGIN_MANAGER.IsUninstallPending(Manifest.ID);
	const std::string PendingUpdateVersion = PLUGIN_MANAGER.GetPendingUpdateVersion(Manifest.ID);
	const FEPluginInstallTask* InstallTask = PLUGIN_BROWSER.FindInstallTask(Manifest.Name);
	const bool bUpdateInProgress = InstallTask != nullptr && InstallTask->bUpdate && InstallTask->State != FEPluginInstallState::Installed && InstallTask->State != FEPluginInstallState::Failed;
	const bool bChangePending = bUninstallPending || !PendingUpdateVersion.empty() || bUpdateInProgress;
	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	const bool bEnabled = CurrentProject != nullptr && CurrentProject->UsesPlugin(Manifest.ID);
	const bool bRequested = EDITOR_PLUGIN_MANAGER.IsPluginRequested(Plugin);
	ImVec4 StatusColor = WarningColor;
	if (bChangePending)
	{
		StatusColor = WarningColor;
	}
	else if (!bRequested)
	{
		StatusColor = LabelColor;
	}
	else if (bAllModulesLoaded)
	{
		StatusColor = LoadedColor;
	}
	else if (LoadedModuleCount == 0)
	{
		StatusColor = ErrorColor;
	}

	ImGui::PushID(&Plugin);

	// Plugins with problems are open, so problems are visible right away.
	if (bRequested && !bAllModulesLoaded)
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
	std::string ModuleCountText = std::to_string(LoadedModuleCount) + "/" + std::to_string(Manifest.Modules.size()) + " loaded";
	if (bUninstallPending)
	{
		ModuleCountText = "removed after restart";
	}
	else if (!PendingUpdateVersion.empty())
	{
		ModuleCountText = "updated to " + PendingUpdateVersion + " after restart";
	}
	else if (bUpdateInProgress)
	{
		ModuleCountText = "updating...";
	}
	else if (!bRequested && CurrentProject == nullptr)
	{
		ModuleCountText = "loaded with projects that enable it";
	}
	else if (!bRequested)
	{
		ModuleCountText = bEnabled ? "loaded after restart" : "not enabled";
	}
	else if (!bEnabled)
	{
		const std::vector<FEUUID> AlwaysLoadedPlugins = FEEditorPluginManager::GetAlwaysLoadedPlugins();
		if (std::find(AlwaysLoadedPlugins.begin(), AlwaysLoadedPlugins.end(), Manifest.ID) != AlwaysLoadedPlugins.end())
			ModuleCountText += ", used by the editor";
	}
	const float ModuleCountX = OpenFolderButtonX - Style.ItemSpacing.x * 2.0f - ImGui::CalcTextSize(ModuleCountText.c_str()).x;
	DrawList->AddText(ImVec2(ModuleCountX, TextY), ImGui::GetColorU32(StatusColor), ModuleCountText.c_str());

	// Checkbox is smaller than usual, so it fits in the header.
	const char* EnabledText = "Enable in project";
	const float CheckboxSize = ImGui::GetFontSize() + 2.0f;
	const float CheckboxX = ModuleCountX - Style.ItemSpacing.x * 2.0f - CheckboxSize - Style.ItemInnerSpacing.x - ImGui::CalcTextSize(EnabledText).x;
	const float RightItemsX = CurrentProject != nullptr ? CheckboxX : ModuleCountX;

	const FEPluginBrowserEntry* BrowserEntry = PLUGIN_BROWSER.FindEntry(Manifest.ID);
	if (!bChangePending && BrowserEntry != nullptr && FEPluginBrowser::CompareVersions(BrowserEntry->Manifest.Version, Manifest.Version) > 0)
	{
		const std::string UpdateText = BrowserEntry->Manifest.Version + " available";
		const float UpdateTextX = RightItemsX - Style.ItemSpacing.x * 2.0f - ImGui::CalcTextSize(UpdateText.c_str()).x;
		const float VersionEndX = TextX + ImGui::CalcTextSize(Manifest.Version.c_str()).x + Style.ItemSpacing.x * 2.0f;
		if (UpdateTextX > VersionEndX)
			DrawList->AddText(ImVec2(UpdateTextX, TextY), ImGui::GetColorU32(WarningColor), UpdateText.c_str());
	}

	if (CurrentProject != nullptr)
	{
		// Plugin that enabled plugins depend on is loaded anyway, so it can not be disabled.
		const std::vector<FEUUID> DependentPlugins = CurrentProject->GetEnabledDependentPlugins(Manifest.ID);
		std::string DependentPluginNames;
		for (size_t i = 0; i < DependentPlugins.size(); i++)
			DependentPluginNames += (i == 0 ? "" : ", ") + FindPluginName(DependentPlugins[i]);

		bool bChecked = bEnabled || !DependentPlugins.empty();
		ImGui::SameLine();
		ImGui::SetCursorScreenPos(ImVec2(CheckboxX, HeaderPosition.y + (FrameHeight - CheckboxSize) * 0.5f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
		ImGui::BeginDisabled(!DependentPlugins.empty());
		if (ImGui::Checkbox(EnabledText, &bChecked))
		{
			if (bChecked)
			{
				CurrentProject->EnablePlugin(Manifest);
			}
			else
			{
				CurrentProject->DisablePlugin(Manifest.ID);
			}
		}
		ImGui::EndDisabled();
		ImGui::PopStyleVar();

		if (DependentPlugins.empty())
		{
			ImGui::SetItemTooltip("Editor loads enabled plugins when this project is opened, change is applied after restart.");
		}
		else
		{
			ImGui::SetItemTooltip("Required by %s.", DependentPluginNames.c_str());
		}
	}

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

	if (ImGui::Button("Create package..."))
		SavePluginPackage(Plugin);
	ImGui::SetItemTooltip("Saves plugin to a file that could be installed with \"Install plugin...\".");

	ImGui::SameLine();
	if (bUninstallPending)
	{
		if (ImGui::Button("Cancel uninstall"))
		{
			std::string ErrorMessage;
			PLUGIN_MANAGER.CancelUninstall(Manifest.ID, ErrorMessage);
		}
		ImGui::SetItemTooltip("Plugin will be removed when the editor is restarted.");
	}
	else if (ImGui::Button("Uninstall..."))
	{
		StartUninstall(Plugin);
	}

	if (!PendingUpdateVersion.empty())
	{
		ImGui::SameLine();
		if (ImGui::Button("Cancel update"))
		{
			std::string ErrorMessage;
			PLUGIN_MANAGER.CancelUpdate(Manifest.ID, ErrorMessage);
		}
		ImGui::SetItemTooltip("Plugin will be updated to %s when the editor is restarted.", PendingUpdateVersion.c_str());
	}
	else if (!bUninstallPending && BrowserEntry != nullptr && GetBrowserEntryStatus(*BrowserEntry).bCanBeUpdated)
	{
		ImGui::SameLine();
		ImGui::BeginDisabled(PLUGIN_BROWSER.IsInstalling());
		const std::string UpdateButtonText = "Update to " + BrowserEntry->Manifest.Version;
		if (ImGui::Button(UpdateButtonText.c_str()))
			StartInstall({ BrowserEntry });
		ImGui::EndDisabled();
	}

	if (PackagedPluginID == Manifest.ID && !PackageResultMessage.empty())
	{
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(bPackageCreated ? LoadedColor : ErrorColor, "%s", PackageResultMessage.c_str());
	}
	ImGui::Spacing();

	ImGui::Unindent();
	ImGui::PopID();
}

void FEEditorPluginsWindow::RenderModulesTable(const DiscoveredPlugin& Plugin)
{
	const PluginManifest& Manifest = Plugin.Manifest;
	if (!ImGui::BeginTable("Modules", 5, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
		return;

	ImGui::TableSetupColumn("Module");
	ImGui::TableSetupColumn("Type");
	ImGui::TableSetupColumn("Graphics modes");
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
		ImGui::TextUnformatted(GetGraphicsModesText(Module).c_str());

		ImGui::TableNextColumn();
		switch (GetModuleStatus(Plugin, Module))
		{
			case ModuleStatus::Loaded:
				RenderStatusDot(LoadedColor);
				ImGui::TextColored(LoadedColor, "Loaded");
				break;

			case ModuleStatus::NotEnabled:
				RenderStatusDot(LabelColor);
				if (PROJECT_MANAGER.GetCurrent() != nullptr)
				{
					ImGui::TextColored(LabelColor, "Not enabled");
					ImGui::SetItemTooltip("Plugin was not enabled in the project when it was opened.");
				}
				else
				{
					ImGui::TextColored(LabelColor, "Not loaded");
					ImGui::SetItemTooltip("Plugin is loaded with projects that enable it.");
				}
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

void FEEditorPluginsWindow::OpenURL(const std::string& URL)
{
	HINSTANCE Result = ShellExecuteA(nullptr, "open", URL.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	if (reinterpret_cast<INT_PTR>(Result) <= 32)
		LOG.Add("FEEditorPluginsWindow::OpenURL: Failed to open \"" + URL + "\"", "FE_EDITOR", FE_LOG_WARNING);
}

// Same units as GitHub shows for release assets.
static std::string FormatFileSize(const uint64_t Size)
{
	char Buffer[32];
	if (Size < 1024 * 1024)
	{
		snprintf(Buffer, sizeof(Buffer), "%.0f KB", static_cast<double>(Size) / 1024.0);
	}
	else
	{
		snprintf(Buffer, sizeof(Buffer), "%.1f MB", static_cast<double>(Size) / (1024.0 * 1024.0));
	}

	return Buffer;
}

// Rounded down to minutes, so text does not change every second.
static std::string GetTimeSinceText(const uint64_t TimeInSeconds)
{
	const uint64_t CurrentTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_SECONDS);
	const uint64_t Seconds = CurrentTime > TimeInSeconds ? CurrentTime - TimeInSeconds : 0;
	if (Seconds < 60)
		return "just now";

	return TIME.TimeToFormattedString(Seconds / 60 * 60, FE_TIME_RESOLUTION_SECONDS) + " ago";
}

static std::string ToLowerText(std::string Text)
{
	std::transform(Text.begin(), Text.end(), Text.begin(), [](unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
	return Text;
}

void FEEditorPluginsWindow::RenderBrowseTab()
{
	if (!PLUGIN_BROWSER.IsAvailable())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
		ImGui::TextWrapped("Browsing needs CurlPlugin. It is downloaded when the editor is configured with CMake, see fe_optional_plugin in CMakeLists.txt of the editor.");
		ImGui::PopStyleColor();
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, LabelColor);
		ImGui::TextWrapped("Plugins can still be installed from a file with \"Install plugin...\".");
		ImGui::PopStyleColor();
		return;
	}

	if (!PLUGIN_BROWSER.WasRefreshed())
		PLUGIN_BROWSER.Refresh();

	RenderBrowseToolbar();
	ImGui::Spacing();
	RenderUpdateAllLine();

	if (PLUGIN_BROWSER.HasFailed())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);
		ImGui::TextWrapped("Could not load plugin list: %s", PLUGIN_BROWSER.GetErrorMessage().c_str());
		ImGui::PopStyleColor();
		ImGui::Spacing();
	}

	const std::vector<FEPluginBrowserEntry>& Entries = PLUGIN_BROWSER.GetEntries();
	if (Entries.empty())
	{
		if (PLUGIN_BROWSER.IsLoading())
		{
			ImGui::TextColored(LabelColor, "Loading plugin list...");
		}
		else if (!PLUGIN_BROWSER.HasFailed())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, LabelColor);
			ImGui::TextWrapped("No public repository of %s has all required topics and a release.", PLUGIN_BROWSER.GetOwner().c_str());
			ImGui::PopStyleColor();
		}

		return;
	}

	const std::string Filter = ToLowerText(BrowseFilter);
	size_t ShownCount = 0;
	for (size_t i = 0; i < Entries.size(); i++)
	{
		const std::string Description = Entries[i].bManifestLoaded ? Entries[i].Manifest.Description : Entries[i].RepositoryDescription;
		if (!Filter.empty() && ToLowerText(Entries[i].RepositoryName).find(Filter) == std::string::npos && ToLowerText(Description).find(Filter) == std::string::npos)
			continue;

		ShownCount++;
		RenderBrowserEntry(Entries[i]);
	}

	if (ShownCount == 0)
		ImGui::TextColored(LabelColor, "No plugin matches the filter.");
}

void FEEditorPluginsWindow::RenderBrowseToolbar()
{
	std::string LoadText;
	if (PLUGIN_BROWSER.IsLoading())
	{
		LoadText = "Loading...";
	}
	else if (PLUGIN_BROWSER.GetLastLoadTime() != 0)
	{
		LoadText = "Updated " + GetTimeSinceText(PLUGIN_BROWSER.GetLastLoadTime());
	}

	// Filter takes width that is left after load status and refresh button.
	const char* RefreshText = "Refresh";
	const ImGuiStyle& Style = ImGui::GetStyle();
	const float RightPartWidth = ImGui::CalcTextSize(LoadText.c_str()).x + ImGui::CalcTextSize(RefreshText).x + Style.FramePadding.x * 2.0f + Style.ItemSpacing.x * 2.0f;
	ImGui::SetNextItemWidth(ImMax(ImGui::GetFontSize() * 6.0f, ImGui::GetContentRegionAvail().x - RightPartWidth));
	ImGui::InputTextWithHint("##BrowseFilter", "Filter by name or description", &BrowseFilter);

	ImGui::SameLine();
	ImGui::TextColored(LabelColor, "%s", LoadText.c_str());

	ImGui::SameLine();
	ImGui::BeginDisabled(PLUGIN_BROWSER.IsLoading());
	if (ImGui::Button(RefreshText))
		PLUGIN_BROWSER.Refresh();
	ImGui::EndDisabled();

	const std::vector<std::string> Topics = PLUGIN_BROWSER.GetRequiredTopics();
	std::string TopicsText;
	for (size_t i = 0; i < Topics.size(); i++)
		TopicsText += (i == 0 ? "" : ", ") + Topics[i];

	ImGui::PushStyleColor(ImGuiCol_Text, LabelColor);
	ImGui::TextWrapped("Public repositories of github.com/%s with topics %s and a release.", PLUGIN_BROWSER.GetOwner().c_str(), TopicsText.c_str());
	ImGui::PopStyleColor();
}

void FEEditorPluginsWindow::RenderBrowserEntry(const FEPluginBrowserEntry& Entry)
{
	const FEBrowserEntryStatus Status = GetBrowserEntryStatus(Entry);
	ImVec4 StatusColor = Status.Color;

	const FEPluginInstallTask* InstallTask = PLUGIN_BROWSER.FindInstallTask(Entry.RepositoryName);
	const bool bInstallInProgress = InstallTask != nullptr && InstallTask->State != FEPluginInstallState::Installed && InstallTask->State != FEPluginInstallState::Failed;
	if (bInstallInProgress)
		StatusColor = WarningColor;

	ImGui::PushID(Entry.RepositoryName.c_str());

	const ImVec2 HeaderPosition = ImGui::GetCursorScreenPos();
	const float HeaderRight = HeaderPosition.x + ImGui::GetContentRegionAvail().x;
	const bool bOpen = ImGui::CollapsingHeader("##Header", ImGuiTreeNodeFlags_AllowOverlap);
	const bool bHeaderHovered = ImGui::IsItemHovered();

	// Drawn the same way as header of installed plugin.
	const ImGuiStyle& Style = ImGui::GetStyle();
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	const float FrameHeight = ImGui::GetFrameHeight();
	const float DotRadius = ImGui::GetFontSize() * 0.25f;
	const float TextY = HeaderPosition.y + Style.FramePadding.y;
	float TextX = HeaderPosition.x + ImGui::GetFontSize() + Style.FramePadding.x * 3.0f;

	DrawList->AddCircleFilled(ImVec2(TextX + DotRadius, HeaderPosition.y + FrameHeight * 0.5f), DotRadius, ImGui::GetColorU32(StatusColor));
	TextX += DotRadius * 2.0f + Style.ItemSpacing.x;
	DrawList->AddText(ImVec2(TextX, TextY), ImGui::GetColorU32(ImGuiCol_Text), Entry.RepositoryName.c_str());
	TextX += ImGui::CalcTextSize(Entry.RepositoryName.c_str()).x + Style.ItemSpacing.x;

	const std::string Version = Entry.bManifestLoaded ? Entry.Manifest.Version : Entry.ReleaseTag;
	DrawList->AddText(ImVec2(TextX, TextY), ImGui::GetColorU32(LabelColor), Version.c_str());
	TextX += ImGui::CalcTextSize(Version.c_str()).x + Style.ItemSpacing.x * 2.0f;

	// Right side of header shows install progress, install button or status.
	float RightPartX = HeaderRight;
	if (bInstallInProgress)
	{
		RightPartX = HeaderRight - Style.FramePadding.x - ImGui::GetFontSize() * 9.0f;
		RenderInstallProgress(*InstallTask, ImVec2(RightPartX, HeaderPosition.y + Style.FramePadding.y * 0.5f), ImVec2(HeaderRight - Style.FramePadding.x, HeaderPosition.y + FrameHeight - Style.FramePadding.y * 0.5f));
	}
	else if (Status.bCanBeInstalled || Status.bCanBeUpdated)
	{
		const char* ButtonText = Status.bCanBeUpdated ? "Update" : "Install";
		RightPartX = HeaderRight - ImGui::CalcTextSize(ButtonText).x - Style.FramePadding.x * 2.0f;
		// SmallButton aligns itself to text baseline of the header line, so header top is enough to center it.
		ImGui::SameLine();
		ImGui::SetCursorScreenPos(ImVec2(RightPartX, HeaderPosition.y));
		ImGui::BeginDisabled(PLUGIN_BROWSER.IsInstalling());
		if (ImGui::SmallButton(ButtonText))
			StartInstall({ &Entry });
		ImGui::EndDisabled();

		if (Status.bCanBeUpdated)
		{
			const std::string InstalledText = "installed " + Status.InstalledVersion;
			RightPartX -= Style.ItemSpacing.x + ImGui::CalcTextSize(InstalledText.c_str()).x;
			DrawList->AddText(ImVec2(RightPartX, TextY), ImGui::GetColorU32(LabelColor), InstalledText.c_str());
		}
	}
	else
	{
		RightPartX = HeaderRight - Style.FramePadding.x - ImGui::CalcTextSize(Status.Text.c_str()).x;
		DrawList->AddText(ImVec2(RightPartX, TextY), ImGui::GetColorU32(StatusColor), Status.Text.c_str());
		if (bHeaderHovered && !Status.Tooltip.empty() && ImGui::GetMousePos().x >= RightPartX)
			ImGui::SetTooltip("%s", Status.Tooltip.c_str());
	}

	// Size is left out in narrow window, so it does not overlap right side.
	const std::string SizeText = Entry.ArchiveSize > 0 ? FormatFileSize(Entry.ArchiveSize) : "";
	if (!SizeText.empty() && TextX + ImGui::CalcTextSize(SizeText.c_str()).x + Style.ItemSpacing.x * 2.0f < RightPartX)
		DrawList->AddText(ImVec2(TextX, TextY), ImGui::GetColorU32(LabelColor), SizeText.c_str());

	if (!bOpen)
	{
		ImGui::PopID();
		return;
	}

	ImGui::Indent();
	ImGui::Spacing();

	const std::string Description = Entry.bManifestLoaded && !Entry.Manifest.Description.empty() ? Entry.Manifest.Description : Entry.RepositoryDescription;
	if (!Description.empty())
	{
		ImGui::TextWrapped("%s", Description.c_str());
		ImGui::Spacing();
	}

	if (!Entry.Problem.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);
		ImGui::TextWrapped("%s", Entry.Problem.c_str());
		ImGui::PopStyleColor();
		ImGui::Spacing();
	}

	if (InstallTask != nullptr && InstallTask->State == FEPluginInstallState::Failed)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);
		ImGui::TextWrapped("%s failed: %s", InstallTask->bUpdate ? "Update" : "Install", InstallTask->ErrorMessage.c_str());
		ImGui::PopStyleColor();
		ImGui::BeginDisabled(PLUGIN_BROWSER.IsInstalling());
		if (ImGui::Button("Try again"))
			StartInstall({ &Entry });
		ImGui::EndDisabled();
		ImGui::Spacing();
	}

	if (ImGui::BeginTable("Details", 2, ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		auto AddRow = [](const char* Label, const std::string& Value) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "%s", Label);
			ImGui::TableNextColumn();
			if (Value.empty())
			{
				ImGui::TextColored(LabelColor, "-");
			}
			else
			{
				ImGui::TextWrapped("%s", Value.c_str());
			}
		};

		AddRow("Release", Entry.ReleaseDate.empty() ? Entry.ReleaseTag : Entry.ReleaseTag + ", " + Entry.ReleaseDate);
		AddRow("Archive", Entry.ArchiveName.empty() ? "" : Entry.ArchiveName + ", " + FormatFileSize(Entry.ArchiveSize));

		if (Entry.bManifestLoaded)
		{
			const PluginManifest& Manifest = Entry.Manifest;
			std::string SupportedOS;
			for (size_t i = 0; i < Manifest.SupportedOS.size(); i++)
				SupportedOS += (i == 0 ? "" : ", ") + OSToString(Manifest.SupportedOS[i]);
			AddRow("Supported OS", SupportedOS);

			std::string Dependencies;
			for (size_t i = 0; i < Manifest.Dependencies.size(); i++)
				Dependencies += (i == 0 ? "" : ", ") + FindInstalledOrAvailablePluginName(Manifest.Dependencies[i]);
			AddRow("Depends on", Dependencies);

			std::string Modules;
			for (size_t i = 0; i < Manifest.Modules.size(); i++)
				Modules += (i == 0 ? "" : ", ") + Manifest.Modules[i].Name + " (" + Manifest.Modules[i].Type + ")";
			AddRow("Modules", Modules);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "ID");
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "%s...", UNIQUE_ID.ToString(Manifest.ID).substr(0, 8).c_str());
			AddCopyIDBehavior(Manifest.ID);
		}

		ImGui::EndTable();
	}

	if (!Entry.ReleaseNotes.empty())
	{
		ImGui::Spacing();
		ImGui::TextColored(LabelColor, "Release notes");
		ImGui::TextWrapped("%s", Entry.ReleaseNotes.c_str());
	}

	ImGui::Spacing();
	const std::string GitHubURL = Entry.ReleaseURL.empty() ? Entry.RepositoryURL : Entry.ReleaseURL;
	if (ImGui::Button("Open on GitHub"))
		OpenURL(GitHubURL);
	ImGui::SetItemTooltip("%s", GitHubURL.c_str());
	ImGui::Spacing();

	ImGui::Unindent();
	ImGui::PopID();
}

FEEditorPluginsWindow::FEBrowserEntryStatus FEEditorPluginsWindow::GetBrowserEntryStatus(const FEPluginBrowserEntry& Entry) const
{
	FEBrowserEntryStatus Result;
	const std::string PluginName = Entry.bManifestLoaded ? Entry.Manifest.Name : Entry.RepositoryName;
	const FEPluginInstallTask* InstallTask = PLUGIN_BROWSER.FindInstallTask(Entry.RepositoryName);
	const bool bInstallFailed = InstallTask != nullptr && InstallTask->State == FEPluginInstallState::Failed;
	const bool bSupportsThisOS = std::find(Entry.Manifest.SupportedOS.begin(), Entry.Manifest.SupportedOS.end(), GetOS()) != Entry.Manifest.SupportedOS.end();

	const DiscoveredPlugin* InstalledPlugin = nullptr;
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const bool bSamePlugin = Entry.bManifestLoaded ? Plugins[i].Manifest.ID == Entry.Manifest.ID : Plugins[i].Manifest.Name == PluginName;
		if (bSamePlugin)
		{
			InstalledPlugin = &Plugins[i];
			break;
		}
	}

	if (InstalledPlugin != nullptr)
	{
		Result.InstalledVersion = InstalledPlugin->Manifest.Version;
		const std::string PendingUpdateVersion = PLUGIN_MANAGER.GetPendingUpdateVersion(InstalledPlugin->Manifest.ID);
		const std::string AvailableVersion = Entry.bManifestLoaded ? Entry.Manifest.Version : Entry.ReleaseTag;
		const int Comparison = FEPluginBrowser::CompareVersions(AvailableVersion, InstalledPlugin->Manifest.Version);
		if (PLUGIN_MANAGER.IsUninstallPending(InstalledPlugin->Manifest.ID))
		{
			Result.Text = "Removed after restart";
			Result.Color = WarningColor;
		}
		else if (!PendingUpdateVersion.empty())
		{
			Result.Text = "Updated to " + PendingUpdateVersion + " after restart";
			Result.Color = WarningColor;
		}
		else if (bInstallFailed)
		{
			Result.Text = "Update failed";
			Result.Color = ErrorColor;
			Result.Tooltip = InstallTask->ErrorMessage;
		}
		else if (Comparison > 0)
		{
			Result.Text = "Update from " + InstalledPlugin->Manifest.Version + " available";
			Result.Color = WarningColor;
			Result.Tooltip = Entry.Problem;
			Result.bCanBeUpdated = Entry.Problem.empty() && bSupportsThisOS;
		}
		else if (Comparison == 0)
		{
			Result.Text = "Installed";
			Result.Color = LoadedColor;
		}
		else
		{
			Result.Text = "Installed " + InstalledPlugin->Manifest.Version;
			Result.Color = LoadedColor;
			Result.Tooltip = "Installed version is newer than the latest release.";
		}

		return Result;
	}

	if (IsInstalledThisSession(PluginName))
	{
		Result.Text = "Restart to load";
		Result.Color = WarningColor;
	}
	else if (bInstallFailed)
	{
		Result.Text = "Install failed";
		Result.Color = ErrorColor;
		Result.Tooltip = InstallTask->ErrorMessage;
	}
	else if (!Entry.Problem.empty())
	{
		Result.Text = "Can not be installed";
		Result.Color = ErrorColor;
		Result.Tooltip = Entry.Problem;
	}
	else if (!bSupportsThisOS)
	{
		Result.Text = "Not available for " + OSToString(GetOS());
		Result.Color = LabelColor;
	}
	else
	{
		Result.Text = "Not installed";
		Result.Color = LabelColor;
		Result.bCanBeInstalled = true;
	}

	return Result;
}

std::vector<const FEPluginBrowserEntry*> FEEditorPluginsWindow::FindAvailableUpdates() const
{
	std::vector<const FEPluginBrowserEntry*> Result;
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const FEPluginBrowserEntry* Entry = PLUGIN_BROWSER.FindEntry(Plugins[i].Manifest.ID);
		if (Entry != nullptr && GetBrowserEntryStatus(*Entry).bCanBeUpdated)
			Result.push_back(Entry);
	}

	return Result;
}

void FEEditorPluginsWindow::RenderUpdateAllLine()
{
	const std::vector<const FEPluginBrowserEntry*> Updates = FindAvailableUpdates();
	if (Updates.empty())
		return;

	ImGui::AlignTextToFramePadding();
	ImGui::TextColored(WarningColor, "%zu %s available", Updates.size(), Updates.size() == 1 ? "update is" : "updates are");
	ImGui::SameLine();
	ImGui::BeginDisabled(PLUGIN_BROWSER.IsInstalling());
	if (ImGui::Button("Update all"))
		StartInstall(Updates);
	ImGui::EndDisabled();
	ImGui::Spacing();
}

bool FEEditorPluginsWindow::IsInstalledThisSession(const std::string& PluginName) const
{
	const std::vector<std::string>& BrowserInstalledNames = PLUGIN_BROWSER.GetInstalledPluginNames();
	return std::find(InstalledPluginNames.begin(), InstalledPluginNames.end(), PluginName) != InstalledPluginNames.end() ||
		   std::find(BrowserInstalledNames.begin(), BrowserInstalledNames.end(), PluginName) != BrowserInstalledNames.end();
}

void FEEditorPluginsWindow::RenderInstallProgress(const FEPluginInstallTask& Task, const ImVec2& Min, const ImVec2& Max)
{
	float Fraction = 0.0f;
	std::string Text;
	switch (Task.State)
	{
		case FEPluginInstallState::Downloading:
			if (Task.TotalBytes > 0)
			{
				Fraction = ImClamp(static_cast<float>(static_cast<double>(Task.DownloadedBytes) / static_cast<double>(Task.TotalBytes)), 0.0f, 1.0f);
				Text = std::to_string(static_cast<int>(Fraction * 100.0f)) + "%";
			}
			else
			{
				Text = FormatFileSize(Task.DownloadedBytes);
			}
			break;

		case FEPluginInstallState::CheckingArchive:
			Text = "Checking...";
			break;

		case FEPluginInstallState::Downloaded:
			Text = PLUGIN_BROWSER.IsWaitingForConfirmation() ? "Waiting" : "Downloaded";
			break;

		case FEPluginInstallState::Installing:
			Text = "Installing...";
			break;

		default:
			break;
	}

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	const float Rounding = ImGui::GetStyle().FrameRounding;
	DrawList->AddRectFilled(Min, Max, ImGui::GetColorU32(ImGuiCol_FrameBg), Rounding);
	if (Fraction > 0.0f)
		DrawList->AddRectFilled(Min, ImVec2(Min.x + (Max.x - Min.x) * Fraction, Max.y), ImGui::GetColorU32(ImGuiCol_PlotHistogram), Rounding);

	const ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
	DrawList->AddText(ImVec2((Min.x + Max.x - TextSize.x) * 0.5f, (Min.y + Max.y - TextSize.y) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), Text.c_str());
}

void FEEditorPluginsWindow::StartInstall(const std::vector<const FEPluginBrowserEntry*>& Targets)
{
	// Names are stored, because entries are replaced when plugin list is refreshed.
	PluginsToInstall.clear();
	InstallTargets.clear();
	MissingDependenciesOfInstall.clear();
	for (const FEPluginBrowserEntry* Target : Targets)
	{
		InstallTargets.push_back(Target->RepositoryName);

		std::vector<FEUUID> MissingDependencies;
		const std::vector<const FEPluginBrowserEntry*> InstallOrder = PLUGIN_BROWSER.GetInstallOrder(*Target, MissingDependencies);
		for (const FEPluginBrowserEntry* EntryToInstall : InstallOrder)
		{
			if (std::find(PluginsToInstall.begin(), PluginsToInstall.end(), EntryToInstall->RepositoryName) == PluginsToInstall.end())
				PluginsToInstall.push_back(EntryToInstall->RepositoryName);
		}

		for (const FEUUID& DependencyID : MissingDependencies)
		{
			if (std::find(MissingDependenciesOfInstall.begin(), MissingDependenciesOfInstall.end(), DependencyID) == MissingDependenciesOfInstall.end())
				MissingDependenciesOfInstall.push_back(DependencyID);
		}
	}

	// User is asked only when plugins that user did not choose are installed too, or when some dependencies are missing.
	if (PluginsToInstall.size() == InstallTargets.size() && MissingDependenciesOfInstall.empty())
	{
		PLUGIN_BROWSER.Install(PluginsToInstall);
		return;
	}

	bShouldOpenInstallDependenciesPopup = true;
}

void FEEditorPluginsWindow::RenderInstallDependenciesPopup()
{
	if (bShouldOpenInstallDependenciesPopup)
	{
		ImGui::OpenPopup("Install plugins");
		bShouldOpenInstallDependenciesPopup = false;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	const bool bPopupOpen = ImGui::BeginPopupModal("Install plugins", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PopStyleVar();
	if (!bPopupOpen)
		return;

	const std::string PluginName = InstallTargets.size() == 1 ? InstallTargets[0] : "Plugins";
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f);
	if (InstallTargets.size() > 1)
	{
		ImGui::TextWrapped("These plugins will be installed or updated:");
	}
	else if (PluginsToInstall.size() > 1)
	{
		ImGui::TextWrapped("%s depends on plugins that are not installed, they will be installed first:", PluginName.c_str());
	}
	else
	{
		ImGui::TextWrapped("%s will be installed:", PluginName.c_str());
	}
	ImGui::Spacing();

	uint64_t TotalSize = 0;
	if (ImGui::BeginTable("PluginsToInstall", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("Plugin");
		ImGui::TableSetupColumn("Action");
		ImGui::TableSetupColumn("Version");
		ImGui::TableSetupColumn("Size");
		ImGui::TableHeadersRow();
		for (const std::string& Name : PluginsToInstall)
		{
			const FEPluginBrowserEntry* Entry = PLUGIN_BROWSER.FindEntryByName(Name);
			const std::string InstalledVersion = Entry != nullptr ? GetBrowserEntryStatus(*Entry).InstalledVersion : "";
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(Name.c_str());
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "%s", InstalledVersion.empty() ? "Install" : ("Update from " + InstalledVersion).c_str());
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "%s", Entry != nullptr ? Entry->Manifest.Version.c_str() : "-");
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "%s", Entry != nullptr ? FormatFileSize(Entry->ArchiveSize).c_str() : "-");
			if (Entry != nullptr)
				TotalSize += Entry->ArchiveSize;
		}

		ImGui::EndTable();
	}

	ImGui::TextColored(LabelColor, "Download size: %s", FormatFileSize(TotalSize).c_str());

	if (!MissingDependenciesOfInstall.empty())
	{
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);
		ImGui::TextWrapped("These dependencies are not installed and are not in the plugin list, %s could fail to load without them:", PluginName.c_str());
		for (const FEUUID& DependencyID : MissingDependenciesOfInstall)
			ImGui::BulletText("%s", FindInstalledOrAvailablePluginName(DependencyID).c_str());
		ImGui::PopStyleColor();
	}
	ImGui::PopTextWrapPos();

	ImGui::Spacing();
	const ImVec2 ButtonSize = ImVec2(ImGui::GetFontSize() * 6.0f, 0.0f);
	if (ImGui::Button("Install", ButtonSize))
	{
		PLUGIN_BROWSER.Install(PluginsToInstall);
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	if (ImGui::Button("Cancel", ButtonSize))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}

void FEEditorPluginsWindow::RenderEngineCommitPopup()
{
	const char* PopupName = "Different engine commit";
	if (PLUGIN_BROWSER.IsWaitingForConfirmation() && !ImGui::IsPopupOpen(PopupName))
		ImGui::OpenPopup(PopupName);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	const bool bPopupOpen = ImGui::BeginPopupModal(PopupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PopStyleVar();
	if (!bPopupOpen)
		return;

	if (!PLUGIN_BROWSER.IsWaitingForConfirmation())
	{
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	const std::string EngineCommitHash = ENGINE.GetEngineCommitHash();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f);
	ImGui::TextWrapped("These plugins were built with a different engine commit than this editor (engine %s), they could fail to load or crash:", EngineCommitHash.empty() ? "unknown" : EngineCommitHash.c_str());
	ImGui::PopTextWrapPos();
	ImGui::Spacing();

	if (ImGui::BeginTable("EngineCommits", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("Plugin");
		ImGui::TableSetupColumn("Built with engine");
		ImGui::TableHeadersRow();
		for (const FEPluginInstallTask& Task : PLUGIN_BROWSER.GetInstallTasks())
		{
			if (FEPluginBrowser::IsSameEngineCommit(Task.EngineCommit))
				continue;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(Task.PluginName.c_str());
			ImGui::TableNextColumn();
			if (Task.EngineCommit.empty())
			{
				ImGui::TextColored(WarningColor, "unknown, archive has no EngineCommit.txt");
			}
			else
			{
				ImGui::TextColored(WarningColor, "%s", Task.EngineCommit.substr(0, 7).c_str());
				ImGui::SetItemTooltip("%s", Task.EngineCommit.c_str());
			}
		}

		ImGui::EndTable();
	}

	ImGui::Spacing();
	const ImVec2 ButtonSize = ImVec2(ImGui::GetFontSize() * 8.0f, 0.0f);
	if (ImGui::Button("Install anyway", ButtonSize))
	{
		PLUGIN_BROWSER.ContinueInstall();
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	if (ImGui::Button("Cancel", ButtonSize))
	{
		PLUGIN_BROWSER.CancelInstall();
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

bool FEEditorPluginsWindow::IsVisible() const
{
	return bVisible;
}

void FEEditorPluginsWindow::SetVisible(const bool bNewValue)
{
	bVisible = bNewValue;
}

void FEEditorPluginsWindow::EnablePluginsInstalledFromBrowser()
{
	const std::vector<std::string>& InstalledNames = PLUGIN_BROWSER.GetInstalledPluginNames();
	for (; HandledBrowserInstallCount < InstalledNames.size(); HandledBrowserInstallCount++)
	{
		FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
		const FEPluginBrowserEntry* Entry = PLUGIN_BROWSER.FindEntryByName(InstalledNames[HandledBrowserInstallCount]);
		if (CurrentProject != nullptr && Entry != nullptr && Entry->bManifestLoaded)
			CurrentProject->EnablePlugin(Entry->Manifest);
	}
}

std::string FEEditorPluginsWindow::GetPendingPluginChangesText() const
{
	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject == nullptr || EDITOR_PLUGIN_MANAGER.IsAutomaticLoadingEnabled())
		return "";

	const std::vector<FEUUID>& RequestedPlugins = EDITOR_PLUGIN_MANAGER.GetRequestedPlugins();
	std::string NamesToEnable;
	for (const FEProjectPluginInfo& Plugin : CurrentProject->GetEnabledPlugins())
	{
		if (std::find(RequestedPlugins.begin(), RequestedPlugins.end(), Plugin.ID) == RequestedPlugins.end())
			NamesToEnable += (NamesToEnable.empty() ? "" : ", ") + Plugin.Name;
	}

	const std::vector<FEUUID> AlwaysLoadedPlugins = FEEditorPluginManager::GetAlwaysLoadedPlugins();
	std::string NamesToDisable;
	for (const DiscoveredPlugin& Plugin : PLUGIN_MANAGER.GetDiscoveredPlugins())
	{
		const FEUUID& PluginID = Plugin.Manifest.ID;
		if (CurrentProject->UsesPlugin(PluginID) || std::find(RequestedPlugins.begin(), RequestedPlugins.end(), PluginID) == RequestedPlugins.end())
			continue;

		if (std::find(AlwaysLoadedPlugins.begin(), AlwaysLoadedPlugins.end(), PluginID) == AlwaysLoadedPlugins.end())
			NamesToDisable += (NamesToDisable.empty() ? "" : ", ") + Plugin.Manifest.Name;
	}

	std::string Result;
	if (!NamesToEnable.empty())
		Result = "enable " + NamesToEnable;

	if (!NamesToDisable.empty())
		Result += (Result.empty() ? "" : "; ") + ("disable " + NamesToDisable);

	return Result;
}

FEEditorPluginsWindow::ModuleStatus FEEditorPluginsWindow::GetModuleStatus(const DiscoveredPlugin& Plugin, const PluginModule& Module) const
{
	if (EDITOR_PLUGIN_MANAGER.IsModuleLoaded(Module))
		return ModuleStatus::Loaded;

	if (!EDITOR_PLUGIN_MANAGER.IsPluginRequested(Plugin))
		return ModuleStatus::NotEnabled;

	for (size_t i = 0; i < Module.Dependencies.size(); i++)
	{
		const PluginModule* Dependency = FindModule(Module.Dependencies[i]);
		if (Dependency == nullptr || !EDITOR_PLUGIN_MANAGER.IsModuleLoaded(*Dependency))
			return ModuleStatus::Skipped;
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

std::string FEEditorPluginsWindow::FindInstalledOrAvailablePluginName(const FEUUID& PluginID) const
{
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		if (Plugins[i].Manifest.ID == PluginID)
			return Plugins[i].Manifest.Name;
	}

	const FEPluginBrowserEntry* Entry = PLUGIN_BROWSER.FindEntry(PluginID);
	if (Entry != nullptr)
		return Entry->Manifest.Name;

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
		ImGui::TextColored(LabelColor, "Folder");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(BrowseButtonText).x - Style.FramePadding.x * 2.0f - Style.ItemSpacing.x);
		ImGui::InputText("##Folder", &NewPluginLocation);
		ImGui::SameLine();
		if (ImGui::Button(BrowseButtonText))
		{
			FILE_SYSTEM.OpenFolderDialog([this](const std::string& Path) {
				if (Path.empty())
					return;

				NewPluginLocation = Path;
				if (NewPluginName.empty())
					NewPluginName = FILE_SYSTEM.GetFileName(Path);
			});
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

		if (NewPluginType == FEPluginProjectType::Editor)
			RenderOptionRow("Without graphics", "##WithoutGraphics", &bNewPluginWorksWithoutGraphics, false, false, "Plugin has no engine module.");
		else
			RenderOptionRow("Without graphics", "##WithoutGraphics", &bNewPluginWorksWithoutGraphics, true, false, "Engine module could also run in applications without window and graphics.");

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
		Settings.bEngineModuleWorksWithoutGraphics = bNewPluginWorksWithoutGraphics;
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
	std::string Folder = NewPluginLocation;
	while (!Folder.empty() && (Folder.back() == '/' || Folder.back() == '\\'))
		Folder.pop_back();

	return Folder;
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

	const std::string NewPluginFolder = GetNewPluginFolder();
	if (NewPluginFolder.empty())
		return "Choose folder of plugin project.";

	if (!FILE_SYSTEM.DoesDirectoryExist(NewPluginFolder) && !FILE_SYSTEM.DoesDirectoryExist(FILE_SYSTEM.GetDirectoryPath(NewPluginFolder)))
		return "Folder that should contain plugin project folder does not exist.";

	return "";
}

void FEEditorPluginsWindow::RenderInstallPluginPopup()
{
	if (bShouldOpenInstallPluginPopup)
	{
		ImGui::OpenPopup("Install plugin");
		bShouldOpenInstallPluginPopup = false;
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetFontSize() * 30.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	const bool bPopupOpen = ImGui::BeginPopupModal("Install plugin", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PopStyleVar();
	if (!bPopupOpen)
		return;

	ImGui::TextColored(LabelColor, "%s", GetDisplayPath(InstallPackagePath).c_str());
	ImGui::Spacing();

	const ImVec2 ButtonSize = ImVec2(ImGui::GetFontSize() * 6.0f, 0.0f);
	const PluginManifest& Manifest = InstallPackageManifest;
	if (bPluginInstalled)
	{
		const std::string InstalledFolder = GetDisplayPath(PLUGIN_MANAGER.GetPluginDirectory() + "/" + Manifest.Name);
		ImGui::TextColored(LoadedColor, "Plugin \"%s\" was installed, restart the editor to load it.", Manifest.Name.c_str());
		ImGui::TextUnformatted(InstalledFolder.c_str());

		ImGui::Spacing();
		if (ImGui::Button("Open folder"))
			OpenFolderInExplorer(InstalledFolder);

		ImGui::SameLine();
		if (ImGui::Button("Close", ButtonSize))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
		return;
	}

	if (bInstallPackageRead)
	{
		ImGui::TextUnformatted(Manifest.Name.c_str());
		ImGui::SameLine();
		ImGui::TextColored(LabelColor, "%s", Manifest.Version.c_str());
		if (!Manifest.Description.empty())
			ImGui::TextWrapped("%s", Manifest.Description.c_str());

		ImGui::Spacing();
		if (ImGui::BeginTable("PackageDetails", 2, ImGuiTableFlags_SizingFixedFit))
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

			for (size_t i = 0; i < Manifest.Modules.size(); i++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (i == 0)
					ImGui::TextColored(LabelColor, "Modules");

				ImGui::TableNextColumn();
				ImGui::TextUnformatted(Manifest.Modules[i].Name.c_str());
				ImGui::SameLine();
				ImGui::TextColored(Manifest.Modules[i].Type == "Engine" ? EngineModuleColor : EditorModuleColor, "%s", Manifest.Modules[i].Type.c_str());
				ImGui::SameLine();
				ImGui::TextColored(LabelColor, "%s", GetGraphicsModesText(Manifest.Modules[i]).c_str());
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "ID");
			ImGui::TableNextColumn();
			ImGui::TextColored(LabelColor, "%s...", UNIQUE_ID.ToString(Manifest.ID).substr(0, 8).c_str());
			AddCopyIDBehavior(Manifest.ID);

			ImGui::EndTable();
		}
	}

	if (!InstallPluginError.empty())
	{
		ImGui::Spacing();
		ImGui::TextColored(ErrorColor, "%s", InstallPluginError.c_str());
	}

	ImGui::Spacing();
	ImGui::BeginDisabled(!bInstallPackageRead);
	if (ImGui::Button("Install", ButtonSize))
	{
		InstallPluginError.clear();
		bPluginInstalled = PLUGIN_MANAGER.InstallPluginPackage(InstallPackagePath, InstallPluginError);
		if (bPluginInstalled)
		{
			InstalledPluginNames.push_back(Manifest.Name);
			if (PROJECT_MANAGER.GetCurrent() != nullptr)
				PROJECT_MANAGER.GetCurrent()->EnablePlugin(Manifest);
		}
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Cancel", ButtonSize))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}

void FEEditorPluginsWindow::SavePluginPackage(const DiscoveredPlugin& Plugin)
{
	const std::string PluginFolder = Plugin.FolderPath;
	const FEUUID PluginID = Plugin.Manifest.ID;
	FILE_SYSTEM.SaveFileDialog(Plugin.Manifest.Name + ".feplugin", PluginPackageFilter, [this, PluginFolder, PluginID](std::string FilePath, int ChosenFilterIndex) {
		if (FILE_SYSTEM.GetFileExtension(FilePath) != ".feplugin")
			FilePath += ".feplugin";

		PackagedPluginID = PluginID;

		// Package saved in plugin folder would be added to the next package of this plugin.
		auto ToLower = [](std::string Text) {
			std::transform(Text.begin(), Text.end(), Text.begin(), [](unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
			return Text;
		};

		if (ToLower(GetDisplayPath(FilePath)).rfind(ToLower(GetDisplayPath(PluginFolder) + "\\"), 0) == 0)
		{
			bPackageCreated = false;
			PackageResultMessage = "Package can not be saved inside of plugin folder.";
			return false;
		}

		std::string ErrorMessage;
		bPackageCreated = PLUGIN_MANAGER.CreatePluginPackage(PluginFolder, FilePath, ErrorMessage);
		PackageResultMessage = bPackageCreated ? "Saved to " + GetDisplayPath(FilePath) : ErrorMessage;
		return bPackageCreated;
	});
}

std::vector<FEUUID> FEEditorPluginsWindow::FindDependentPlugins(const FEUUID& PluginID) const
{
	std::vector<FEUUID> Result;
	std::vector<FEUUID> IDsToCheck = { PluginID };
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < IDsToCheck.size(); i++)
	{
		for (const DiscoveredPlugin& Plugin : Plugins)
		{
			const FEUUID& CandidateID = Plugin.Manifest.ID;
			if (CandidateID == PluginID || PLUGIN_MANAGER.IsUninstallPending(CandidateID) || std::find(Result.begin(), Result.end(), CandidateID) != Result.end())
				continue;

			const std::vector<FEUUID>& Dependencies = Plugin.Manifest.Dependencies;
			if (std::find(Dependencies.begin(), Dependencies.end(), IDsToCheck[i]) == Dependencies.end())
				continue;

			Result.push_back(CandidateID);
			IDsToCheck.push_back(CandidateID);
		}
	}

	return Result;
}

void FEEditorPluginsWindow::StartUninstall(const DiscoveredPlugin& Plugin)
{
	PluginToUninstall = Plugin.Manifest.ID;
	DependentPluginsOfUninstall = FindDependentPlugins(PluginToUninstall);
	UninstallErrorMessage.clear();
	bShouldOpenUninstallPopup = true;
}

bool FEEditorPluginsWindow::UninstallPlugins(const std::vector<FEUUID>& PluginIDs)
{
	UninstallErrorMessage.clear();
	for (const FEUUID& PluginID : PluginIDs)
	{
		std::string ErrorMessage;
		if (!PLUGIN_MANAGER.UninstallPlugin(PluginID, ErrorMessage))
			UninstallErrorMessage += (UninstallErrorMessage.empty() ? "" : "\n") + ErrorMessage;
	}

	return UninstallErrorMessage.empty();
}

void FEEditorPluginsWindow::RenderUninstallPopup()
{
	if (bShouldOpenUninstallPopup)
	{
		ImGui::OpenPopup("Uninstall plugin");
		bShouldOpenUninstallPopup = false;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	const bool bPopupOpen = ImGui::BeginPopupModal("Uninstall plugin", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PopStyleVar();
	if (!bPopupOpen)
		return;

	const std::string PluginName = FindPluginName(PluginToUninstall);
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f);
	ImGui::TextWrapped("%s will be removed when the editor is restarted.", PluginName.c_str());

	if (!DependentPluginsOfUninstall.empty())
	{
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
		ImGui::TextWrapped("These installed plugins depend on %s and would fail to load without it:", PluginName.c_str());
		for (const FEUUID& DependentID : DependentPluginsOfUninstall)
			ImGui::BulletText("%s", FindPluginName(DependentID).c_str());
		ImGui::PopStyleColor();
	}

	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	std::vector<FEUUID> PluginsToCheck = DependentPluginsOfUninstall;
	PluginsToCheck.insert(PluginsToCheck.begin(), PluginToUninstall);
	std::string PluginsUsedByProject;
	for (const FEUUID& PluginID : PluginsToCheck)
	{
		if (CurrentProject != nullptr && CurrentProject->UsesPlugin(PluginID))
			PluginsUsedByProject += (PluginsUsedByProject.empty() ? "" : ", ") + FindPluginName(PluginID);
	}

	if (!PluginsUsedByProject.empty())
	{
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
		ImGui::TextWrapped("Open project uses %s. After restart it will be listed as missing, and native scripts that use its API will not compile.", PluginsUsedByProject.c_str());
		ImGui::PopStyleColor();
	}

	if (!UninstallErrorMessage.empty())
	{
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);
		ImGui::TextWrapped("%s", UninstallErrorMessage.c_str());
		ImGui::PopStyleColor();
	}
	ImGui::PopTextWrapPos();

	ImGui::Spacing();
	const ImVec2 ButtonSize = ImVec2(ImGui::GetFontSize() * 6.0f, 0.0f);
	if (DependentPluginsOfUninstall.empty())
	{
		if (ImGui::Button("Uninstall", ButtonSize) && UninstallPlugins({ PluginToUninstall }))
			ImGui::CloseCurrentPopup();
	}
	else
	{
		if (ImGui::Button("Uninstall all") && UninstallPlugins(PluginsToCheck))
			ImGui::CloseCurrentPopup();

		ImGui::SameLine();
		const std::string UninstallOnlyText = "Uninstall only " + PluginName;
		if (ImGui::Button(UninstallOnlyText.c_str()) && UninstallPlugins({ PluginToUninstall }))
			ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	if (ImGui::Button("Cancel", ButtonSize))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}