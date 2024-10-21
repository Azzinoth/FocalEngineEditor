#pragma once

#include "../../FEEditorDragAndDropManager.h"
#include "../../FEEditorVirtualFileSystem.h"

class FEProject
{
	friend class FEProjectManager;
	friend class FEEditor;
	friend class FEProjectBuildSystem;
	friend class FEPrefabEditorManager;
public:
	FEProject(std::string Name, std::string ProjectFolder);
	~FEProject();

	std::string GetID();

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

	std::string GetEditorCameraIDBySceneID(std::string SceneID);

	std::string GetProperMainCameraIDBySceneID(std::string SceneID);
	bool SetProperMainCameraIDBySceneID(std::string SceneID, std::string CameraID);
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

	std::unordered_map<std::string, std::string> SceneIDToProperMainCameraID;

	// FIX ME! Move that to a RESOURCE_MANAGER
	void LoadResources(std::string DirectoryPath);
	void SaveResources(std::string DirectoryPath, bool bFullSave = false);

	// TO-DO: Make that process in memory, not involving creating temporary files.
	FEAssetPackage* SaveResourcesToAssetPackage();
	void LoadResourcesFromAssetPackage(FEAssetPackage* AssetPackage);
	// FIX ME! Move that to a RESOURCE_MANAGER

	bool LoadVFSData(std::string FilePath);
	void AddMissingVFSData();

	void SetProperMainCamerasInsteadOfEditorCameras();
	void SetEditorCamerasInsteadOfProperMainCameras();
	void SaveProperMainCameras();
};

#define PROJECTS_FILE_VER 0.05f
#define PROJECT_RESOURCE_FILE_VERSION 0.01f