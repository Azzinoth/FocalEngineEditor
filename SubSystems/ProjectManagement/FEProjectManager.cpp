#include "FEProjectManager.h"
#include "../../FEEditor.h"

static const std::string ProjectListFile = "Resources/ProjectList.txt";

FEProjectManager::FEProjectManager() {}
FEProjectManager::~FEProjectManager() {}

FEProject* FEProjectManager::GetCurrent()
{
	return Current;
}

void FEProjectManager::SetCurrent(FEProject* Project)
{
	Current = Project;
}

const std::vector<FEProjectInfo*>& FEProjectManager::GetProjectList()
{
	return List;
}

std::string FEProjectManager::GetDefaultProjectsFolder() const
{
	return FILE_SYSTEM.GetNormalizedPath(FILE_SYSTEM.GetCurrentWorkingPath() + "/Projects");
}

void FEProjectManager::LoadProjectList()
{
	bProjectListFileEnabled = true;

	std::ifstream File;
	File.open(ProjectListFile);
	if (File.is_open())
	{
		const std::string FileData((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
		File.close();

		Json::Value Root;
		JSONCPP_STRING Error;
		Json::CharReaderBuilder Builder;
		const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
		if (Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Error) && Root.isObject())
		{
			bListView = Root["ListView"].asBool();
			Sorting = std::clamp(Root["Sorting"].asInt(), 0, 2);

			for (const Json::Value& ProjectData : Root["Projects"])
			{
				const std::string Folder = ProjectData["Folder"].asString();
				if (Folder.empty() || GetProjectInfo(Folder) != nullptr)
					continue;

				FEProjectInfo* NewProject = new FEProjectInfo(Folder);
				NewProject->ID = UNIQUE_ID.FromString(ProjectData["ID"].asString());
				NewProject->Name = ProjectData["Name"].asString();
				NewProject->LastOpenedTime = ProjectData["LastOpened"].asUInt64();
				NewProject->AddedTime = ProjectData["Added"].asUInt64();
				NewProject->Refresh();
				List.push_back(NewProject);
			}
		}
		else
		{
			LOG.Add("FEProjectManager::LoadProjectList: Failed to read \"" + ProjectListFile + "\"", "FE_LOG_LOADING", FE_LOG_ERROR);
		}
	}

	AddProjectsFromFolderToList(GetDefaultProjectsFolder());
	SaveProjectList();
}

void FEProjectManager::SaveProjectList()
{
	if (!bProjectListFileEnabled)
		return;

	Json::Value Root;
	Root["Version"] = PROJECT_LIST_FILE_VERSION;
	Root["ListView"] = bListView;
	Root["Sorting"] = Sorting;

	Json::Value ProjectsData(Json::arrayValue);
	for (size_t i = 0; i < List.size(); i++)
	{
		Json::Value ProjectData;
		ProjectData["Folder"] = List[i]->Folder;
		ProjectData["ID"] = UNIQUE_ID.ToString(List[i]->ID);
		ProjectData["Name"] = List[i]->Name;
		ProjectData["LastOpened"] = static_cast<Json::UInt64>(List[i]->LastOpenedTime);
		ProjectData["Added"] = static_cast<Json::UInt64>(List[i]->AddedTime);
		ProjectsData.append(ProjectData);
	}
	Root["Projects"] = ProjectsData;

	std::ofstream File;
	File.open(ProjectListFile);
	if (!File.is_open())
	{
		LOG.Add("FEProjectManager::SaveProjectList: Failed to write \"" + ProjectListFile + "\"", "FE_LOG_SAVING", FE_LOG_ERROR);
		return;
	}

	Json::StreamWriterBuilder Builder;
	File << Json::writeString(Builder, Root);
	File.close();
}

void FEProjectManager::RefreshProjectList()
{
	for (size_t i = 0; i < List.size(); i++)
		List[i]->Refresh();
}

FEProjectInfo* FEProjectManager::GetProjectInfo(const std::string& ProjectFolder)
{
	for (size_t i = 0; i < List.size(); i++)
	{
		if (FILE_SYSTEM.IsSamePath(List[i]->Folder, ProjectFolder))
			return List[i];
	}

	return nullptr;
}

bool FEProjectManager::IsInProjectList(const FEProjectInfo* Project) const
{
	return Project != nullptr && std::find(List.begin(), List.end(), Project) != List.end();
}

FEProjectInfo* FEProjectManager::AddToProjectList(const std::string& ProjectFolder)
{
	if (!ContainProject(ProjectFolder))
	{
		LOG.Add("FEProjectManager::AddToProjectList: \"" + ProjectFolder + "\" is not a project folder", "FE_LOG_LOADING", FE_LOG_ERROR);
		return nullptr;
	}

	bool bListChanged = false;
	FEProjectInfo* Result = GetProjectInfo(ProjectFolder);
	if (Result == nullptr)
	{
		const FEUUID ProjectID = FEProject::ReadProjectID(ProjectFolder);
		if (UNIQUE_ID.IsNull(ProjectID))
		{
			LOG.Add("FEProjectManager::AddToProjectList: Failed to read Project.txt in \"" + ProjectFolder + "\"", "FE_LOG_LOADING", FE_LOG_ERROR);
			return nullptr;
		}

		for (size_t i = 0; i < List.size(); i++)
		{
			if (!(List[i]->ID == ProjectID))
				continue;

			if (List[i]->bMissing)
			{
				// Project was moved to that folder.
				Result = List[i];
				Result->SetFolder(ProjectFolder);
			}
			else
			{
				// It is a copy of a project from the list, so it gets its own ID.
				if (!FEProject::SetNewIDInProjectFile(ProjectFolder))
				{
					LOG.Add("FEProjectManager::AddToProjectList: Failed to write Project.txt in \"" + ProjectFolder + "\"", "FE_LOG_SAVING", FE_LOG_ERROR);
					return nullptr;
				}
			}

			break;
		}

		if (Result == nullptr)
		{
			Result = new FEProjectInfo(ProjectFolder);
			Result->AddedTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_SECONDS);
			List.push_back(Result);
		}

		bListChanged = true;
	}

	Result->Refresh();
	if (Current != nullptr && FILE_SYSTEM.IsSamePath(Current->GetProjectFolder(), Result->Folder))
	{
		Result->LastOpenedTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_SECONDS);
		bListChanged = true;
	}

	if (bListChanged)
		SaveProjectList();

	return Result;
}

int FEProjectManager::AddProjectsFromFolderToList(const std::string& Folder)
{
	if (ContainProject(Folder))
		return AddToProjectList(Folder) != nullptr ? 1 : 0;

	int Result = 0;
	const std::vector<std::string> SubFolders = FILE_SYSTEM.GetDirectoryList(Folder);
	for (size_t i = 0; i < SubFolders.size(); i++)
	{
		const std::string ProjectFolder = Folder + "/" + SubFolders[i];
		if (!ContainProject(ProjectFolder))
			continue;

		if (GetProjectInfo(ProjectFolder) != nullptr || AddToProjectList(ProjectFolder) != nullptr)
			Result++;
	}

	return Result;
}

bool FEProjectManager::CanBeRemovedFromProjectList(const FEProjectInfo* Project) const
{
	if (!IsInProjectList(Project))
		return false;

	if (Project->bMissing)
		return true;

	return !FILE_SYSTEM.IsSamePath(FILE_SYSTEM.GetDirectoryPath(Project->Folder), GetDefaultProjectsFolder());
}

void FEProjectManager::RemoveEntry(FEProjectInfo* Project)
{
	List.erase(std::remove(List.begin(), List.end(), Project), List.end());
	if (Selected == Project)
		Selected = nullptr;

	if (PopupProject == Project)
		PopupProject = nullptr;

	delete Project;
	SaveProjectList();
}

bool FEProjectManager::RemoveFromProjectList(FEProjectInfo* Project)
{
	if (!CanBeRemovedFromProjectList(Project))
		return false;

	RemoveEntry(Project);
	return true;
}

bool FEProjectManager::DeleteProject(FEProjectInfo* Project)
{
	if (!IsInProjectList(Project))
		return false;

	if (Current != nullptr && FILE_SYSTEM.IsSamePath(Current->GetProjectFolder(), Project->Folder))
	{
		LOG.Add("FEProjectManager::DeleteProject: Project \"" + Project->Name + "\" is opened", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	if (!Project->bMissing)
	{
		// Root directory and folder that contains the editor are never deleted.
		const std::string EditorFolder = FILE_SYSTEM.GetCurrentWorkingPath();
		const bool bRootDirectory = FILE_SYSTEM.IsSamePath(FILE_SYSTEM.GetDirectoryPath(Project->Folder), Project->Folder);
		if (bRootDirectory || FILE_SYSTEM.IsSamePath(EditorFolder, Project->Folder) || FILE_SYSTEM.IsPathInsideDirectory(EditorFolder, Project->Folder))
		{
			LOG.Add("FEProjectManager::DeleteProject: Folder \"" + Project->Folder + "\" can not be deleted", "FE_LOG_GENERAL", FE_LOG_ERROR);
			return false;
		}

		if (!FILE_SYSTEM.MoveToRecycleBin(Project->Folder))
		{
			LOG.Add("FEProjectManager::DeleteProject: Failed to move \"" + Project->Folder + "\" to the Recycle Bin", "FE_LOG_GENERAL", FE_LOG_ERROR);
			return false;
		}
	}

	RemoveEntry(Project);
	return true;
}

bool FEProjectManager::RenameProject(FEProjectInfo* Project, const std::string& NewName)
{
	if (!IsInProjectList(Project) || NewName.empty())
		return false;

	if (!FEProject::SetNameInProjectFile(Project->Folder, NewName))
		return false;

	Project->Name = NewName;
	SaveProjectList();
	return true;
}

bool FEProjectManager::LocateProject(FEProjectInfo* Project, const std::string& NewFolder)
{
	if (!IsInProjectList(Project))
		return false;

	const FEUUID ProjectID = FEProject::ReadProjectID(NewFolder);
	if (UNIQUE_ID.IsNull(ProjectID) || !(ProjectID == Project->ID))
		return false;

	const FEProjectInfo* ProjectOfNewFolder = GetProjectInfo(NewFolder);
	if (ProjectOfNewFolder != nullptr && ProjectOfNewFolder != Project)
		return false;

	Project->SetFolder(NewFolder);
	Project->Refresh();
	SaveProjectList();
	return true;
}

void FEProjectManager::CloseCurrentProject()
{
	// Closing all windows or popups.
	WindowsManager::GetInstance().CloseAllWindows();
	WindowsManager::GetInstance().CloseAllPopups();

	SELECTED.ClearAll();
	GIZMO_MANAGER.ClearAllSceneData();
	PREVIEW_MANAGER.Clear();

	delete Current;
	Current = nullptr;
	RefreshProjectList();

	VIRTUAL_FILE_SYSTEM.SetCurrentPath("/");
	SCENE_MANAGER.Clear();
}

bool FEProjectManager::OpenProject(const std::string& ProjectFolder)
{
	if (!ContainProject(ProjectFolder))
	{
		LOG.Add("FEProjectManager::OpenProject: \"" + ProjectFolder + "\" is not a project folder", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (Current != nullptr)
	{
		LOG.Add("FEProjectManager::OpenProject: Project \"" + Current->GetName() + "\" should be closed first", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	// Thumbnails are needed only by the project list.
	for (size_t i = 0; i < List.size(); i++)
		List[i]->ReleaseThumbnail();

	const std::string Folder = FILE_SYSTEM.GetNormalizedPath(ProjectFolder);
	Current = new FEProject(FILE_SYSTEM.GetFileName(Folder), Folder + "/");
	Current->LoadProject();
	if (Current->bWasJustCreated)
	{
		Current->bWasJustCreated = false;
		FEScene* StartScene = SCENE_MANAGER.GetStartingScene();
		if (StartScene != nullptr)
			EDITOR.CreateEditorWindowForScene(StartScene->GetID(), Current);
	}

	FEProjectInfo* Project = GetProjectInfo(Folder);
	if (Project != nullptr)
	{
		Project->LastOpenedTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_SECONDS);
		SaveProjectList();
	}

	PREVIEW_MANAGER.Clear();
	SELECTED.ClearAll();
	return true;
}

void FEProjectManager::OpenProjectFromList(FEProjectInfo* Project)
{
	if (!IsInProjectList(Project) || Project->bMissing)
		return;

	const std::string ProjectFolder = Project->Folder;
	if (EDITOR.IsRestartNeededForProject(ProjectFolder))
	{
		EDITOR.RestartWithProject(ProjectFolder);
	}
	else
	{
		OpenProject(ProjectFolder);
	}
}

bool FEProjectManager::CreateNewProject(std::string ProjectName, std::string ProjectPath)
{
	if (ProjectName.empty() || ProjectPath.empty())
	{
		LOG.Add("FEProjectManager::CreateNewProject: Project name or path is empty!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (ContainProject(ProjectPath))
	{
		LOG.Add("FEProjectManager::CreateNewProject: Project already exists!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	FEProject* NewProject = new FEProject(ProjectName, ProjectPath);
	FEScene* NewScene = SCENE_MANAGER.CreateScene(ProjectName + "_main_scene", FEUUID(), FESceneFlag::Active);

	FEEntity* SunEntity = NewScene->CreateEntity("Sun");
	SunEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = SunEntity->GetComponent<FELightComponent>();
	SunEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(4.0f);
	LightComponent.SetCastShadows(true);

	FEEntity* SkyDome = NewScene->CreateEntity("SkyDome");
	SkyDome->GetComponent<FETransformComponent>().SetScale(glm::vec3(150.0f));
	SkyDome->AddComponent<FESkyDomeComponent>();

	NewProject->bWasJustCreated = true;
	SCENE_MANAGER.SetStartingScene(NewScene->GetID());
	NewProject->InjectEditorCamera(NewScene);
	NewProject->AddMissingVFSData();
	NewProject->SaveProject(true);

	CloseCurrentProject();
	delete NewProject;
	return true;
}

bool FEProjectManager::ContainProject(const std::string Path)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(Path.c_str()))
		return false;

	if (!FILE_SYSTEM.DoesFileExist((Path + "/Project.txt").c_str()))
		return false;

	return true;
}