#pragma once

#include "../../FEEditorDragAndDropManager.h"
#include "../../FEDearImguiWrapper/FEDearImguiWrapper.h"
#include "../../FEEditorGizmoManager.h"
#include "../../FEEditorPreviewManager.h"
#include "../../FEEditorVirtualFileSystem.h"

#include "FEProjectInfo.h"
#include <functional>

class FEProjectManager
{
	std::vector<FEProjectInfo*> List;
	FEProject* Current = nullptr;
	bool bProjectListFileEnabled = false;
public:
	SINGLETON_PUBLIC_PART(FEProjectManager)
	SINGLETON_PRIVATE_PART(FEProjectManager)

	FEProject* GetCurrent();
	void SetCurrent(FEProject* Project);

	const std::vector<FEProjectInfo*>& GetProjectList();
	void LoadProjectList();
	std::string GetDefaultProjectsFolder() const;

	FEProjectInfo* AddToProjectList(const std::string& ProjectFolder);
	int AddProjectsFromFolderToList(const std::string& Folder);
	// Projects of default projects folder are always in the list.
	bool CanBeRemovedFromProjectList(const FEProjectInfo* Project) const;
	bool RemoveFromProjectList(FEProjectInfo* Project);
	// Folder of the project is moved to the Recycle Bin.
	bool DeleteProject(FEProjectInfo* Project);
	bool RenameProject(FEProjectInfo* Project, const std::string& NewName);
	bool LocateProject(FEProjectInfo* Project, const std::string& NewFolder);

	bool CreateNewProject(std::string ProjectName, std::string ProjectPath);
	bool OpenProject(const std::string& ProjectFolder);
	void CloseCurrentProject();

	void DisplayProjectSelection();
	// Look of the project list for windows that are shown over it.
	void PushProjectListStyle();
	void PopProjectListStyle();

	bool ContainProject(std::string Path);
private:
	FEProjectInfo* GetProjectInfo(const std::string& ProjectFolder);
	bool IsInProjectList(const FEProjectInfo* Project) const;
	void RemoveEntry(FEProjectInfo* Project);
	void SaveProjectList();
	void RefreshProjectList();
	// Project is opened two frames later, the list shows a message in these frames.
	void OpenProjectFromList(FEProjectInfo* Project);
	// Editor is restarted if project needs other plugins.
	void OpenRequestedProject();
	FEProjectInfo* ProjectToOpen = nullptr;
	bool bOpeningMessageShown = false;

	FEProjectInfo* Selected = nullptr;
	bool bListView = false;
	int Sorting = 0;
	char Filter[256] = "";

	std::string RequestedPopup;
	FEProjectInfo* PopupProject = nullptr;
	char PopupName[512] = "";
	char NewProjectLocation[1024] = "";
	std::string PopupError;
	std::string Message;
	// Executed after list was rendered, because it could change the list.
	std::function<void()> DeferredAction;

	std::vector<FEProjectInfo*> GetVisibleProjects();
	void DisplayTopBar(ImVec2 Origin, float Width);
	void DisplayGridView(const std::vector<FEProjectInfo*>& Projects, ImVec2 Origin, float Width, float Height);
	void DisplayListView(const std::vector<FEProjectInfo*>& Projects, ImVec2 Origin, float Width, float Height);
	void DisplayDetailsPanel(ImVec2 Origin, float Height);
	void DisplayBottomBar(ImVec2 Origin, float Width, float Height);
	void DisplayOpeningMessage(ImVec2 Origin, float Width, float Height);
	void DisplayProjectMenu(FEProjectInfo* Project);
	void DisplayPopups();
	void ShowPopup(const std::string& Name, FEProjectInfo* Project);
	void ShowMessage(const std::string& Text);
	void LocateProjectWithDialog(FEProjectInfo* Project);
};

#define PROJECT_LIST_FILE_VERSION 0.01f
#define PROJECT_MANAGER FEProjectManager::GetInstance()