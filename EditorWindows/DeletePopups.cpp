#include "DeletePopups.h"
#include "../FEEditor.h"

DeleteTexturePopup::DeleteTexturePopup()
{
	PopupCaption = "Delete texture";
	ObjToWorkWith = nullptr;
}

void DeleteTexturePopup::Show(FETexture* TextureToDelete)
{
	bShouldOpen = true;
	ObjToWorkWith = TextureToDelete;
}

void DeleteTexturePopup::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::Close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this texture is used in some materials
		// to-do: should be done through counter, not by searching each time.
		const std::vector<FEMaterial*> MaterialsThatUseTexture = MaterialsThatUsesTexture(ObjToWorkWith);

		ImGui::Text(("Do you want to delete \"" + ObjToWorkWith->GetName() + "\" texture ?").c_str());
		if (!MaterialsThatUseTexture.empty())
			ImGui::Text(("It is used in " + std::to_string(MaterialsThatUseTexture.size()) + " materials !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			DeleteTexture(ObjToWorkWith);
			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

std::vector<FEMaterial*> DeleteTexturePopup::MaterialsThatUsesTexture(const FETexture* Texture)
{
	std::vector<FEMaterial*> Result;
	const std::vector<std::string> MateriasList = RESOURCE_MANAGER.GetMaterialIDList();

	for (size_t i = 0; i < MateriasList.size(); i++)
	{
		FEMaterial* CurrentMaterial = RESOURCE_MANAGER.GetMaterial(MateriasList[i]);

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
		const std::vector<std::string> TerrainList = EDITOR.GetFocusedScene()->GetEntityIDListWithComponent<FETerrainComponent>();
		for (size_t i = 0; i < TerrainList.size(); i++)
		{
			/*FEEntity* CurrentTerrain = SCENE.GetEntity(TerrainList[i]);
			FETerrainComponent& TerrainComponent = CurrentTerrain->GetComponent<FETerrainComponent>();
			if (TerrainComponent.HeightMap != nullptr && TerrainComponent.HeightMap->GetObjectID() == Texture->GetObjectID())
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
	// to-do: should be done through counter, not by searching each time.
	const std::vector<FEMaterial*> MaterialsThatUseTexture = MaterialsThatUsesTexture(Texture);
	std::vector<std::string> GameModelListToUpdate;

	std::string name = Texture->GetName();
	// re-create game model preview that was using material that uses this texture
	if (!MaterialsThatUseTexture.empty())
	{
		const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
		for (size_t i = 0; i < GameModelList.size(); i++)
		{
			const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
			for (size_t j = 0; j < MaterialsThatUseTexture.size(); j++)
			{
				if (CurrentGameModel->Material == MaterialsThatUseTexture[j])
					GameModelListToUpdate.push_back(CurrentGameModel->GetObjectID());
			}
		}
	}

	Texture->SetDirtyFlag(true);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
	PROJECT_MANAGER.GetCurrent()->AddFileToDeleteList(PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + Texture->GetObjectID() + ".texture");
	RESOURCE_MANAGER.DeleteFETexture(Texture);

	// re-create game model preview
	for (size_t i = 0; i < GameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.CreateGameModelPreview(GameModelListToUpdate[i]);
}

DeleteMeshPopup::DeleteMeshPopup()
{
	PopupCaption = "Delete mesh";
	ObjToWorkWith = nullptr;
}

void DeleteMeshPopup::Show(FEMesh* MeshToDelete)
{
	bShouldOpen = true;
	ObjToWorkWith = MeshToDelete;
}

void DeleteMeshPopup::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::Close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this mesh is used in some game model
		// to-do: should be done through counter, not by searching each time.
		const int Result = TimesMeshUsed(ObjToWorkWith);

		ImGui::Text(("Do you want to delete \"" + ObjToWorkWith->GetName() + "\" mesh ?").c_str());
		if (Result > 0)
			ImGui::Text(("It is used in " + std::to_string(Result) + " game models !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			DeleteMesh(ObjToWorkWith);
			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

int DeleteMeshPopup::TimesMeshUsed(const FEMesh* Mesh)
{
	int Result = 0;
	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();

	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Mesh == Mesh)
			Result++;
	}

	return Result;
}

void DeleteMeshPopup::DeleteMesh(FEMesh* Mesh)
{
	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(Mesh);

	const std::string name = Mesh->GetName();

	// re-create game model preview
	std::vector<std::string> GameModelListToUpdate;
	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Mesh == Mesh)
			GameModelListToUpdate.push_back(CurrentGameModel->GetObjectID());
	}

	Mesh->SetDirtyFlag(true);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
	PROJECT_MANAGER.GetCurrent()->AddFileToDeleteList(PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + Mesh->GetObjectID() + ".model");
	RESOURCE_MANAGER.DeleteFEMesh(Mesh);

	// re-create game model preview
	for (size_t i = 0; i < GameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.CreateGameModelPreview(GameModelListToUpdate[i]);

	delete PREVIEW_MANAGER.MeshPreviewTextures[name];
	PREVIEW_MANAGER.MeshPreviewTextures.erase(name);
}

DeleteGameModelPopup::DeleteGameModelPopup()
{
	PopupCaption = "Delete game model";
	ObjToWorkWith = nullptr;
}

void DeleteGameModelPopup::Show(FEGameModel* GameModel)
{
	bShouldOpen = true;
	ObjToWorkWith = GameModel;
}

void DeleteGameModelPopup::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::Close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this game model is used in some prefabs
		// to-do: should be done through counter, not by searching each time.
		const int Result = TimesGameModelUsed(ObjToWorkWith);

		ImGui::Text(("Do you want to delete \"" + ObjToWorkWith->GetName() + "\" game model ?").c_str());
		if (Result > 0)
			ImGui::Text(("It is used in " + std::to_string(Result) + " prefabs !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			DeleteGameModel(ObjToWorkWith);

			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

int DeleteGameModelPopup::TimesGameModelUsed(const FEGameModel* GameModel)
{
	int Result = 0;

	std::vector<std::string> SceneList = SCENE_MANAGER.GetSceneIDList();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetScene(SceneList[i]);
		std::vector<std::string> EntitiesList = CurrentScene->GetEntityIDListWithComponent<FEGameModelComponent>();
		for (size_t j = 0; j < EntitiesList.size(); j++)
		{
			FEEntity* CurrentEntity = CurrentScene->GetEntity(EntitiesList[j]);
			FEGameModelComponent& GameModelComponent = CurrentEntity->GetComponent<FEGameModelComponent>();
			if (GameModelComponent.GetGameModel() == GameModel)
				Result++;
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
	ObjToWorkWith = nullptr;
}

void DeletePrefabPopup::Show(FEPrefab* Prefab)
{
	bShouldOpen = true;
	ObjToWorkWith = Prefab;
}

void DeletePrefabPopup::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::Close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this prefab is used in some entities
		// to-do: should be done through counter, not by searching each time.
		const int Result = TimesPrefabUsed(ObjToWorkWith);

		ImGui::Text(("Do you want to delete \"" + ObjToWorkWith->GetName() + "\" prefab ?").c_str());
		if (Result > 0)
			ImGui::Text(("It is used in " + std::to_string(Result) + " entities !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			DeletePrefab(ObjToWorkWith);

			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

int DeletePrefabPopup::TimesPrefabUsed(const FEPrefab* Prefab)
{
	int Result = 0;
	std::vector<std::string> SceneList = SCENE_MANAGER.GetSceneIDList();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetScene(SceneList[i]);
		std::vector<std::string> EntitiesList = CurrentScene->GetEntityIDListWithComponent<FEPrefabInstanceComponent>();
		for (size_t j = 0; j < EntitiesList.size(); j++)
		{
			FEEntity* CurrentEntity = CurrentScene->GetEntity(EntitiesList[j]);
			FEPrefabInstanceComponent& PrefabInstanceComponent = CurrentEntity->GetComponent<FEPrefabInstanceComponent>();
			if (PrefabInstanceComponent.GetPrefab() == Prefab)
				Result++;
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
	ObjToWorkWith = nullptr;
}

void DeleteMaterialPopup::Show(FEMaterial* Material)
{
	bShouldOpen = true;
	ObjToWorkWith = Material;
}

void DeleteMaterialPopup::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::Close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this material is used in some game model
		// to-do: should be done through counter, not by searching each time.
		const int Result = TimesMaterialUsed(ObjToWorkWith);

		ImGui::Text(("Do you want to delete \"" + ObjToWorkWith->GetName() + "\" material ?").c_str());
		if (Result > 0)
			ImGui::Text(("It is used in " + std::to_string(Result) + " game models !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			DeleteMaterial(ObjToWorkWith);

			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

int DeleteMaterialPopup::TimesMaterialUsed(const FEMaterial* Material)
{
	int Result = 0;
	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();

	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Material == Material)
			Result++;
	}

	return Result;
}

void DeleteMaterialPopup::DeleteMaterial(FEMaterial* Material)
{
	VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(Material);

	const std::string name = Material->GetName();
	// re-create game model preview
	std::vector<std::string> GameModelListToUpdate;
	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Material == Material)
			GameModelListToUpdate.push_back(CurrentGameModel->GetObjectID());
	}

	Material->SetDirtyFlag(true);
	PROJECT_MANAGER.GetCurrent()->SetModified(true);
	RESOURCE_MANAGER.DeleteMaterial(Material);

	// re-create game model preview
	for (size_t i = 0; i < GameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.CreateGameModelPreview(GameModelListToUpdate[i]);

	delete PREVIEW_MANAGER.MaterialPreviewTextures[name];
	PREVIEW_MANAGER.MaterialPreviewTextures.erase(name);
}

DeleteDirectoryPopup::DeleteDirectoryPopup()
{
	PopupCaption = "Delete folder";
	ObjToWorkWith = "";
}

void DeleteDirectoryPopup::Show(const std::string DirectoryName)
{
	bShouldOpen = true;
	ObjToWorkWith = DirectoryName;

	PathToDirectory = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
	if (PathToDirectory.back() != '/')
		PathToDirectory += '/';

	PathToDirectory += DirectoryName;
}

void DeleteDirectoryPopup::RecursiveDeletion(const std::string Path)
{
	const auto content = VIRTUAL_FILE_SYSTEM.GetDirectoryContent(Path);
	for (size_t i = 0; i < content.size(); i++)
	{
		if (content[i]->GetType() == FE_NULL)
		{
			std::string TempPath = Path;
			if (TempPath.back() != '/')
				TempPath += '/';

			TempPath += content[i]->GetName();
			RecursiveDeletion(TempPath);
		}
		else if (content[i]->GetType() == FE_SHADER)
		{
			//RESOURCE_MANAGER.deleteShader(RESOURCE_MANAGER.getShader(content[i]->getObjectID()));
		}
		else if (content[i]->GetType() == FE_MESH)
		{
			DeleteMeshPopup::DeleteMesh(RESOURCE_MANAGER.GetMesh(content[i]->GetObjectID()));
		}
		else if (content[i]->GetType() == FE_TEXTURE)
		{
			DeleteTexturePopup::DeleteTexture(RESOURCE_MANAGER.GetTexture(content[i]->GetObjectID()));
		}
		else if (content[i]->GetType() == FE_MATERIAL)
		{
			DeleteMaterialPopup::DeleteMaterial(RESOURCE_MANAGER.GetMaterial(content[i]->GetObjectID()));
		}
		else if (content[i]->GetType() == FE_GAMEMODEL)
		{
			DeleteGameModelPopup::DeleteGameModel(RESOURCE_MANAGER.GetGameModel(content[i]->GetObjectID()));
		}
	}

	VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(Path);
}

void DeleteDirectoryPopup::Render()
{
	ImGuiModalPopup::Render();

	if (!PathToDirectory.empty() && VIRTUAL_FILE_SYSTEM.GetDirectoryContent(PathToDirectory).empty())
	{
		VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(PathToDirectory);
		ObjToWorkWith = "";
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text(("Do you want to delete \"" + ObjToWorkWith + "\" folder ?").c_str());
		ImGui::Text("It is not empty !");

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			RecursiveDeletion(PathToDirectory);
			//VIRTUAL_FILE_SYSTEM.deleteDirectory(pathToDirectory);
			PROJECT_MANAGER.GetCurrent()->SetModified(true);

			// I should do it in a way as windows FS is doing it.
			// You can't delete non-empty folder
			// Firstly you recursively have to delete all files and folders in it


			//PREVIEW_MANAGER. recreateAll ?
			//SCENE.prepareForGameModelDeletion(objToWorkWith);
			//PROJECT_MANAGER.getCurrent()->addFileToDeleteList(PROJECT_MANAGER.getCurrent()->getProjectFolder() + objToWorkWith->getObjectID() + ".model");

			ObjToWorkWith = "";
			ImGuiModalPopup::Close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}