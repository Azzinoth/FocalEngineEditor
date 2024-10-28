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
};

#define EDITOR_PROJECT_BUILD_SYSTEM FEProjectBuildSystem::GetInstance()
