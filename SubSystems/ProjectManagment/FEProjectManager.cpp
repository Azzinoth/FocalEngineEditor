#include "FEProjectManager.h"
#include "../../FEEditor.h"

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
	// Closing all windows or popups.
	WindowsManager::GetInstance().CloseAllWindows();
	WindowsManager::GetInstance().CloseAllPopups();

	SELECTED.ClearAll();
	PREVIEW_MANAGER.Clear();
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
	if (ProjectIndex < 0 || ProjectIndex >= static_cast<int>(List.size()))
	{
		LOG.Add("FEProjectManager::OpenProject: Project index is out of range!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	PROJECT_MANAGER.SetCurrent(List[ProjectIndex]);
	PROJECT_MANAGER.GetCurrent()->LoadProject();
	IndexChosen = -1;

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

	FEProject* NewProject = new FEProject(ProjectName, ProjectPath);
	FEScene* NewScene = SCENE_MANAGER.CreateScene(ProjectName + "_main_scene", "", FESceneFlag::Active);
	
	FEEntity* SunEntity = NewScene->CreateEntity("Sun");
	SunEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = SunEntity->GetComponent<FELightComponent>();
	SunEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(4.0f);
	LightComponent.SetCastShadows(true);

	FEEntity* SkyDome = NewScene->CreateEntity("SkyDome");
	SkyDome->GetComponent<FETransformComponent>().SetScale(glm::vec3(150.0f));
	SkyDome->AddComponent<FESkyDomeComponent>();

	EDITOR.AddEditorScene(NewScene);
	NewProject->InjectEditorCamera(NewScene);
	NewProject->AddMissingVFSData();
	NewProject->SaveProject(true);

	CloseCurrentProject();
	delete NewProject;
}

bool FEProjectManager::ContainProject(const std::string Path)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(Path.c_str()))
		return false;

	if (!FILE_SYSTEM.DoesFileExist((Path + "/Project.txt").c_str()))
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