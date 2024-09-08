#include "FEProject.h"
#include "FEEditor.h"

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
	if (!FILE_SYSTEM.DoesDirectoryExist(PROJECTS_FOLDER))
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
	WindowsManager::GetInstance().CloseAllWindows();
	WindowsManager::GetInstance().CloseAllPopups();

	SELECTED.ClearAll();
	PREVIEW_MANAGER.Clear();
	SCENE_MANAGER.DeleteScene(PROJECT_MANAGER.GetCurrent()->ProjectScene);
	for (size_t i = 0; i < List.size(); i++)
	{
		delete List[i];
	}
	List.clear();
	PROJECT_MANAGER.SetCurrent(nullptr);
	LoadProjectList();

	VIRTUAL_FILE_SYSTEM.SetCurrentPath("/");
}

void FEProjectManager::OpenProject(const int ProjectIndex)
{
	PROJECT_MANAGER.SetCurrent(List[ProjectIndex]);
	PROJECT_MANAGER.GetCurrent()->LoadProject();
	PROJECT_MANAGER.GetCurrent()->ProjectScene->SetFlag(FESceneFlag::Renderable, true);
	PROJECT_MANAGER.GetCurrent()->ProjectScene->SetFlag(FESceneFlag::EditorMode, true);
	EDITOR.AddEditorScene(PROJECT_MANAGER.GetCurrent()->ProjectScene, true);
	EDITOR.FocusedEditorSceneID = PROJECT_MANAGER.GetCurrent()->ProjectScene->GetObjectID();
	IndexChosen = -1;

	// In each loaded project we need to inject editor camera.
	InjectEditorCamera(PROJECT_MANAGER.GetCurrent()->ProjectScene);

	// After loading project we should update our previews
	PREVIEW_MANAGER.UpdateAll();
	SELECTED.ClearAll();
}

void FEProjectManager::DisplayProjectSelection()
{
	static float LowerPanelHight = 90.0f;
	const float MainWindowW = static_cast<float>(APPLICATION.GetMainWindow()->GetWidth());
	const float MainWindowH = static_cast<float>(APPLICATION.GetMainWindow()->GetHeight());

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
			ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
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

					CreateNewProject(ProjectName, std::string(PROJECTS_FOLDER) + std::string("/") + ProjectName + "/");

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

void FEProjectManager::CreateNewProject(std::string ProjectName, std::string ProjectPath)
{
	if (ProjectName.empty() || ProjectPath.empty())
	{
		LOG.Add("FEProjectManager::CreateNewProject: Project name or path is empty!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	if (ContainProject(ProjectPath))
	{
		LOG.Add("FEProjectManager::CreateNewProject: Project already exists!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	Json::Value Root;
	std::ofstream ProjectFile;
	Root["version"] = PROJECTS_FILE_VER;

	FEScene* NewScene = SCENE_MANAGER.CreateScene(ProjectName, "", FESceneFlag::None);
	
	FEEntity* SunEntity = NewScene->CreateEntity("Sun");
	SunEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = SunEntity->GetComponent<FELightComponent>();
	SunEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(4.0f);
	LightComponent.SetCastShadows(true);

	FEEntity* SkyDome = NewScene->CreateEntity("SkyDome");
	SkyDome->GetComponent<FETransformComponent>().SetScale(glm::vec3(150.0f));
	SkyDome->AddComponent<FESkyDomeComponent>();

	Json::Value SceneData;
	Json::Value SceneDataNode;
	SceneDataNode["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(NewScene);
	Json::Value SceneHierarchy = NewScene->SceneGraph.ToJson([](FEEntity* Entity) -> bool {
		if (Entity->GetTag() == EDITOR_RESOURCE_TAG)
			return false;

		return true;
	});
	SceneDataNode["Scene hierarchy"] = SceneHierarchy;

	SceneData[NewScene->GetObjectID()] = SceneDataNode;
	Root["Scenes"] = SceneData;
	Root["Main scene"] = NewScene->GetObjectID();

	// Saving into file.
	Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, Root);

	ProjectFile.open(ProjectPath + "scene.txt");
	ProjectFile << JsonFile;
	ProjectFile.close();

	SCENE_MANAGER.DeleteScene(NewScene);
}

bool FEProjectManager::ContainProject(const std::string Path)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(Path.c_str()))
		return false;

	if (!FILE_SYSTEM.DoesFileExist((Path + "/scene.txt").c_str()))
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

	RESOURCE_MANAGER.Clear();
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

void FEProject::SaveProject(bool bFullSave)
{
	Json::Value Root;
	std::ofstream ProjectFile;

	Root["version"] = PROJECTS_FILE_VER;
	
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
		if (Mesh->GetTag() == ENGINE_RESOURCE_TAG ||
			Mesh->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		MeshData[Mesh->GetObjectID()]["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Mesh);
		MeshData[Mesh->GetObjectID()]["FileName"] = Mesh->GetObjectID() + ".model";

		if (bFullSave)
			RESOURCE_MANAGER.SaveFEMesh(Mesh, (GetProjectFolder() + Mesh->GetObjectID() + std::string(".model")).c_str());

		Mesh->SetDirtyFlag(false);
	}
	Root["Meshes"] = MeshData;

	// Saving Textures.
	std::vector<std::string> TexturesList = RESOURCE_MANAGER.GetTextureList();
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
			RESOURCE_MANAGER.SaveFETexture(Texture, (GetProjectFolder() + Texture->GetObjectID() + std::string(".texture")).c_str());

		Texture->SetDirtyFlag(false);
	}
	Root["Textures"] = TexturesData;

	// Saving Materials.
	std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialList();
	Json::Value MaterialData;
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		FEMaterial* Material = RESOURCE_MANAGER.GetMaterial(MaterialList[i]);
		if (Material->GetTag() == ENGINE_RESOURCE_TAG ||
			Material->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		for (size_t j = 0; j < FE_MAX_TEXTURES_PER_MATERIAL; j++)
		{
			if (Material->Textures[j] != nullptr)
				MaterialData[Material->GetObjectID()]["Textures"][std::to_string(j).c_str()] = Material->Textures[j]->GetObjectID();

			if (Material->TextureBindings[j] != -1)
				MaterialData[Material->GetObjectID()]["Texture bindings"][std::to_string(j).c_str()] = Material->TextureBindings[j];

			if (Material->TextureChannels[j] != -1)
				MaterialData[Material->GetObjectID()]["Texture channels"][std::to_string(j).c_str()] = Material->TextureChannels[j];
		}

		MaterialData[Material->GetObjectID()]["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Material);
		MaterialData[Material->GetObjectID()]["Metalness"] = Material->GetMetalness();
		MaterialData[Material->GetObjectID()]["Roughness"] = Material->GetRoughness();
		MaterialData[Material->GetObjectID()]["NormalMap intensity"] = Material->GetNormalMapIntensity();
		MaterialData[Material->GetObjectID()]["AmbientOcclusion intensity"] = Material->GetAmbientOcclusionIntensity();
		MaterialData[Material->GetObjectID()]["AmbientOcclusionMap intensity"] = Material->GetAmbientOcclusionMapIntensity();
		MaterialData[Material->GetObjectID()]["RoughnessMap intensity"] = Material->GetRoughnessMapIntensity();
		MaterialData[Material->GetObjectID()]["MetalnessMap intensity"] = Material->GetMetalnessMapIntensity();
		MaterialData[Material->GetObjectID()]["Tiling"] = Material->GetTiling();
		MaterialData[Material->GetObjectID()]["Compack packing"] = Material->IsCompackPacking();

		Material->SetDirtyFlag(false);
	}
	Root["Materials"] = MaterialData;

	// Saving GameModels.
	std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelList();
	Json::Value GameModelData;
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (GameModel->GetTag() == ENGINE_RESOURCE_TAG ||
			GameModel->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		GameModelData[GameModel->GetObjectID()]["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(GameModel);
		GameModelData[GameModel->GetObjectID()]["Mesh"] = GameModel->Mesh->GetObjectID();
		GameModelData[GameModel->GetObjectID()]["Material"] = GameModel->Material->GetObjectID();
		GameModelData[GameModel->GetObjectID()]["ScaleFactor"] = GameModel->GetScaleFactor();

		GameModelData[GameModel->GetObjectID()]["LODs"]["HaveLODlevels"] = GameModel->IsUsingLOD();
		if (GameModel->IsUsingLOD())
		{
			GameModelData[GameModel->GetObjectID()]["LODs"]["CullDistance"] = GameModel->GetCullDistance();
			GameModelData[GameModel->GetObjectID()]["LODs"]["Billboard zero rotaion"] = GameModel->GetBillboardZeroRotaion();
			GameModelData[GameModel->GetObjectID()]["LODs"]["LODCount"] = GameModel->GetLODCount();
			for (size_t j = 0; j < GameModel->GetLODCount(); j++)
			{
				GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["Mesh"] = GameModel->GetLODMesh(j)->GetObjectID();
				GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["Max draw distance"] = GameModel->GetLODMaxDrawDistance(j);
				GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["IsBillboard"] = GameModel->IsLODBillboard(j);
				if (GameModel->IsLODBillboard(j))
					GameModelData[GameModel->GetObjectID()]["LODs"][std::to_string(j)]["Billboard material"] = GameModel->GetBillboardMaterial()->GetObjectID();
			}
		}

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

		PrefabData[Prefab->GetObjectID()]["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Prefab);

		Json::Value PrefabSceneData;
		PrefabSceneData["ID"] = Prefab->GetScene()->GetObjectID();
		Json::Value SceneHierarchy = Prefab->GetScene()->SceneGraph.ToJson();
		PrefabSceneData["Scene hierarchy"] = SceneHierarchy;
		PrefabData[Prefab->GetObjectID()]["Scene"] = PrefabSceneData;
	}
	Root["Prefabs"] = PrefabData;

	// Saving scenes.
	std::vector<FEScene*> SceneList = SCENE_MANAGER.GetAllScenes();
	Json::Value SceneData;
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		if (SceneList[i]->GetTag() == ENGINE_RESOURCE_TAG ||
			SceneList[i]->GetTag() == EDITOR_RESOURCE_TAG)
			continue;
		
		Json::Value SceneDataNode;
		SceneDataNode["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(SceneList[i]);
		Json::Value SceneHierarchy = SceneList[i]->SceneGraph.ToJson([](FEEntity* Entity) -> bool {
			if (Entity->GetTag() == EDITOR_RESOURCE_TAG)
				return false;

			return true;
		});
		SceneDataNode["Scene hierarchy"] = SceneHierarchy;

		SceneData[SceneList[i]->GetObjectID()] = SceneDataNode;
	}
	Root["Scenes"] = SceneData;
	Root["Main scene"] = ProjectScene->GetObjectID();

	// Saving into file.
	Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, Root);

	ProjectFile.open(ProjectFolder + "scene.txt");
	ProjectFile << JsonFile;
	ProjectFile.close();

	for (size_t i = 0; i < FilesToDelete.size(); i++)
	{
		FILE_SYSTEM.DeleteFile(FilesToDelete[i].c_str());
	}

	VIRTUAL_FILE_SYSTEM.SaveState(ProjectFolder + "VFS.txt");
	bModified = false;
}

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
std::string ListExportedFunctions(const std::string& dllPath) {
	std::stringstream result;
	HMODULE hModule = LoadLibraryA(dllPath.c_str());
	if (hModule == NULL) {
		return "Failed to load DLL.";
	}

	ULONG size;
	PIMAGE_EXPORT_DIRECTORY exportDir;

	exportDir = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(
		hModule, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size);

	if (exportDir == NULL) {
		FreeLibrary(hModule);
		return "Failed to get export directory.";
	}

	DWORD* names = (DWORD*)((BYTE*)hModule + exportDir->AddressOfNames);
	WORD* ordinals = (WORD*)((BYTE*)hModule + exportDir->AddressOfNameOrdinals);
	DWORD* functions = (DWORD*)((BYTE*)hModule + exportDir->AddressOfFunctions);

	for (DWORD i = 0; i < exportDir->NumberOfNames; i++) {
		char* name = (char*)((BYTE*)hModule + names[i]);
		WORD ordinal = ordinals[i];
		DWORD rva = functions[ordinal];

		result << "Function: " << name << ", Ordinal: " << ordinal << ", RVA: 0x"
			<< std::hex << rva << std::dec << "\n";
	}

	FreeLibrary(hModule);
	return result.str();
}

void FEProject::LoadProject()
{
	std::ifstream SceneFile;
	SceneFile.open(ProjectFolder + "Scene.txt");

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
	}

	// Loading meshes.
	std::vector<Json::String> MeshList = Root["Meshes"].getMemberNames();
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Meshes"][MeshList[i]]["FEObjectData"]);
		RESOURCE_MANAGER.LoadFEMesh((ProjectFolder + Root["Meshes"][MeshList[i]]["FileName"].asCString()).c_str(), LoadedObjectData.Name);
	}

	// Loading textures.
	std::vector<Json::String> TexturesList = Root["Textures"].getMemberNames();
	for (size_t i = 0; i < TexturesList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Textures"][TexturesList[i]]["FEObjectData"]);
		// Terrain textures should be loaded right away, not async.
		if (LoadedObjectData.Tag == TERRAIN_SYSTEM_RESOURCE_TAG)
		{
			RESOURCE_MANAGER.LoadFETexture((ProjectFolder + Root["Textures"][TexturesList[i]]["FileName"].asCString()).c_str());
		}
		else
		{
			RESOURCE_MANAGER.LoadFETextureAsync((ProjectFolder + Root["Textures"][TexturesList[i]]["FileName"].asCString()).c_str(), LoadedObjectData.Name, nullptr, LoadedObjectData.ID);
		}
	}

	// Loading materials.
	std::vector<Json::String> MaterialsList = Root["Materials"].getMemberNames();
	for (size_t i = 0; i < MaterialsList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Materials"][MaterialsList[i]]["FEObjectData"]);
		
		FEMaterial* NewMaterial = RESOURCE_MANAGER.CreateMaterial(LoadedObjectData.Name, LoadedObjectData.ID);
		RESOURCE_MANAGER.SetTag(NewMaterial, LoadedObjectData.Tag);
		NewMaterial->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

		std::vector<Json::String> MembersList = Root["Materials"][MaterialsList[i]].getMemberNames();
		for (size_t j = 0; j < MembersList.size(); j++)
		{
			if (MembersList[j] == "Textures")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (Root["Materials"][MaterialsList[i]]["Textures"].isMember(std::to_string(k).c_str()))
					{
						std::string TextureID = Root["Materials"][MaterialsList[i]]["Textures"][std::to_string(k).c_str()].asCString();
						NewMaterial->Textures[k] = RESOURCE_MANAGER.GetTexture(TextureID);
						if (NewMaterial->Textures[k] == nullptr)
							NewMaterial->Textures[k] = RESOURCE_MANAGER.NoTexture;
					}
				}
			}

			if (MembersList[j] == "Texture bindings")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (Root["Materials"][MaterialsList[i]]["Texture bindings"].isMember(std::to_string(k).c_str()))
					{
						int Binding = Root["Materials"][MaterialsList[i]]["Texture bindings"][std::to_string(k).c_str()].asInt();
						NewMaterial->TextureBindings[k] = Binding;
					}
				}
			}

			if (MembersList[j] == "Texture channels")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (Root["Materials"][MaterialsList[i]]["Texture channels"].isMember(std::to_string(k).c_str()))
					{
						int binding = Root["Materials"][MaterialsList[i]]["Texture channels"][std::to_string(k).c_str()].asInt();
						NewMaterial->TextureChannels[k] = binding;
					}
				}
			}
		}

		NewMaterial->SetMetalness(Root["Materials"][MaterialsList[i]]["Metalness"].asFloat());
		NewMaterial->SetRoughness(Root["Materials"][MaterialsList[i]]["Roughness"].asFloat());
		NewMaterial->SetNormalMapIntensity(Root["Materials"][MaterialsList[i]]["NormalMap intensity"].asFloat());
		NewMaterial->SetAmbientOcclusionIntensity(Root["Materials"][MaterialsList[i]]["AmbientOcclusion intensity"].asFloat());
		NewMaterial->SetAmbientOcclusionMapIntensity(Root["Materials"][MaterialsList[i]]["AmbientOcclusionMap intensity"].asFloat());
		NewMaterial->SetRoughnessMapIntensity(Root["Materials"][MaterialsList[i]]["RoughnessMap intensity"].asFloat());
		NewMaterial->SetMetalnessMapIntensity(Root["Materials"][MaterialsList[i]]["MetalnessMap intensity"].asFloat());

		if (Root["Materials"][MaterialsList[i]].isMember("Tiling"))
			NewMaterial->SetTiling(Root["Materials"][MaterialsList[i]]["Tiling"].asFloat());
		NewMaterial->SetCompackPacking(Root["Materials"][MaterialsList[i]]["Compack packing"].asBool());
	}

	// Loading game models.
	std::vector<Json::String> GameModelList = Root["GameModels"].getMemberNames();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["GameModels"][GameModelList[i]]["FEObjectData"]);
		
		FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(RESOURCE_MANAGER.GetMesh(Root["GameModels"][GameModelList[i]]["Mesh"].asCString()),
																	 RESOURCE_MANAGER.GetMaterial(Root["GameModels"][GameModelList[i]]["Material"].asCString()),
																	 LoadedObjectData.Name, LoadedObjectData.ID);
		RESOURCE_MANAGER.SetTag(NewGameModel, LoadedObjectData.Tag);

		NewGameModel->SetScaleFactor(Root["GameModels"][GameModelList[i]]["ScaleFactor"].asFloat());

		bool bHaveLODLevels = Root["GameModels"][GameModelList[i]]["LODs"]["HaveLODlevels"].asBool();
		NewGameModel->SetUsingLOD(bHaveLODLevels);
		if (bHaveLODLevels)
		{
			NewGameModel->SetCullDistance(Root["GameModels"][GameModelList[i]]["LODs"]["CullDistance"].asFloat());
			NewGameModel->SetBillboardZeroRotaion(Root["GameModels"][GameModelList[i]]["LODs"]["Billboard zero rotaion"].asFloat());

			size_t LODCount = Root["GameModels"][GameModelList[i]]["LODs"]["LODCount"].asInt();
			for (size_t j = 0; j < LODCount; j++)
			{
				NewGameModel->SetLODMesh(j, RESOURCE_MANAGER.GetMesh(Root["GameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["Mesh"].asString()));
				NewGameModel->SetLODMaxDrawDistance(j, Root["GameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["Max draw distance"].asFloat());

				bool bLODBillboard = Root["GameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["IsBillboard"].asBool();
				NewGameModel->SetIsLODBillboard(j, bLODBillboard);
				if (bLODBillboard)
					NewGameModel->SetBillboardMaterial(RESOURCE_MANAGER.GetMaterial(Root["GameModels"][GameModelList[i]]["LODs"][std::to_string(j)]["Billboard material"].asString()));
			}
		}
	}

	// Loading prefabs.
	std::unordered_map<std::string, FEPrefab*> PrefabToSceneMap;
	std::vector<Json::String> PrefabList = Root["Prefabs"].getMemberNames();
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Prefabs"][PrefabList[i]]["FEObjectData"]);
		
		FEPrefab*  NewPrefab = RESOURCE_MANAGER.CreatePrefab(LoadedObjectData.Name, LoadedObjectData.ID);
		RESOURCE_MANAGER.SetTag(NewPrefab, LoadedObjectData.Tag);

		// At this point we are just reading scene ID.
		std::string SceneID = Root["Prefabs"][PrefabList[i]]["Scene"]["ID"].asCString();
		// So we need to store them in map and load them later.
		PrefabToSceneMap[SceneID] = NewPrefab;
	}

	// Loading all prefab scenes.
	std::vector<Json::String> SceneList = Root["Scenes"].getMemberNames();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Scenes"][SceneList[i]]["FEObjectData"]);
		bool bPrefabScene = PrefabToSceneMap.find(LoadedObjectData.ID) != PrefabToSceneMap.end();
		if (!bPrefabScene)
			continue;

		FEScene* NewScene = SCENE_MANAGER.CreateScene(LoadedObjectData.Name, LoadedObjectData.ID, FESceneFlag::None);
		RESOURCE_MANAGER.SetTag(NewScene, LoadedObjectData.Tag);

		NewScene->SceneGraph.FromJson(Root["Scenes"][SceneList[i]]["Scene hierarchy"]);
	}

	// Now we can connect prefabs with scenes.
	for (auto& Pair : PrefabToSceneMap)
	{
		Pair.second->SetScene(SCENE_MANAGER.GetScene(Pair.first));
	}
	
	// Only after all prefabs are connected with scenes we can load other scenes.
	std::string MainSceneID = Root["Main scene"].asString();
	for (size_t i = 0; i < SceneList.size(); i++)
	{
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["Scenes"][SceneList[i]]["FEObjectData"]);
		bool bPrefabScene = PrefabToSceneMap.find(LoadedObjectData.ID) != PrefabToSceneMap.end();
		if (bPrefabScene)
			continue;

		bool bShouldBeActive = LoadedObjectData.ID == MainSceneID;
		FEScene* NewScene = SCENE_MANAGER.CreateScene(LoadedObjectData.Name, LoadedObjectData.ID, bShouldBeActive ? FESceneFlag::Active : FESceneFlag::None);
		RESOURCE_MANAGER.SetTag(NewScene, LoadedObjectData.Tag);
		
		NewScene->SceneGraph.FromJson(Root["Scenes"][SceneList[i]]["Scene hierarchy"]);
	}

	ProjectScene = SCENE_MANAGER.GetScene(MainSceneID);

	// VFS
	if (FILE_SYSTEM.DoesFileExist((ProjectFolder + "VFS.txt").c_str())) // _error
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

		std::vector<std::string> StandardShaderList = RESOURCE_MANAGER.GetEnginePrivateShadersList();
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

		std::vector<std::string> StandardShaderList = RESOURCE_MANAGER.GetEnginePrivateShadersList();
		for (size_t i = 0; i < StandardShaderList.size(); i++)
		{
			if (OBJECT_MANAGER.GetFEObject(StandardShaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(StandardShaderList[i]), "/Shaders");
		}

		std::vector<std::string> OtherResourceList = RESOURCE_MANAGER.GetMeshList();
		std::vector<std::string> TextureList = RESOURCE_MANAGER.GetTextureList();
		OtherResourceList.insert(OtherResourceList.end(), TextureList.begin(), TextureList.end());
		std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialList();
		OtherResourceList.insert(OtherResourceList.end(), MaterialList.begin(), MaterialList.end());
		std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelList();
		OtherResourceList.insert(OtherResourceList.end(), GameModelList.begin(), GameModelList.end());

		for (size_t i = 0; i < OtherResourceList.size(); i++)
		{
			if (OBJECT_MANAGER.GetFEObject(OtherResourceList[i])->GetTag() == ENGINE_RESOURCE_TAG ||
				OBJECT_MANAGER.GetFEObject(OtherResourceList[i])->GetTag() == EDITOR_RESOURCE_TAG)
				continue;

			VIRTUAL_FILE_SYSTEM.CreateFile(OBJECT_MANAGER.GetFEObject(OtherResourceList[i]), "/");
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

void FEProject::SetProjectFolder(const std::string NewValue)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(NewValue.c_str()))
		return;

	ProjectFolder = NewValue;
}

void FEProject::SaveSceneTo(const std::string NewPath)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(NewPath.c_str()))
		return;

	SetProjectFolder(NewPath);
	ENGINE.SaveScreenshot((GetProjectFolder() + "projectScreenShot.texture").c_str(), GetScene());
	SaveProject(true);
}

FEScene* FEProject::GetScene()
{
	return ProjectScene;
}

void FEProjectManager::InjectEditorCamera(FEScene* Scene)
{
	FEEntity* CameraEntity = nullptr;
	std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Free camera prefab");
	if (CameraPrefab.size() == 0)
	{
		LOG.Add("FEProjectManager::InjectEditorCamera: Camera prefab not found! Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

		CameraEntity = Scene->CreateEntity("Camera");
		CameraEntity->AddComponent<FECameraComponent>();
	}
	else
	{
		FEPrefab* CameraPrefabToUse = CameraPrefab[0];
		std::vector<FEEntity*> AddedEntities = SCENE_MANAGER.InstantiatePrefab(CameraPrefabToUse, Scene, true);
		if (AddedEntities.empty())
		{
			LOG.Add("FEProjectManager::InjectEditorCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
		else
		{
			CameraEntity = AddedEntities[0];
		}

		if (CameraEntity == nullptr)
		{
			LOG.Add("FEProjectManager::InjectEditorCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
	}

	RESOURCE_MANAGER.SetTag(CameraEntity, EDITOR_RESOURCE_TAG);
	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.SetRenderTargetSize(128, 128);
	CameraComponent.SetDistanceFogEnabled(false);
	CAMERA_SYSTEM.SetMainCamera(CameraEntity);
	FETransformComponent& CameraTransform = CameraEntity->GetComponent<FETransformComponent>();
	CameraTransform.SetPosition(glm::vec3(-4.2269f, 15.7178f, 19.6429f));
}