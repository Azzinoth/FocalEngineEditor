#pragma once

#include "../../FEEditorDragAndDropManager.h"
#include "../../FEEditorVirtualFileSystem.h"

#include "FENativeScriptProject.h"

class FEProject
{
	friend class FEProjectManager;
	friend class FEEditor;
	friend class FEEditorScriptingSystem;
public:
	FEProject(std::string Name, std::string ProjectFolder);
	~FEProject();

	std::string GetName();
	void SetName(std::string NewValue);

	std::string GetProjectFolder();

	void LoadProject();
	void SaveProject(bool bFullSave = false);
	void SaveProjectTo(std::string NewPath);

	FETexture* SceneScreenshot;
	void CreateDummyScreenshot();

	void AddFileToDeleteList(std::string FileName);

	bool IsModified();
	void SetModified(bool NewValue);

	void AddUnSavedObject(FEObject* Object);
private:
	std::string ID;
	std::string Name;
	std::string ProjectFolder;

	bool bModified = false;
	std::vector<FEObject*> UnSavedObjects;

	std::vector<std::string> FilesToDelete;

	void SetProjectFolder(std::string NewValue);

	std::unordered_map<std::string, std::string> SceneIDToEditorCameraID;
	void InjectEditorCamera(FEScene* Scene);

	void LoadResources(std::string FilePath);
	void SaveResourcesTo(std::string FilePath, bool bFullSave = false);

	void LoadNativeScriptProjectData(Json::Value Root);

	FENativeScriptProject* NativeScriptProject = nullptr;

	bool LoadVFSData(std::string FilePath);
	void GenerateVFSData();
};

#define PROJECTS_FILE_VER 0.05f
#define PROJECT_RESOURCE_FILE_VERSION 0.01f