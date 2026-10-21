#include "PluginsWindow.h"
#include <shellapi.h>

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

	if (ImGui::Begin("Plugins", nullptr, ImGuiWindowFlags_None))
	{
		const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
		if (Plugins.empty())
			ImGui::TextUnformatted("No plugins were found.");

		for (size_t i = 0; i < Plugins.size(); i++)
			RenderPluginDetails(Plugins[i]);
	}
	ImGui::End();
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

	ImGui::PushID(&Plugin);
	const std::string Header = Manifest.Name + " " + Manifest.Version + " (" + std::to_string(LoadedModuleCount) + "/" + std::to_string(Manifest.Modules.size()) + " modules loaded)";
	if (!ImGui::CollapsingHeader(Header.c_str()))
	{
		ImGui::PopID();
		return;
	}

	ImGui::TextUnformatted("ID:");
	ImGui::SameLine();
	RenderIDWithCopyButton(Manifest.ID);

	if (!Manifest.Description.empty())
		ImGui::TextWrapped("%s", Manifest.Description.c_str());

	std::string SupportedOS;
	for (size_t i = 0; i < Manifest.SupportedOS.size(); i++)
		SupportedOS += (i == 0 ? "" : ", ") + OSToString(Manifest.SupportedOS[i]);
	ImGui::Text("Supported OS: %s", SupportedOS.c_str());

	std::string Dependencies;
	for (size_t i = 0; i < Manifest.Dependencies.size(); i++)
		Dependencies += (i == 0 ? "" : ", ") + FindPluginName(Manifest.Dependencies[i]);
	ImGui::Text("Dependencies: %s", Dependencies.empty() ? "None" : Dependencies.c_str());

	std::string FolderPath = FILE_SYSTEM.GetAbsolutePath(Plugin.FolderPath);
	std::replace(FolderPath.begin(), FolderPath.end(), '/', '\\');
	if (ImGui::SmallButton("Open folder"))
	{
		HINSTANCE Result = ShellExecuteA(nullptr, "open", FolderPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		if (reinterpret_cast<INT_PTR>(Result) <= 32)
			LOG.Add("FEEditorPluginsWindow::RenderPluginDetails: Failed to open \"" + FolderPath + "\" in file explorer", "FE_EDITOR", FE_LOG_WARNING);
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(FolderPath.c_str());

	if (ImGui::BeginTable("Modules", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("Module");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("Status");
		ImGui::TableSetupColumn("ID");
		ImGui::TableSetupColumn("Dependencies");
		ImGui::TableHeadersRow();

		for (size_t i = 0; i < Manifest.Modules.size(); i++)
		{
			const PluginModule& Module = Manifest.Modules[i];
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(Module.Name.c_str());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(Module.Type.c_str());

			ImGui::TableNextColumn();
			if (EDITOR_PLUGIN_MANAGER.IsModuleLoaded(Module))
			{
				ImGui::TextUnformatted("Loaded");
			}
			else
			{
				ImGui::TextDisabled("Not loaded");
				ImGui::SetItemTooltip("Reason is in the log.");
			}

			ImGui::TableNextColumn();
			RenderIDWithCopyButton(Module.ID);

			ImGui::TableNextColumn();
			std::string ModuleDependencies;
			for (size_t j = 0; j < Module.Dependencies.size(); j++)
				ModuleDependencies += (j == 0 ? "" : ", ") + FindModuleName(Module.Dependencies[j]);
			ImGui::TextUnformatted(ModuleDependencies.empty() ? "None" : ModuleDependencies.c_str());
		}

		ImGui::EndTable();
	}

	ImGui::PopID();
}

void FEEditorPluginsWindow::RenderIDWithCopyButton(const FEUUID& ID)
{
	const std::string IDString = UNIQUE_ID.ToString(ID);
	ImGui::TextUnformatted(IDString.c_str());
	ImGui::SameLine();
	ImGui::PushID(IDString.c_str());
	if (ImGui::SmallButton("Copy"))
		ImGui::SetClipboardText(IDString.c_str());
	ImGui::PopID();
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

std::string FEEditorPluginsWindow::FindModuleName(const FEUUID& ModuleID) const
{
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const PluginManifest& Manifest = Plugins[i].Manifest;
		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			if (Manifest.Modules[j].ID == ModuleID)
				return Manifest.Name + "/" + Manifest.Modules[j].Name;
		}
	}

	return UNIQUE_ID.ToString(ModuleID) + " (not found)";
}