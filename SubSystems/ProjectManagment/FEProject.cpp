#include "FEProject.h"
#include "../../FEEditor.h"

FEProject::FEProject(const std::string Name, const std::string ProjectFolder)
{
	ID = APPLICATION.GetUniqueHexID();
	this->Name = Name;
	this->ProjectFolder = ProjectFolder;

	std::ifstream ScreenshotFile((this->GetProjectFolder() + "Thumbnail.texture").c_str());

	if (!ScreenshotFile.good())
	{
		SceneScreenshot = RESOURCE_MANAGER.NoTexture;
}
	else
	{
		SceneScreenshot = RESOURCE_MANAGER.LoadFETextureUnmanaged((this->GetProjectFolder() + "Thumbnail.texture").c_str());
	}
	ScreenshotFile.close();
}

FEProject::~FEProject()
{
	if (SceneScreenshot != RESOURCE_MANAGER.NoTexture)
		delete SceneScreenshot;

	RESOURCE_MANAGER.Clear();
	VIRTUAL_FILE_SYSTEM.Clear();
	SCENE_MANAGER.Clear();
}

std::string FEProject::GetName()
{
	return Name;
}

void FEProject::SetName(const std::string NewValue)
{
	Name = NewValue;
}

std::string FEProject::GetProjectFolder()
{
	return ProjectFolder;
}

void FEProject::SaveResources(std::string DirectoryPath, bool bFullSave)
{
	Json::Value Root;
	std::ofstream ResourcesFile;

	Root["Version"] = PROJECT_RESOURCE_FILE_VERSION;
	Root["ProjectID"] = ID;

	if (!bFullSave)
	{
		// Saving all unsaved objects.
		for (size_t i = 0; i < UnSavedObjects.size(); i++)
		{
			switch (UnSavedObjects[i]->GetType())
			{
			case FE_MESH:
			{
				FEMesh* MeshToSave = RESOURCE_MANAGER.GetMesh(UnSavedObjects[i]->GetObjectID());
				if (MeshToSave != nullptr)
					RESOURCE_MANAGER.SaveFEMesh(MeshToSave, (DirectoryPath + MeshToSave->GetObjectID() + std::string(".model")).c_str());
				break;
			}

			case FE_TEXTURE:
			{
				FETexture* TextureToSave = RESOURCE_MANAGER.GetTexture(UnSavedObjects[i]->GetObjectID());
				if (TextureToSave != nullptr)
					RESOURCE_MANAGER.SaveFETexture(TextureToSave, (DirectoryPath + TextureToSave->GetObjectID() + std::string(".texture")).c_str());
				break;
			}
			}
		}
	}

	// Saving Meshes.
	std::vector<std::string> MeshList = RESOURCE_MANAGER.GetMeshIDList();
	Json::Value MeshData;
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		FEMesh* Mesh = RESOURCE_MANAGER.GetMesh(MeshList[i]);
		if (Mesh->GetTag() == ENGINE_RESOURCE_TAG ||
			Mesh->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		MeshData[Mesh->GetObjectID()]["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Mesh);
		MeshData[Mesh->GetObjectID()]["FileName"] = Mesh->GetObjectID() + ".model";

		if (bFullSave)
			RESOURCE_MANAGER.SaveFEMesh(Mesh, (DirectoryPath + Mesh->GetObjectID() + std::string(".model")).c_str());

		Mesh->SetDirtyFlag(false);
	}
	Root["Meshes"] = MeshData;

	// Saving Textures.
	std::vector<std::string> TexturesList = RESOURCE_MANAGER.GetTextureIDList();
	Json::Value TexturesData;
	for (size_t i = 0; i < TexturesList.size(); i++)
	{
		FETexture* Texture = RESOURCE_MANAGER.GetTexture(TexturesList[i]);
		if (Texture->GetTag() == ENGINE_RESOURCE_TAG ||
			Texture->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		TexturesData[Texture->GetObjectID()]["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Texture);
		TexturesData[Texture->GetObjectID()]["FileName"] = Texture->GetObjectID() + ".texture";

		if (bFullSave)
			RESOURCE_MANAGER.SaveFETexture(Texture, (DirectoryPath + Texture->GetObjectID() + std::string(".texture")).c_str());

		Texture->SetDirtyFlag(false);
	}
	Root["Textures"] = TexturesData;

	// Saving Materials.
	std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialIDList();
	Json::Value MaterialData;
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		FEMaterial* Material = RESOURCE_MANAGER.GetMaterial(MaterialList[i]);
		if (Material->GetTag() == ENGINE_RESOURCE_TAG ||
			Material->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		MaterialData[Material->GetObjectID()] = RESOURCE_MANAGER.SaveMaterialToJSON(Material);
		Material->SetDirtyFlag(false);
	}
	Root["Materials"] = MaterialData;

	// Saving GameModels.
	std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	Json::Value GameModelData;
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (GameModel->GetTag() == ENGINE_RESOURCE_TAG ||
			GameModel->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		GameModelData[GameModel->GetObjectID()] = RESOURCE_MANAGER.SaveGameModelToJSON(GameModel);
		GameModel->SetDirtyFlag(false);
	}
	Root["GameModels"] = GameModelData;

	// Saving Prefabs.
	std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabIDList();
	Json::Value PrefabData;
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(PrefabList[i]);
		if (Prefab->GetTag() == ENGINE_RESOURCE_TAG ||
			Prefab->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		PrefabData[Prefab->GetObjectID()] = RESOURCE_MANAGER.SavePrefabToJSON(Prefab);
	}
	Root["Prefabs"] = PrefabData;

	// Saving NativeScriptModules.
	Json::Value NativeScriptModulesData;
	std::vector<std::string> NativeScriptModulesList = RESOURCE_MANAGER.GetNativeScriptModuleIDList();
	for (size_t i = 0; i < NativeScriptModulesList.size(); i++)
	{
		FENativeScriptModule* NativeScriptModule = RESOURCE_MANAGER.GetNativeScriptModule(NativeScriptModulesList[i]);
		if (NativeScriptModule->GetTag() == ENGINE_RESOURCE_TAG ||
			NativeScriptModule->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		NativeScriptModulesData[NativeScriptModule->GetObjectID()]["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(NativeScriptModule);
		NativeScriptModulesData[NativeScriptModule->GetObjectID()]["FileName"] = NativeScriptModule->GetObjectID() + ".nativescriptmodule";

		RESOURCE_MANAGER.SaveFENativeScriptModule(NativeScriptModule, DirectoryPath + NativeScriptModule->GetObjectID() + ".nativescriptmodule");
	}
	Root["NativeScriptModules"] = NativeScriptModulesData;

	// Saving Scenes.
	std::vector<FEScene*> SceneList = SCENE_MANAGER.GetAllScenes();
	Json::Value SceneData;
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		bool bShouldProceed = true;

		std::vector<std::string> Tags = RESOURCE_MANAGER.GetTagsThatWillPreventDeletion();
		for (size_t j = 0; j < Tags.size(); j++)
		{
			if (SceneList[i]->GetTag() == Tags[j])
			{
				bShouldProceed = false;
				break;
			}
		}

		if (SceneList[i]->GetTag() == PREFAB_SCENE_DESCRIPTION_TAG)
		{
			for (size_t j = 0; j < PrefabList.size(); j++)
			{
				FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(PrefabList[j]);
				if (Prefab->GetScene() != SceneList[i])
					continue;

				for (size_t k = 0; k < Tags.size(); k++)
				{
					if (Tags[k] == Prefab->GetTag())
					{
						bShouldProceed = false;
						break;
					}
				}

				if (!bShouldProceed)
					break;
			}
		}

		if (!bShouldProceed)
			continue;

		SceneData[SceneList[i]->GetObjectID()] = SCENE_MANAGER.SaveSceneToJSON(SceneList[i], [](FEEntity* Entity) -> bool {
			if (Entity->GetTag() == EDITOR_RESOURCE_TAG)
				return false;

			return true;
		});
	}
	Root["Scenes"] = SceneData;

	// Saving into file.
	Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, Root);

	ResourcesFile.open(DirectoryPath + "Resources.txt");
	ResourcesFile << JsonFile;
	ResourcesFile.close();
}

// TODO: Make that process in memory, not involving creating temporary files.
FEAssetPackage* FEProject::SaveResourcesToAssetPackage()
{
	FEAssetPackage* Result = new FEAssetPackage();

	std::string TemporaryFolder = FILE_SYSTEM.GetCurrentWorkingPath() + "/Temporary_Project_Resources/";
	if (FILE_SYSTEM.DoesDirectoryExist(TemporaryFolder))
	{
		LOG.Add("FEProject::SaveResourcesToAssetPackage: Temporary_Project_Resources directory already exist, trying to delete it.", "FE_LOG_LOADING", FE_LOG_WARNING);

		if (!FILE_SYSTEM.DeleteDirectory(TemporaryFolder))
		{
			LOG.Add("FEProject::SaveResourcesToAssetPackage: Error deleting Temporary_Project_Resources directory", "FE_LOG_LOADING", FE_LOG_ERROR);
			return nullptr;
		}
	}

	if (!FILE_SYSTEM.CreateDirectory(TemporaryFolder))
	{
		LOG.Add("FEProject::SaveResourcesToAssetPackage: Error creating Temporary_Project_Resources directory", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	// Because of editor camera system, we need to adjust main camera for each scene before saving.
	SetProperMainCamerasInsteadOfEditorCameras();

	// Saving resources.
	SaveResources(TemporaryFolder, true);

	// After saving resources we restore editor cameras as main cameras.
	SetEditorCamerasInsteadOfProperMainCameras();
	
	// Saving all files to package.
	std::vector<std::string> Files = FILE_SYSTEM.GetFileNamesInDirectory(TemporaryFolder);
	for (size_t i = 0; i < Files.size(); i++)
	{
		if (Result->ImportAssetFromFile(TemporaryFolder + Files[i]).empty())
		{
			LOG.Add("FEProject::SaveResourcesToAssetPackage: Error importing asset " + Files[i] + " to package", "FE_LOG_LOADING", FE_LOG_WARNING);
			//return nullptr;
		}
	}

	if (!FILE_SYSTEM.DeleteDirectory(TemporaryFolder))
		LOG.Add("FEProject::SaveResourcesToAssetPackage: Error deleting Temporary_Project_Resources directory", "FE_LOG_LOADING", FE_LOG_WARNING);
		
	return Result;
}

void FEProject::LoadResourcesFromAssetPackage(FEAssetPackage* AssetPackage)
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

	// Because of editor camera system, we need to save proper main camera for each scene.
	SaveProperMainCameras();

	if (!FILE_SYSTEM.DeleteDirectory(TemporaryFolder))
		LOG.Add("FEProject::LoadResourcesFromAssetPackage: Error deleting Temporary_Project_Resources directory", "FE_LOG_LOADING", FE_LOG_WARNING);
}

void FEProject::SaveProject(bool bFullSave)
{
	Json::Value Root;
	std::ofstream ProjectFile;

	Root["Version"] = PROJECTS_FILE_VER;
	Root["ID"] = ID;
	Root["Name"] = Name;

	FEScene* SceneForScreenshot = EDITOR.GetFocusedScene();
	if (SceneForScreenshot != nullptr)
	{
		ENGINE.SaveScreenshot((GetProjectFolder() + "Thumbnail.texture").c_str(), SceneForScreenshot);
	}
	else
	{
		CreateDummyScreenshot();
	}

	// Save starting scene.
	FEScene* StartingScene = SCENE_MANAGER.GetStartingScene();
	if (StartingScene != nullptr)
		Root["StartingScene"] = StartingScene->GetObjectID();
	
	// Saving editor scenes information.
	std::vector<std::string> EditorSceneIDs = EDITOR.GetEditorOpenedScenesIDs();
	for (size_t i = 0; i < EditorSceneIDs.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetScene(EditorSceneIDs[i]);
		if (CurrentScene == nullptr)
			continue;

		// If scene is editor only it means it is prefab scene.
		if (CurrentScene->GetTag() == EDITOR_RESOURCE_TAG)
		{
			// So we need to find corresponding prefab.
			FEEditorSceneWindow* EditorWindow = EDITOR.GetEditorSceneWindow(CurrentScene->GetObjectID());
			if (EditorWindow == nullptr)
				continue;

			FEPrefab* Prefab = PREFAB_EDITOR_MANAGER.GetPrefabFromEditorWindow(EditorWindow);
			if (Prefab == nullptr)
				continue;

			Root["EditorScenes"]["Opened"][std::to_string(i)]["ID"] = Prefab->GetScene()->GetObjectID();
			Root["EditorScenes"]["Opened"][std::to_string(i)]["IsPrefabDescription"] = true;
		}
		else
		{
			Root["EditorScenes"]["Opened"][std::to_string(i)]["ID"] = CurrentScene->GetObjectID();
			Root["EditorScenes"]["Opened"][std::to_string(i)]["IsPrefabDescription"] = false;
		}
	}

	FEScene* FocusedScene = EDITOR.GetFocusedScene();
	std::string FocusedSceneID = "";
	if (FocusedScene != nullptr)
		FocusedSceneID = FocusedScene->GetObjectID();
	
	Root["EditorScenes"]["FocusedSceneID"] = FocusedSceneID;
	
	// Because of editor camera system, we need to adjust main camera for each scene before saving.
	SetProperMainCamerasInsteadOfEditorCameras();

	// Saving resources.
	SaveResources(ProjectFolder, bFullSave);

	// After saving resources we restore editor cameras as main cameras.
	SetEditorCamerasInsteadOfProperMainCameras();

	// All of editor cameras would not be saved because of editor only tag.
	// But we need to save their state.
	Json::Value EditorCamerasData;
	int Index = 0;
	auto EditorCameraIterator = SceneIDToEditorCameraID.begin();
	while (EditorCameraIterator != SceneIDToEditorCameraID.end())
	{
		FEScene* Scene = SCENE_MANAGER.GetScene(EditorCameraIterator->first);
		if (Scene == nullptr)
		{
			EditorCameraIterator++;
			continue;
		}

		FEEntity* CameraEntity = Scene->GetEntity(EditorCameraIterator->second);
		if (CameraEntity == nullptr)
		{
			EditorCameraIterator++;
			continue;
		}

		if (!CameraEntity->HasComponent<FECameraComponent>())
		{
			EditorCameraIterator++;
			continue;
		}

		EditorCamerasData[Index]["SceneID"] = EditorCameraIterator->first;
		EditorCamerasData[Index]["EntityData"] = CameraEntity->ToJson();

		Index++;
		EditorCameraIterator++;
	}
	Root["EditorCameras"] = EditorCamerasData;

	// Saving into file.
	Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, Root);

	ProjectFile.open(ProjectFolder + "Project.txt");
	ProjectFile << JsonFile;
	ProjectFile.close();

	for (size_t i = 0; i < FilesToDelete.size(); i++)
	{
		FILE_SYSTEM.DeleteFile(FilesToDelete[i].c_str());
	}

	VIRTUAL_FILE_SYSTEM.SaveState(ProjectFolder + "VFS.txt");
	bModified = false;
}

void FEProject::LoadResources(std::string DirectoryPath)
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

	// Read file version.
	float FileVersion = Root["Version"].asFloat();
	if (FileVersion != PROJECT_RESOURCE_FILE_VERSION)
	{
		LOG.Add("FEProject::LoadResources: File version is not matching!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

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

void FEProject::LoadProject()
{
	std::ifstream ProjectFile;
	ProjectFile.open(ProjectFolder + "Project.txt");

	std::string FileData((std::istreambuf_iterator<char>(ProjectFile)), std::istreambuf_iterator<char>());
	ProjectFile.close();

	Json::Value Root;
	JSONCPP_STRING Error;
	Json::CharReaderBuilder Builder;

	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	if (!Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Error))
		return;

	// Check file version.
	float FileVersion = Root["Version"].asFloat();
	if (FileVersion != PROJECTS_FILE_VER)
	{
		LOG.Add("FEProject::LoadProject: File version is not matching!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	ID = Root["ID"].asCString();
	Name = Root["Name"].asCString();

	LoadResources(ProjectFolder);

	// Because of editor camera system, we need to save proper main camera for each scene.
	SaveProperMainCameras();

	// After we loaded all resources we can load editor cameras.
	Json::Value EditorCamerasData = Root["EditorCameras"];
	for (size_t i = 0; i < EditorCamerasData.size(); i++)
	{
		std::string SceneID = EditorCamerasData[static_cast<int>(i)]["SceneID"].asCString();
		FEScene* Scene = SCENE_MANAGER.GetScene(SceneID);
		if (Scene == nullptr)
			continue;

		FEEntity* EditorCameraEntity = Scene->CreateEntityFromJson(EditorCamerasData[static_cast<int>(i)]["EntityData"]);
		SceneIDToEditorCameraID[SceneID] = EditorCameraEntity->GetObjectID();
		CAMERA_SYSTEM.SetMainCamera(EditorCameraEntity);
	}

	// Set starting scene.
	if (Root.isMember("StartingScene"))
	{
		std::string StartingSceneID = Root["StartingScene"].asCString();
		SCENE_MANAGER.SetStartingScene(StartingSceneID);
	}

	// Now we can restore editor scenes.
	if (Root.isMember("EditorScenes"))
	{
		Json::Value OpenedScenes = Root["EditorScenes"]["Opened"];
		for (size_t i = 0; i < OpenedScenes.size(); i++)
		{
			FEScene* Scene = SCENE_MANAGER.GetScene(OpenedScenes[std::to_string(i)]["ID"].asCString());
			if (Scene == nullptr)
				continue;

			bool bIsPrefabDescription = OpenedScenes[std::to_string(i)]["IsPrefabDescription"].asBool();
			if (bIsPrefabDescription)
			{
				FEPrefab* PrefabThatHaveThatScene = nullptr;
				std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabIDList();
				for (size_t j = 0; j < PrefabList.size(); j++)
				{
					FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(PrefabList[j]);
					if (Prefab->GetScene() == Scene)
					{
						PrefabThatHaveThatScene = Prefab;
						break;
					}
				}

				if (PrefabThatHaveThatScene != nullptr)
					PREFAB_EDITOR_MANAGER.PrepareEditWinow(PrefabThatHaveThatScene);
			}
			else
			{
				Scene->SetFlag(FESceneFlag::Active, true);
				Scene->SetFlag(FESceneFlag::Renderable, true);
				Scene->SetFlag(FESceneFlag::EditorMode, true);
				EDITOR.AddEditorScene(Scene);
			}
		}

		std::string FocusedSceneID = Root["EditorScenes"]["FocusedSceneID"].asCString();
		FEScene* FocusedScene = SCENE_MANAGER.GetScene(FocusedSceneID);
		if (FocusedScene != nullptr)
			EDITOR.SetFocusedScene(FocusedScene);
	}

	if (!LoadVFSData(ProjectFolder + "VFS.txt"))
		LOG.Add("Can't find VIRTUAL_FILE_SYSTEM file in project folder.", "FE_LOG_LOADING", FE_LOG_WARNING);
	
	AddMissingVFSData();
}

bool FEProject::LoadVFSData(std::string FilePath)
{
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add("FEProject::LoadVFSData: File " + FilePath + " does not exist!", "FE_LOG_LOADING", FE_LOG_WARNING);
		return false;
	}

	VIRTUAL_FILE_SYSTEM.LoadState(FilePath);
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, "/Shaders");

	auto Files = VIRTUAL_FILE_SYSTEM.GetDirectoryContent("/Shaders");
	for (size_t i = 0; i < Files.size(); i++)
	{
		VIRTUAL_FILE_SYSTEM.DeleteFile(Files[i], "/Shaders");
	}

	std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetShaderIDList();
	for (size_t i = 0; i < ShaderList.size(); i++)
	{
		if (OBJECT_MANAGER.GetFEObject(ShaderList[i]) == nullptr)
			continue;
		VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(ShaderList[i]), "/Shaders");
	}

	std::vector<std::string> StandardShaderList = RESOURCE_MANAGER.GetEnginePrivateShaderIDList();
	for (size_t i = 0; i < StandardShaderList.size(); i++)
	{
		if (OBJECT_MANAGER.GetFEObject(StandardShaderList[i]) == nullptr)
			continue;
		VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(StandardShaderList[i]), "/Shaders");
	}

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, "/Shaders");

	return true;
}

void FEProject::AddMissingVFSData()
{
	std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetShaderIDList();
	for (size_t i = 0; i < ShaderList.size(); i++)
	{
		if (VIRTUAL_FILE_SYSTEM.DoesFileExistAnywhere(OBJECT_MANAGER.GetFEObject(ShaderList[i])))
			continue;

		VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(ShaderList[i]), "/Shaders");
	}

	std::vector<std::string> StandardShaderList = RESOURCE_MANAGER.GetEnginePrivateShaderIDList();
	for (size_t i = 0; i < StandardShaderList.size(); i++)
	{
		if (VIRTUAL_FILE_SYSTEM.DoesFileExistAnywhere(OBJECT_MANAGER.GetFEObject(StandardShaderList[i])))
			continue;

		VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(StandardShaderList[i]), "/Shaders");
		VIRTUAL_FILE_SYSTEM.SetFileReadOnly(true, OBJECT_MANAGER.GetFEObject(StandardShaderList[i]), "/Shaders");
	}

	std::vector<std::string> OtherResourceList = RESOURCE_MANAGER.GetMeshIDList();
	std::vector<std::string> TextureList = RESOURCE_MANAGER.GetTextureIDList();
	OtherResourceList.insert(OtherResourceList.end(), TextureList.begin(), TextureList.end());
	std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialIDList();
	OtherResourceList.insert(OtherResourceList.end(), MaterialList.begin(), MaterialList.end());
	std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	OtherResourceList.insert(OtherResourceList.end(), GameModelList.begin(), GameModelList.end());
	std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabIDList();
	OtherResourceList.insert(OtherResourceList.end(), PrefabList.begin(), PrefabList.end());
	std::vector<std::string> NativeScriptModuleList = RESOURCE_MANAGER.GetNativeScriptModuleIDList();
	OtherResourceList.insert(OtherResourceList.end(), NativeScriptModuleList.begin(), NativeScriptModuleList.end());
	std::vector<std::string> SceneList = SCENE_MANAGER.GetSceneIDList();
	// Filter prefab scenes.
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* Scene = SCENE_MANAGER.GetScene(SceneList[i]);
		if (Scene->GetTag() == PREFAB_SCENE_DESCRIPTION_TAG)
			continue;

		OtherResourceList.push_back(SceneList[i]);
	}

	for (size_t i = 0; i < OtherResourceList.size(); i++)
	{
		if (VIRTUAL_FILE_SYSTEM.DoesFileExistAnywhere(OBJECT_MANAGER.GetFEObject(OtherResourceList[i])))
			continue;

		VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(OtherResourceList[i]), "/");

		if (OBJECT_MANAGER.GetFEObject(OtherResourceList[i])->GetTag() == ENGINE_RESOURCE_TAG ||
			OBJECT_MANAGER.GetFEObject(OtherResourceList[i])->GetTag() == EDITOR_RESOURCE_TAG)
		{
			VIRTUAL_FILE_SYSTEM.SetFileReadOnly(true, OBJECT_MANAGER.GetFEObject(OtherResourceList[i]), "/");
		}
	}
}

void FEProject::CreateDummyScreenshot()
{
	const size_t Width = ENGINE.GetDefaultViewport()->GetWidth();
	const size_t Height = ENGINE.GetDefaultViewport()->GetHeight();

	unsigned char* Pixels = new unsigned char[4 * Width * Height];
	for (size_t j = 0; j < Height; j++)
	{
		for (size_t i = 0; i < 4 * Width; i += 4)
		{
			Pixels[i + (j * 4 * Width)] = 0;
			Pixels[i + 1 + (j * 4 * Width)] = static_cast<char>(162);
			Pixels[i + 2 + (j * 4 * Width)] = static_cast<char>(232);
			Pixels[i + 3 + (j * 4 * Width)] = static_cast<char>(255);
		}
	}

	FETexture* TempTexture = RESOURCE_MANAGER.RawDataToFETexture(Pixels, static_cast<int>(Width), static_cast<int>(Height));
	RESOURCE_MANAGER.SaveFETexture(TempTexture, (GetProjectFolder() + "/Thumbnail.texture").c_str());
	RESOURCE_MANAGER.DeleteFETexture(TempTexture);
	delete[] Pixels;
}

void FEProject::AddFileToDeleteList(const std::string FileName)
{
	FilesToDelete.push_back(FileName);
}

bool FEProject::IsModified()
{
	return bModified;
}

void FEProject::SetModified(const bool NewValue)
{
	bModified = NewValue;
}

void FEProject::AddUnSavedObject(FEObject* Object)
{
	UnSavedObjects.push_back(Object);
}

void FEProject::SetProjectFolder(const std::string NewValue)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(NewValue.c_str()))
	{
		LOG.Add("FEProject::SetProjectFolder: Directory " + NewValue + " does not exist!", "FE_LOG_LOADING", FE_LOG_WARNING);
		return;
	}

	ProjectFolder = NewValue;
}

void FEProject::SaveProjectTo(const std::string NewPath)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(NewPath.c_str()))
	{
		LOG.Add("FEProject::SaveProjectTo: Directory " + NewPath + " does not exist!", "FE_LOG_SAVING", FE_LOG_WARNING);
		return;
	}

	SetProjectFolder(NewPath);
	SaveProject(true);
}

void FEProject::InjectEditorCamera(FEScene* Scene)
{
	if (Scene == nullptr)
	{
		LOG.Add("FEProject::InjectEditorCamera: Scene is nullptr!", "FE_LOG_LOADING", FE_LOG_WARNING);
		return;
	}

	if (SceneIDToEditorCameraID.find(Scene->GetObjectID()) != SceneIDToEditorCameraID.end())
	{
		FEEntity* CameraEntity = Scene->GetEntity(SceneIDToEditorCameraID[Scene->GetObjectID()]);
		if (CameraEntity != nullptr)
		{
			LOG.Add("FEProject::InjectEditorCamera: Editor camera already exists in scene " + Scene->GetName(), "FE_LOG_LOADING", FE_LOG_WARNING);
			return;
		}
		else
		{
			SceneIDToEditorCameraID.erase(Scene->GetObjectID());
		}
	}

	FEEntity* CameraEntity = nullptr;
	std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Free camera prefab");
	if (CameraPrefab.size() == 0)
	{
		LOG.Add("FEProject::InjectEditorCamera: Camera prefab not found! Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

		CameraEntity = Scene->CreateEntity("Camera");
		CameraEntity->AddComponent<FECameraComponent>();
	}
	else
	{
		FEPrefab* CameraPrefabToUse = CameraPrefab[0];
		std::vector<FEEntity*> AddedEntities = SCENE_MANAGER.InstantiatePrefab(CameraPrefabToUse, Scene, true);
		if (AddedEntities.empty())
		{
			LOG.Add("FEProject::InjectEditorCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
		else
		{
			CameraEntity = AddedEntities[0];
		}

		if (CameraEntity == nullptr)
		{
			LOG.Add("FEProject::InjectEditorCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
	}

	RESOURCE_MANAGER.SetTag(CameraEntity, EDITOR_RESOURCE_TAG);
	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.TryToSetViewportSize(128, 128);
	CameraComponent.SetDistanceFogEnabled(false);
	CAMERA_SYSTEM.SetMainCamera(CameraEntity);
	FETransformComponent& CameraTransform = CameraEntity->GetComponent<FETransformComponent>();
	CameraTransform.SetPosition(glm::vec3(-4.2269f, 15.7178f, 19.6429f));

	SceneIDToEditorCameraID[Scene->GetObjectID()] = CameraEntity->GetObjectID();
}

std::string FEProject::GetEditorCameraIDBySceneID(std::string SceneID)
{
	if (SceneIDToEditorCameraID.find(SceneID) == SceneIDToEditorCameraID.end())
	{
		LOG.Add("FEProject::GetEditorCameraIDBySceneID: Editor camera not found for scene " + SceneID, "FE_LOG_LOADING", FE_LOG_WARNING);
		return "";
	}

	return SceneIDToEditorCameraID[SceneID];
}

std::string FEProject::GetProperMainCameraIDBySceneID(std::string SceneID)
{
	if (SceneIDToProperMainCameraID.find(SceneID) == SceneIDToProperMainCameraID.end())
	{
		LOG.Add("FEProject::GetProperMainCameraIDBySceneID: Editor camera not found for scene " + SceneID, "FE_LOG_LOADING", FE_LOG_WARNING);
		return "";
	}

	return SceneIDToProperMainCameraID[SceneID];
}

bool FEProject::SetProperMainCameraIDBySceneID(std::string SceneID, std::string CameraID)
{
	FEScene* Scene = SCENE_MANAGER.GetScene(SceneID);
	if (Scene == nullptr)
	{
		LOG.Add("FEProject::SetProperMainCameraIDBySceneID: Scene " + SceneID + " not found!", "FE_LOG_LOADING", FE_LOG_WARNING);
		return false;
	}

	if (!CameraID.empty())
	{
		FEEntity* CameraEntity = Scene->GetEntity(CameraID);
		if (CameraEntity == nullptr)
		{
			LOG.Add("FEProject::SetProperMainCameraIDBySceneID: Camera " + CameraID + " not found in scene " + SceneID, "FE_LOG_LOADING", FE_LOG_WARNING);
			return false;
		}

		if (!CameraEntity->HasComponent<FECameraComponent>())
		{
			LOG.Add("FEProject::SetProperMainCameraIDBySceneID: Entity " + CameraID + " in scene " + SceneID + " does not have camera component!", "FE_LOG_LOADING", FE_LOG_WARNING);
			return false;
		}
	}

	SceneIDToProperMainCameraID[SceneID] = CameraID;
	return true;
}

void FEProject::SetProperMainCamerasInsteadOfEditorCameras()
{
	auto Iterator = SceneIDToProperMainCameraID.begin();
	while (Iterator != SceneIDToProperMainCameraID.end())
	{
		FEScene* Scene = SCENE_MANAGER.GetScene(Iterator->first);
		if (Scene != nullptr)
		{
			FEEntity* CurrentMainCameraEntity = CAMERA_SYSTEM.GetMainCamera(Scene);
			if (CurrentMainCameraEntity != nullptr && CurrentMainCameraEntity->GetObjectID() != SceneIDToProperMainCameraID[Scene->GetObjectID()])
			{
				CAMERA_SYSTEM.SetMainCamera(Scene->GetEntity(Iterator->second));
			}
		}
		Iterator++;
	}
}

void FEProject::SetEditorCamerasInsteadOfProperMainCameras()
{
	auto Iterator = SceneIDToEditorCameraID.begin();
	while (Iterator != SceneIDToEditorCameraID.end())
	{
		FEScene* Scene = SCENE_MANAGER.GetScene(Iterator->first);
		if (Scene != nullptr)
		{
			FEEntity* CurrentMainCameraEntity = CAMERA_SYSTEM.GetMainCamera(Scene);
			if (CurrentMainCameraEntity != nullptr && CurrentMainCameraEntity->GetObjectID() != SceneIDToEditorCameraID[Scene->GetObjectID()])
			{
				CAMERA_SYSTEM.SetMainCamera(Scene->GetEntity(Iterator->second));
			}
		}
		Iterator++;
	}
}

void FEProject::SaveProperMainCameras()
{
	std::vector<std::string> SceneList = SCENE_MANAGER.GetSceneIDList();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetScene(SceneList[i]);
		if (CurrentScene == nullptr)
			continue;

		FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(CurrentScene);
		if (MainCameraEntity != nullptr)
			SceneIDToProperMainCameraID[CurrentScene->GetObjectID()] = MainCameraEntity->GetObjectID();
	}
}

std::string FEProject::GetID()
{
	return ID;
}