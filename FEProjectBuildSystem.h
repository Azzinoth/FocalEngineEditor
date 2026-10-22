#pragma once

#include "FEEditor.h"
using namespace FocalEngine;

class FEProjectBuildSystem
{
public:
	SINGLETON_PUBLIC_PART(FEProjectBuildSystem)

	bool BuildExecutable(FEProject* ProjectToBuild);
	bool CopyVisualNodeSystemSubProjectFiles(const std::string& OutputPath);
	bool InitializeCMakeFileAndScriptFiles(FEProject* ProjectToBuild, const std::string& VSProjectDirectory);

	bool ConfigureAndBuildCMake(const std::string& VSProjectDirectory);
	bool CreateFinalExecutableDirectory(FEProject* ProjectToBuild);

	// Space in project name is not allowed
	std::string GetVSProjectName(FEProject* ProjectToBuild);

	//bool PackProjectResources(FEProject* ProjectToPack, const std::string& OutputPath);
private:
	SINGLETON_PRIVATE_PART(FEProjectBuildSystem)

	// Plugins that have static library of every engine module, only they are linked into the game.
	std::vector<DiscoveredPlugin> StaticPlugins;
	std::vector<DiscoveredPlugin> FindStaticPlugins();
};

#define EDITOR_PROJECT_BUILD_SYSTEM FEProjectBuildSystem::GetInstance()
