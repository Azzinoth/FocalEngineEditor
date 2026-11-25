#pragma once

#include "../FocalEngine/FEngine.h"
#include "FEEditorAPI.h"
using namespace FocalEngine;

class FEEditorAPIImplementation;

enum class FEPluginProjectType
{
	Engine,
	EngineAndEditor,
	Editor
};

struct FEEditorPluginProjectSettings
{
	FEPluginProjectType Type = FEPluginProjectType::Engine;
	// Public interface of engine module, always added for Engine + Editor, because editor module uses it.
	bool bAddPublicInterface = false;
	// Public interface of editor module, editor modules of other plugins get it through FEEditorAPI::GetEditorModule.
	bool bAddEditorPublicInterface = false;
	// Engine module could also be loaded by applications without window and graphics.
	bool bEngineModuleWorksWithoutGraphics = false;
	std::string Description;
};

class FEEditorPluginManager
{
public:
	SINGLETON_PUBLIC_PART(FEEditorPluginManager)

	void Initialize();
	// Editor modules are loaded only for given plugins and for plugins whose engine modules are loaded.
	void Initialize(const std::vector<FEUUID>& PluginIDs);
	// False if Initialize was called with list of plugins.
	bool IsAutomaticLoadingEnabled() const;
	const std::vector<FEUUID>& GetRequestedPlugins() const;
	// Plugin was given to Initialize or is loaded because given plugin depends on it, always true with automatic loading.
	bool IsPluginRequested(const DiscoveredPlugin& Plugin) const;
	// Plugins that editor itself needs, they are loaded even without project.
	static std::vector<FEUUID> GetAlwaysLoadedPlugins();
	void Update(double DeltaTime);
	void Shutdown();

	void RenderMenu();
	void RenderWindows();
	void RenderInspectorSections(FEEntity* Entity);

	bool IsModuleLoaded(const PluginModule& Module) const;
	// Checks both engine and editor modules.
	bool IsModuleLoaded(const FEUUID& ModuleID) const;
	// Returns nullptr if editor module of that plugin is not loaded.
	FEEditorPluginCore* GetEditorModule(const FEUUID& PluginID) const;

	// Folder is created if it does not exist, project is built and opened in Visual Studio.
	bool CreatePluginProject(std::string& PluginName, const std::string& VSProjectDirectory, const FEEditorPluginProjectSettings& Settings);
private:
	SINGLETON_PRIVATE_PART(FEEditorPluginManager)

	struct ActiveEditorPlugin
	{
		FEPlugin Plugin;
		FEEditorPluginCore* Instance = nullptr;
		FEEditorAPIImplementation* API = nullptr;
	};
	std::vector<ActiveEditorPlugin> ActiveEditorPlugins;

	bool bAutomaticLoading = true;
	std::vector<FEUUID> RequestedPluginIDs;
	void LoadEditorModules();

	bool LoadEditorModule(const DiscoveredPlugin& Plugin, size_t ModuleIndex);
	// "<Plugin name>/<Module name>", or ID if module is not found.
	std::string GetModuleDisplayName(const FEUUID& ModuleID) const;

	bool GenerateEditorModuleSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory, const FEEditorPluginProjectSettings& Settings);
	bool GenerateEditorPublicInterfaceFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory);
	bool AddEditorModuleToManifest(const std::string& PluginName, const std::string& VSProjectDirectory);
	bool AddEditorModuleToCMake(const std::string& VSProjectDirectory);
};

#define EDITOR_PLUGIN_MANAGER FEEditorPluginManager::GetInstance()
