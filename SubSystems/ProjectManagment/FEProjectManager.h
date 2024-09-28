#pragma once

#include "../../FEEditorDragAndDropManager.h"
#include "../../FEDearImguiWrapper/FEDearImguiWrapper.h"
#include "../../FEEditorGizmoManager.h"
#include "../../FEEditorPreviewManager.h"
#include "../../FEEditorVirtualFileSystem.h"

#include "FEProject.h"

class FEProjectManager
{
	std::vector<FEProject*> List;
	int IndexChosen = -1;
	FEProject* Current = nullptr;
	std::string CustomProjectFolder = "../FocalEngineProjects-master";

	void CreateNewProject(std::string ProjectName, std::string ProjectPath);
public:
	SINGLETON_PUBLIC_PART(FEProjectManager)
	SINGLETON_PRIVATE_PART(FEProjectManager)

	void InitializeResources();

	FEProject* GetCurrent();
	void SetCurrent(FEProject* Project);

	std::vector<FEProject*> GetList();

	void OpenProject(int ProjectIndex);
	void LoadProjectList();
	void CloseCurrentProject();

	void DisplayProjectSelection();

	bool ContainProject(std::string Path);
	void SetProjectsFolder(std::string FolderPath);
};

#define PROJECTS_FOLDER PROJECT_MANAGER.CustomProjectFolder.c_str()
#define PROJECT_MANAGER FEProjectManager::GetInstance()