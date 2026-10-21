#pragma once

#include "../FocalEngine/FEngine.h"
#include "FEEditorAPI.h"
using namespace FocalEngine;

class FEEditorAPIImplementation;

class FEEditorPluginManager
{
public:
	SINGLETON_PUBLIC_PART(FEEditorPluginManager)

	void Initialize();
	void Update(double DeltaTime);
	void Shutdown();

	void RenderMenu();
	void RenderWindows();
	void RenderInspectorSections(FEEntity* Entity);

	bool IsModuleLoaded(const PluginModule& Module) const;

	bool CreateEditorPluginProject(std::string& PluginName, const std::string& VSProjectDirectory, bool bEditorOnly = false);
private:
	SINGLETON_PRIVATE_PART(FEEditorPluginManager)

	struct ActiveEditorPlugin
	{
		FEPlugin Plugin;
		FEEditorPluginCore* Instance = nullptr;
		FEEditorAPIImplementation* API = nullptr;
	};
	std::vector<ActiveEditorPlugin> ActiveEditorPlugins;

	bool GenerateEditorModuleSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory);
	bool AddEditorModuleToManifest(const std::string& PluginName, const std::string& VSProjectDirectory);
	bool AddEditorModuleToCMake(const std::string& VSProjectDirectory);
};

#define EDITOR_PLUGIN_MANAGER FEEditorPluginManager::GetInstance()