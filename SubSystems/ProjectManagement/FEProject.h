#pragma once

#include "../../FEEditorDragAndDropManager.h"
#include "../../FEEditorVirtualFileSystem.h"

struct FEProjectPluginInfo
{
	FEUUID ID;
	std::string Name;
	std::string Version;
};

class FEProject
{
	friend class FEProjectManager;
	friend class FEEditor;
	friend class FEEditorSceneWindowManager;
	friend class FEProjectBuildSystem;
	friend class FEPrefabEditorManager;
	friend class FEEditorContentBrowserWindow;
	friend class VolumetricTextureViewWindow;
public:
	FEProject(std::string Name, std::string ProjectFolder);
	~FEProject();

	FEUUID GetID();

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

	FEUUID GetEditorCameraIDBySceneID(const FEUUID& SceneID);

	FEUUID GetProperMainCameraIDBySceneID(const FEUUID& SceneID);
	bool SetProperMainCameraIDBySceneID(const FEUUID& SceneID, const FEUUID& CameraID);

	// Plugins that project uses, but that are not installed.
	std::vector<FEProjectPluginInfo> GetMissingPlugins();
	void RemoveMissingPlugin(const FEUUID& PluginID);
	// True if plugin is enabled in the project.
	bool UsesPlugin(const FEUUID& PluginID) const;
	const std::vector<FEProjectPluginInfo>& GetEnabledPlugins() const;
	// Installed plugins that it depends on are enabled too, change is applied when project is opened next time.
	void EnablePlugin(const PluginManifest& Manifest);
	void DisablePlugin(const FEUUID& PluginID);
	// Enabled plugins that depend on given plugin, directly or through other plugins.
	std::vector<FEUUID> GetEnabledDependentPlugins(const FEUUID& PluginID) const;
	std::vector<FEUUID> GetEnabledPluginsWithDependencies() const;
	// Reads only list of plugins from Project.txt, it could be called before engine is initialized.
	static std::vector<FEUUID> ReadPluginIDs(const std::string& ProjectFolder);
private:
	// Plugins enabled in the project, editor loads them when project is opened.
	// Missing plugins stay in the list until they are removed explicitly.
	std::vector<FEProjectPluginInfo> Plugins;

	FEUUID ID;
	std::string Name;
	std::string ProjectFolder;

	bool bWasJustCreated = false;
	bool bModified = false;
	std::vector<FEObject*> UnSavedObjects;

	std::vector<std::string> FilesToDelete;

	void SetProjectFolder(std::string NewValue);

	std::unordered_map<FEUUID, FEUUID> SceneIDToEditorCameraID;
	void InjectEditorCamera(FEScene* Scene);

	std::unordered_map<FEUUID, FEUUID> SceneIDToProperMainCameraID;

	// FIXME: Consider moving this to RESOURCE_MANAGER
	void LoadResources(std::string DirectoryPath);
	void SaveResources(std::string DirectoryPath, bool bFullSave = false);
	// TODO: Implement in-memory processing instead of creating temporary files.
	// FIXME: Consider moving this to RESOURCE_MANAGER
	FEAssetPackage* SaveResourcesToAssetPackage();
	void LoadResourcesFromAssetPackage(FEAssetPackage* AssetPackage);

	bool LoadVFSData(std::string FilePath);
	void AddMissingVFSData();

	void SetProperMainCamerasInsteadOfEditorCameras();
	void SetEditorCamerasInsteadOfProperMainCameras();
	void SaveProperMainCameras();
};

#define PROJECTS_FILE_VER 0.05f
#define PROJECT_RESOURCE_FILE_VERSION 0.01f