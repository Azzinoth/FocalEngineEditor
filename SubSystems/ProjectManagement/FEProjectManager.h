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
	std::string CustomProjectFolder;
public:
	SINGLETON_PUBLIC_PART(FEProjectManager)
	SINGLETON_PRIVATE_PART(FEProjectManager)

	void InitializeResources();

	FEProject* GetCurrent();
	void SetCurrent(FEProject* Project);

	std::vector<FEProject*> GetList();

	void CreateNewProject(std::string ProjectName, std::string ProjectPath);
	void OpenProject(int ProjectIndex);
	// Folder that holds the project becomes projects folder if project is not in the list.
	bool OpenProject(const std::string& ProjectFolder);
	void LoadProjectList();
	void CloseCurrentProject();

	void DisplayProjectSelection();

	bool ContainProject(std::string Path);
	void SetProjectsFolder(std::string FolderPath);
	std::string GetProjectsFolder() const;
private:
	int GetProjectIndex(const std::string& ProjectFolder);
	void OpenChosenProject();
};

#define PROJECTS_FOLDER PROJECT_MANAGER.CustomProjectFolder.c_str()
#define PROJECT_MANAGER FEProjectManager::GetInstance()