#include "DeletePopups.h"
#include "../FEEditor.h"

struct DeletePopupContent
{
	std::string Heading;
	FETexture* Preview = nullptr;
	std::string Name;
	std::vector<std::string> Facts;
	// Written above the names of objects that use the deleted one.
	std::string UsersCaption;
	std::vector<std::string> UserNames;
};

static const char* FileIsRemovedOnSaveText = "The file is removed when the project is saved.";

// Consequence is what happens to an object that uses the deleted one, without the subject.
static std::string GetUsersCaption(const size_t Count, const std::string& UserKind, const std::string& UserKindPlural, const std::string& Consequence)
{
	return "Used by " + GetNumberWithSeparators(Count) + " " + (Count == 1 ? UserKind + ", it " : UserKindPlural + ", each ") + Consequence + ":";
}

// Returns index of the pressed button: 0 is Cancel, 1 is Delete, -1 when none was pressed.
static int RenderDeletePopupContent(const DeletePopupContent& Content)
{
	RenderPopupHeading(Content.Heading);
	RenderPopupCard(Content.Preview, Content.Name, Content.Facts);
	if (Content.UserNames.empty())
	{
		RenderPopupText("Nothing in the project uses it.");
	}
	else
	{
		RenderPopupText(Content.UsersCaption, FEEditorStyle::Warning);
		RenderPopupNameList(Content.UserNames);
	}

	return RenderPopupButtons({ { "Cancel", FEPopupButtonKind::Normal }, { "Delete", FEPopupButtonKind::Danger } });
}

DeleteTexturePopup::DeleteTexturePopup()
{
	PopupCaption = "Delete texture";
	ObjectToWorkWith = nullptr;
}

void DeleteTexturePopup::Show(FETexture* TextureToDelete)
{
	bShouldOpen = true;
	ObjectToWorkWith = TextureToDelete;
}

void DeleteTexturePopup::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str(), FEPopupTone::Danger);
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		ImGuiModalPopup::Close();
	}
	else if (bOpened)
	{
		DeletePopupContent Content;
		Content.Heading = "Delete texture?";
		Content.Preview = PREVIEW_MANAGER.GetPreview(ObjectToWorkWith);
		Content.Name = ObjectToWorkWith->GetName();
		Content.Facts = { "Texture, " + std::to_string(ObjectToWorkWith->GetWidth()) + " x " + std::to_string(ObjectToWorkWith->GetHeight()), FileIsRemovedOnSaveText };

		// TO-DO: should be done through counter, not by searching each time.
		const std::vector<FEMaterial*> MaterialsThatUseTexture = MaterialsThatUsesTexture(ObjectToWorkWith);
		for (size_t i = 0; i < MaterialsThatUseTexture.size(); i++)
			Content.UserNames.push_back(MaterialsThatUseTexture[i]->GetName());
		Content.UsersCaption = GetUsersCaption(Content.UserNames.size(), "material", "materials", "gets the \"no texture\" placeholder");

		const int PressedButton = RenderDeletePopupContent(Content);
		if (PressedButton == 1)
		{
			DeleteTexture(ObjectToWorkWith);
			ObjectToWorkWith = nullptr;
		}

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}

std::vector<FEMaterial*> DeleteTexturePopup::MaterialsThatUsesTexture(const FETexture* Texture)
{
	std::vector<FEMaterial*> Result;
	const std::vector<FEUUID> MaterialsList = RESOURCE_MANAGER.GetMaterialIDList();

	for (size_t i = 0; i < MaterialsList.size(); i++)
	{
		FEMaterial* CurrentMaterial = RESOURCE_MANAGER.GetMaterial(MaterialsList[i]);

		for (size_t j = 0; j < CurrentMaterial->Textures.size(); j++)
		{
			if (CurrentMaterial->Textures[j] == Texture)
			{
				Result.push_back(CurrentMaterial);
				break;
			}
		}
	}

	if (EDITOR.GetFocusedScene() != nullptr)
	{
		const std::vector<FEUUID> TerrainList = EDITOR.GetFocusedScene()->GetEntityIDListWithComponent<FETerrainComponent>();
		for (size_t i = 0; i < TerrainList.size(); i++)
		{
			/*FEEntity* CurrentTerrain = SCENE.GetEntity(TerrainList[i]);
			FETerrainComponent& TerrainComponent = CurrentTerrain->GetComponent<FETerrainComponent>();
			if (TerrainComponent.HeightMap != nullptr && TerrainComponent.HeightMap->GetID() == Texture->GetID())
			{
				Result.push_back(nullptr);
			}*/
		}
	}

	return Result;
}

void DeleteTexturePopup::DeleteTexture(FETexture* Texture)
{
	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(Texture);

	// check if this texture is used in some materials
	// TO-DO: should be done through counter, not by searching each time.
	const std::vector<FEMaterial*> MaterialsThatUseTexture = MaterialsThatUsesTexture(Texture);
	std::vector<FEUUID> GameModelListToUpdate;

	std::string Name = Texture->GetName();
	// re-create game model preview that was using material that uses this texture
	if (!MaterialsThatUseTexture.empty())
	{
		const std::vector<FEUUID> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
		for (size_t i = 0; i < GameModelList.size(); i++)
		{
			const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
			for (size_t j = 0; j < MaterialsThatUseTexture.size(); j++)
			{
				if (CurrentGameModel->Material == MaterialsThatUseTexture[j])
					GameModelListToUpdate.push_back(CurrentGameModel->GetID());
			}
		}
	}

	Texture->SetDirtyFlag(true);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
	PROJECT_MANAGER.GetCurrent()->AddFileToDeleteList(PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + UNIQUE_ID.ToString(Texture->GetID()) + ".texture");
	RESOURCE_MANAGER.DeleteFETexture(Texture);

	// re-create game model preview
	for (size_t i = 0; i < GameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.CreateGameModelPreview(GameModelListToUpdate[i]);
}

DeleteMeshPopup::DeleteMeshPopup()
{
	PopupCaption = "Delete mesh";
	ObjectToWorkWith = nullptr;
}

void DeleteMeshPopup::Show(FEMesh* MeshToDelete)
{
	bShouldOpen = true;
	ObjectToWorkWith = MeshToDelete;
}

void DeleteMeshPopup::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str(), FEPopupTone::Danger);
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		ImGuiModalPopup::Close();
	}
	else if (bOpened)
	{
		DeletePopupContent Content;
		Content.Heading = "Delete mesh?";
		Content.Preview = PREVIEW_MANAGER.GetMeshPreview(ObjectToWorkWith->GetID());
		Content.Name = ObjectToWorkWith->GetName();
		Content.Facts = { "Mesh, " + GetNumberWithSeparators(ObjectToWorkWith->GetVertexCount() / 3) + " triangles", FileIsRemovedOnSaveText };

		// TO-DO: should be done through counter, not by searching each time.
		Content.UserNames = GetUserNames(ObjectToWorkWith);
		Content.UsersCaption = GetUsersCaption(Content.UserNames.size(), "game model", "game models", "gets the sphere mesh");

		const int PressedButton = RenderDeletePopupContent(Content);
		if (PressedButton == 1)
		{
			DeleteMesh(ObjectToWorkWith);
			ObjectToWorkWith = nullptr;
		}

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}

std::vector<std::string> DeleteMeshPopup::GetUserNames(const FEMesh* Mesh)
{
	std::vector<std::string> Result;
	const std::vector<FEUUID> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();

	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Mesh == Mesh)
			Result.push_back(CurrentGameModel->GetName());
	}

	return Result;
}

void DeletePointCloudPopup::DeletePointCloud(FEPointCloud* PointCloud)
{
	std::vector<FEUUID> SceneList = SCENE_MANAGER.GetSceneIDList();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetSceneByID(SceneList[i]);
		std::vector<FEUUID> EntitiesList = CurrentScene->GetEntityIDListWithComponent<FEPointCloudComponent>();
		for (size_t j = 0; j < EntitiesList.size(); j++)
		{
			FEEntity* CurrentEntity = CurrentScene->GetEntity(EntitiesList[j]);
			FEPointCloudComponent& PointCloudComponent = CurrentEntity->GetComponent<FEPointCloudComponent>();
			if (PointCloudComponent.GetPointCloud() == PointCloud)
			{
				CurrentEntity->RemoveComponent<FEPointCloudComponent>();
			}
		}
	}

	const FEUUID PointCloudID = PointCloud->GetID();

	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(PointCloud);

	PointCloud->SetDirtyFlag(true);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
	// FIX ME! Check if it is working
	PROJECT_MANAGER.GetCurrent()->AddFileToDeleteList(PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + UNIQUE_ID.ToString(PointCloud->GetID()) + ".pointcloud");
	RESOURCE_MANAGER.DeleteFEPointCloud(PointCloud);

	PREVIEW_MANAGER.RemovePreview(PointCloudID);
}

DeletePointCloudPopup::DeletePointCloudPopup()
{
	PopupCaption = "Delete point cloud";
	ObjectToWorkWith = nullptr;
}

void DeletePointCloudPopup::Show(FEPointCloud* PointCloud)
{
	bShouldOpen = true;
	ObjectToWorkWith = PointCloud;
}

void DeletePointCloudPopup::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str(), FEPopupTone::Danger);
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		ImGuiModalPopup::Close();
	}
	else if (bOpened)
	{
		DeletePopupContent Content;
		Content.Heading = "Delete point cloud?";
		Content.Preview = PREVIEW_MANAGER.GetPointCloudPreview(ObjectToWorkWith->GetID());
		Content.Name = ObjectToWorkWith->GetName();
		Content.Facts = { "Point cloud, " + GetNumberWithSeparators(ObjectToWorkWith->GetPointCount()) + " points", FileIsRemovedOnSaveText };

		// TO-DO: should be done through counter, not by searching each time.
		Content.UserNames = GetUserNames(ObjectToWorkWith);
		Content.UsersCaption = GetUsersCaption(Content.UserNames.size(), "entity", "entities", "loses its Point Cloud component");

		const int PressedButton = RenderDeletePopupContent(Content);
		if (PressedButton == 1)
		{
			DeletePointCloud(ObjectToWorkWith);
			ObjectToWorkWith = nullptr;
		}

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}

std::vector<std::string> DeletePointCloudPopup::GetUserNames(const FEPointCloud* PointCloud)
{
	std::vector<std::string> Result;

	std::vector<FEUUID> SceneList = SCENE_MANAGER.GetSceneIDList();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetSceneByID(SceneList[i]);
		std::vector<FEUUID> EntitiesList = CurrentScene->GetEntityIDListWithComponent<FEPointCloudComponent>();
		for (size_t j = 0; j < EntitiesList.size(); j++)
		{
			FEEntity* CurrentEntity = CurrentScene->GetEntity(EntitiesList[j]);
			FEPointCloudComponent& PointCloudComponent = CurrentEntity->GetComponent<FEPointCloudComponent>();
			if (PointCloudComponent.GetPointCloud() == PointCloud)
				Result.push_back(CurrentEntity->GetName());
		}
	}

	return Result;
}

void DeleteMeshPopup::DeleteMesh(FEMesh* Mesh)
{
	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(Mesh);

	const FEUUID MeshID = Mesh->GetID();

	// re-create game model preview
	std::vector<FEUUID> GameModelListToUpdate;
	const std::vector<FEUUID> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Mesh == Mesh)
			GameModelListToUpdate.push_back(CurrentGameModel->GetID());
	}

	Mesh->SetDirtyFlag(true);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
	PROJECT_MANAGER.GetCurrent()->AddFileToDeleteList(PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + UNIQUE_ID.ToString(Mesh->GetID()) + ".model");
	RESOURCE_MANAGER.DeleteFEMesh(Mesh);

	// re-create game model preview
	for (size_t i = 0; i < GameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.CreateGameModelPreview(GameModelListToUpdate[i]);

	PREVIEW_MANAGER.RemovePreview(MeshID);
}

DeleteGameModelPopup::DeleteGameModelPopup()
{
	PopupCaption = "Delete game model";
	ObjectToWorkWith = nullptr;
}

void DeleteGameModelPopup::Show(FEGameModel* GameModel)
{
	bShouldOpen = true;
	ObjectToWorkWith = GameModel;
}

void DeleteGameModelPopup::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str(), FEPopupTone::Danger);
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		ImGuiModalPopup::Close();
	}
	else if (bOpened)
	{
		DeletePopupContent Content;
		Content.Heading = "Delete game model?";
		Content.Preview = PREVIEW_MANAGER.GetGameModelPreview(ObjectToWorkWith->GetID());
		Content.Name = ObjectToWorkWith->GetName();
		Content.Facts = { "Game model" };

		// TO-DO: should be done through counter, not by searching each time.
		Content.UserNames = GetUserNames(ObjectToWorkWith);
		Content.UsersCaption = GetUsersCaption(Content.UserNames.size(), "entity", "entities", "gets the standard game model");

		const int PressedButton = RenderDeletePopupContent(Content);
		if (PressedButton == 1)
		{
			DeleteGameModel(ObjectToWorkWith);
			ObjectToWorkWith = nullptr;
		}

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}

std::vector<std::string> DeleteGameModelPopup::GetUserNames(const FEGameModel* GameModel)
{
	std::vector<std::string> Result;

	std::vector<FEUUID> SceneList = SCENE_MANAGER.GetSceneIDList();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetSceneByID(SceneList[i]);
		std::vector<FEUUID> EntitiesList = CurrentScene->GetEntityIDListWithComponent<FEGameModelComponent>();
		for (size_t j = 0; j < EntitiesList.size(); j++)
		{
			FEEntity* CurrentEntity = CurrentScene->GetEntity(EntitiesList[j]);
			FEGameModelComponent& GameModelComponent = CurrentEntity->GetComponent<FEGameModelComponent>();
			if (GameModelComponent.GetGameModel() == GameModel)
				Result.push_back(CurrentEntity->GetName());
		}
	}

	return Result;
}

void DeleteGameModelPopup::DeleteGameModel(FEGameModel* GameModel)
{
	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(GameModel);

	if (EDITOR.GetFocusedScene() != nullptr)
	{
		EDITOR.GetFocusedScene()->PrepareForGameModelDeletion(GameModel);
	}
	RESOURCE_MANAGER.DeleteGameModel(GameModel);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
}

DeletePrefabPopup::DeletePrefabPopup()
{
	PopupCaption = "Delete prefab";
	ObjectToWorkWith = nullptr;
}

void DeletePrefabPopup::Show(FEPrefab* Prefab)
{
	bShouldOpen = true;
	ObjectToWorkWith = Prefab;
}

void DeletePrefabPopup::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str(), FEPopupTone::Danger);
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		ImGuiModalPopup::Close();
	}
	else if (bOpened)
	{
		DeletePopupContent Content;
		Content.Heading = "Delete prefab?";
		Content.Preview = PREVIEW_MANAGER.GetPrefabPreview(ObjectToWorkWith->GetID());
		Content.Name = ObjectToWorkWith->GetName();
		Content.Facts = { "Prefab" };

		// TO-DO: should be done through counter, not by searching each time.
		Content.UserNames = GetUserNames(ObjectToWorkWith);
		Content.UsersCaption = GetUsersCaption(Content.UserNames.size(), "entity", "entities", "loses its Prefab Instance component");

		const int PressedButton = RenderDeletePopupContent(Content);
		if (PressedButton == 1)
		{
			DeletePrefab(ObjectToWorkWith);
			ObjectToWorkWith = nullptr;
		}

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}

std::vector<std::string> DeletePrefabPopup::GetUserNames(const FEPrefab* Prefab)
{
	std::vector<std::string> Result;
	std::vector<FEUUID> SceneList = SCENE_MANAGER.GetSceneIDList();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetSceneByID(SceneList[i]);
		std::vector<FEUUID> EntitiesList = CurrentScene->GetEntityIDListWithComponent<FEPrefabInstanceComponent>();
		for (size_t j = 0; j < EntitiesList.size(); j++)
		{
			FEEntity* CurrentEntity = CurrentScene->GetEntity(EntitiesList[j]);
			FEPrefabInstanceComponent& PrefabInstanceComponent = CurrentEntity->GetComponent<FEPrefabInstanceComponent>();
			if (PrefabInstanceComponent.GetPrefab() == Prefab)
				Result.push_back(CurrentEntity->GetName());
		}
	}

	return Result;
}

void DeletePrefabPopup::DeletePrefab(FEPrefab* Prefab)
{
	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(Prefab);

	if (EDITOR.GetFocusedScene() != nullptr)
	{
		EDITOR.GetFocusedScene()->PrepareForPrefabDeletion(Prefab);
	}
	RESOURCE_MANAGER.DeletePrefab(Prefab);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
}

DeleteMaterialPopup::DeleteMaterialPopup()
{
	PopupCaption = "Delete material";
	ObjectToWorkWith = nullptr;
}

void DeleteMaterialPopup::Show(FEMaterial* Material)
{
	bShouldOpen = true;
	ObjectToWorkWith = Material;
}

void DeleteMaterialPopup::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str(), FEPopupTone::Danger);
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		ImGuiModalPopup::Close();
	}
	else if (bOpened)
	{
		DeletePopupContent Content;
		Content.Heading = "Delete material?";
		Content.Preview = PREVIEW_MANAGER.GetMaterialPreview(ObjectToWorkWith->GetID());
		Content.Name = ObjectToWorkWith->GetName();
		Content.Facts = { "Material" };

		// TO-DO: should be done through counter, not by searching each time.
		Content.UserNames = GetUserNames(ObjectToWorkWith);
		Content.UsersCaption = GetUsersCaption(Content.UserNames.size(), "game model", "game models", "gets the solid color material");

		const int PressedButton = RenderDeletePopupContent(Content);
		if (PressedButton == 1)
		{
			DeleteMaterial(ObjectToWorkWith);
			ObjectToWorkWith = nullptr;
		}

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}

std::vector<std::string> DeleteMaterialPopup::GetUserNames(const FEMaterial* Material)
{
	std::vector<std::string> Result;
	const std::vector<FEUUID> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();

	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Material == Material)
			Result.push_back(CurrentGameModel->GetName());
	}

	return Result;
}

void DeleteMaterialPopup::DeleteMaterial(FEMaterial* Material)
{
	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(Material);

	const FEUUID MaterialID = Material->GetID();
	// re-create game model preview
	std::vector<FEUUID> GameModelListToUpdate;
	const std::vector<FEUUID> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Material == Material)
			GameModelListToUpdate.push_back(CurrentGameModel->GetID());
	}

	Material->SetDirtyFlag(true);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
	RESOURCE_MANAGER.DeleteMaterial(Material);

	// re-create game model preview
	for (size_t i = 0; i < GameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.CreateGameModelPreview(GameModelListToUpdate[i]);

	PREVIEW_MANAGER.RemovePreview(MaterialID);
}

DeleteDirectoryPopup::DeleteDirectoryPopup()
{
	PopupCaption = "Delete folder";
	ObjectToWorkWith = "";
}

void DeleteDirectoryPopup::Show(const std::string DirectoryName)
{
	bShouldOpen = true;
	ObjectToWorkWith = DirectoryName;

	PathToDirectory = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
	if (PathToDirectory.back() != '/')
		PathToDirectory += '/';

	PathToDirectory += DirectoryName;
}

void DeleteDirectoryPopup::RecursiveDeletion(const std::string Path)
{
	const auto DirectoryContent = VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(Path);
	for (size_t i = 0; i < DirectoryContent.size(); i++)
	{
		FEObject* CurrentObject = OBJECT_MANAGER.GetFEObject(DirectoryContent[i]);
		if (CurrentObject == nullptr)
			continue;

		if (CurrentObject->GetType() == FE_NULL)
		{
			std::string TemporaryPath = Path;
			if (TemporaryPath.back() != '/')
				TemporaryPath += '/';

			TemporaryPath += CurrentObject->GetName();
			RecursiveDeletion(TemporaryPath);
		}
		else if (CurrentObject->GetType() == FE_SHADER)
		{
			//RESOURCE_MANAGER.deleteShader(RESOURCE_MANAGER.getShader(content[i]->getObjectID()));
		}
		else if (CurrentObject->GetType() == FE_MESH)
		{
			DeleteMeshPopup::DeleteMesh(RESOURCE_MANAGER.GetMesh(CurrentObject->GetID()));
		}
		else if (CurrentObject->GetType() == FE_POINT_CLOUD)
		{
			DeletePointCloudPopup::DeletePointCloud(RESOURCE_MANAGER.GetPointCloud(CurrentObject->GetID()));
		}
		else if (CurrentObject->GetType() == FE_TEXTURE)
		{
			DeleteTexturePopup::DeleteTexture(RESOURCE_MANAGER.GetTexture(CurrentObject->GetID()));
		}
		else if (CurrentObject->GetType() == FE_MATERIAL)
		{
			DeleteMaterialPopup::DeleteMaterial(RESOURCE_MANAGER.GetMaterial(CurrentObject->GetID()));
		}
		else if (CurrentObject->GetType() == FE_GAMEMODEL)
		{
			DeleteGameModelPopup::DeleteGameModel(RESOURCE_MANAGER.GetGameModel(CurrentObject->GetID()));
		}
	}

	VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(Path);
}

void DeleteDirectoryPopup::Render()
{
	ImGuiModalPopup::Render();

	if (!PathToDirectory.empty() && VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(PathToDirectory).empty())
	{
		VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(PathToDirectory);
		ObjectToWorkWith = "";
		return;
	}

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str(), FEPopupTone::Danger);
	if (bOpened)
	{
		DeletePopupContent Content;
		Content.Heading = "Delete folder?";
		Content.Name = ObjectToWorkWith;

		const std::vector<FEUUID> DirectoryContent = VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(PathToDirectory);
		for (size_t i = 0; i < DirectoryContent.size(); i++)
		{
			const FEObject* CurrentObject = OBJECT_MANAGER.GetFEObject(DirectoryContent[i]);
			if (CurrentObject != nullptr)
				Content.UserNames.push_back(CurrentObject->GetName());
		}
		Content.Facts = { "Folder, " + GetNumberWithSeparators(DirectoryContent.size()) + (DirectoryContent.size() == 1 ? " item inside" : " items inside") };
		Content.UsersCaption = "Everything inside is deleted too:";

		const int PressedButton = RenderDeletePopupContent(Content);
		if (PressedButton == 1)
		{
			RecursiveDeletion(PathToDirectory);
			PROJECT_MANAGER.GetCurrent()->SetModified(true);
			ObjectToWorkWith = "";
		}

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);
}