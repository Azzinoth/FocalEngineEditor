#include "FEEditor.h"

FEEditor::FEEditor()
{
	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigWindowsMoveFromTitleBarOnly = true;
	ENGINE.SetVsyncEnabled(true);
	RESOURCE_MANAGER.AddTagThatWillPreventDeletion(EDITOR_RESOURCE_TAG);
}

FEEditor::~FEEditor() {}

double FEEditor::GetLastMouseX() const
{
	return LastMouseX;
}

double FEEditor::GetLastMouseY() const
{
	return LastMouseY;
}

double FEEditor::GetMouseX() const
{
	return MouseX;
}

double FEEditor::GetMouseY() const
{
	return MouseY;
}

void FEEditor::SetLastMouseX(const double NewValue)
{
	LastMouseX = NewValue;
}

void FEEditor::SetLastMouseY(const double NewValue)
{
	LastMouseY = NewValue;
}

void FEEditor::SetMouseX(const double NewValue)
{
	MouseX = NewValue;
}

void FEEditor::SetMouseY(const double NewValue)
{
	MouseY = NewValue;
}

std::string FEEditor::GetSceneEntityIDInClipboard()
{
	return SceneEntityIDInClipboard;
}

void FEEditor::SetSceneEntityIDInClipboard(const std::string NewValue)
{
	SceneEntityIDInClipboard = NewValue;
}

void FEEditor::MouseButtonCallback(const int Button, const int Action, int Mods)
{
	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
		DRAG_AND_DROP_MANAGER.DropAction();

	for (size_t i = 0; i < EDITOR.EditorSceneWindows.size(); i++)
	{
		EDITOR.EditorSceneWindows[i]->bWindowHovered = false;

		if (EDITOR.EditorSceneWindows[i]->Scene != EDITOR.GetFocusedScene())
			continue;

		if (ImGui::GetCurrentContext()->HoveredWindow != nullptr && EDITOR.EditorSceneWindows[i]->GetWindow() != nullptr)
			EDITOR.EditorSceneWindows[i]->bWindowHovered = ImGui::GetCurrentContext()->HoveredWindow->Name == EDITOR.EditorSceneWindows[i]->GetWindow()->Name;

		FEEntity* CurrentMainCamera = CAMERA_SYSTEM.GetMainCamera(EDITOR.EditorSceneWindows[i]->Scene);

		if (ImGui::GetIO().WantCaptureMouse && !EDITOR.EditorSceneWindows[i]->bWindowHovered)
		{
			if (EDITOR.EditorSceneWindows[i]->Scene != nullptr && CurrentMainCamera != nullptr)
			{
				if (CurrentMainCamera != nullptr)
					CurrentMainCamera->GetComponent<FECameraComponent>().SetActive(false);
			}

			continue;
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

		if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_PRESS)
		{
			if (EDITOR.EditorSceneWindows[i]->bWindowHovered)
			{
				bool bEditingTerrain = false;
				if (SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene) != nullptr && SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene)->HasComponent<FETerrainComponent>())
				{
					bEditingTerrain = TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE;
				}

				if (!bEditingTerrain)
				{
					FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(EDITOR.EditorSceneWindows[i]->Scene->GetObjectID());
					if (CurrentSelectionData != nullptr)
					{
						SELECTED.DetermineEntityUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY(), EDITOR.EditorSceneWindows[i]->Scene);
						CurrentSelectionData->CheckForSelectionisNeeded = true;
					}
				}
			}

			INSPECTOR_WINDOW.bLeftMousePressed = true;
		}
		else if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
		{
			INSPECTOR_WINDOW.bLeftMousePressed = false;
			GIZMO_MANAGER.DeactivateAllGizmo(EDITOR.EditorSceneWindows[i]->Scene);
		}
	}
}

void FEEditor::KeyButtonCallback(int Key, int Scancode, int Action, int Mods)
{
	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
	{
		if (PROJECT_MANAGER.GetCurrent() == nullptr)
			ENGINE.Terminate();
		ProjectWasModifiedPopUp::GetInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
	}

	for (size_t i = 0; i < EDITOR.EditorSceneWindows.size(); i++)
	{
		if (EDITOR.EditorSceneWindows[i]->Scene != EDITOR.GetFocusedScene())
			continue;

		FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(EDITOR.EditorSceneWindows[i]->Scene->GetObjectID());
		if (!ImGui::GetIO().WantCaptureKeyboard && Key == GLFW_KEY_DELETE)
		{
			if (SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene) != nullptr)
			{
				if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1 && SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene)->HasComponent<FEInstancedComponent>())
				{
					INSTANCED_RENDERING_SYSTEM.DeleteIndividualInstance(SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene), CurrentSelectionData->InstancedSubObjectIndexSelected);
				}
				else
				{
					if (EDITOR.GetFocusedScene() != nullptr)
					{
						EDITOR.GetFocusedScene()->DeleteEntity(SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene));
					}
				}

				SELECTED.Clear(EDITOR.EditorSceneWindows[i]->Scene);
				PROJECT_MANAGER.GetCurrent()->SetModified(true);
			}
		}

		if (!ImGui::GetIO().WantCaptureKeyboard && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_C && Action == GLFW_RELEASE)
		{
			if (SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene) != nullptr)
				EDITOR.SetSceneEntityIDInClipboard(SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene)->GetObjectID());
		}

		if (!ImGui::GetIO().WantCaptureKeyboard && (Key == GLFW_KEY_RIGHT_SHIFT || Key == GLFW_KEY_LEFT_SHIFT) && Action == GLFW_RELEASE)
		{
			FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(EDITOR.EditorSceneWindows[i]->Scene->GetObjectID());
			int NewState = GizmoSceneData->GizmosState + 1;
			if (NewState > 2)
				NewState = 0;
			GIZMO_MANAGER.UpdateGizmoState(NewState, EDITOR.EditorSceneWindows[i]->Scene);
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_V && Action == GLFW_RELEASE)
	{
		if (!EDITOR.GetSceneEntityIDInClipboard().empty())
		{
			if (EDITOR.GetFocusedScene() != nullptr)
			{
				FEEntity* EntityToDuplicate = EDITOR.GetFocusedScene()->GetEntity(EDITOR.GetSceneEntityIDInClipboard());
				// Skip if entity was deleted or belongs to another scene
				if (EntityToDuplicate != nullptr)
				{
					FENaiveSceneGraphNode* NodeToDuplicate = EDITOR.GetFocusedScene()->SceneGraph.GetNodeByEntityID(EntityToDuplicate->GetObjectID());
					FENaiveSceneGraphNode* DuplicatedNode = EDITOR.GetFocusedScene()->SceneGraph.DuplicateNode(NodeToDuplicate->GetObjectID(), NodeToDuplicate->GetParent()->GetObjectID());
					if (DuplicatedNode != nullptr)
					{
						FEEntity* DuplicatedEntity = DuplicatedNode->GetEntity();
						SELECTED.SetSelected(DuplicatedEntity);
					}
				}
			}
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && (Key == GLFW_KEY_RIGHT_SHIFT || Key == GLFW_KEY_LEFT_SHIFT) && Action == GLFW_RELEASE)
	{
		INSPECTOR_WINDOW.bShiftPressed = false;
	}
	else if (!ImGui::GetIO().WantCaptureKeyboard && (Key == GLFW_KEY_RIGHT_SHIFT || Key == GLFW_KEY_LEFT_SHIFT) && Action == GLFW_PRESS)
	{
		INSPECTOR_WINDOW.bShiftPressed = true;
	}
}

void FEEditor::InitializeResources()
{
	INPUT.AddKeyCallback(KeyButtonCallback);
	INPUT.AddMouseButtonCallback(MouseButtonCallback);
	INPUT.AddMouseMoveCallback(MouseMoveCallback);
	ENGINE.AddOnViewportResizeCallback(OnViewportResize);
	ENGINE.AddDropCallback(DropCallback);
	
	SELECTED.InitializeResources();
	PROJECT_MANAGER.InitializeResources();
	PREVIEW_MANAGER.InitializeResources();
	DRAG_AND_DROP_MANAGER.InitializeResources();
	
	GIZMO_MANAGER.InitializeResources();
	SCENE_GRAPH_WINDOW.InitializeResources();
	CONTENT_BROWSER_WINDOW.InitializeResources();
	INSPECTOR_WINDOW.InitializeResources();
	
	ENGINE.AddOnAfterUpdateCallback(AfterEngineUpdate);
	ENGINE.AddWindowCloseCallback(CloseWindowCallBack);

	SetUpImgui();
}

void FEEditor::MouseMoveCallback(double Xpos, double Ypos)
{
	EDITOR.SetLastMouseX(EDITOR.GetMouseX());
	EDITOR.SetLastMouseY(EDITOR.GetMouseY());

	EDITOR.SetMouseX(Xpos);
	EDITOR.SetMouseY(Ypos);

	DRAG_AND_DROP_MANAGER.MouseMove();

	for (size_t i = 0; i < EDITOR.EditorSceneWindows.size(); i++)
	{
		if (EDITOR.EditorSceneWindows[i]->Scene != EDITOR.GetFocusedScene())
			continue;

		if (SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene) != nullptr)
		{
			if (SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene) != nullptr && SELECTED.GetSelected(EDITOR.EditorSceneWindows[i]->Scene)->HasComponent<FETerrainComponent>())
			{
				if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
					return;
			}

			GIZMO_MANAGER.MouseMove(EDITOR.GetLastMouseX(), EDITOR.GetLastMouseY(), EDITOR.GetMouseX(), EDITOR.GetMouseY(), EDITOR.EditorSceneWindows[i]->Scene);
		}
	}
}

void FEEditor::AfterEngineUpdate()
{
	SELECTED.OnCameraUpdate();
	GIZMO_MANAGER.Update();
}

void FEEditor::Render()
{
	EDITOR_SCRIPTING_SYSTEM.Update();

	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::Renderable);
	if (ActiveScenes.empty())
	{
		EditorSceneWindows.clear();
		SetFocusedScene(nullptr);
	}

	DRAG_AND_DROP_MANAGER.Render();

	if (PROJECT_MANAGER.GetCurrent())
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		DockspaceID = ImGui::GetMainViewport()->ID;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save project"))
				{
					PROJECT_MANAGER.GetCurrent()->SaveProject();
				}

				if (ImGui::MenuItem("Save project as..."))
				{
					std::string Path;
					FILE_SYSTEM.ShowFolderOpenDialog(Path);
					if (!Path.empty())
					{
						PROJECT_MANAGER.GetCurrent()->SaveProjectTo(Path + "\\");
					}
				}

				if (ImGui::MenuItem("Close project"))
				{
					if (PROJECT_MANAGER.GetCurrent()->IsModified())
					{
						ProjectWasModifiedPopUp::GetInstance().Show(PROJECT_MANAGER.GetCurrent(), false);
					}
					else
					{
						OnProjectClose();

						ImGui::PopStyleVar();
						ImGui::EndMenu();
						ImGui::EndMainMenuBar();

						return;
					}
				}

				if (ImGui::MenuItem("Exit"))
				{
					if (PROJECT_MANAGER.GetCurrent()->IsModified())
					{
						APPLICATION.GetMainWindow()->CancelClose();
						ProjectWasModifiedPopUp::GetInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
					}
					else
					{
						OnProjectClose();
						ENGINE.Terminate();
						return;
					}
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Scene Entities", nullptr, SCENE_GRAPH_WINDOW.bVisible))
				{
					SCENE_GRAPH_WINDOW.bVisible = !SCENE_GRAPH_WINDOW.bVisible;
				}

				if (ImGui::MenuItem("Inspector", nullptr, INSPECTOR_WINDOW.bVisible))
				{
					INSPECTOR_WINDOW.bVisible = !INSPECTOR_WINDOW.bVisible;
				}

				if (ImGui::MenuItem("Content Browser", nullptr, CONTENT_BROWSER_WINDOW.bVisible))
				{
					CONTENT_BROWSER_WINDOW.bVisible = !CONTENT_BROWSER_WINDOW.bVisible;
				}

				if (ImGui::MenuItem("Effects", nullptr, bEditorCamerasWindowVisible))
				{
					bEditorCamerasWindowVisible = !bEditorCamerasWindowVisible;
				}

				if (ImGui::MenuItem("Log", nullptr, bLogWindowVisible))
				{
					bLogWindowVisible = !bLogWindowVisible;
				}

				if (ImGui::BeginMenu("Debug"))
				{
					auto PossibleWindows = RENDERER.GetDebugOutputTextures();

					auto iterator = PossibleWindows.begin();
					while (iterator != PossibleWindows.end())
					{
						if (iterator->second != nullptr)
						{
							FEImGuiWindow* CurrentWindow = FE_IMGUI_WINDOW_MANAGER.GetWindowByCaption(iterator->first.c_str());
							bool bVisible = false;
							if (CurrentWindow != nullptr)
								bVisible = CurrentWindow->IsVisible();
							
							if (ImGui::MenuItem(iterator->first.c_str(), nullptr, bVisible))
							{
								bVisible = !bVisible;

								if (bVisible)
								{
									if (CurrentWindow == nullptr)
									{
										CurrentWindow = new DebugTextureViewWindow(iterator->second);
										CurrentWindow->SetCaption(iterator->first);
										CurrentWindow->Show();
									}
									else
									{
										CurrentWindow->Show();
									}
								}
								else
								{
									if (CurrentWindow != nullptr)
										CurrentWindow->SetVisible(false);
								}
							}
						}
						iterator++;
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
		ImGui::PopStyleVar();

		bool bFocusedSceneCouldBeUsedForGameMode = false;
		if (EDITOR.GetFocusedScene() != nullptr)
		{
			FEEditorSceneWindow* SceneWindow = GetEditorSceneWindow(EDITOR.GetFocusedScene()->GetObjectID());
			// Focused scene could be prefab scene
			if (!PREFAB_EDITOR_MANAGER.IsEditorWindowIsPrefabWindow(SceneWindow))
				bFocusedSceneCouldBeUsedForGameMode = true;
		}


		std::string ButtonText = "Run game mode";
		
		if (EDITOR.IsInGameMode())
		{
			ButtonText = "Exit game mode";
		}
		else
		{
			if (!bFocusedSceneCouldBeUsedForGameMode)
			{
				ButtonText = "Can not run game mode (focused scene is prefab scene).";
			}
		}

		if (!bFocusedSceneCouldBeUsedForGameMode)
			ImGui::BeginDisabled();

		if (ImGui::Button(ButtonText.c_str(), ImVec2(220, 0)))
		{
			EDITOR.SetGameMode(!EDITOR.IsInGameMode());
		}

		if (!bFocusedSceneCouldBeUsedForGameMode)
			ImGui::EndDisabled();

		for (size_t i = 0; i < EditorSceneWindows.size(); i++)
		{
			// Rendeting would be done by RenderAllSubWindows().
			// 
			// Check if some window is waiting for removal
			if (EditorSceneWindows[i]->bWaitingForRemoval)
			{
				if (FocusedEditorSceneID == EditorSceneWindows[i]->Scene->GetObjectID())
				{
					FocusedEditorSceneID = "";
					for (size_t j = 0; j < EditorSceneWindows.size(); j++)
					{
						if (EditorSceneWindows[j] != EditorSceneWindows[i])
						{
							SetFocusedScene(EditorSceneWindows[j]->Scene->GetObjectID());
							break;
						}
					}
				}
				
				FEScene* Scene = EditorSceneWindows[i]->Scene;
				delete EditorSceneWindows[i];
				EditorSceneWindows.erase(EditorSceneWindows.begin() + i);
				i--;
				continue;
			}
		}

		SCENE_GRAPH_WINDOW.Render();
		CONTENT_BROWSER_WINDOW.Render();
		INSPECTOR_WINDOW.Render();
		DisplayEditorCamerasWindow();
		DisplayLogWindow();
		if (!GyzmosSettingsWindowObject.IsVisible())
			GyzmosSettingsWindowObject.Show();
		GyzmosSettingsWindowObject.Render();

		for (size_t i = 0; i < EditorSceneWindows.size(); i++)
		{
			if (EditorSceneWindows[i]->Scene != EDITOR.GetFocusedScene())
				continue;

			const int ObjectIndex = SELECTED.GetIndexOfObjectUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY(), EditorSceneWindows[i]->Scene);
			if (ObjectIndex >= 0)
			{
				if (!GIZMO_MANAGER.WasSelected(ObjectIndex, EditorSceneWindows[i]->Scene))
				{
					SELECTED.SetSelectedByIndex(ObjectIndex, EditorSceneWindows[i]->Scene);
				}
			}
		}

		RenderAllSubWindows();
	}
	else
	{
		PROJECT_MANAGER.DisplayProjectSelection();
	}
}

void FEEditor::CloseWindowCallBack()
{
	if (PROJECT_MANAGER.GetCurrent() == nullptr)
	{
		ENGINE.Terminate();
		return;
	}

	if (PROJECT_MANAGER.GetCurrent()->IsModified())
	{
		APPLICATION.GetMainWindow()->CancelClose();
		ProjectWasModifiedPopUp::GetInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
	}
	else
	{
		EDITOR.OnProjectClose();
		ENGINE.Terminate();
		return;
	}
}

void FEEditor::OnViewportResize(std::string ViewportID)
{
	if (PROJECT_MANAGER.GetCurrent() == nullptr)
		return;

	for (size_t i = 0; i < EDITOR.EditorSceneWindows.size(); i++)
	{
		FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
		if (CurrentProject == nullptr)
			return;

		FEEntity* CameraEntity = nullptr;
		if (EDITOR.EditorSceneWindows[i]->Scene->HasFlag(FESceneFlag::EditorMode))
		{
			std::string EditorCameraID = CurrentProject->GetEditorCameraIDBySceneID(EDITOR.EditorSceneWindows[i]->Scene->GetObjectID());
			CameraEntity = EDITOR.EditorSceneWindows[i]->Scene->GetEntity(EditorCameraID);
		}
		else if (EDITOR.EditorSceneWindows[i]->Scene->HasFlag(FESceneFlag::GameMode))
		{
			CameraEntity = CAMERA_SYSTEM.GetMainCamera(EDITOR.EditorSceneWindows[i]->Scene);
		}

		if (CameraEntity == nullptr)
			continue;

		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		if (CameraComponent.GetViewport()->GetID() == ViewportID)
		{
			SELECTED.UpdateResources(EDITOR.EditorSceneWindows[i]->Scene);
		}
	}
}

void FEEditor::DropCallback(const int Count, const char** Paths)
{
	for (size_t i = 0; i < size_t(Count); i++)
	{
		if (FILE_SYSTEM.DoesDirectoryExist(Paths[i]) && Count == 1)
		{
			if (PROJECT_MANAGER.GetCurrent() == nullptr)
			{
				PROJECT_MANAGER.SetProjectsFolder(Paths[i]);
			}
		}

		if (PROJECT_MANAGER.GetCurrent() != nullptr)
		{
			if (EDITOR.GetFocusedScene() != nullptr)
			{
				std::vector<FEObject*> LoadedObjects = EDITOR.GetFocusedScene()->ImportAsset(Paths[i]);
				for (size_t j = 0; j < LoadedObjects.size(); j++)
				{
					if (LoadedObjects[j] != nullptr)
					{
						if (LoadedObjects[j]->GetType() == FE_ENTITY)
						{
							//SCENE.CreateEntity(reinterpret_cast<FEEntity*>(LoadedObjects[j]));
						}
						else
						{
							VIRTUAL_FILE_SYSTEM.CreateFile(LoadedObjects[j], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
							PROJECT_MANAGER.GetCurrent()->SetModified(true);
							PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(LoadedObjects[j]);
						}
					}
				}
			}
		}
	}
}

void FEEditor::DisplayEditorCamerasWindow() const
{
	if (!bEditorCamerasWindowVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Editor Cameras", nullptr, ImGuiWindowFlags_None);

	auto EditorCameraIterator = PROJECT_MANAGER.GetCurrent()->SceneIDToEditorCameraID.begin();
	while (EditorCameraIterator != PROJECT_MANAGER.GetCurrent()->SceneIDToEditorCameraID.end())
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

		if (ImGui::CollapsingHeader(CameraEntity->GetObjectID().c_str(), 0))
		{
			INSPECTOR_WINDOW.DisplayCameraProperties(CameraEntity);
		}

		EditorCameraIterator++;
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditor::RenderAllSubWindows()
{
	SELECT_FEOBJECT_POPUP.Render();

	DeleteTexturePopup::GetInstance().Render();
	DeleteMeshPopup::GetInstance().Render();
	DeleteGameModelPopup::GetInstance().Render();
	DeleteMaterialPopup::GetInstance().Render();
	DeletePrefabPopup::GetInstance().Render();
	DeleteDirectoryPopup::GetInstance().Render();

	ResizeTexturePopup::GetInstance().Render();
	JustTextWindowObj.Render();

	RenamePopUp::GetInstance().Render();
	RenameFailedPopUp::GetInstance().Render();
	MessagePopUp::GetInstance().Render();
	
	ProjectWasModifiedPopUp::GetInstance().Render();

	FE_IMGUI_WINDOW_MANAGER.RenderAllWindows();
}

void FEEditor::SetImguiStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	style->WindowRounding = 2.0f;
	style->ScrollbarRounding = 3.0f;
	style->GrabRounding = 2.0f;
	style->AntiAliasedLines = true;
	style->AntiAliasedFill = true;
	style->WindowRounding = 2;
	style->ChildRounding = 2;
	style->ScrollbarSize = 16;
	style->ScrollbarRounding = 3;
	style->GrabRounding = 2;
	style->ItemSpacing.x = 10;
	style->ItemSpacing.y = 4;
	style->IndentSpacing = 22;
	style->FramePadding.x = 6;
	style->FramePadding.y = 4;
	style->Alpha = 1.0f;
	style->FrameRounding = 3.0f;

	colors[ImGuiCol_Text] = ImVec4(1.0f, 243.0f / 255.0f, 1.0f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(158.0f / 255.0f, 158.0f / 255.0f, 158.0f / 255.0f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(43.0f / 255.0f, 43.0f / 255.0f, 43.0f / 255.0f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 0.98f);

	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_Border] = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.47f, 0.83f, 1.00f);

	colors[ImGuiCol_MenuBarBg] = ImVec4(92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.184f, 0.407f, 0.193f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.71f, 0.78f, 0.69f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.725f, 0.805f, 0.702f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.793f, 0.900f, 0.836f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.71f, 0.78f, 0.69f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.71f, 0.78f, 0.69f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.71f, 0.78f, 0.69f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
}

void FEEditor::SetUpImgui()
{
	ImGuiIO& io = ImGui::GetIO();

	std::string ResourcesFolder = "Resources\\";

	const size_t PathLen = strlen((ResourcesFolder + "imgui.ini").c_str()) + 1;
	char* ImguiIniFile = new char[PathLen];
	strcpy_s(ImguiIniFile, PathLen, (ResourcesFolder + "imgui.ini").c_str());
	io.IniFilename = ImguiIniFile;
	io.Fonts->AddFontFromFileTTF((ResourcesFolder + "Cousine-Regular.ttf").c_str(), 20);
	io.Fonts->AddFontFromFileTTF((ResourcesFolder + "Cousine-Regular.ttf").c_str(), 32);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	unsigned char* TexPixels = nullptr;
	int TexW, TexH;
	io.Fonts->GetTexDataAsRGBA32(&TexPixels, &TexW, &TexH);

	io.DisplaySize = ImVec2(static_cast<float>(APPLICATION.GetMainWindow()->GetWidth()), static_cast<float>(APPLICATION.GetMainWindow()->GetHeight()));
	ImGui::StyleColorsDark();

	SetImguiStyle();
}

void FEEditor::OnProjectClose()
{
	EDITOR_MATERIAL_WINDOW.Stop();
	EditorSceneWindows.clear();
	SetFocusedScene(nullptr);

	PROJECT_MANAGER.CloseCurrentProject();
	CONTENT_BROWSER_WINDOW.Clear();
	SCENE_GRAPH_WINDOW.Clear();
	PREFAB_EDITOR_MANAGER.Clear();
}

FEEditorSceneWindow* FEEditor::GetEditorSceneWindow(std::string SceneID)
{
	for (size_t i = 0; i < EditorSceneWindows.size(); i++)
	{
		if (EditorSceneWindows[i]->Scene->GetObjectID() == SceneID)
			return EditorSceneWindows[i];
	}

	return nullptr;
}

void FEEditor::AddEditorScene(FEScene* Scene)
{
	FEEditorSceneWindow* NewSceneWindow = new FEEditorSceneWindow(Scene);
	NewSceneWindow->SetVisible(true);
	EditorSceneWindows.push_back(NewSceneWindow);
}

void FEEditor::AddCustomEditorScene(FEEditorSceneWindow* SceneWindow)
{
	if (SceneWindow == nullptr)
		return;

	SceneWindow->SetVisible(true);
	EditorSceneWindows.push_back(SceneWindow);
}

void FEEditor::BeforeChangeOfFocusedScene(FEScene* NewSceneInFocus)
{
	if (!FocusedEditorSceneID.empty() && NewSceneInFocus != EDITOR.GetFocusedScene())
	{
		SELECTED.Clear(EDITOR.GetFocusedScene());
		GIZMO_MANAGER.HideAllGizmo(EDITOR.GetFocusedScene());
	}
}

bool FEEditor::IsInGameMode() const
{
	return bGameMode;
}

void FEEditor::SetGameMode(const bool GameMode)
{
	if (SetGameModeInternal(GameMode))
		bGameMode = GameMode;
}

bool FEEditor::DuplicateScenesForGameMode()
{
	if (EDITOR.GetFocusedScene() == nullptr)
	{
		LOG.Add("FEEditor::DuplicateScenesForGameMode: No scene to duplicate for game mode.", "FE_EDITOR_GAME_MODE", FE_LOG_ERROR);
		return false;
	}

	// Here we are setting flag right away, because we want to make sure that scripts are started.
	FEScene* GameModeScene = SCENE_MANAGER.DuplicateScene(EDITOR.GetFocusedScene()->GetObjectID(), "GameMode", [](FEEntity* EntityToCheck) {
		return !(EntityToCheck->GetTag() == EDITOR_RESOURCE_TAG);
	}, FESceneFlag::Active | FESceneFlag::GameMode | FESceneFlag::Renderable);

	if (GameModeScene == nullptr)
	{
		LOG.Add("FEEditor::DuplicateScenesForGameMode: Failed to duplicate scene for game mode.", "FE_EDITOR_GAME_MODE", FE_LOG_ERROR);
		return false;
	}

	ParentIDToScenesInGameMode[EDITOR.GetFocusedScene()->GetObjectID()] = GameModeScene;

	EDITOR.AddEditorScene(GameModeScene);

	return true;
}

bool FEEditor::SetGameModeInternal(bool GameMode)
{
	if (EDITOR.IsInGameMode() == GameMode)
		return false;

	if (EDITOR.GetFocusedScene() == nullptr)
	{
		LOG.Add("FEEditor::SetGameModeInternal: No scene to duplicate for game mode.", "FE_EDITOR_GAME_MODE", FE_LOG_ERROR);
		return false;
	}

	if (GameMode)
	{
		if (!DuplicateScenesForGameMode())
		{
			LOG.Add("FEEditor::SetGameModeInternal: Failed to set game mode.", "FE_EDITOR_GAME_MODE", FE_LOG_ERROR);
			return false;
		}
	}
	else
	{
		auto SceneIterator = ParentIDToScenesInGameMode.begin();
		while (SceneIterator != ParentIDToScenesInGameMode.end())
		{
			DeleteScene(SceneIterator->second->GetObjectID());
			SceneIterator = ParentIDToScenesInGameMode.erase(SceneIterator);
		}
	}

	return true;
}

void FEEditor::DeleteScene(std::string SceneID)
{
	FEScene* SceneToDelete = SCENE_MANAGER.GetScene(SceneID);
	if (SceneToDelete == nullptr)
	{
		LOG.Add("FEEditor::DeleteScene: Scene to delete not found.", "FE_EDITOR", FE_LOG_ERROR);
		return;
	}

	FEEditorSceneWindow* SceneWindow = GetEditorSceneWindow(SceneToDelete->GetObjectID());
	if (SceneWindow != nullptr)
	{
		if (EDITOR.FocusedEditorSceneID == SceneToDelete->GetObjectID())
			EDITOR.FocusedEditorSceneID = "";

		for (size_t i = 0; i < EditorSceneWindows.size(); i++)
		{
			if (EditorSceneWindows[i] == SceneWindow)
			{
				EditorSceneWindows.erase(EditorSceneWindows.begin() + i);
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

std::vector<std::string> FEEditor::GetEditorOpenedScenesIDs() const
{
	std::vector<std::string> OpenedScenesIDs;
	for (size_t i = 0; i < EditorSceneWindows.size(); i++)
	{
		OpenedScenesIDs.push_back(EditorSceneWindows[i]->Scene->GetObjectID());
	}

	return OpenedScenesIDs;
}

FEScene* FEEditor::GetFocusedScene() const
{
	return SCENE_MANAGER.GetScene(FocusedEditorSceneID);
}

bool FEEditor::SetFocusedScene(FEScene* NewSceneInFocus)
{
	if (NewSceneInFocus == nullptr)
	{
		FocusedEditorSceneID = "";
		return true;
	}

	BeforeChangeOfFocusedScene(NewSceneInFocus);

	return SetFocusedScene(NewSceneInFocus->GetObjectID());
}

bool FEEditor::SetFocusedScene(std::string NewSceneInFocusID)
{
	if (SCENE_MANAGER.GetScene(NewSceneInFocusID) == nullptr)
	{
		LOG.Add("FEEditor::SetFocusedEditorScene: Scene not found.", "FE_EDITOR", FE_LOG_ERROR);
		return false;
	}

	if (GetEditorSceneWindow(NewSceneInFocusID) == nullptr)
	{
		LOG.Add("FEEditor::SetFocusedEditorScene: Scene window not found.", "FE_EDITOR", FE_LOG_ERROR);
		return false;
	}

	FocusedEditorSceneID = NewSceneInFocusID;
	return true;
}

void FEEditor::UpdateBeforeRender()
{
	// Before rendering, we need to ensure that if scene is in editor mode, it's main camera would be editor camera.
	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject != nullptr)
	{
		std::vector<FEScene*> Scenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::Renderable | FESceneFlag::EditorMode);
		for (size_t i = 0; i < Scenes.size(); i++)
		{
			FEEntity* CurrentMainCameraEntity = CAMERA_SYSTEM.GetMainCamera(Scenes[i]);
			std::string EditorCameraID = CurrentProject->GetEditorCameraIDBySceneID(Scenes[i]->GetObjectID());
			if (!EditorCameraID.empty())
			{
				if (CurrentMainCameraEntity != nullptr && CurrentMainCameraEntity->GetObjectID() != EditorCameraID)
				{
					//SceneIDToOldMainCameraID[Scenes[i]->GetObjectID()] = CurrentMainCameraEntity->GetObjectID();
					//CurrentProject->SceneIDToProperMainCameraID[Scenes[i]->GetObjectID()] = EditorCameraID;
					CAMERA_SYSTEM.SetMainCamera(Scenes[i]->GetEntity(EditorCameraID));
				}
			}
		}
	}
}