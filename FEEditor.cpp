#include "FEEditor.h"

FEEditor* FEEditor::Instance = nullptr;

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

		FEEntity* CurrentMainCamera = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.EditorSceneWindows[i]->Scene);
		
		if (ImGui::GetIO().WantCaptureMouse && !EDITOR.EditorSceneWindows[i]->bWindowHovered)
		{
			if (EDITOR.EditorSceneWindows[i]->Scene != nullptr)
			{
				if (CurrentMainCamera != nullptr)
					CAMERA_SYSTEM.SetIsIndividualInputActive(CurrentMainCamera, false);
			}

			continue;
		}

		if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS)
		{
			CAMERA_SYSTEM.SetIsIndividualInputActive(CurrentMainCamera, true);
		}
		else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
		{
			CAMERA_SYSTEM.SetIsIndividualInputActive(CurrentMainCamera, false);
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
					SELECTED.DetermineEntityUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY(), EDITOR.EditorSceneWindows[i]->Scene);
					CurrentSelectionData->CheckForSelectionisNeeded = true;
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
		if (EDITOR.IsInGameMode())
		{
			EDITOR.SetGameMode(false);
		}
		else
		{
			if (PROJECT_MANAGER.GetCurrent() == nullptr)
				ENGINE.Terminate();
			ProjectWasModifiedPopUp::getInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
		}
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
				// Enitity could be deleted in the meantime
				// or it could be from another scene
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
	ENGINE.AddKeyCallback(KeyButtonCallback);
	ENGINE.AddMouseButtonCallback(MouseButtonCallback);
	ENGINE.AddMouseMoveCallback(MouseMoveCallback);
	ENGINE.AddOnViewportResizeCallback(OnViewportResize);
	ENGINE.AddDropCallback(DropCallback);
	
	SELECTED.InitializeResources();
	if (PROJECT_MANAGER.GetCurrent() != nullptr && PROJECT_MANAGER.GetCurrent()->ProjectScene != nullptr)
	{
		FEEntity* MainCamera = CAMERA_SYSTEM.GetMainCameraEntity(PROJECT_MANAGER.GetCurrent()->ProjectScene);
		CAMERA_SYSTEM.SetIsIndividualInputActive(MainCamera, false);
	}
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
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	if (ActiveScenes.empty())
	{
		EditorSceneWindows.clear();
		SetFocusedScene(nullptr);
	}

	DRAG_AND_DROP_MANAGER.Render();

	if (PROJECT_MANAGER.GetCurrent())
	{
		if (bGameMode)
			return;

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
					ENGINE.SaveScreenshot((PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + "projectScreenShot.texture").c_str(), PROJECT_MANAGER.GetCurrent()->GetScene());
				}

				if (ImGui::MenuItem("Save project as..."))
				{
					std::string path;
					FILE_SYSTEM.ShowFolderOpenDialog(path);
					if (!path.empty())
					{
						PROJECT_MANAGER.GetCurrent()->SaveSceneTo(path + "\\");
					}
				}

				if (ImGui::MenuItem("Close project"))
				{
					if (PROJECT_MANAGER.GetCurrent()->IsModified())
					{
						ProjectWasModifiedPopUp::getInstance().Show(PROJECT_MANAGER.GetCurrent(), false);
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
						ProjectWasModifiedPopUp::getInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
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

				if (ImGui::MenuItem("Effects", nullptr, bEffectsWindowVisible))
				{
					bEffectsWindowVisible = !bEffectsWindowVisible;
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

		for (size_t i = 0; i < EditorSceneWindows.size(); i++)
		{
			// Rendeting would be done by RenderAllSubWindows().

			// Check if some window is waiting for removal
			if (EditorSceneWindows[i]->bWaitingForRemoval)
			{
				if (FocusedEditorSceneID == EditorSceneWindows[i]->Scene->GetObjectID())
				{
					FocusedEditorSceneID = "";
					for (size_t j = 0; j < EditorSceneWindows.size(); j++)
					{
						if (EditorSceneWindows[j]->bMain && EditorSceneWindows[j] != EditorSceneWindows[i])
						{
							FocusedEditorSceneID = EditorSceneWindows[j]->Scene->GetObjectID();
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
		DisplayEffectsWindow();
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
		ProjectWasModifiedPopUp::getInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
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
		FEEntity* CameraEntity = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.EditorSceneWindows[i]->Scene);
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
		if (FILE_SYSTEM.CheckDirectory(Paths[i]) && Count == 1)
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

bool FEEditor::IsInGameMode() const
{
	return bGameMode;
}

void FEEditor::SetGameMode(const bool GameMode)
{
	this->bGameMode = GameMode;
}

void FEEditor::DisplayEffectsWindow() const
{
	if (!bEffectsWindowVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Effects settings", nullptr, ImGuiWindowFlags_None);

	int GUIID = 0;
	static float ButtonWidth = 80.0f;
	static float FieldWidth = 250.0f;

	static ImGuiButton* ResetButton = new ImGuiButton("Reset");
	static bool bFirstCall = true;
	if (bFirstCall)
	{
		ResetButton->SetSize(ImVec2(ButtonWidth, 28.0f));
		bFirstCall = false;
	}

	if (ImGui::CollapsingHeader("Sky", 0))
	{
		bool bEnabledSky = SKY_DOME_SYSTEM.IsEnabled();
		if (ImGui::Checkbox("enable sky", &bEnabledSky))
		{
			SKY_DOME_SYSTEM.SetEnabled(bEnabledSky);
		}

		/*ImGui::Text("Sphere size:");
		ImGui::SetNextItemWidth(FieldWidth);
		float size = RENDERER.GetDistanceToSky();
		ImGui::DragFloat("##Sphere size", &size, 0.01f, 0.0f, 200.0f);
		RENDERER.SetDistanceToSky(size);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Update();
		if (ResetButton->IsClicked())
		{
			RENDERER.SetDistanceToSky(50.0f);
		}
		ImGui::PopID();*/
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditor::RenderAllSubWindows()
{
	SELECT_FEOBJECT_POPUP.Render();

	DeleteTexturePopup::getInstance().Render();
	DeleteMeshPopup::getInstance().Render();
	DeleteGameModelPopup::getInstance().Render();
	DeleteMaterialPopup::getInstance().Render();
	DeletePrefabPopup::getInstance().Render();
	DeleteDirectoryPopup::getInstance().Render();

	ResizeTexturePopup::getInstance().Render();
	JustTextWindowObj.Render();

	RenamePopUp::getInstance().Render();
	RenameFailedPopUp::getInstance().Render();
	MessagePopUp::getInstance().Render();
	
	ProjectWasModifiedPopUp::getInstance().Render();

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

void FEEditor::AddEditorScene(FEScene* Scene, bool bMain)
{
	FEEditorSceneWindow* NewSceneWindow = new FEEditorSceneWindow(Scene, bMain);
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

FEScene* FEEditor::GetFocusedScene() const
{
	return SCENE_MANAGER.GetScene(FocusedEditorSceneID);
}

void FEEditor::SetFocusedScene(FEScene* NewSceneInFocus)
{
	if (NewSceneInFocus == nullptr)
	{
		FocusedEditorSceneID = "";
		return;
	}

	BeforeChangeOfFocusedScene(NewSceneInFocus);

	FocusedEditorSceneID = NewSceneInFocus->GetObjectID();
}

void FEEditor::BeforeChangeOfFocusedScene(FEScene* NewSceneInFocus)
{
	if (!FocusedEditorSceneID.empty() && NewSceneInFocus != EDITOR.GetFocusedScene())
	{
		SELECTED.Clear(EDITOR.GetFocusedScene());
		GIZMO_MANAGER.HideAllGizmo(EDITOR.GetFocusedScene());
	}
}