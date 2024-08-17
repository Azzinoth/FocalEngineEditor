#pragma once

#include "../FEngine.h"
#include "FEEditorDragAndDropManager.h"
#include "FEDearImguiWrapper/FEDearImguiWrapper.h"
#include "FEEditorGizmoManager.h"
#include "FEEditorPreviewManager.h"
#include "FEEditorVirtualFileSystem.h"

class FEProject
{
	friend class FEProjectManager;
	friend class FEEditor;
public:
	FEProject(std::string Name, std::string ProjectFolder);
	~FEProject();

	std::string GetName();
	void SetName(std::string NewValue);

	std::string GetProjectFolder();
	void SaveProject(bool bFullSave = false);
	void SaveSceneTo(std::string NewPath);
	void LoadProject();

	FETexture* SceneScreenshot;
	void CreateDummyScreenshot();

	void AddFileToDeleteList(std::string FileName);

	bool IsModified();
	void SetModified(bool NewValue);

	void AddUnSavedObject(FEObject* Object);

	FEScene* GetScene();
private:
	FEScene* ProjectScene = nullptr;
	std::string Name;
	std::string ProjectFolder;
	bool bModified = false;
	std::vector<FEObject*> UnSavedObjects;

	std::vector<std::string> FilesToDelete;

	void SetProjectFolder(std::string NewValue);
};

#define PROJECTS_FILE_VER 0.04f

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
#define PROJECT_MANAGER FEProjectManager::getInstance()