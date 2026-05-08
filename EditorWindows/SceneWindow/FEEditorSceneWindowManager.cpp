#include "FEEditorSceneWindowManager.h"
#include "../../FEEditor.h"
#include "../../SubSystems/ProjectManagment/FEProject.h"

FEEditorSceneWindowManager::FEEditorSceneWindowManager()
{
	ENGINE.AddOnViewportResizeCallback(OnViewportResize);
}

FEEditorSceneWindowManager::~FEEditorSceneWindowManager() {}

void FEEditorSceneWindowManager::CreateSceneWindow(const std::string& SceneID, FEProject* CurrentProject)
{
	FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
	if (Scene == nullptr)
	{
		LOG.Add("FEEditorSceneWindowManager::CreateSceneWindow: Scene not found.", "EDITOR_SCENE_WINDOW_MANAGER", FE_LOG_ERROR);
		return;
	}

	if (CurrentProject == nullptr)
	{
		CurrentProject = PROJECT_MANAGER.GetCurrent();
		if (CurrentProject == nullptr)
		{
			LOG.Add("FEEditorSceneWindowManager::CreateSceneWindow: No project opened.", "EDITOR_SCENE_WINDOW_MANAGER", FE_LOG_ERROR);
			return;
		}
	}

	Scene->SetFlag(FESceneFlag::Active, true);
	Scene->SetFlag(FESceneFlag::Renderable, true);
	// If scene is not in game mode, then it should be flagged as in editor mode.
	if (!Scene->HasFlag(FESceneFlag::GameMode))
		Scene->SetFlag(FESceneFlag::EditorMode, true);

	FEEditorSceneWindow* NewSceneWindow = new FEEditorSceneWindow(Scene);
	// If it is in game mode, then editor camera is not needed.
	if (!Scene->HasFlag(FESceneFlag::GameMode))
		CurrentProject->InjectEditorCamera(Scene);

	RegisterSceneWindow(NewSceneWindow);
}

void FEEditorSceneWindowManager::RegisterSceneWindow(FEEditorSceneWindow* SceneWindow)
{
	if (SceneWindow == nullptr)
		return;

	SceneWindow->SetVisible(true);
	SceneWindows.push_back(SceneWindow);

	if (SeenScenesID.find(SceneWindow->GetScene()->GetObjectID()) == SeenScenesID.end())
	{
		SeenScenesID[SceneWindow->GetScene()->GetObjectID()] = true;
		SceneWindow->bShouldDockToCentralNode = true;
	}
}

void FEEditorSceneWindowManager::DeleteSceneAndCleanup(std::string SceneID)
{
	FEScene* SceneToDelete = SCENE_MANAGER.GetSceneByID(SceneID);
	if (SceneToDelete == nullptr)
	{
		LOG.Add("FEEditorSceneWindowManager::DeleteSceneAndCleanup: Scene to delete not found.", "EDITOR_SCENE_WINDOW_MANAGER", FE_LOG_ERROR);
		return;
	}

	FEEditorSceneWindow* SceneWindow = GetSceneWindow(SceneToDelete->GetObjectID());
	if (SceneWindow != nullptr)
	{
		if (FocusedSceneID == SceneToDelete->GetObjectID())
			FocusedSceneID = "";

		for (size_t i = 0; i < SceneWindows.size(); i++)
		{
			if (SceneWindows[i] == SceneWindow)
			{
				SceneWindows.erase(SceneWindows.begin() + i);
				delete SceneWindow;
				break;
			}
		}
	}

	if (SELECTED.PerSceneData.find(SceneToDelete->GetObjectID()) != SELECTED.PerSceneData.end())
		SELECTED.PerSceneData.erase(SceneToDelete->GetObjectID());

	if (GIZMO_MANAGER.PerSceneData.find(SceneToDelete->GetObjectID()) != GIZMO_MANAGER.PerSceneData.end())
		GIZMO_MANAGER.PerSceneData.erase(SceneToDelete->GetObjectID());

	SCENE_MANAGER.DeleteScene(SceneToDelete->GetObjectID());
}

FEEditorSceneWindow* FEEditorSceneWindowManager::GetSceneWindow(std::string SceneID)
{
	for (size_t i = 0; i < SceneWindows.size(); i++)
	{
		if (SceneWindows[i]->Scene->GetObjectID() == SceneID)
			return SceneWindows[i];
	}

	return nullptr;
}

std::vector<std::string> FEEditorSceneWindowManager::GetOpenedScenesIDs() const
{
	std::vector<std::string> OpenedScenesIDs;
	for (size_t i = 0; i < SceneWindows.size(); i++)
		OpenedScenesIDs.push_back(SceneWindows[i]->Scene->GetObjectID());

	return OpenedScenesIDs;
}

const std::vector<FEEditorSceneWindow*>& FEEditorSceneWindowManager::GetAllSceneWindows() const
{
	return SceneWindows;
}

bool FEEditorSceneWindowManager::SetFocusedScene(FEScene* NewSceneInFocus)
{
	if (NewSceneInFocus == nullptr)
	{
		FocusedSceneID = "";
		return true;
	}

	BeforeChangeOfFocusedScene(NewSceneInFocus);

	return SetFocusedScene(NewSceneInFocus->GetObjectID());
}

bool FEEditorSceneWindowManager::SetFocusedScene(std::string NewSceneInFocusID)
{
	if (SCENE_MANAGER.GetSceneByID(NewSceneInFocusID) == nullptr)
	{
		LOG.Add("FEEditorSceneWindowManager::SetFocusedScene: Scene not found.", "EDITOR_SCENE_WINDOW_MANAGER", FE_LOG_ERROR);
		return false;
	}

	if (GetSceneWindow(NewSceneInFocusID) == nullptr)
	{
		LOG.Add("FEEditorSceneWindowManager::SetFocusedScene: Scene window not found.", "EDITOR_SCENE_WINDOW_MANAGER", FE_LOG_ERROR);
		return false;
	}

	FocusedSceneID = NewSceneInFocusID;
	return true;
}

FEScene* FEEditorSceneWindowManager::GetFocusedScene() const
{
	return SCENE_MANAGER.GetSceneByID(FocusedSceneID);
}

FEEditorSceneWindow* FEEditorSceneWindowManager::GetFocusedSceneWindow() const
{
	for (size_t i = 0; i < SceneWindows.size(); i++)
	{
		if (SceneWindows[i]->Scene != nullptr && SceneWindows[i]->Scene->GetObjectID() == FocusedSceneID)
			return SceneWindows[i];
	}

	return nullptr;
}

void FEEditorSceneWindowManager::MouseButtonCallback(int Button, int Action, int Mods)
{
	for (size_t i = 0; i < SceneWindows.size(); i++)
		SceneWindows[i]->bWindowHovered = false;

	FEEditorSceneWindow* Focused = GetFocusedSceneWindow();
	if (Focused == nullptr)
		return;

	if (ImGui::GetCurrentContext()->HoveredWindow != nullptr && Focused->GetWindow() != nullptr)
		Focused->bWindowHovered = ImGui::GetCurrentContext()->HoveredWindow->Name == Focused->GetWindow()->Name;

	FEEntity* CurrentMainCamera = CAMERA_SYSTEM.GetMainCamera(Focused->Scene);

	if (ImGui::GetIO().WantCaptureMouse && !Focused->bWindowHovered)
	{
		if (Focused->Scene != nullptr && CurrentMainCamera != nullptr)
			CurrentMainCamera->GetComponent<FECameraComponent>().SetActive(false);

		return;
	}

	if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS)
	{
		if (CurrentMainCamera != nullptr)
			CurrentMainCamera->GetComponent<FECameraComponent>().SetActive(true);
	}
	else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
	{
		if (CurrentMainCamera != nullptr)
			CurrentMainCamera->GetComponent<FECameraComponent>().SetActive(false);
	}

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_PRESS && Focused->bWindowHovered)
	{
		bool bEditingTerrain = false;
		if (SELECTED.GetSelected(Focused->Scene) != nullptr && SELECTED.GetSelected(Focused->Scene)->HasComponent<FETerrainComponent>())
			bEditingTerrain = TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE;

		if (!bEditingTerrain)
		{
			FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(Focused->Scene->GetObjectID());
			if (CurrentSelectionData != nullptr)
			{
				SELECTED.DetermineEntityUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY(), Focused->Scene);
				CurrentSelectionData->CheckForSelectionisNeeded = true;
			}
		}
	}

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
		GIZMO_MANAGER.DeactivateAllGizmo(Focused->Scene);
}

void FEEditorSceneWindowManager::KeyButtonCallback(int Key, int Scancode, int Action, int Mods)
{
	FEEditorSceneWindow* Focused = GetFocusedSceneWindow();
	if (Focused == nullptr)
		return;

	FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(Focused->Scene->GetObjectID());
	if (!ImGui::GetIO().WantCaptureKeyboard && Key == GLFW_KEY_DELETE)
	{
		if (SELECTED.GetSelected(Focused->Scene) != nullptr)
		{
			if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1 && SELECTED.GetSelected(Focused->Scene)->HasComponent<FEInstancedComponent>())
			{
				INSTANCED_RENDERING_SYSTEM.DeleteIndividualInstance(SELECTED.GetSelected(Focused->Scene), CurrentSelectionData->InstancedSubObjectIndexSelected);
			}
			else
			{
				Focused->Scene->DeleteEntity(SELECTED.GetSelected(Focused->Scene));
			}

			SELECTED.Clear(Focused->Scene);
			PROJECT_MANAGER.GetCurrent()->SetModified(true);
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_C && Action == GLFW_RELEASE)
	{
		if (SELECTED.GetSelected(Focused->Scene) != nullptr)
			EDITOR.SetSceneEntityIDInClipboard(SELECTED.GetSelected(Focused->Scene)->GetObjectID());
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && (Key == GLFW_KEY_RIGHT_SHIFT || Key == GLFW_KEY_LEFT_SHIFT) && Action == GLFW_RELEASE)
	{
		FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Focused->Scene->GetObjectID());
		int NewState = GizmoSceneData->GizmosState + 1;
		if (NewState > 2)
			NewState = 0;
		GIZMO_MANAGER.UpdateGizmoState(NewState, Focused->Scene);
	}
}

void FEEditorSceneWindowManager::MouseMoveCallback(double Xpos, double Ypos, double LastXpos, double LastYpos)
{
	FEEditorSceneWindow* Focused = GetFocusedSceneWindow();
	if (Focused == nullptr)
		return;

	if (SELECTED.GetSelected(Focused->Scene) == nullptr)
		return;

	if (SELECTED.GetSelected(Focused->Scene)->HasComponent<FETerrainComponent>())
	{
		if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
			return;
	}

	GIZMO_MANAGER.MouseMove(LastXpos, LastYpos, Xpos, Ypos, Focused->Scene);
}

void FEEditorSceneWindowManager::BeforeChangeOfFocusedScene(FEScene* NewSceneInFocus)
{
	if (!FocusedSceneID.empty() && NewSceneInFocus != GetFocusedScene())
	{
		SELECTED.Clear(GetFocusedScene());
		GIZMO_MANAGER.HideAllGizmo(GetFocusedScene());
	}
}

void FEEditorSceneWindowManager::Clear()
{
	SceneWindows.clear();
	FocusedSceneID = "";
}

void FEEditorSceneWindowManager::RemoveUserClosedWindows()
{
	for (size_t i = 0; i < SceneWindows.size(); i++)
	{
		if (!SceneWindows[i]->GetUserRequestedClose())
			continue;

		if (FocusedSceneID == SceneWindows[i]->Scene->GetObjectID())
		{
			FocusedSceneID = "";
			for (size_t j = 0; j < SceneWindows.size(); j++)
			{
				if (SceneWindows[j] != SceneWindows[i])
				{
					SetFocusedScene(SceneWindows[j]->Scene->GetObjectID());
					break;
				}
			}
		}

		delete SceneWindows[i];
		SceneWindows.erase(SceneWindows.begin() + i);
		i--;
	}
}

void FEEditorSceneWindowManager::Update()
{
	RemoveUserClosedWindows();

	const auto& SceneWindows = GetAllSceneWindows();
	for (size_t i = 0; i < SceneWindows.size(); i++)
	{
		if (SceneWindows[i]->Scene != GetFocusedScene())
			continue;

		const int ObjectIndex = SELECTED.GetIndexOfObjectUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY(), SceneWindows[i]->Scene);
		if (ObjectIndex >= 0)
		{
			if (!GIZMO_MANAGER.WasSelected(ObjectIndex, SceneWindows[i]->Scene))
			{
				SELECTED.SetSelectedByIndex(ObjectIndex, SceneWindows[i]->Scene);
			}
		}
	}
}

void FEEditorSceneWindowManager::OnViewportResize(std::string ViewportID)
{
	if (PROJECT_MANAGER.GetCurrent() == nullptr)
		return;

	auto SceneWindows = EDITOR_SCENE_WINDOW_MANAGER.GetAllSceneWindows();
	for (size_t i = 0; i < SceneWindows.size(); i++)
	{
		FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
		if (CurrentProject == nullptr)
			return;

		FEEntity* CameraEntity = nullptr;
		if (SceneWindows[i]->Scene->HasFlag(FESceneFlag::EditorMode))
		{
			std::string EditorCameraID = CurrentProject->GetEditorCameraIDBySceneID(SceneWindows[i]->Scene->GetObjectID());
			CameraEntity = SceneWindows[i]->Scene->GetEntity(EditorCameraID);
		}
		else if (SceneWindows[i]->Scene->HasFlag(FESceneFlag::GameMode))
		{
			CameraEntity = CAMERA_SYSTEM.GetMainCamera(SceneWindows[i]->Scene);
		}

		if (CameraEntity == nullptr)
			continue;

		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		if (CameraComponent.GetViewport()->GetID() == ViewportID)
		{
			SELECTED.UpdateResources(SceneWindows[i]->Scene);
		}
	}
}