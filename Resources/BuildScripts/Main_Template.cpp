#include "Main.h"
using namespace FocalEngine;

void LoadResources(std::string DirectoryPath)
{
	std::ifstream ResourcesFile;
	ResourcesFile.open(DirectoryPath + "Resources.txt");

	std::string FileData((std::istreambuf_iterator<char>(ResourcesFile)), std::istreambuf_iterator<char>());
	ResourcesFile.close();

	Json::Value Root;
	JSONCPP_STRING Error;
	Json::CharReaderBuilder Builder;

	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	if (!Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Error))
		return;

	// Loading meshes.
	std::vector<Json::String> MeshList = Root["Meshes"].getMemberNames();
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Meshes"][MeshList[i]]["FEObjectData"]);
		RESOURCE_MANAGER.LoadFEMesh((DirectoryPath + Root["Meshes"][MeshList[i]]["FileName"].asCString()).c_str(), LoadedObjectData.Name);
	}

	// Loading textures.
	std::vector<Json::String> TexturesList = Root["Textures"].getMemberNames();
	for (size_t i = 0; i < TexturesList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Textures"][TexturesList[i]]["FEObjectData"]);
		// Terrain textures should be loaded right away, not async.
		if (LoadedObjectData.Tag == TERRAIN_SYSTEM_RESOURCE_TAG)
		{
			RESOURCE_MANAGER.LoadFETexture((DirectoryPath + Root["Textures"][TexturesList[i]]["FileName"].asCString()).c_str());
		}
		else
		{
			RESOURCE_MANAGER.LoadFETextureAsync((DirectoryPath + Root["Textures"][TexturesList[i]]["FileName"].asCString()).c_str(), LoadedObjectData.Name, nullptr, LoadedObjectData.ID);
		}
	}

	// Loading materials.
	std::vector<Json::String> MaterialsList = Root["Materials"].getMemberNames();
	for (size_t i = 0; i < MaterialsList.size(); i++)
	{
		if (RESOURCE_MANAGER.LoadMaterialFromJSON(Root["Materials"][MaterialsList[i]]) == nullptr)
			LOG.Add("FEProject::LoadResources: Error loading material " + MaterialsList[i], "FE_LOG_LOADING", FE_LOG_ERROR);
	}

	// Loading game models.
	std::vector<Json::String> GameModelList = Root["GameModels"].getMemberNames();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		if (RESOURCE_MANAGER.LoadGameModelFromJSON(Root["GameModels"][GameModelList[i]]) == nullptr)
			LOG.Add("FEProject::LoadResources: Error loading game model " + GameModelList[i], "FE_LOG_LOADING", FE_LOG_ERROR);
	}

	// Loading NativeScriptModules.
	std::vector<Json::String> NativeScriptModulesList = Root["NativeScriptModules"].getMemberNames();
	for (size_t i = 0; i < NativeScriptModulesList.size(); i++)
	{
		FENativeScriptModule* LoadedNativeScriptModule = RESOURCE_MANAGER.LoadFENativeScriptModule((DirectoryPath + Root["NativeScriptModules"][NativeScriptModulesList[i]]["FileName"].asCString()).c_str());
		if (LoadedNativeScriptModule == nullptr)
			continue;

		NATIVE_SCRIPT_SYSTEM.ActivateNativeScriptModule(LoadedNativeScriptModule);
	}

	// Prefabs and Scenes are interconnected so we need to load them in two steps.
	// Currently Prefabs can not contain other Prefabs or Scenes, if it will change we need to change this code.
	// First load all prefab scenes.
	std::vector<Json::String> SceneList = Root["Scenes"].getMemberNames();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Scenes"][SceneList[i]]["FEObjectData"]);
		if (LoadedObjectData.Tag != PREFAB_SCENE_DESCRIPTION_TAG)
			continue;

		if (SCENE_MANAGER.LoadSceneFromJSON(Root["Scenes"][SceneList[i]], FESceneFlag::PrefabDescription) == nullptr)
			LOG.Add("FEProject::LoadResources: Error loading scene " + SceneList[i], "FE_LOG_LOADING", FE_LOG_ERROR);
	}

	// Then load Prefabs and initialize them with corresponding scenes.
	std::vector<Json::String> PrefabList = Root["Prefabs"].getMemberNames();
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		if (RESOURCE_MANAGER.LoadPrefabFromJSON(Root["Prefabs"][PrefabList[i]]) == nullptr)
			LOG.Add("FEProject::LoadResources: Error loading prefab " + PrefabList[i], "FE_LOG_LOADING", FE_LOG_ERROR);
	}

	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Scenes"][SceneList[i]]["FEObjectData"]);
		if (LoadedObjectData.Tag == PREFAB_SCENE_DESCRIPTION_TAG)
			continue;

		if (SCENE_MANAGER.LoadSceneFromJSON(Root["Scenes"][SceneList[i]], FESceneFlag::Active) == nullptr)
			LOG.Add("FEProject::LoadResources: Error loading scene " + SceneList[i], "FE_LOG_LOADING", FE_LOG_ERROR);
	}
}

void LoadResourcesFromAssetPackage(FEAssetPackage* AssetPackage)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FEProject::LoadResourcesFromAssetPackage: AssetPackage is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::string TemporaryFolder = FILE_SYSTEM.GetCurrentWorkingPath() + "/Temporary_Project_Resources/";
	if (FILE_SYSTEM.DoesDirectoryExist(TemporaryFolder))
	{
		LOG.Add("FEProject::LoadResourcesFromAssetPackage: Temporary_Project_Resources directory already exist, trying to delete it.", "FE_LOG_LOADING", FE_LOG_WARNING);

		if (!FILE_SYSTEM.DeleteDirectory(TemporaryFolder))
		{
			LOG.Add("FEProject::LoadResourcesFromAssetPackage: Error deleting Temporary_Project_Resources directory", "FE_LOG_LOADING", FE_LOG_ERROR);
			return;
		}
	}

	if (!FILE_SYSTEM.CreateDirectory(TemporaryFolder))
	{
		LOG.Add("FEProject::LoadResourcesFromAssetPackage: Error creating Temporary_Project_Resources directory", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	// Exporting all assets to files.
	std::vector<FEAssetPackageAssetInfo> AssetList = AssetPackage->GetEntryList();
	for (size_t i = 0; i < AssetList.size(); i++)
	{
		if (!AssetPackage->ExportAssetToFile(AssetList[i].ID, TemporaryFolder + AssetList[i].Name))
		{
			LOG.Add("FEProject::LoadResourcesFromAssetPackage: Error exporting asset " + AssetList[i].Name + " to file", "FE_LOG_LOADING", FE_LOG_WARNING);
			continue;
		}
	}

	LoadResources(TemporaryFolder);

	if (!FILE_SYSTEM.DeleteDirectory(TemporaryFolder))
		LOG.Add("FEProject::LoadResourcesFromAssetPackage: Error deleting Temporary_Project_Resources directory", "FE_LOG_LOADING", FE_LOG_WARNING);
}

FEEntity* CameraEntity = nullptr;
void mouseButtonCallback(int Button, int Action, int Mods)
{
	if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS)
	{
		if (CameraEntity != nullptr)
			CameraEntity->GetComponent<FECameraComponent>().SetActive(true);
	}
	else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
	{
		if (CameraEntity != nullptr)
			CameraEntity->GetComponent<FECameraComponent>().SetActive(false);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ENGINE.InitWindow();
	THREAD_POOL.SetConcurrentThreadCount(10);

	// Here we need to load all resources
	// And then load main scene?
	FEAssetPackage* AssetPackage = new FEAssetPackage();
	if (!AssetPackage->LoadFromFile("Resources.fepackage"))
	{
		LOG.Add("Error: loading asset package", "FE_STARTING_APPLICATION", FE_LOG_ERROR);
		return 1;
	}

	LoadResourcesFromAssetPackage(AssetPackage);

	FEScene* StartingScene = SCENE_MANAGER.GetStartingScene();
	if (StartingScene == nullptr)
	{
		LOG.Add("Error: no starting scene", "FE_STARTING_APPLICATION", FE_LOG_ERROR);
		return 1;
	}
	
	StartingScene->SetFlag(FESceneFlag::Active | FESceneFlag::Renderable | FESceneFlag::GameMode, true);

	CameraEntity = CAMERA_SYSTEM.GetMainCamera(StartingScene);
	if (CameraEntity != nullptr)
	{
		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		CAMERA_SYSTEM.SetCameraViewport(CameraEntity, ENGINE.GetDefaultViewport()->GetID());
	}
	else
	{
		LOG.Add("Error: no camera in starting scene", "FE_STARTING_APPLICATION", FE_LOG_ERROR);
	}

	INPUT.AddMouseButtonCallback(mouseButtonCallback);

	while (ENGINE.IsNotTerminated())
	{
		ENGINE.BeginFrame();
		ENGINE.Render();
		ENGINE.EndFrame();
	}

	return 0;
}