#pragma once

#include "../SubSystems/PluginSystem/FEEditorPluginManager.h"

class FEEditorPluginsWindow
{
	friend class FEEditor;
	SINGLETON_PRIVATE_PART(FEEditorPluginsWindow)

	bool bVisible = false;

	bool bShouldOpenNewPluginPopup = false;
	std::string NewPluginName;
	std::string NewPluginLocation;
	std::string NewPluginDescription;
	FEPluginProjectType NewPluginType = FEPluginProjectType::Engine;
	bool bNewPluginPublicInterface = false;
	bool bNewPluginEditorPublicInterface = false;
	std::string NewPluginCreationError;
	// Not empty after plugin project was created.
	std::string CreatedPluginFolder;

	enum class ModuleStatus
	{
		Loaded,
		// Editor module that was not loaded, because module that it depends on is not loaded.
		Skipped,
		Failed
	};

	void Render();
	void RenderPluginDetails(const DiscoveredPlugin& Plugin);
	void RenderModulesTable(const DiscoveredPlugin& Plugin);
	void RenderStatusDot(const ImVec4& Color);
	// Should be called right after item that shows ID, item gets tooltip with full ID and copies it on click.
	void AddCopyIDBehavior(const FEUUID& ID);
	void OpenFolderInExplorer(const std::string& Path);

	void RenderNewPluginPopup();
	void RenderNewPluginForm();
	void RenderNewPluginCreatedMessage();
	std::string GetNewPluginFolder() const;
	// Returns description of the first problem, or empty string if plugin project could be created.
	std::string ValidateNewPlugin() const;

	ModuleStatus GetModuleStatus(const PluginModule& Module) const;
	std::string GetDisplayPath(const std::string& Path) const;
	std::string FindPluginName(const FEUUID& PluginID) const;
	const PluginModule* FindModule(const FEUUID& ModuleID) const;
	// Module of the same plugin is named without plugin name.
	std::string FindModuleName(const FEUUID& ModuleID, const FEUUID& CurrentPluginID) const;
public:
	SINGLETON_PUBLIC_PART(FEEditorPluginsWindow)
};

#define PLUGINS_WINDOW FEEditorPluginsWindow::GetInstance()