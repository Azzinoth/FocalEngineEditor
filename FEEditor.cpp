#include "FEEditor.h"

FEEditor* FEEditor::Instance = nullptr;
ImGuiWindow* FEEditor::SceneWindow = nullptr;

bool SceneWindowDragAndDropCallBack(FEObject* Object, void** UserData)
{
	if (Object->GetType() == FE_PREFAB)
	{
		// FIX ME!
		FEEntity* NewEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefab(Object->GetObjectID()));
		FENewEntity* NewNewEntity = SCENE.GetNewStyleEntityByOldStyleID(NewEntity->GetObjectID());
		NewNewEntity->GetComponent<FETransformComponent>().SetPosition(ENGINE.GetCamera()->GetPosition() + ENGINE.GetCamera()->GetForward() * 10.0f);
		//NewEntity->Transform.SetPosition(ENGINE.GetCamera()->GetPosition() + ENGINE.GetCamera()->GetForward() * 10.0f);
		SELECTED.SetSelected(NewNewEntity);
		PROJECT_MANAGER.GetCurrent()->SetModified(true);

		return true;
	}

	return false;
}

FEEditor::FEEditor()
{
	ENGINE.SetRenderTargetMode(FE_CUSTOM_MODE);
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	if (ENGINE.GetCamera()->GetCameraType() == 1)
		ENGINE.RenderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.GetCamera()));
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
	if (ImGui::GetCurrentContext()->HoveredWindow != nullptr && FEEditor::SceneWindow != nullptr)
	{
		EDITOR.bSceneWindowHovered = ImGui::GetCurrentContext()->HoveredWindow->Name == EDITOR.SceneWindow->Name;
	}
	else
	{
		EDITOR.bSceneWindowHovered = false;
	}
	
	if (FEEditor::SceneWindow == nullptr || !FEEditor::SceneWindow->Active)
		EDITOR.bSceneWindowHovered = false;

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
		DRAG_AND_DROP_MANAGER.DropAction();

	if (ImGui::GetIO().WantCaptureMouse && !EDITOR.bSceneWindowHovered)
	{
		EDITOR.bIsCameraInputActive = false;
		ENGINE.GetCamera()->SetIsInputActive(false);

		return;
	}

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_PRESS)
	{
		bool bEditingTerrain = false;
		if (SELECTED.GetTerrain() != nullptr)
		{
			bEditingTerrain = SELECTED.GetTerrain()->GetBrushMode() != FE_TERRAIN_BRUSH_NONE;
		}
		
		if (!bEditingTerrain)
		{
			SELECTED.DetermineEntityUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY());
			SELECTED.CheckForSelectionisNeeded = true;
		}
		
		INSPECTOR_WINDOW.bLeftMousePressed = true;
	}
	else if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
	{
		INSPECTOR_WINDOW.bLeftMousePressed = false;
		GIZMO_MANAGER.DeactivateAllGizmo();
	}

	if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS)
	{
		EDITOR.bIsCameraInputActive = true;
		ENGINE.GetCamera()->SetIsInputActive(true);
	}
	else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
	{
		EDITOR.bIsCameraInputActive = false;
		ENGINE.GetCamera()->SetIsInputActive(false);
	}
}

void FEEditor::KeyButtonCallback(int Key, int Scancode, int Action, int Mods)
{
	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
	{
		if (FEEditor::getInstance().IsInGameMode())
		{
			FEEditor::getInstance().SetGameMode(false);
		}
		else
		{
			if (PROJECT_MANAGER.GetCurrent() == nullptr)
				ENGINE.Terminate();
			ProjectWasModifiedPopUp::getInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && Key == GLFW_KEY_DELETE)
	{
		if (SELECTED.GetSelected() != nullptr && SELECTED.GetSelected()->GetType() == FE_ENTITY_INSTANCED)
		{
			if (SELECTED.InstancedSubObjectIndexSelected != -1)
			{
				FEEntityInstanced* SelectedEntityInstanced = SCENE.GetEntityInstanced(SELECTED.GetSelected()->GetObjectID());
				SelectedEntityInstanced->DeleteInstance(SELECTED.InstancedSubObjectIndexSelected);
				SELECTED.Clear();
				PROJECT_MANAGER.GetCurrent()->SetModified(true);
			}
		}

		if (SELECTED.GetEntity() != nullptr)
		{
			SCENE.DeleteEntity(SELECTED.GetEntity()->GetObjectID());
			SELECTED.Clear();
			PROJECT_MANAGER.GetCurrent()->SetModified(true);
		}
		else if (SELECTED.GetTerrain() != nullptr)
		{
			SCENE.DeleteTerrain(SELECTED.GetTerrain()->GetObjectID());
			SELECTED.Clear();
			PROJECT_MANAGER.GetCurrent()->SetModified(true);
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_C && Action == GLFW_RELEASE)
	{
		if (SELECTED.GetSelected() != nullptr)
			EDITOR.SetSceneEntityIDInClipboard(SELECTED.GetSelected()->GetObjectID());
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_V && Action == GLFW_RELEASE)
	{
		if (!EDITOR.GetSceneEntityIDInClipboard().empty())
		{
			// FIX ME!
			// There should be proper FEScene::DuplicateNewStyleEntity that will duplicate the entity and all its components
			// Also place it in same scene graph node as the original entity ?
			FEEntity* NewEntity = SCENE.AddEntity(SCENE.GetEntity(EDITOR.GetSceneEntityIDInClipboard())->Prefab, "");
			FENewEntity* NewNewEntity = SCENE.GetNewStyleEntityByOldStyleID(NewEntity->GetObjectID());
			FENewEntity* EntityToCopy = SCENE.GetNewStyleEntity(EDITOR.GetSceneEntityIDInClipboard());
			NewNewEntity->GetComponent<FETransformComponent>() = EntityToCopy->GetComponent<FETransformComponent>();
			NewNewEntity->GetComponent<FETransformComponent>().SetPosition(EntityToCopy->GetComponent<FETransformComponent>().GetPosition() * 1.1f);
			
			SELECTED.SetSelected(NewNewEntity);
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && (Key == GLFW_KEY_RIGHT_SHIFT || Key == GLFW_KEY_LEFT_SHIFT) && Action == GLFW_RELEASE)
	{
		int NewState = GIZMO_MANAGER.GizmosState + 1;
		if (NewState > 2)
			NewState = 0;
		GIZMO_MANAGER.UpdateGizmoState(NewState);
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
	ENGINE.AddRenderTargetResizeCallback(RenderTargetResizeCallback);
	ENGINE.AddDropCallback(DropCallback);
	
	SELECTED.InitializeResources();
	ENGINE.GetCamera()->SetIsInputActive(bIsCameraInputActive);
	PROJECT_MANAGER.InitializeResources();
	PREVIEW_MANAGER.InitializeResources();
	DRAG_AND_DROP_MANAGER.InitializeResources();
	SceneWindowTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_PREFAB, SceneWindowDragAndDropCallBack, nullptr, "Drop to add to scene");
	
	// **************************** Gizmos ****************************
	GIZMO_MANAGER.InitializeResources();

	// hide all resources for gizmos from content browser
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(RESOURCE_MANAGER.GetMesh("45191B6F172E3B531978692E"/*"transformationGizmoMesh"*/));
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(RESOURCE_MANAGER.GetMesh("637C784B2E5E5C6548190E1B"/*"scaleGizmoMesh"*/));
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(RESOURCE_MANAGER.GetMesh("19622421516E5B317E1B5360"/*"rotateGizmoMesh"*/));

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(PREVIEW_MANAGER.PreviewEntity);

	SCENE_GRAPH_WINDOW.InitializeResources();
	CONTENT_BROWSER_WINDOW.InitializeResources();
	INSPECTOR_WINDOW.InitializeResources();
	
	ENGINE.GetCamera()->SetOnUpdate(OnCameraUpdate);
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

	if (SELECTED.GetSelected() != nullptr)
	{
		if (SELECTED.GetTerrain() != nullptr)
		{
			if (SELECTED.GetTerrain()->GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
				return;
		}

		GIZMO_MANAGER.MouseMove(EDITOR.GetLastMouseX(), EDITOR.GetLastMouseY(), EDITOR.GetMouseX(), EDITOR.GetMouseY());
	}
}

void FEEditor::OnCameraUpdate(FEBasicCamera* Camera)
{
	SELECTED.OnCameraUpdate();
	GIZMO_MANAGER.Render();
}

void FEEditor::Render()
{
	DRAG_AND_DROP_MANAGER.Render();

	if (PROJECT_MANAGER.GetCurrent())
	{
		if (bGameMode)
			return;

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save project"))
				{
					PROJECT_MANAGER.GetCurrent()->SaveScene();
					ENGINE.TakeScreenshot((PROJECT_MANAGER.GetCurrent()->GetProjectFolder() + "projectScreenShot.texture").c_str());
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
						PROJECT_MANAGER.CloseCurrentProject();
						CONTENT_BROWSER_WINDOW.Clear();
						SCENE_GRAPH_WINDOW.Clear();

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
						PROJECT_MANAGER.CloseCurrentProject();
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

		ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollbar);
		
		SceneWindow = ImGui::GetCurrentWindow();
		SceneWindowTarget->StickToCurrentWindow();
		
		ENGINE.SetRenderTargetSize((size_t)SceneWindow->ContentRegionRect.GetWidth(), (size_t)SceneWindow->ContentRegionRect.GetHeight());

		ENGINE.SetRenderTargetXShift((int)SceneWindow->ContentRegionRect.GetTL().x);
		ENGINE.SetRenderTargetYShift((int)SceneWindow->ContentRegionRect.GetTL().y);

		if (ENGINE.GetCamera()->GetCameraType() == 1)
			ENGINE.RenderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.GetCamera()));

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowBorderSize = 0.0f;
		style.WindowPadding = ImVec2(0.0f, 0.0f);

		if (RENDERER.FinalScene != nullptr)
		{
			ImGui::Image((void*)(intptr_t)RENDERER.FinalScene->GetTextureID(), ImVec2(ImGui::GetCurrentWindow()->ContentRegionRect.GetWidth(), ImGui::GetCurrentWindow()->ContentRegionRect.GetHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
		else if (RENDERER.SceneToTextureFB->GetColorAttachment() != nullptr)
		{
			ImGui::Image((void*)(intptr_t)RENDERER.SceneToTextureFB->GetColorAttachment()->GetTextureID(), ImVec2(ImGui::GetCurrentWindow()->ContentRegionRect.GetWidth(), ImGui::GetCurrentWindow()->ContentRegionRect.GetHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
		// Something went terribly wrong!
		else
		{

		}

		ImGui::End();

		SCENE_GRAPH_WINDOW.Render();
		CONTENT_BROWSER_WINDOW.Render();
		INSPECTOR_WINDOW.Render();
		DisplayEffectsWindow();
		DisplayLogWindow();
		if (!GyzmosSettingsWindowObject.IsVisible())
			GyzmosSettingsWindowObject.Show();
		GyzmosSettingsWindowObject.Render();

		const int ObjectIndex = SELECTED.GetIndexOfObjectUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY());
		if (ObjectIndex >= 0)
		{
			if (!GIZMO_MANAGER.WasSelected(ObjectIndex))
			{
				SELECTED.SetSelectedByIndex(ObjectIndex);
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
		PROJECT_MANAGER.CloseCurrentProject();
		ENGINE.Terminate();
		return;
	}
}

void FEEditor::RenderTargetResizeCallback(int NewW, int NewH)
{
	if (ENGINE.GetCamera()->GetCameraType() == 1)
		ENGINE.RenderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.GetCamera()));
	SELECTED.ReInitializeResources();
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
			std::vector<FEObject*> LoadedObjects = SCENE.ImportAsset(Paths[i]);
			for (size_t j = 0; j < LoadedObjects.size(); j++)
			{
				if (LoadedObjects[j] != nullptr)
				{
					if (LoadedObjects[j]->GetType() == FE_ENTITY)
					{
						//SCENE.AddEntity(reinterpret_cast<FEEntity*>(LoadedObjects[j]));
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

bool FEEditor::IsInGameMode() const
{
	return bGameMode;
}

void FEEditor::SetGameMode(const bool GameMode)
{
	this->bGameMode = GameMode;
	if (this->bGameMode)
	{
		ENGINE.SetRenderTargetMode(FE_GLFW_MODE);
		if (ENGINE.GetCamera()->GetCameraType() == 1)
			ENGINE.RenderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.GetCamera()));
	}
	else
	{
		ENGINE.SetRenderTargetMode(FE_CUSTOM_MODE);
		if (ENGINE.GetCamera()->GetCameraType() == 1)
			ENGINE.RenderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.GetCamera()));
	}
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

	if (ImGui::CollapsingHeader("Gamma Correction & Exposure", 0))
	{
		ImGui::Text("Gamma Correction:");
		float Gamma = ENGINE.GetCamera()->GetGamma();
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::DragFloat("##Gamma Correction", &Gamma, 0.01f, 0.001f, 10.0f);
		ENGINE.GetCamera()->SetGamma(Gamma);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			ENGINE.GetCamera()->SetGamma(2.2f);
		}
		ImGui::PopID();

		ImGui::Text("Exposure:");
		float Exposure = ENGINE.GetCamera()->GetExposure();
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::DragFloat("##Exposure", &Exposure, 0.01f, 0.001f, 100.0f);
		ENGINE.GetCamera()->SetExposure(Exposure);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			ENGINE.GetCamera()->SetExposure(1.0f);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Anti-Aliasing(FXAA)", 0))
	{
		static const char* options[5] = { "none", "1x", "2x", "4x", "8x" };
		static std::string SelectedOption = "1x";

		static bool bFirstLook = true;
		if (bFirstLook)
		{
			const float FXAASpanMax = RENDERER.GetFXAASpanMax();
			if (FXAASpanMax == 0.0f)
			{
				SelectedOption = options[0];
			}
			else if (FXAASpanMax > 0.1f && FXAASpanMax < 1.1f)
			{
				SelectedOption = options[1];
			}
			else if (FXAASpanMax > 1.1f && FXAASpanMax < 2.1f)
			{
				SelectedOption = options[2];
			}
			else if (FXAASpanMax > 2.1f && FXAASpanMax < 4.1f)
			{
				SelectedOption = options[3];
			}
			else if (FXAASpanMax > 4.1f && FXAASpanMax < 8.1f)
			{
				SelectedOption = options[4];
			}
			else
			{
				SelectedOption = options[5];
			}

			bFirstLook = false;
		}

		static bool bDebugSettings = false;
		if (ImGui::Checkbox("debug view", &bDebugSettings))
		{
			const float FXAASpanMax = RENDERER.GetFXAASpanMax();
			if (FXAASpanMax == 0.0f)
			{
				SelectedOption = options[0];
			}
			else if (FXAASpanMax > 0.1f && FXAASpanMax < 1.1f)
			{
				SelectedOption = options[1];
			}
			else if (FXAASpanMax > 1.1f && FXAASpanMax < 2.1f)
			{
				SelectedOption = options[2];
			}
			else if (FXAASpanMax > 2.1f && FXAASpanMax < 4.1f)
			{
				SelectedOption = options[3];
			}
			else if (FXAASpanMax > 4.1f && FXAASpanMax < 8.1f)
			{
				SelectedOption = options[4];
			}
			else
			{
				SelectedOption = options[5];
			}
		}

		if (!bDebugSettings)
		{
			ImGui::Text("Anti Aliasing Strength:");
			if (ImGui::BeginCombo("##Anti Aliasing Strength", SelectedOption.c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < 5; i++)
				{
					const bool is_selected = (SelectedOption == options[i]);
					if (ImGui::Selectable(options[i], is_selected))
					{
						RENDERER.SetFXAASpanMax(float(pow(2.0, (i - 1))));
						if (i == 0)
							RENDERER.SetFXAASpanMax(0.0f);
						SelectedOption = options[i];
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			ImGui::Text("FXAASpanMax:");
			ImGui::SetNextItemWidth(FieldWidth);
			float FXAASpanMax = RENDERER.GetFXAASpanMax();
			ImGui::DragFloat("##FXAASpanMax", &FXAASpanMax, 0.0f, 0.001f, 32.0f);
			RENDERER.SetFXAASpanMax(FXAASpanMax);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			ResetButton->Render();
			if (ResetButton->IsClicked())
			{
				RENDERER.SetFXAASpanMax(8.0f);
			}
			ImGui::PopID();

			ImGui::Text("FXAAReduceMin:");
			ImGui::SetNextItemWidth(FieldWidth);
			float FXAAReduceMin = RENDERER.GetFXAAReduceMin();
			ImGui::DragFloat("##FXAAReduceMin", &FXAAReduceMin, 0.01f, 0.001f, 100.0f);
			RENDERER.SetFXAAReduceMin(FXAAReduceMin);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			ResetButton->Render();
			if (ResetButton->IsClicked())
			{
				RENDERER.SetFXAAReduceMin(0.008f);
			}
			ImGui::PopID();

			ImGui::Text("FXAAReduceMul:");
			ImGui::SetNextItemWidth(FieldWidth);
			float FXAAReduceMul = RENDERER.GetFXAAReduceMul();
			ImGui::DragFloat("##FXAAReduceMul", &FXAAReduceMul, 0.01f, 0.001f, 100.0f);
			RENDERER.SetFXAAReduceMul(FXAAReduceMul);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			ResetButton->Render();
			if (ResetButton->IsClicked())
			{
				RENDERER.SetFXAAReduceMul(0.400f);
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Bloom", 0))
	{
		ImGui::Text("Threshold:");
		float Threshold = RENDERER.GetBloomThreshold();
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::DragFloat("##Threshold", &Threshold, 0.01f, 0.001f, 30.0f);
		RENDERER.SetBloomThreshold(Threshold);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			RENDERER.SetBloomThreshold(1.5f);
		}
		ImGui::PopID();

		ImGui::Text("Size:");
		float Size = RENDERER.GetBloomSize();
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::DragFloat("##BloomSize", &Size, 0.01f, 0.001f, 100.0f);
		RENDERER.SetBloomSize(Size);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			RENDERER.SetBloomSize(5.0f);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Depth of Field", 0))
	{
		ImGui::Text("Near distance:");
		ImGui::SetNextItemWidth(FieldWidth);
		float DepthThreshold = RENDERER.GetDOFNearDistance();
		ImGui::DragFloat("##depthThreshold", &DepthThreshold, 0.0f, 0.001f, 100.0f);
		RENDERER.SetDOFNearDistance(DepthThreshold);

		ImGui::Text("Far distance:");
		ImGui::SetNextItemWidth(FieldWidth);
		float DepthThresholdFar = RENDERER.GetDOFFarDistance();
		ImGui::DragFloat("##depthThresholdFar", &DepthThresholdFar, 0.0f, 0.001f, 100.0f);
		RENDERER.SetDOFFarDistance(DepthThresholdFar);

		ImGui::Text("Strength:");
		ImGui::SetNextItemWidth(FieldWidth);
		float Strength = RENDERER.GetDOFStrength();
		ImGui::DragFloat("##Strength", &Strength, 0.0f, 0.001f, 10.0f);
		RENDERER.SetDOFStrength(Strength);

		ImGui::Text("Distance dependent strength:");
		ImGui::SetNextItemWidth(FieldWidth);
		float IntMult = RENDERER.GetDOFDistanceDependentStrength();
		ImGui::DragFloat("##Distance dependent strength", &IntMult, 0.0f, 0.001f, 100.0f);
		RENDERER.SetDOFDistanceDependentStrength(IntMult);
	}

	if (ImGui::CollapsingHeader("Distance fog", 0))
	{
		bool bEnabledFog = RENDERER.IsDistanceFogEnabled();
		if (ImGui::Checkbox("Enable fog", &bEnabledFog))
		{
			RENDERER.SetDistanceFogEnabled(bEnabledFog);
		}

		if (bEnabledFog)
		{
			ImGui::Text("Density:");
			ImGui::SetNextItemWidth(FieldWidth);
			float FogDensity = RENDERER.GetDistanceFogDensity();
			ImGui::DragFloat("##fogDensity", &FogDensity, 0.0001f, 0.0f, 5.0f);
			RENDERER.SetDistanceFogDensity(FogDensity);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			ResetButton->Render();
			if (ResetButton->IsClicked())
			{
				RENDERER.SetDistanceFogDensity(0.007f);
			}
			ImGui::PopID();

			ImGui::Text("Gradient:");
			ImGui::SetNextItemWidth(FieldWidth);
			float FogGradient = RENDERER.GetDistanceFogGradient();
			ImGui::DragFloat("##fogGradient", &FogGradient, 0.001f, 0.0f, 5.0f);
			RENDERER.SetDistanceFogGradient(FogGradient);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			ResetButton->Render();
			if (ResetButton->IsClicked())
			{
				RENDERER.SetDistanceFogGradient(2.5f);
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Chromatic Aberration", 0))
	{
		ImGui::Text("Shift strength:");
		ImGui::SetNextItemWidth(FieldWidth);
		float intensity = RENDERER.GetChromaticAberrationIntensity();
		ImGui::DragFloat("##intensity", &intensity, 0.01f, 0.0f, 30.0f);
		RENDERER.SetChromaticAberrationIntensity(intensity);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			RENDERER.SetChromaticAberrationIntensity(1.0f);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Sky", 0))
	{
		bool bEnabledSky = RENDERER.IsSkyEnabled();
		if (ImGui::Checkbox("enable sky", &bEnabledSky))
		{
			RENDERER.SetSkyEnabled(bEnabledSky);
		}

		ImGui::Text("Sphere size:");
		ImGui::SetNextItemWidth(FieldWidth);
		float size = RENDERER.GetDistanceToSky();
		ImGui::DragFloat("##Sphere size", &size, 0.01f, 0.0f, 200.0f);
		RENDERER.SetDistanceToSky(size);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			RENDERER.SetDistanceToSky(50.0f);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("SSAO", 0))
	{
		static const char* options[5] = { "Off", "Low", "Medium", "High", "Custom"};
		static std::string SelectedOption = "Medium";

		static bool bFirstLook = true;
		if (bFirstLook)
		{
			const int SampleCount = RENDERER.GetSSAOSampleCount();

			if (!RENDERER.IsSSAOEnabled())
			{
				SelectedOption = options[0];
			}
			else if (SampleCount == 4)
			{
				SelectedOption = options[1];
			}
			else if (SampleCount == 16 && RENDERER.GetSSAORadiusSmallDetails())
			{
				SelectedOption = options[2];
			}
			else if (SampleCount == 32 && RENDERER.GetSSAORadiusSmallDetails())
			{
				SelectedOption = options[3];
			}
			else
			{
				SelectedOption = options[4];
			}

			//bFirstLook = false;
		}

		static bool bDebugSettings = false;
		if (ImGui::Checkbox("Debug view", &bDebugSettings))
		{

		}
		
		if (!bDebugSettings)
		{
			ImGui::Text("SSAO Quality:");
			ImGui::SetNextItemWidth(150);
			if (ImGui::BeginCombo("##SSAO Quality", SelectedOption.c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < 5; i++)
				{
					const bool is_selected = (SelectedOption == options[i]);
					if (ImGui::Selectable(options[i], is_selected))
					{
						RENDERER.SetSSAOResultBlured(true);
						RENDERER.SetSSAOBias(0.013f);
						RENDERER.SetSSAORadius(10.0f);
						RENDERER.SetSSAORadiusSmallDetails(0.4f);
						RENDERER.SetSSAOSmallDetailsWeight(0.2f);

						if (i == 0)
						{
							RENDERER.SetSSAOEnabled(false);
						}
						else if (i == 1)
						{
							RENDERER.SetSSAOEnabled(true);

							RENDERER.SetSSAOSampleCount(4);
							RENDERER.SetSSAOSmallDetailsEnabled(false);
						}
						else if (i == 2)
						{
							RENDERER.SetSSAOEnabled(true);

							RENDERER.SetSSAOSampleCount(16);
							RENDERER.SetSSAOSmallDetailsEnabled(true);
						}
						else if (i == 3)
						{
							RENDERER.SetSSAOEnabled(true);

							RENDERER.SetSSAOSampleCount(32);
							RENDERER.SetSSAOSmallDetailsEnabled(true);
						}

						SelectedOption = options[i];
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			bool TempBool = RENDERER.IsSSAOEnabled();
			ImGui::Checkbox("SSAO active", &TempBool);
			RENDERER.SetSSAOEnabled(TempBool);

			TempBool = RENDERER.IsSSAOSmallDetailsEnabled();
			ImGui::Checkbox("SSAO small details", &TempBool);
			RENDERER.SetSSAOSmallDetailsEnabled(TempBool);

			TempBool = RENDERER.IsSSAOResultBlured();
			ImGui::Checkbox("SSAO blured", &TempBool);
			RENDERER.SetSSAOResultBlured(TempBool);

			int TempInt = RENDERER.GetSSAOSampleCount();
			ImGui::SetNextItemWidth(100);
			ImGui::DragInt("SSAO sample count", &TempInt);
			RENDERER.SetSSAOSampleCount(TempInt);

			float TempFloat = RENDERER.GetSSAOBias();
			ImGui::SetNextItemWidth(100);
			ImGui::DragFloat("SSAO bias", &TempFloat, 0.1f);
			RENDERER.SetSSAOBias(TempFloat);

			TempFloat = RENDERER.GetSSAORadius();
			ImGui::SetNextItemWidth(100);
			ImGui::DragFloat("SSAO radius", &TempFloat, 0.1f);
			RENDERER.SetSSAORadius(TempFloat);

			TempFloat = RENDERER.GetSSAORadiusSmallDetails();
			ImGui::SetNextItemWidth(100);
			ImGui::DragFloat("SSAO radius small details", &TempFloat, 0.1f);
			RENDERER.SetSSAORadiusSmallDetails(TempFloat);

			TempFloat = RENDERER.GetSSAOSmallDetailsWeight();
			ImGui::SetNextItemWidth(100);
			ImGui::DragFloat("SSAO small details weight", &TempFloat, 0.01f);
			RENDERER.SetSSAOSmallDetailsWeight(TempFloat);
		}

		/*bool bEnabledSky = RENDERER.IsSkyEnabled();
		if (ImGui::Checkbox("enable sky", &bEnabledSky))
		{
			RENDERER.SetSkyEnabld(bEnabledSky);
		}

		ImGui::Text("Sphere size:");
		ImGui::SetNextItemWidth(FieldWidth);
		float size = RENDERER.GetDistanceToSky();
		ImGui::DragFloat("##Sphere size", &size, 0.01f, 0.0f, 200.0f);
		RENDERER.SetDistanceToSky(size);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
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
	SelectFEObjectPopUp::getInstance().Render();

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

	io.DisplaySize = ImVec2(static_cast<float>(ENGINE.GetWindowWidth()), static_cast<float>(ENGINE.GetWindowHeight()));
	ImGui::StyleColorsDark();

	SetImguiStyle();
}