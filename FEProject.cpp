#include "FEProject.h"

FEProjectManager* FEProjectManager::Instance = nullptr;
FEProjectManager::FEProjectManager() {}
FEProjectManager::~FEProjectManager() {}

void FEProjectManager::InitializeResources()
{
	IndexChosen = -1;
	LoadProjectList();
}

FEProject* FEProjectManager::GetCurrent()
{
	return Current;
}

void FEProjectManager::SetCurrent(FEProject* Project)
{
	Current = Project;
}

std::vector<FEProject*> FEProjectManager::GetList()
{
	return List;
}

void FEProjectManager::LoadProjectList()
{
	if (!FILE_SYSTEM.CheckDirectory(PROJECTS_FOLDER))
		CustomProjectFolder = "";

	const std::vector<std::string> ProjectNameList = FILE_SYSTEM.GetDirectoryList(PROJECTS_FOLDER);

	for (size_t i = 0; i < ProjectNameList.size(); i++)
	{
		if (ContainProject(std::string(PROJECTS_FOLDER) + std::string("/") + ProjectNameList[i]))
			List.push_back(new FEProject(ProjectNameList[i].c_str(), std::string(PROJECTS_FOLDER) + std::string("/") + ProjectNameList[i].c_str() + "/"));
	}
}

void FEProjectManager::CloseCurrentProject()
{
	//closing all windows or popups.
	WindowsManager::getInstance().CloseAllWindows();
	WindowsManager::getInstance().CloseAllPopups();

	SELECTED.Clear();
	for (size_t i = 0; i < List.size(); i++)
	{
		delete List[i];
	}
	List.clear();
	PROJECT_MANAGER.SetCurrent(nullptr);
	PREVIEW_MANAGER.Clear();

	LoadProjectList();

	VIRTUAL_FILE_SYSTEM.SetCurrentPath("/");
}

void FEProjectManager::OpenProject(const int ProjectIndex)
{
	SCENE.Clear();

	PROJECT_MANAGER.SetCurrent(List[ProjectIndex]);
	PROJECT_MANAGER.GetCurrent()->LoadScene();
	IndexChosen = -1;

	GIZMO_MANAGER.ReInitializeEntities();

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateZGizmoEntity);

	PREVIEW_MANAGER.ReInitializeEntities();
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(PREVIEW_MANAGER.PreviewEntity);

	// all parts of Gizmos are standard resources except entities, so we need to register them again.
	// if it is first start and those entities are already registered these calls just returns false.
	/*auto it = EDITOR_INTERNAL_RESOURCES.InternalEditorObjects.begin();
	while (it != EDITOR_INTERNAL_RESOURCES.InternalEditorObjects.end())
	{
		if (it->second->GetType() == FE_ENTITY)
			SCENE.AddEntity(reinterpret_cast<FEEntity*>(it->second));
		
		it++;
	}*/

	// after loading project we should update our previews
	PREVIEW_MANAGER.UpdateAll();
	SELECTED.Clear();

	// Cleaning dirty flag of entities.
	//const std::vector<std::string> EntityList = SCENE.GetEntityList();
	//for (size_t i = 0; i < EntityList.size(); i++)
	//{
	//	FEEntity* Entity = SCENE.GetEntity(EntityList[i]);
	//	// But before that update AABB.
	//	Entity->GetAABB();
	//	Entity->Transform.SetDirtyFlag(false);
	//}
}

void FEProjectManager::DisplayProjectSelection()
{
	static float LowerPanelHight = 90.0f;
	const float MainWindowW = static_cast<float>(ENGINE.GetWindowWidth());
	const float MainWindowH = static_cast<float>(ENGINE.GetWindowHeight());

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(MainWindowW, MainWindowH - LowerPanelHight));
	ImGui::Begin("Project Browser", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	{
		ImGui::SetWindowFontScale(2.0f);
		ImGui::Text("CHOOSE WHAT PROJECT TO LOAD :");
		ImGui::SetWindowFontScale(1.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor(0.5f, 0.5f, 0.5f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::ImColor(0.95f, 0.90f, 0.0f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f)));

		const int ColumnCount = static_cast<int>(MainWindowW / (512.0f + 32.0f));
		ImGui::Columns(ColumnCount, "projectColumns", false);
		static bool bPushedStyle = false;
		for (size_t i = 0; i < List.size(); i++)
		{
			ImGui::PushID(static_cast<int>(i));
			bPushedStyle = false;
			if (IndexChosen == i)
			{
				bPushedStyle = true;
				ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor(0.1f, 1.0f, 0.1f, 1.0f)));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f)));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f)));
			}

			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexChosen != -1)
				{
					OpenProject(IndexChosen);
				}
			}

			if (ImGui::ImageButton((void*)static_cast<intptr_t>(List[i]->SceneScreenshot->GetTextureID()), ImVec2(512.0f, 288.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
			{
				IndexChosen = static_cast<int>(i);
			}

			ImGui::Text(List[i]->GetName().c_str());

			if (bPushedStyle)
			{
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}

			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, MainWindowH - LowerPanelHight));
	ImGui::SetNextWindowSize(ImVec2(MainWindowW, LowerPanelHight));
	ImGui::Begin("##create project", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor(0.0f, 162.0f / 255.0f, 232.0f / 255.0f, 1.0f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::ImColor(23.0f / 255.0f, 186.0f / 255.0f, 1.0f, 1.0f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::ImColor(0.0f, 125.0f / 255.0f, 179.0f / 255.0f, 1.0f)));

		if (ImGui::Button("Create New Project", ImVec2(200.0f, 64.0f)))
			ImGui::OpenPopup("New Project");

		ImGui::SameLine();
		if (ImGui::Button("Open Project", ImVec2(200.0f, 64.0f)) && IndexChosen != -1)
		{
			OpenProject(IndexChosen);
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Project", ImVec2(200.0f, 64.0f)) && IndexChosen != -1 && IndexChosen < static_cast<int>(List.size()))
		{
			std::string ProjectFolder = List[IndexChosen]->GetProjectFolder();
			ProjectFolder.erase(ProjectFolder.begin() + ProjectFolder.size() - 1);

			// Getting list of all files in project folder.
			const auto FileList = FILE_SYSTEM.GetFileList(List[IndexChosen]->GetProjectFolder().c_str());
			// We would delete all files in project folder, editor would not create folders there
			// so we are deleting only files.
			for (size_t i = 0; i < FileList.size(); i++)
			{
				FILE_SYSTEM.DeleteFile((List[IndexChosen]->GetProjectFolder() + FileList[i]).c_str());
			}
			// Then we can try to delete project folder, but if user created some folders in it we will fail.
			FILE_SYSTEM.DeleteDirectory(ProjectFolder.c_str());

			for (size_t i = 0; i < List.size(); i++)
			{
				delete List[i];
			}
			List.clear();
			PROJECT_MANAGER.SetCurrent(nullptr);

			LoadProjectList();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor(0.0f, 242.0f / 255.0f, 79.0f / 255.0f, 1.0f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::ImColor(34.0f / 255.0f, 1.0f, 106.0f / 255.0f, 1.0f)));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::ImColor(0.0f, 202.0f / 255.0f, 66.0f / 255.0f, 1.0f)));

		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x + 280.0f + 64.0f, ImGui::GetCursorPos().y));
		if (ImGui::Button("Choose projects directory", ImVec2(280.0f, 64.0f)))
		{
			std::string Path;
			FILE_SYSTEM.ShowFolderOpenDialog(Path);
			SetProjectsFolder(Path);
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetWindowPos(ImVec2(ENGINE.GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.GetWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
			ImGui::Text("Insert name of new project :");
			static char ProjectName[512] = "";

			ImGui::InputText("##newProjectName", ProjectName, IM_ARRAYSIZE(ProjectName));

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				bool bAlreadyCreated = false;
				for (size_t i = 0; i < List.size(); i++)
				{
					if (List[i]->GetName() == std::string(ProjectName))
					{
						bAlreadyCreated = true;
						break;
					}
				}

				if (strlen(ProjectName) != 0 && !bAlreadyCreated)
				{
					FILE_SYSTEM.CreateDirectory((std::string(PROJECTS_FOLDER) + std::string("/") + ProjectName + "/").c_str());
					List.push_back(new FEProject(ProjectName, std::string(PROJECTS_FOLDER) + std::string("/") + ProjectName + "/"));
					List.back()->CreateDummyScreenshot();
					//SCENE.addLight(FE_DIRECTIONAL_LIGHT, "sun");
					std::fstream File;
					File.open((std::string(PROJECTS_FOLDER) + std::string("/") + ProjectName + "/" + "scene.txt").c_str(), std::ios::out);
					File.write(BASIC_SCENE, strlen(BASIC_SCENE));
					File.close();

					//openProject(indexChosen);
					//current->saveScene();

					ImGui::CloseCurrentPopup();
					strcpy_s(ProjectName, "");
				}
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	ImGui::PopStyleVar();
	ImGui::End();
	ImGui::PopStyleVar();
}

bool FEProjectManager::ContainProject(const std::string Path)
{
	if (!FILE_SYSTEM.CheckDirectory(Path.c_str()))
		return false;

	if (!FILE_SYSTEM.CheckFile((Path + "/scene.txt").c_str()))
		return false;

	return true;
}

void FEProjectManager::SetProjectsFolder(const std::string FolderPath)
{
	if (FolderPath != "")
	{
		CustomProjectFolder = FolderPath;
		for (size_t i = 0; i < List.size(); i++)
		{
			delete List[i];
		}
		List.clear();
		LoadProjectList();
	}
}

FEProject::FEProject(const std::string Name, const std::string ProjectFolder)
{
	this->Name = Name;
	this->ProjectFolder = ProjectFolder;

	std::ifstream ScreenshotFile((this->GetProjectFolder() + "projectScreenShot.texture").c_str());

	if (!ScreenshotFile.good())
	{
		SceneScreenshot = RESOURCE_MANAGER.NoTexture;
	}
	else
	{
		SceneScreenshot = RESOURCE_MANAGER.LoadFETextureUnmanaged((this->GetProjectFolder() + "projectScreenShot.texture").c_str());
	}

	ScreenshotFile.close();
}

FEProject::~FEProject()
{
	if (SceneScreenshot != RESOURCE_MANAGER.NoTexture)
		delete SceneScreenshot;

	EDITOR_INTERNAL_RESOURCES.ClearListByType(FE_ENTITY);
	SCENE.Clear();
	RESOURCE_MANAGER.Clear();
	ENGINE.ResetCamera();
	VIRTUAL_FILE_SYSTEM.Clear();
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

void FEProject::WriteTransformToJson(Json::Value& Root, const FETransformComponent* Transform)
{
	Root["position"]["X"] = Transform->GetPosition()[0];
	Root["position"]["Y"] = Transform->GetPosition()[1];
	Root["position"]["Z"] = Transform->GetPosition()[2];
	Root["rotation"]["X"] = Transform->GetRotation()[0];
	Root["rotation"]["Y"] = Transform->GetRotation()[1];
	Root["rotation"]["Z"] = Transform->GetRotation()[2];
	Root["scale"]["uniformScaling"] = Transform->IsUniformScalingSet();
	Root["scale"]["X"] = Transform->GetScale()[0];
	Root["scale"]["Y"] = Transform->GetScale()[1];
	Root["scale"]["Z"] = Transform->GetScale()[2];
}

void FEProject::SaveScene(bool bFullSave)
{
	Json::Value Root;
	std::ofstream SceneFile;

	Root["version"] = PROJECTS_FILE_VER;

	// Saving scene hierarchy.
	Json::Value SceneHierarchy = SCENE.SceneGraph.ToJson();

	std::vector<std::string> KeysToDelete;
	Json::Value& SceneNodes = SceneHierarchy["Nodes"];

	std::vector<std::string> ListOfOmitedObjects = { "gizmo", "Gizmo", "editorPreviewEntity" };
	// Parsing scene hierarchy to delete all internal editor objects.
	for (auto it = SceneNodes.begin(); it != SceneNodes.end(); it++)
	{
		auto test = it.key().asString();
		auto Name = SceneNodes[it.key().asString()]["Name"].asString();

		for (size_t j = 0; j < ListOfOmitedObjects.size(); j++)
		{
			if (Name.find(ListOfOmitedObjects[j]) != std::string::npos)
			{
				KeysToDelete.push_back(it.key().asString());
				break;
			}
		}
	}

	for (size_t i = 0; i < KeysToDelete.size(); i++)
	{
		SceneNodes.removeMember(KeysToDelete[i]);
	}
	Root["sceneHierarchy"] = SceneHierarchy;

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
						RESOURCE_MANAGER.SaveFEMesh(MeshToSave, (GetProjectFolder() + MeshToSave->GetObjectID() + std::string(".model")).c_str());
					break;
				}

				case FE_TEXTURE:
				{
					FETexture* TextureToSave = RESOURCE_MANAGER.GetTexture(UnSavedObjects[i]->GetObjectID());
					if (TextureToSave != nullptr)
						RESOURCE_MANAGER.SaveFETexture(TextureToSave, (GetProjectFolder() + TextureToSave->GetObjectID() + std::string(".texture")).c_str());
					break;
				}
			}
		}
	}

	// Saving Meshes.
	std::vector<std::string> MeshList = RESOURCE_MANAGER.GetMeshList();
	Json::Value MeshData;
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		FEMesh* Mesh = RESOURCE_MANAGER.GetMesh(MeshList[i]);
		MeshData[Mesh->GetObjectID()]["ID"] = Mesh->GetObjectID();
		MeshData[Mesh->GetObjectID()]["name"] = Mesh->GetName();
		MeshData[Mesh->GetObjectID()]["fileName"] = Mesh->GetObjectID() + ".model";

		if (bFullSave)
			RESOURCE_MANAGER.SaveFEMesh(Mesh, (GetProjectFolder() + Mesh->GetObjectID() + std::string(".model")).c_str());

		Mesh->SetDirtyFlag(false);
	}
	Root["meshes"] = MeshData;

	// Saving Textures.
	std::vector<std::string> TexturesList = RESOURCE_MANAGER.GetTextureList();
	Json::Value TexturesData;
	for (size_t i = 0; i < TexturesList.size(); i++)
	{
		FETexture* Texture = RESOURCE_MANAGER.GetTexture(TexturesList[i]);
		if (!ShouldIncludeInSceneFile(Texture))
			continue;

		TexturesData[Texture->GetObjectID()]["ID"] = Texture->GetObjectID();
		TexturesData[Texture->GetObjectID()]["name"] = Texture->GetName();
		TexturesData[Texture->GetObjectID()]["fileName"] = Texture->GetObjectID() + ".texture";
		TexturesData[Texture->GetObjectID()]["type"] = Texture->GetInternalFormat();

		if (bFullSave)
			RESOURCE_MANAGER.SaveFETexture(Texture, (GetProjectFolder() + Texture->GetObjectID() + std::string(".texture")).c_str());

		Texture->SetDirtyFlag(false);
	}
	Root["textures"] = TexturesData;

	// Saving Materials.
	std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialList();
	Json::Value MaterialData;
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		FEMaterial* Material = RESOURCE_MANAGER.GetMaterial(MaterialList[i]);

		for (size_t j = 0; j < FE_MAX_TEXTURES_PER_MATERIAL; j++)
		{
			if (Material->Textures[j] != nullptr)
				MaterialData[Material->GetObjectID()]["textures"][std::to_string(j).c_str()] = Material->Textures[j]->GetObjectID();

			if (Material->TextureBindings[j] != -1)
				MaterialData[Material->GetObjectID()]["textureBindings"][std::to_string(j).c_str()] = Material->TextureBindings[j];

			if (Material->TextureChannels[j] != -1)
				MaterialData[Material->GetObjectID()]["textureChannels"][std::to_string(j).c_str()] = Material->TextureChannels[j];
		}

		MaterialData[Material->GetObjectID()]["ID"] = Material->GetObjectID();
		MaterialData[Material->GetObjectID()]["name"] = Material->GetName();
		MaterialData[Material->GetObjectID()]["metalness"] = Material->GetMetalness();
		MaterialData[Material->GetObjectID()]["roughness"] = Material->GetRoughness();
		MaterialData[Material->GetObjectID()]["normalMapIntensity"] = Material->GetNormalMapIntensity();
		MaterialData[Material->GetObjectID()]["ambientOcclusionIntensity"] = Material->GetAmbientOcclusionIntensity();
		MaterialData[Material->GetObjectID()]["ambientOcclusionMapIntensity"] = Material->GetAmbientOcclusionMapIntensity();
		MaterialData[Material->GetObjectID()]["roughnessMapIntensity"] = Material->GetRoughnessMapIntensity();
		MaterialData[Material->GetObjectID()]["metalnessMapIntensity"] = Material->GetMetalnessMapIntensity();
		MaterialData[Material->GetObjectID()]["tiling"] = Material->GetTiling();
		MaterialData[Material->GetObjectID()]["compackPacking"] = Material->IsCompackPacking();

		Material->SetDirtyFlag(false);
	}
	Root["materials"] = MaterialData;

	// Saving GameModels.
	std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelList();
	Json::Value GameModelData;
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		GameModelData[GameModel->GetObjectID()]["ID"] = GameModel->GetObjectID();
		GameModelData[GameModel->GetObjectID()]["name"] = GameModel->GetName();
		GameModelData[GameModel->GetObjectID()]["mesh"] = GameModel->Mesh->GetObjectID();
		GameModelData[GameModel->GetObjectID()]["material"] = GameModel->Material->GetObjectID();
		GameModelData[GameModel->GetObjectID()]["scaleFactor"] = GameModel->GetScaleFactor();

		GameModelData[GameModel->GetObjectID()]["LODs"]["haveLODlevels"] = GameModel->IsUsingLOD();
		if (GameModel->IsUsingLOD())
		{
			GameModelData[GameModel->GetObjectID()]["LODs"]["cullDistance"] = GameModel->GetCullDistance();
			GameModelData[GameModel->GetObjectID()]["LODs"]["billboardZeroRotaion"] = GameModel->GetBillboardZeroRotaion();
			GameModelData[GameModel->GetObjectID()]["LODs"]["LODCount"] = GameModel->GetLODCount();
			for (size_t j = 0; j < GameModel->GetLODCount(); j++)
			{
				GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["mesh"] = GameModel->GetLODMesh(j)->GetObjectID();
				GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["maxDrawDistance"] = GameModel->GetLODMaxDrawDistance(j);
				GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["isBillboard"] = GameModel->IsLODBillboard(j);
				if (GameModel->IsLODBillboard(j))
					GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["billboardMaterial"] = GameModel->GetBillboardMaterial()->GetObjectID();
			}
		}

		GameModel->SetDirtyFlag(false);
	}
	Root["gameModels"] = GameModelData;

	// Saving Prefabs.
	std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabList();
	Json::Value PrefabData;
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(PrefabList[i]);
		PrefabData[Prefab->GetObjectID()]["ID"] = Prefab->GetObjectID();
		PrefabData[Prefab->GetObjectID()]["name"] = Prefab->GetName();

		Json::Value ComponentsData;
		for (int j = 0; j < Prefab->ComponentsCount(); j++)
		{
			ComponentsData[j]["gameModel"]["ID"] = Prefab->GetComponent(j)->GameModel->GetObjectID();
			WriteTransformToJson(ComponentsData[j]["transformation"], &Prefab->GetComponent(j)->Transform);
		}

		PrefabData[Prefab->GetObjectID()]["components"] = ComponentsData;
	}
	Root["prefabs"] = PrefabData;

	// Saving Entities.
	//std::vector<std::string> EntityList = SCENE.GetEntityList();
	//Json::Value EntityData;
	//for (size_t i = 0; i < EntityList.size(); i++)
	//{
	//	FEEntity* Entity = SCENE.GetEntity(EntityList[i]);
	//	if (EDITOR_INTERNAL_RESOURCES.IsInInternalEditorList(Entity))
	//		continue;

	//	// Temporary solution, becuase of the lack of proper ECS system
	//	bool bOmit = false;
	//	for (size_t j = 0; j < ListOfOmitedObjects.size(); j++)
	//	{
	//		if (Entity->GetName().find(ListOfOmitedObjects[j]) != std::string::npos)
	//		{
	//			bOmit = true;
	//			break;
	//		}
	//	}
	//	if (bOmit)
	//		continue;

	//	EntityData[Entity->GetObjectID()]["ID"] = Entity->GetObjectID();
	//	EntityData[Entity->GetObjectID()]["type"] = FEObjectTypeToString(Entity->GetType());
	//	EntityData[Entity->GetObjectID()]["name"] = Entity->GetName();
	//	EntityData[Entity->GetObjectID()]["visible"] = Entity->IsVisible();
	//	// Temporary solution, because of the lack of proper ECS system
	//	if (Entity->Prefab != nullptr)
	//	{
	//		EntityData[Entity->GetObjectID()]["prefab"] = Entity->Prefab->GetObjectID();
	//	}
	//	else
	//	{
	//		EntityData[Entity->GetObjectID()]["prefab"] = "none";
	//	}
	//	//WriteTransformToJson(EntityData[Entity->GetObjectID()]["transformation"], &Entity->Transform);

	//	//if (Entity->GetType() == FE_ENTITY_INSTANCED)
	//	//{
	//	//	FEEntityInstanced* InstancedEntity = reinterpret_cast<FEEntityInstanced*>(Entity);
	//	//	EntityData[Entity->GetObjectID()]["modificationsToSpawn"] = InstancedEntity->GetSpawnModificationCount() == 0 ? false : true;
	//	//	if (InstancedEntity->GetSpawnModificationCount())
	//	//	{
	//	//		std::ofstream InfoFile;
	//	//		Json::Value EntityFileRoot;
	//	//		InfoFile.open(ProjectFolder + Entity->GetObjectID() + ".txt");

	//	//		Json::Value ModificationsData;
	//	//		auto ModificationList = InstancedEntity->GetSpawnModifications();
	//	//		for (int j = 0; j < ModificationList.size(); j++)
	//	//		{
	//	//			ModificationsData[j]["type"] = static_cast<int>(ModificationList[j].Type);
	//	//			ModificationsData[j]["index"] = ModificationList[j].Index;
	//	//			if (ModificationList[j].Type != FE_CHANGE_DELETED)
	//	//			{
	//	//				for (int k = 0; k < 4; k++)
	//	//				{
	//	//					for (int p = 0; p < 4; p++)
	//	//					{
	//	//						ModificationsData[j]["modification"][k][p] = ModificationList[j].Modification[k][p];
	//	//					}
	//	//				}
	//	//			}
	//	//		}
	//	//		EntityFileRoot["modifications"] = ModificationsData;

	//	//		InfoFile << EntityFileRoot;
	//	//		InfoFile.close();
	//	//	}

	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["seed"] = InstancedEntity->SpawnInfo.Seed;
	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["objectCount"] = InstancedEntity->SpawnInfo.ObjectCount;
	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["radius"] = InstancedEntity->SpawnInfo.Radius;
	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["minScale"] = InstancedEntity->SpawnInfo.GetMinScale();
	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["maxScale"] = InstancedEntity->SpawnInfo.GetMaxScale();
	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["rotationDeviation.x"] = InstancedEntity->SpawnInfo.RotationDeviation.x;
	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["rotationDeviation.y"] = InstancedEntity->SpawnInfo.RotationDeviation.y;
	//	//	EntityData[Entity->GetObjectID()]["spawnInfo"]["rotationDeviation.z"] = InstancedEntity->SpawnInfo.RotationDeviation.z;
	//	//	if (InstancedEntity->GetSnappedToTerrain() == nullptr)
	//	//	{
	//	//		EntityData[Entity->GetObjectID()]["snappedToTerrain"] = "none";
	//	//	}
	//	//	else
	//	//	{
	//	//		// FIX ME!
	//	//		//EntityData[Entity->GetObjectID()]["snappedToTerrain"] = InstancedEntity->GetSnappedToTerrain()->GetObjectID();
	//	//		EntityData[Entity->GetObjectID()]["terrainLayer"] = InstancedEntity->GetTerrainLayer();
	//	//		EntityData[Entity->GetObjectID()]["minimalLayerIntensity"] = InstancedEntity->GetMinimalLayerIntensity();
	//	//	}
	//	//}

	//	Entity->SetDirtyFlag(false);
	//}
	//Root["entities"] = EntityData;

	// Saving Terrains.
	std::vector<std::string> TerrainList = SCENE.GetEntityIDListWith<FETerrainComponent>();
	Json::Value TerrainData;
	for (size_t i = 0; i < TerrainList.size(); i++)
	{
		FEEntity* Terrain = SCENE.GetEntity(TerrainList[i]);
		FETerrainComponent& TerrainComponent = Terrain->GetComponent<FETerrainComponent>();

		TerrainData[Terrain->GetObjectID()]["ID"] = Terrain->GetObjectID();
		TerrainData[Terrain->GetObjectID()]["name"] = Terrain->GetName();

		TerrainData[Terrain->GetObjectID()]["heightMap"]["ID"] = TerrainComponent.HeightMap->GetObjectID();
		TerrainData[Terrain->GetObjectID()]["heightMap"]["name"] = TerrainComponent.HeightMap->GetName();
		TerrainData[Terrain->GetObjectID()]["heightMap"]["fileName"] = TerrainComponent.HeightMap->GetObjectID() + ".texture";
		RESOURCE_MANAGER.SaveFETexture(TerrainComponent.HeightMap, (GetProjectFolder() + TerrainComponent.HeightMap->GetObjectID() + std::string(".texture")).c_str());

		TerrainData[Terrain->GetObjectID()]["hightScale"] = TerrainComponent.GetHightScale();
		TerrainData[Terrain->GetObjectID()]["displacementScale"] = TerrainComponent.GetDisplacementScale();
		TerrainData[Terrain->GetObjectID()]["tileMult"]["X"] = TerrainComponent.GetTileMult().x;
		TerrainData[Terrain->GetObjectID()]["tileMult"]["Y"] = TerrainComponent.GetTileMult().y;
		TerrainData[Terrain->GetObjectID()]["LODlevel"] = TerrainComponent.GetLODLevel();
		TerrainData[Terrain->GetObjectID()]["chunkPerSide"] = TerrainComponent.GetChunkPerSide();

		// FIX ME!
		WriteTransformToJson(TerrainData[Terrain->GetObjectID()]["transformation"], &Terrain->GetComponent<FETransformComponent>());
		//WriteTransformToJson(TerrainData[Terrain->GetObjectID()]["transformation"], &Terrain->Transform);

		// Saving terrains Layers.
		for (int j = 0; j < TerrainComponent.LayerMaps.size(); j++)
		{
			if (TerrainComponent.LayerMaps[j] != nullptr)
			{
				TerrainData[Terrain->GetObjectID()]["layerMaps"][j]["ID"] = TerrainComponent.LayerMaps[j]->GetObjectID();
				TerrainData[Terrain->GetObjectID()]["layerMaps"][j]["name"] = TerrainComponent.LayerMaps[j]->GetName();
				TerrainData[Terrain->GetObjectID()]["layerMaps"][j]["fileName"] = TerrainComponent.LayerMaps[j]->GetObjectID() + ".texture";
				RESOURCE_MANAGER.SaveFETexture(TerrainComponent.LayerMaps[j], (GetProjectFolder() + TerrainComponent.LayerMaps[j]->GetObjectID() + std::string(".texture")).c_str());
			}
		}

		for (int j = 0; j < FE_TERRAIN_MAX_LAYERS; j++)
		{
			FETerrainLayer* CurrentLayer = TerrainComponent.GetLayerInSlot(j);
			if (CurrentLayer == nullptr)
			{
				TerrainData[Terrain->GetObjectID()]["layers"][j]["acive"] = false;
				break;
			}

			TerrainData[Terrain->GetObjectID()]["layers"][j]["acive"] = true;
			TerrainData[Terrain->GetObjectID()]["layers"][j]["name"] = CurrentLayer->GetMaterial()->GetName();
			TerrainData[Terrain->GetObjectID()]["layers"][j]["materialID"] = CurrentLayer->GetMaterial()->GetObjectID();
		}

		Terrain->SetDirtyFlag(false);
	}
	Root["terrains"] = TerrainData;

	//// Saving Lights.
	//std::vector<std::string> LightList = SCENE.GetLightsList();
	//Json::Value LightData;
	//for (size_t i = 0; i < LightList.size(); i++)
	//{
	//	FELight* Light = SCENE.GetLight(LightList[i]);

	//	// general light information
	//	LightData[Light->GetObjectID()]["ID"] = Light->GetObjectID();
	//	LightData[Light->GetObjectID()]["type"] = Light->GetType();
	//	LightData[Light->GetObjectID()]["name"] = Light->GetName();
	//	LightData[Light->GetObjectID()]["intensity"] = Light->GetIntensity();
	//	WriteTransformToJson(LightData[Light->GetObjectID()]["transformation"], &Light->Transform);
	//	LightData[Light->GetObjectID()]["castShadows"] = Light->IsCastShadows();
	//	LightData[Light->GetObjectID()]["enabled"] = Light->IsLightEnabled();
	//	LightData[Light->GetObjectID()]["color"]["R"] = Light->GetColor()[0];
	//	LightData[Light->GetObjectID()]["color"]["G"] = Light->GetColor()[1];
	//	LightData[Light->GetObjectID()]["color"]["B"] = Light->GetColor()[2];
	//	LightData[Light->GetObjectID()]["staticShadowBias"] = Light->IsStaticShadowBias();
	//	LightData[Light->GetObjectID()]["shadowBias"] = Light->GetShadowBias();
	//	LightData[Light->GetObjectID()]["shadowBiasVariableIntensity"] = Light->GetShadowBiasVariableIntensity();
	//	LightData[Light->GetObjectID()]["shadowBlurFactor"] = Light->GetShadowBlurFactor();

	//	// Type specific information.
	//	if (Light->GetType() == FE_DIRECTIONAL_LIGHT)
	//	{
	//		FEDirectionalLight* DirectionalLight = reinterpret_cast<FEDirectionalLight*>(Light);

	//		LightData[DirectionalLight->GetObjectID()]["direction"]["X"] = DirectionalLight->GetDirection()[0];
	//		LightData[DirectionalLight->GetObjectID()]["direction"]["Y"] = DirectionalLight->GetDirection()[1];
	//		LightData[DirectionalLight->GetObjectID()]["direction"]["Z"] = DirectionalLight->GetDirection()[2];
	//		LightData[DirectionalLight->GetObjectID()]["CSM"]["activeCascades"] = DirectionalLight->GetActiveCascades();
	//		LightData[DirectionalLight->GetObjectID()]["CSM"]["shadowCoverage"] = DirectionalLight->GetShadowCoverage();
	//		LightData[DirectionalLight->GetObjectID()]["CSM"]["CSMZDepth"] = DirectionalLight->GetCSMZDepth();
	//		LightData[DirectionalLight->GetObjectID()]["CSM"]["CSMXYDepth"] = DirectionalLight->GetCSMXYDepth();
	//	}
	//	else if (Light->GetType() == FE_SPOT_LIGHT)
	//	{
	//		FESpotLight* SpotLight = reinterpret_cast<FESpotLight*>(Light);

	//		LightData[SpotLight->GetObjectID()]["spotAngle"] = SpotLight->GetSpotAngle();
	//		LightData[SpotLight->GetObjectID()]["spotAngleOuter"] = SpotLight->GetSpotAngleOuter();
	//		LightData[SpotLight->GetObjectID()]["direction"]["X"] = SpotLight->GetDirection()[0];
	//		LightData[SpotLight->GetObjectID()]["direction"]["Y"] = SpotLight->GetDirection()[1];
	//		LightData[SpotLight->GetObjectID()]["direction"]["Z"] = SpotLight->GetDirection()[2];
	//	}
	//	else if (Light->GetType() == FE_POINT_LIGHT)
	//	{
	//		FEPointLight* PointLight = reinterpret_cast<FEPointLight*>(Light);

	//		LightData[PointLight->GetObjectID()]["range"] = PointLight->GetRange();
	//	}
	//}
	//Root["lights"] = LightData;

	// Saving Effects settings.
	Json::Value EffectsData;
	// *********** Gamma Correction & Exposure ***********
	EffectsData["Gamma Correction & Exposure"]["Gamma"] = ENGINE.GetCamera()->GetGamma();
	EffectsData["Gamma Correction & Exposure"]["Exposure"] = ENGINE.GetCamera()->GetExposure();
	// *********** Anti-Aliasing(FXAA) ***********
	EffectsData["Anti-Aliasing(FXAA)"]["FXAASpanMax"] = RENDERER.GetFXAASpanMax();
	EffectsData["Anti-Aliasing(FXAA)"]["FXAAReduceMin"] = RENDERER.GetFXAAReduceMin();
	EffectsData["Anti-Aliasing(FXAA)"]["FXAAReduceMul"] = RENDERER.GetFXAAReduceMul();
	// *********** Bloom ***********
	EffectsData["Bloom"]["thresholdBrightness"] = RENDERER.GetBloomThreshold();
	EffectsData["Bloom"]["BloomSize"] = RENDERER.GetBloomSize();
	// *********** Depth of Field ***********
	EffectsData["Depth of Field"]["Near distance"] = RENDERER.GetDOFNearDistance();
	EffectsData["Depth of Field"]["Far distance"] = RENDERER.GetDOFFarDistance();
	EffectsData["Depth of Field"]["Strength"] = RENDERER.GetDOFStrength();
	EffectsData["Depth of Field"]["Distance dependent strength"] = RENDERER.GetDOFDistanceDependentStrength();
	// *********** Distance fog ***********
	EffectsData["Distance fog"]["isDistanceFogEnabled"] = RENDERER.IsDistanceFogEnabled();
	EffectsData["Distance fog"]["Density"] = RENDERER.GetDistanceFogDensity();
	EffectsData["Distance fog"]["Gradient"] = RENDERER.GetDistanceFogGradient();
	// *********** Chromatic Aberration ***********
	EffectsData["Chromatic Aberration"]["Shift strength"] = RENDERER.GetChromaticAberrationIntensity();
	// *********** Sky ***********
	//EffectsData["Sky"]["Enabled"] = RENDERER.IsSkyEnabled() ? 1.0f : 0.0f;
	//EffectsData["Sky"]["Sphere size"] = RENDERER.GetDistanceToSky();

	Root["effects"] = EffectsData;

	// Saving Camera settings.
	Json::Value CameraData;

	CameraData["position"]["X"] = ENGINE.GetCamera()->GetPosition()[0];
	CameraData["position"]["Y"] = ENGINE.GetCamera()->GetPosition()[1];
	CameraData["position"]["Z"] = ENGINE.GetCamera()->GetPosition()[2];

	CameraData["fov"] = ENGINE.GetCamera()->GetFov();
	CameraData["nearPlane"] = ENGINE.GetCamera()->GetNearPlane();
	CameraData["farPlane"] = ENGINE.GetCamera()->GetFarPlane();

	CameraData["yaw"] = ENGINE.GetCamera()->GetYaw();
	CameraData["pitch"] = ENGINE.GetCamera()->GetPitch();
	CameraData["roll"] = ENGINE.GetCamera()->GetRoll();

	CameraData["aspectRatio"] = ENGINE.GetCamera()->GetAspectRatio();

	CameraData["movementSpeed"] = ENGINE.GetCamera()->GetMovementSpeed();

	Root["camera"] = CameraData;

	// Saving into file.
	Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, Root);

	SceneFile.open(ProjectFolder + "scene.txt");
	SceneFile << JsonFile;
	SceneFile.close();

	for (size_t i = 0; i < FilesToDelete.size(); i++)
	{
		FILE_SYSTEM.DeleteFile(FilesToDelete[i].c_str());
	}

	// VFS
	VIRTUAL_FILE_SYSTEM.SaveState(ProjectFolder + "VFS.txt");

	bModified = false;
}

void FEProject::ReadTransformToJson(Json::Value& Root, FETransformComponent* Transform)
{
	Transform->SetPosition(glm::vec3(Root["position"]["X"].asFloat(),
		Root["position"]["Y"].asFloat(),
		Root["position"]["Z"].asFloat()));

	Transform->SetRotation(glm::vec3(Root["rotation"]["X"].asFloat(),
		Root["rotation"]["Y"].asFloat(),
		Root["rotation"]["Z"].asFloat()));

	Transform->SetUniformScaling(Root["scale"]["uniformScaling"].asBool());

	Transform->SetScale(glm::vec3(Root["scale"]["X"].asFloat(),
		Root["scale"]["Y"].asFloat(),
		Root["scale"]["Z"].asFloat()));
}

void FEProject::LoadScene()
{
	std::ifstream SceneFile;
	SceneFile.open(ProjectFolder + "scene.txt");

	std::string FileData((std::istreambuf_iterator<char>(SceneFile)), std::istreambuf_iterator<char>());
	SceneFile.close();

	Json::Value Root;
	JSONCPP_STRING Error;
	Json::CharReaderBuilder Builder;

	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	if (!Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Error))
		return;

	// Read project file version.
	float ProjectVersion = Root["version"].asFloat();

	// Project file was created before any version was written to project files.
	if (ProjectVersion != PROJECTS_FILE_VER)
	{
		if (ProjectVersion == 0.0f)
		{
			LOG.Add("Can't find version in scene file of project from " + ProjectFolder, "FE_LOG_LOADING", FE_LOG_WARNING);
			return;
		}
		else if (ProjectVersion == 0.01f)
		{
			// Do nothing.
		}
	}

	// Loading Meshes
	std::vector<Json::String> MeshList = Root["meshes"].getMemberNames();
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		RESOURCE_MANAGER.LoadFEMesh((ProjectFolder + Root["meshes"][MeshList[i]]["fileName"].asCString()).c_str(), Root["meshes"][MeshList[i]]["name"].asCString());
	}

	// loading Textures
	std::vector<Json::String> TexturesList = Root["textures"].getMemberNames();
	for (size_t i = 0; i < TexturesList.size(); i++)
	{
		// read type of texture if it is not standard then skip it.
		if (Root["textures"][TexturesList[i]]["type"] == 33322)
		{
			continue;
		}

		RESOURCE_MANAGER.LoadFETextureAsync((ProjectFolder + Root["textures"][TexturesList[i]]["fileName"].asCString()).c_str(), Root["textures"][TexturesList[i]]["name"].asString(), nullptr, Root["textures"][TexturesList[i]]["ID"].asString());
	}

	// loading Materials
	std::vector<Json::String> MaterialsList = Root["materials"].getMemberNames();
	for (size_t i = 0; i < MaterialsList.size(); i++)
	{
		FEMaterial* NewMaterial = RESOURCE_MANAGER.CreateMaterial(Root["materials"][MaterialsList[i]]["name"].asString(), Root["materials"][MaterialsList[i]]["ID"].asString());

		//newMat->shader = RESOURCE_MANAGER.getShader("FEPhongShader");
		//newMat->shader = RESOURCE_MANAGER.getShader("FESolidColorShader");
		NewMaterial->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

		std::vector<Json::String> MembersList = Root["materials"][MaterialsList[i]].getMemberNames();
		for (size_t j = 0; j < MembersList.size(); j++)
		{
			if (MembersList[j] == "textures")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (Root["materials"][MaterialsList[i]]["textures"].isMember(std::to_string(k).c_str()))
					{
						std::string TextureID = Root["materials"][MaterialsList[i]]["textures"][std::to_string(k).c_str()].asCString();
						NewMaterial->Textures[k] = RESOURCE_MANAGER.GetTexture(TextureID);
						if (NewMaterial->Textures[k] == nullptr)
							NewMaterial->Textures[k] = RESOURCE_MANAGER.NoTexture;
					}
				}
			}

			if (MembersList[j] == "textureBindings")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (Root["materials"][MaterialsList[i]]["textureBindings"].isMember(std::to_string(k).c_str()))
					{
						int Binding = Root["materials"][MaterialsList[i]]["textureBindings"][std::to_string(k).c_str()].asInt();
						NewMaterial->TextureBindings[k] = Binding;
					}
				}
			}

			if (MembersList[j] == "textureChannels")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (Root["materials"][MaterialsList[i]]["textureChannels"].isMember(std::to_string(k).c_str()))
					{
						int binding = Root["materials"][MaterialsList[i]]["textureChannels"][std::to_string(k).c_str()].asInt();
						NewMaterial->TextureChannels[k] = binding;
					}
				}
			}
		}

		NewMaterial->SetMetalness(Root["materials"][MaterialsList[i]]["metalness"].asFloat());
		NewMaterial->SetRoughness(Root["materials"][MaterialsList[i]]["roughness"].asFloat());
		NewMaterial->SetNormalMapIntensity(Root["materials"][MaterialsList[i]]["normalMapIntensity"].asFloat());
		NewMaterial->SetAmbientOcclusionIntensity(Root["materials"][MaterialsList[i]]["ambientOcclusionIntensity"].asFloat());
		NewMaterial->SetAmbientOcclusionMapIntensity(Root["materials"][MaterialsList[i]]["ambientOcclusionMapIntensity"].asFloat());
		NewMaterial->SetRoughnessMapIntensity(Root["materials"][MaterialsList[i]]["roughnessMapIntensity"].asFloat());
		NewMaterial->SetMetalnessMapIntensity(Root["materials"][MaterialsList[i]]["metalnessMapIntensity"].asFloat());

		if (ProjectVersion >= 0.02f)
		{
			if (Root["materials"][MaterialsList[i]].isMember("tiling"))
				NewMaterial->SetTiling(Root["materials"][MaterialsList[i]]["tiling"].asFloat());
			NewMaterial->SetCompackPacking(Root["materials"][MaterialsList[i]]["compackPacking"].asBool());
		}
	}

	// loading gameModels
	std::vector<Json::String> GameModelList = Root["gameModels"].getMemberNames();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(RESOURCE_MANAGER.GetMesh(Root["gameModels"][GameModelList[i]]["mesh"].asCString()),
																	 RESOURCE_MANAGER.GetMaterial(Root["gameModels"][GameModelList[i]]["material"].asCString()),
																	 Root["gameModels"][GameModelList[i]]["name"].asString(), Root["gameModels"][GameModelList[i]]["ID"].asString());

		NewGameModel->SetScaleFactor(Root["gameModels"][GameModelList[i]]["scaleFactor"].asFloat());

		bool bHaveLODLevels = Root["gameModels"][GameModelList[i]]["LODs"]["haveLODlevels"].asBool();
		NewGameModel->SetUsingLOD(bHaveLODLevels);
		if (bHaveLODLevels)
		{
			NewGameModel->SetCullDistance(Root["gameModels"][GameModelList[i]]["LODs"]["cullDistance"].asFloat());
			NewGameModel->SetBillboardZeroRotaion(Root["gameModels"][GameModelList[i]]["LODs"]["billboardZeroRotaion"].asFloat());

			size_t LODCount = Root["gameModels"][GameModelList[i]]["LODs"]["LODCount"].asInt();
			for (size_t j = 0; j < LODCount; j++)
			{
				NewGameModel->SetLODMesh(j, RESOURCE_MANAGER.GetMesh(Root["gameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["mesh"].asString()));
				NewGameModel->SetLODMaxDrawDistance(j, Root["gameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["maxDrawDistance"].asFloat());

				bool bLODBillboard = Root["gameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["isBillboard"].asBool();
				NewGameModel->SetIsLODBillboard(j, bLODBillboard);
				if (bLODBillboard)
					NewGameModel->SetBillboardMaterial(RESOURCE_MANAGER.GetMaterial(Root["gameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["billboardMaterial"].asString()));
			}
		}
	}

	// loading prefabs
	std::vector<Json::String> PrefabList = Root["prefabs"].getMemberNames();
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(nullptr, Root["prefabs"][PrefabList[i]]["name"].asString(), Root["prefabs"][PrefabList[i]]["ID"].asString());

		if (Root["prefabs"][PrefabList[i]].isMember("components"))
		{
			Json::Value Components = Root["prefabs"][PrefabList[i]]["components"];
			for (int j = 0; j < static_cast<int>(Components.size()); j++)
			{
				FETransformComponent ComponentTransform;
				ReadTransformToJson(Components[j]["transformation"], &ComponentTransform);

				NewPrefab->AddComponent(RESOURCE_MANAGER.GetGameModel(Components[j]["gameModel"]["ID"].asCString()), ComponentTransform);
			}
		}
	}

	// Loading Terrains.
	std::vector<Json::String> TerrainList = Root["terrains"].getMemberNames();
	for (size_t i = 0; i < TerrainList.size(); i++)
	{
		FEEntity* Entity = SCENE.AddEntity(Root["terrains"][TerrainList[i]]["name"].asString(), Root["terrains"][TerrainList[i]]["ID"].asString());
		FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
		Entity->AddComponent<FETerrainComponent>();
		FETerrainComponent& TerrainComponent = Entity->GetComponent<FETerrainComponent>();
		TERRAIN_SYSTEM.LoadHeightMap((ProjectFolder + Root["terrains"][TerrainList[i]]["heightMap"]["fileName"].asCString()).c_str(), Entity);

		TerrainComponent.SetHightScale(Root["terrains"][TerrainList[i]]["hightScale"].asFloat());
		TerrainComponent.SetDisplacementScale(Root["terrains"][TerrainList[i]]["displacementScale"].asFloat());
		glm::vec2 TileMult;
		TileMult.x = Root["terrains"][TerrainList[i]]["tileMult"]["X"].asFloat();
		TileMult.y = Root["terrains"][TerrainList[i]]["tileMult"]["Y"].asFloat();
		TerrainComponent.SetTileMult(TileMult);
		TerrainComponent.SetLODLevel(Root["terrains"][TerrainList[i]]["LODlevel"].asFloat());
		TerrainComponent.SetChunkPerSide(Root["terrains"][TerrainList[i]]["chunkPerSide"].asFloat());
		TerrainComponent.SetHightScale(Root["terrains"][TerrainList[i]]["hightScale"].asFloat());
		TerrainComponent.SetHightScale(Root["terrains"][TerrainList[i]]["hightScale"].asFloat());
		ReadTransformToJson(Root["terrains"][TerrainList[i]]["transformation"], &TransformComponent);

		if (ProjectVersion >= 0.02f)
		{
			for (int j = 0; j < FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE; j++)
			{
				if (Root["terrains"][TerrainList[i]].isMember("layerMaps"))
				{
					FETexture* LoadedTexture = RESOURCE_MANAGER.LoadFETexture((ProjectFolder + Root["terrains"][TerrainList[i]]["layerMaps"][j]["fileName"].asCString()).c_str(), Root["terrains"][TerrainList[i]]["layerMaps"][j]["name"].asString());
					TerrainComponent.LayerMaps[j] = LoadedTexture;
				}
			}

			for (int j = 0; j < FE_TERRAIN_MAX_LAYERS; j++)
			{
				if (Root["terrains"][TerrainList[i]]["layers"][j]["acive"].asBool())
				{
					TERRAIN_SYSTEM.ActivateVacantLayerSlot(Entity, RESOURCE_MANAGER.GetMaterial(Root["terrains"][TerrainList[i]]["layers"][j]["materialID"].asCString()));
					TerrainComponent.GetLayerInSlot(j)->SetName(Root["terrains"][TerrainList[i]]["layers"][j]["name"].asCString());
				}
			}
		}
	}

	// Loading entities.
	std::vector<Json::String> EntityList = Root["entities"].getMemberNames();
	for (size_t i = 0; i < EntityList.size(); i++)
	{
		FEEntity* Entity = SCENE.AddEntity(Root["entities"][EntityList[i]]["name"].asString(), Root["entities"][EntityList[i]]["ID"].asString());

		if (Root["entities"][EntityList[i]].isMember("type"))
		{
			if (Root["entities"][EntityList[i]]["type"] == "FE_ENTITY_INSTANCED")
			{
				// FIX ME! Converting Prefab to GameModels.
				FEEntity* EntityWithFirstPrefab = Entity;
				FEPrefab* OldPrefab = RESOURCE_MANAGER.GetPrefab(Root["entities"][EntityList[i]]["prefab"].asCString());
				for (size_t c = 0; c < OldPrefab->ComponentsCount(); c++)
				{
					if (c > 0)
					{
						Entity = SCENE.AddEntity(Root["entities"][EntityList[i]]["name"].asString() + "_Prefabs_" + std::to_string(c));
					}
					FEPrefabComponent* CurrentComponent = OldPrefab->GetComponent(static_cast<int>(c));
					FEGameModel* GameModel = CurrentComponent->GameModel;

					Entity->AddComponent<FEGameModelComponent>(GameModel);
					FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
					Entity->AddComponent<FEInstancedComponent>();
					FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

					if (abs(ProjectVersion - 0.025f) <= FLT_EPSILON)
						GameModelComponent.SetVisibility(Root["entities"][EntityList[i]]["visible"].asBool());

					ReadTransformToJson(Root["entities"][EntityList[i]]["transformation"], &Entity->GetComponent<FETransformComponent>());
					// FIX ME!
					Entity->GetComponent<FETransformComponent>().Combine(CurrentComponent->Transform);

					FESpawnInfo SpawnInfo;
					SpawnInfo.Seed = Root["entities"][EntityList[i]]["spawnInfo"]["seed"].asInt();
					SpawnInfo.ObjectCount = Root["entities"][EntityList[i]]["spawnInfo"]["objectCount"].asInt();
					SpawnInfo.Radius = Root["entities"][EntityList[i]]["spawnInfo"]["radius"].asFloat();
					SpawnInfo.SetMinScale(Root["entities"][EntityList[i]]["spawnInfo"]["minScale"].asFloat());
					SpawnInfo.SetMaxScale(Root["entities"][EntityList[i]]["spawnInfo"]["maxScale"].asFloat());
					SpawnInfo.RotationDeviation.x = Root["entities"][EntityList[i]]["spawnInfo"]["rotationDeviation.x"].asFloat();
					SpawnInfo.RotationDeviation.y = Root["entities"][EntityList[i]]["spawnInfo"]["rotationDeviation.y"].asFloat();
					SpawnInfo.RotationDeviation.z = Root["entities"][EntityList[i]]["spawnInfo"]["rotationDeviation.z"].asFloat();

					if (Root["entities"][EntityList[i]]["snappedToTerrain"].asString() != "none")
					{
						FEEntity* TerrainEntity = SCENE.GetEntity(Root["entities"][EntityList[i]]["snappedToTerrain"].asString());
						TERRAIN_SYSTEM.SnapInstancedEntity(TerrainEntity, Entity);

						if (Root["entities"][EntityList[i]].isMember("terrainLayer"))
						{
							if (Root["entities"][EntityList[i]]["terrainLayer"].asInt() != -1)
							{
								TERRAIN_SYSTEM.ConnectInstancedEntityToLayer(TerrainEntity, Entity, Root["entities"][EntityList[i]]["terrainLayer"].asInt());
							}

							InstancedComponent.SetMinimalLayerIntensityToSpawn(Root["entities"][EntityList[i]]["minimalLayerIntensity"].asFloat());
						}
					}

					// FIX ME! That should be done after all entities are loaded.
					// And scene was updated.
					INSTANCED_RENDERING_SYSTEM.PopulateInstance(Entity, SpawnInfo);

					if (Root["entities"][EntityList[i]]["modificationsToSpawn"].asBool())
					{
						std::ifstream InfoFile;
						InfoFile.open(ProjectFolder + EntityWithFirstPrefab->GetObjectID() + ".txt");

						std::string InfoFileData((std::istreambuf_iterator<char>(InfoFile)), std::istreambuf_iterator<char>());

						Json::Value EntityFileRoot;
						JSONCPP_STRING Err;
						Json::CharReaderBuilder Builder;

						const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
						if (!Reader->parse(InfoFileData.c_str(), InfoFileData.c_str() + InfoFileData.size(), &EntityFileRoot, &Err))
							return;

						size_t Count = EntityFileRoot["modifications"].size();
						for (int j = 0; j < Count; j++)
						{
							if (EntityFileRoot["modifications"][j]["type"].asInt() == FE_CHANGE_DELETED)
							{
								INSTANCED_RENDERING_SYSTEM.DeleteIndividualInstance(Entity, EntityFileRoot["modifications"][j]["index"].asInt());
							}
							else if (EntityFileRoot["modifications"][j]["type"].asInt() == FE_CHANGE_MODIFIED)
							{
								glm::mat4 ModifedMatrix;
								for (int k = 0; k < 4; k++)
								{
									for (int p = 0; p < 4; p++)
									{
										ModifedMatrix[k][p] = EntityFileRoot["modifications"][j]["modification"][k][p].asFloat();
									}
								}

								INSTANCED_RENDERING_SYSTEM.ModifyIndividualInstance(Entity, EntityFileRoot["modifications"][j]["index"].asInt(), ModifedMatrix);
							}
							else if (EntityFileRoot["modifications"][j]["type"].asInt() == FE_CHANGE_ADDED)
							{
								glm::mat4 ModifedMatrix;
								for (int k = 0; k < 4; k++)
								{
									for (int p = 0; p < 4; p++)
									{
										ModifedMatrix[k][p] = EntityFileRoot["modifications"][j]["modification"][k][p].asFloat();
									}
								}

								INSTANCED_RENDERING_SYSTEM.AddIndividualInstance(Entity, ModifedMatrix);
							}
						}
					}
				}
			}
			else
			{
				// For compatibility with old projects.
				if (Root["entities"][EntityList[i]].isMember("gameModel"))
				{
					//FEPrefab* TempPrefab = RESOURCE_MANAGER.CreatePrefab(RESOURCE_MANAGER.GetGameModel(Root["entities"][EntityList[i]]["gameModel"].asCString()));
					//SCENE.AddEntity(TempPrefab, Root["entities"][EntityList[i]]["name"].asString(), Root["entities"][EntityList[i]]["ID"].asString());
					
					FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Root["entities"][EntityList[i]]["gameModel"].asCString());
					Entity->AddComponent<FEGameModelComponent>(GameModel);
				}
				else
				{
					/*SCENE.AddEntity(RESOURCE_MANAGER.GetPrefab(Root["entities"][EntityList[i]]["prefab"].asCString()),
															     Root["entities"][EntityList[i]]["name"].asString(),
															     Root["entities"][EntityList[i]]["ID"].asString());*/

					FEGameModel* GameModel = RESOURCE_MANAGER.GetPrefab(Root["entities"][EntityList[i]]["prefab"].asCString())->GetComponent(0)->GameModel;
					Entity->AddComponent<FEGameModelComponent>(GameModel);
				}

				if (abs(ProjectVersion - 0.025f) <= FLT_EPSILON)
				{
					//SCENE.GetEntity(EntityList[i])->SetVisibility(Root["entities"][EntityList[i]]["visible"].asBool());
					Entity->GetComponent<FEGameModelComponent>().SetVisibility(Root["entities"][EntityList[i]]["visible"].asBool());
				}
					
				ReadTransformToJson(Root["entities"][EntityList[i]]["transformation"], &Entity->GetComponent<FETransformComponent>());
				/*ReadTransformToJson(Root["entities"][EntityList[i]]["transformation"], &SCENE.GetEntity(EntityList[i])->Transform);
				FEEntity* Entity = SCENE.GetNewStyleEntityByOldStyleID(EntityList[i]);
				if (Entity != nullptr)
				{
					Entity->GetComponent<FETransformComponent>() = SCENE.GetEntity(EntityList[i])->Transform;
				}*/
			}
		}
		// For compatibility with old projects.
		else
		{
			//FEEntity* Entity = SCENE.AddEntity(Root["entities"][EntityList[i]]["name"].asString(), Root["entities"][EntityList[i]]["ID"].asString());
			FEGameModel* GameModel = RESOURCE_MANAGER.GetPrefab(Root["entities"][EntityList[i]]["prefab"].asCString())->GetComponent(0)->GameModel;
			Entity->AddComponent<FEGameModelComponent>(GameModel);
			ReadTransformToJson(Root["entities"][EntityList[i]]["transformation"], &Entity->GetComponent<FETransformComponent>());

			/*SCENE.AddEntity(RESOURCE_MANAGER.GetPrefab(Root["entities"][EntityList[i]]["prefab"].asCString()),
							EntityList[i],
							Root["entities"][EntityList[i]]["ID"].asString());
			ReadTransformToJson(Root["entities"][EntityList[i]]["transformation"], &SCENE.GetEntity(EntityList[i])->Transform);*/
		}
	}

	if (abs(ProjectVersion - 0.025f) <= FLT_EPSILON)
	{
		// Loading scene hierarchy.
		SCENE.SceneGraph.FromJson(Root["sceneHierarchy"]);
	}

	// Loading Lights.
	std::vector<Json::String> LightList = Root["lights"].getMemberNames();
	for (size_t i = 0; i < LightList.size(); i++)
	{
		FEEntity* Entity = SCENE.AddEntity("Light Entity");
		auto OldType = static_cast<FE_OBJECT_TYPE>(Root["lights"][LightList[i]]["type"].asInt());
		FE_LIGHT_TYPE NewType = FE_DIRECTIONAL_LIGHT;
		if (OldType == FE_POINT_LIGHT_DEPRECATED)
		{
			NewType = FE_POINT_LIGHT;
		}
		else if (OldType == FE_SPOT_LIGHT_DEPRECATED)
		{
			NewType = FE_SPOT_LIGHT;
		}
		Entity->AddComponent<FELightComponent>(NewType);
		FELightComponent& LightComponent = Entity->GetComponent<FELightComponent>();

		LightComponent.SetIntensity(Root["lights"][LightList[i]]["intensity"].asFloat());
		ReadTransformToJson(Root["lights"][LightList[i]]["transformation"], &Entity->GetComponent<FETransformComponent>());
		LightComponent.SetCastShadows(Root["lights"][LightList[i]]["castShadows"].asBool());
		LightComponent.SetLightEnabled(Root["lights"][LightList[i]]["enabled"].asBool());
		LightComponent.SetColor(glm::vec3(Root["lights"][LightList[i]]["color"]["R"].asFloat(),
										  Root["lights"][LightList[i]]["color"]["G"].asFloat(),
										  Root["lights"][LightList[i]]["color"]["B"].asFloat()));
		LightComponent.SetIsStaticShadowBias(Root["lights"][LightList[i]]["staticShadowBias"].asBool());
		LightComponent.SetShadowBias(Root["lights"][LightList[i]]["shadowBias"].asFloat());
		LightComponent.SetShadowBiasVariableIntensity(Root["lights"][LightList[i]]["shadowBiasVariableIntensity"].asFloat());
		if (ProjectVersion >= 0.02f && Root["lights"][LightList[i]].isMember("shadowBlurFactor"))
			LightComponent.SetShadowBlurFactor(Root["lights"][LightList[i]]["shadowBlurFactor"].asFloat());

		if (LightComponent.GetType() == FE_POINT_LIGHT)
		{
			LightComponent.SetRange(Root["lights"][LightList[i]]["range"].asFloat());
		}
		else if (LightComponent.GetType() == FE_SPOT_LIGHT)
		{
			LightComponent.SetSpotAngle(Root["lights"][LightList[i]]["spotAngle"].asFloat());
			LightComponent.SetSpotAngleOuter(Root["lights"][LightList[i]]["spotAngleOuter"].asFloat());

			LightComponent.SetSpotAngle(Root["lights"][LightList[i]]["spotAngle"].asFloat());
			LightComponent.SetSpotAngleOuter(Root["lights"][LightList[i]]["spotAngleOuter"].asFloat());
		}
		else if (LightComponent.GetType() == FE_DIRECTIONAL_LIGHT)
		{
			LightComponent.SetActiveCascades(Root["lights"][LightList[i]]["CSM"]["activeCascades"].asInt());
			LightComponent.SetShadowCoverage(Root["lights"][LightList[i]]["CSM"]["shadowCoverage"].asFloat());
			LightComponent.SetCSMZDepth(Root["lights"][LightList[i]]["CSM"]["CSMZDepth"].asFloat());
			LightComponent.SetCSMXYDepth(Root["lights"][LightList[i]]["CSM"]["CSMXYDepth"].asFloat());

			LightComponent.SetActiveCascades(Root["lights"][LightList[i]]["CSM"]["activeCascades"].asInt());
			LightComponent.SetShadowCoverage(Root["lights"][LightList[i]]["CSM"]["shadowCoverage"].asFloat());
			LightComponent.SetCSMZDepth(Root["lights"][LightList[i]]["CSM"]["CSMZDepth"].asFloat());
			LightComponent.SetCSMXYDepth(Root["lights"][LightList[i]]["CSM"]["CSMXYDepth"].asFloat());
		}
	}

	// loading Effects settings
	// *********** Gamma Correction & Exposure ***********
	ENGINE.GetCamera()->SetGamma(Root["effects"]["Gamma Correction & Exposure"]["Gamma"].asFloat());
	ENGINE.GetCamera()->SetExposure(Root["effects"]["Gamma Correction & Exposure"]["Exposure"].asFloat());
	// *********** Anti-Aliasing(FXAA) ***********
	RENDERER.SetFXAASpanMax(Root["effects"]["Anti-Aliasing(FXAA)"]["FXAASpanMax"].asFloat());
	RENDERER.SetFXAAReduceMin(Root["effects"]["Anti-Aliasing(FXAA)"]["FXAAReduceMin"].asFloat());
	RENDERER.SetFXAAReduceMul(Root["effects"]["Anti-Aliasing(FXAA)"]["FXAAReduceMul"].asFloat());
	// *********** Bloom ***********
	//PPEffect = RENDERER.getPostProcessEffect("bloom");
	RENDERER.SetBloomThreshold(Root["effects"]["Bloom"]["thresholdBrightness"].asFloat());
	RENDERER.SetBloomSize(Root["effects"]["Bloom"]["BloomSize"].asFloat());
	// *********** Depth of Field ***********
	RENDERER.SetDOFNearDistance(Root["effects"]["Depth of Field"]["Near distance"].asFloat());
	RENDERER.SetDOFFarDistance(Root["effects"]["Depth of Field"]["Far distance"].asFloat());
	RENDERER.SetDOFStrength(Root["effects"]["Depth of Field"]["Strength"].asFloat());
	RENDERER.SetDOFDistanceDependentStrength(Root["effects"]["Depth of Field"]["Distance dependent strength"].asFloat());
	// *********** Distance fog ***********
	if (Root["effects"]["Distance fog"].isMember("isDistanceFogEnabled"))
	{
		RENDERER.SetDistanceFogEnabled(Root["effects"]["Distance fog"]["isDistanceFogEnabled"].asBool());
	}
	else
	{
		RENDERER.SetDistanceFogEnabled(Root["effects"]["Distance fog"]["Density"].asFloat() > -1.0f ? true : false);
	}
	RENDERER.SetDistanceFogDensity(Root["effects"]["Distance fog"]["Density"].asFloat());
	RENDERER.SetDistanceFogGradient(Root["effects"]["Distance fog"]["Gradient"].asFloat());
	// *********** Chromatic Aberration ***********
	RENDERER.SetChromaticAberrationIntensity(Root["effects"]["Chromatic Aberration"]["Shift strength"].asFloat());
	// *********** Sky ***********
	//SKY_DOME_SYSTEM.SetEnabled(Root["effects"]["Sky"]["Enabled"].asFloat() > 0.0f ? true : false);
	//SKY_DOME_SYSTEM.SetDistanceToSky(Root["effects"]["Sky"]["Sphere size"].asFloat());
	//Fix Me!
	FEEntity* SkyDome = SCENE.AddEntity("SkyDome");
	SkyDome->GetComponent<FETransformComponent>().SetScale(glm::vec3(150.0f));
	SKY_DOME_SYSTEM.AddToEntity(SkyDome);

	// loading Camera settings
	ENGINE.GetCamera()->SetPosition(glm::vec3(Root["camera"]["position"]["X"].asFloat(),
		Root["camera"]["position"]["Y"].asFloat(),
		Root["camera"]["position"]["Z"].asFloat()));

	ENGINE.GetCamera()->SetFov(Root["camera"]["fov"].asFloat());
	ENGINE.GetCamera()->SetNearPlane(Root["camera"]["nearPlane"].asFloat());
	ENGINE.GetCamera()->SetFarPlane(Root["camera"]["farPlane"].asFloat());

	ENGINE.GetCamera()->SetYaw(Root["camera"]["yaw"].asFloat());
	ENGINE.GetCamera()->SetPitch(Root["camera"]["pitch"].asFloat());
	ENGINE.GetCamera()->SetRoll(Root["camera"]["roll"].asFloat());

	ENGINE.GetCamera()->SetAspectRatio(Root["camera"]["aspectRatio"].asFloat());

	if (ProjectVersion >= 0.02f && Root["camera"].isMember("movementSpeed"))
		ENGINE.GetCamera()->SetMovementSpeed(Root["camera"]["movementSpeed"].asFloat());

	// After all scene objects are loaded, we need to update all objects.
	SCENE.Update();

	// FIX ME! Only after that all systems should be updated and triggered.

	// VFS
	if (FILE_SYSTEM.CheckFile((ProjectFolder + "VFS.txt").c_str()))
	{
		VIRTUAL_FILE_SYSTEM.LoadState(ProjectFolder + "VFS.txt");

		VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, "/Shaders");

		auto Files = VIRTUAL_FILE_SYSTEM.GetDirectoryContent("/Shaders");
		for (size_t i = 0; i < Files.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.DeleteFile(Files[i], "/Shaders");
		}

		std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetShadersList();
		for (size_t i = 0; i < ShaderList.size(); i++)
		{
			if (OBJECT_MANAGER.GetFEObject(ShaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(ShaderList[i]), "/Shaders");
		}

		std::vector<std::string> StandardShaderList = RESOURCE_MANAGER.GetStandardShadersList();
		for (size_t i = 0; i < StandardShaderList.size(); i++)
		{
			if (OBJECT_MANAGER.GetFEObject(StandardShaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(StandardShaderList[i]), "/Shaders");
		}

		VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, "/Shaders");
	}
	else
	{
		LOG.Add("Can't find VIRTUAL_FILE_SYSTEM file in project folder. Creating basic VIRTUAL_FILE_SYSTEM layout.", "FE_LOG_LOADING", FE_LOG_WARNING);
		VIRTUAL_FILE_SYSTEM.CreateDirectory("Shaders", "/");

		std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetShadersList();
		for (size_t i = 0; i < ShaderList.size(); i++)
		{
			if (OBJECT_MANAGER.GetFEObject(ShaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(ShaderList[i]), "/Shaders");
		}

		std::vector<std::string> StandardShaderList = RESOURCE_MANAGER.GetStandardShadersList();
		for (size_t i = 0; i < StandardShaderList.size(); i++)
		{
			if (OBJECT_MANAGER.GetFEObject(StandardShaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(StandardShaderList[i]), "/Shaders");
		}

		std::vector<std::string> MeshListForVirtualFileSystem = RESOURCE_MANAGER.GetMeshList();
		for (size_t i = 0; i < MeshListForVirtualFileSystem.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(MeshListForVirtualFileSystem[i]), "/");
		}

		std::vector<std::string> TextureList = RESOURCE_MANAGER.GetTextureList();
		for (size_t i = 0; i < TextureList.size(); i++)
		{
			bool bShouldAdd = true;
			FETexture* TextureToAdd = RESOURCE_MANAGER.GetTexture(TextureList[i]);
			//if (textureToAdd->getInternalFormat() == GL_R16)
			//{
			//	// Potentially it could be texture hight map.
			//	std::vector<std::string> terrainList = SCENE.getTerrainList();
			//	for (size_t j = 0; j < terrainList.size(); j++)
			//	{
			//		if (SCENE.getTerrain(terrainList[j])->heightMap == textureToAdd)
			//		{
			//			shouldAdd = false;
			//			break;
			//		}
			//	}
			//}

			if (bShouldAdd)
				VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(TextureList[i]), "/");
		}

		std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialList();
		for (size_t i = 0; i < MaterialList.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(MaterialList[i]), "/");
		}

		std::vector<std::string> GameModelListForVirtualFileSystem = RESOURCE_MANAGER.GetGameModelList();
		for (size_t i = 0; i < GameModelListForVirtualFileSystem.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(GameModelListForVirtualFileSystem[i]), "/");
		}
	}
}

void FEProject::CreateDummyScreenshot()
{
	const size_t Width = ENGINE.GetRenderTargetWidth();
	const size_t Height = ENGINE.GetRenderTargetHeight();

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
	RESOURCE_MANAGER.SaveFETexture(TempTexture, (GetProjectFolder() + "/projectScreenShot.texture").c_str());
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

bool FEProject::ShouldIncludeInSceneFile(const FETexture* Texture)
{
	// Terrain should manage it's textures in a different way.
	const std::vector<std::string> TerrainList = SCENE.GetEntityIDListWith<FETerrainComponent>();
	Json::Value TerrainData;
	for (size_t i = 0; i < TerrainList.size(); i++)
	{
		FEEntity* Terrain = SCENE.GetEntity(TerrainList[i]);
		FETerrainComponent& TerrainComponent = Terrain->GetComponent<FETerrainComponent>();

		if (TerrainComponent.HeightMap->GetObjectID() == Texture->GetObjectID())
			return false;

		if (TerrainComponent.LayerMaps[0] != nullptr && TerrainComponent.LayerMaps[0]->GetObjectID() == Texture->GetObjectID())
			return false;

		if (TerrainComponent.LayerMaps[1] != nullptr && TerrainComponent.LayerMaps[1]->GetObjectID() == Texture->GetObjectID())
			return false;
	}

	return true;
}

void FEProject::SetProjectFolder(const std::string NewValue)
{
	if (!FILE_SYSTEM.CheckDirectory(NewValue.c_str()))
		return;

	ProjectFolder = NewValue;
}

void FEProject::SaveSceneTo(const std::string NewPath)
{
	if (!FILE_SYSTEM.CheckDirectory(NewPath.c_str()))
		return;

	SetProjectFolder(NewPath);
	ENGINE.TakeScreenshot((GetProjectFolder() + "projectScreenShot.texture").c_str());
	SaveScene(true);
}