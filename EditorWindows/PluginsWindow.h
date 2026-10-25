#pragma once

#include "../SubSystems/PluginSystem/FEEditorPluginManager.h"

class FEProject;

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
	bool bNewPluginWorksWithoutGraphics = false;
	std::string NewPluginCreationError;
	// Not empty after plugin project was created.
	std::string CreatedPluginFolder;

	bool bShouldOpenInstallPluginPopup = false;
	std::string InstallPackagePath;
	PluginManifest InstallPackageManifest;
	bool bInstallPackageRead = false;
	bool bPluginInstalled = false;
	std::string InstallPluginError;
	// Installed during this session, they are loaded after restart.
	std::vector<std::string> InstalledPluginNames;

	// Result of the last "Create package..." is shown next to the button of that plugin.
	FEUUID PackagedPluginID;
	bool bPackageCreated = false;
	std::string PackageResultMessage;

	enum class ModuleStatus
	{
		Loaded,
		// Editor module that was not loaded, because module that it depends on is not loaded.
		Skipped,
		Failed
	};

	void Render();
	void RenderMissingPlugins(FEProject* Project);
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

	void RenderInstallPluginPopup();
	void SavePluginPackage(const DiscoveredPlugin& Plugin);

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