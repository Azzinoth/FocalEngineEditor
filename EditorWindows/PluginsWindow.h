#pragma once

#include "../SubSystems/PluginSystem/FEEditorPluginManager.h"
#include "../SubSystems/PluginSystem/FEPluginBrowser.h"

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

	std::string BrowseFilter;

	bool bShouldOpenUninstallPopup = false;
	FEUUID PluginToUninstall;
	// Installed plugins that depend on plugin to uninstall, directly or through other plugins.
	std::vector<FEUUID> DependentPluginsOfUninstall;
	std::string UninstallErrorMessage;

	bool bShouldOpenInstallDependenciesPopup = false;
	// Dependencies first, plugin that user chose to install is the last.
	std::vector<std::string> PluginsToInstall;
	// Plugins that user chose to install or update, without their dependencies.
	std::vector<std::string> InstallTargets;
	std::vector<FEUUID> MissingDependenciesOfInstall;

	struct FEBrowserEntryStatus
	{
		std::string Text;
		ImVec4 Color;
		std::string Tooltip;
		bool bCanBeInstalled = false;
		bool bCanBeUpdated = false;
		// Version of installed plugin, empty if it is not installed.
		std::string InstalledVersion;
	};

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

	void StartUninstall(const DiscoveredPlugin& Plugin);
	void RenderUninstallPopup();
	std::vector<FEUUID> FindDependentPlugins(const FEUUID& PluginID) const;
	// Returns false and keeps error message if some plugin could not be uninstalled.
	bool UninstallPlugins(const std::vector<FEUUID>& PluginIDs);

	void RenderBrowseTab();
	void RenderBrowseToolbar();
	void RenderBrowserEntry(const FEPluginBrowserEntry& Entry);
	// Status of browser entry compared with installed plugins, shown on the right side of its header.
	FEBrowserEntryStatus GetBrowserEntryStatus(const FEPluginBrowserEntry& Entry) const;
	void RenderInstallProgress(const FEPluginInstallTask& Task, const ImVec2& Min, const ImVec2& Max);
	// Installs or updates plugins with their dependencies, asks user first if dependencies should be installed too.
	void StartInstall(const std::vector<const FEPluginBrowserEntry*>& Targets);
	// Installed plugins that have newer release that could be installed.
	std::vector<const FEPluginBrowserEntry*> FindAvailableUpdates() const;
	void RenderUpdateAllLine();
	void RenderInstallDependenciesPopup();
	void RenderEngineCommitPopup();
	// Installed with "Install plugin..." or from plugin browser, plugin is loaded after restart.
	bool IsInstalledThisSession(const std::string& PluginName) const;
	void OpenURL(const std::string& URL);

	ModuleStatus GetModuleStatus(const PluginModule& Module) const;
	std::string GetDisplayPath(const std::string& Path) const;
	std::string FindPluginName(const FEUUID& PluginID) const;
	// Also looks in plugin browser, for dependencies of plugins that are not installed.
	std::string FindInstalledOrAvailablePluginName(const FEUUID& PluginID) const;
	const PluginModule* FindModule(const FEUUID& ModuleID) const;
	// Module of the same plugin is named without plugin name.
	std::string FindModuleName(const FEUUID& ModuleID, const FEUUID& CurrentPluginID) const;
public:
	SINGLETON_PUBLIC_PART(FEEditorPluginsWindow)
};

#define PLUGINS_WINDOW FEEditorPluginsWindow::GetInstance()