#include "FEEditor.h"

FEEditor::FEEditor()
{
	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigWindowsMoveFromTitleBarOnly = true;
	ENGINE.SetVsyncEnabled(true);
	RESOURCE_MANAGER.AddTagThatWillPreventDeletion(EDITOR_RESOURCE_TAG);
}

FEEditor::~FEEditor() {}

#include "VersionInfo/FE_EDITOR_Version.h"
#include "VersionInfo/FEVersionInfo.h"
FE_DEFINE_VERSION_INFO(FE_EDITOR_)

std::string FEEditor::GetEditorVersion()
{
	return GetFE_EDITOR_VersionInfo().GetVersion();
}

int FEEditor::GetEditorBuildNumber()
{
	return GetFE_EDITOR_VersionInfo().BuildNumber;
}

std::string FEEditor::GetEditorBuildTimestamp()
{
	return GetFE_EDITOR_VersionInfo().BuildTimestamp;
}

std::string FEEditor::GetEditorBuildInfo()
{
	return GetFE_EDITOR_VersionInfo().GetBuildInfo();
}

std::string FEEditor::GetEditorFullVersion()
{
	return "Focal Engine Editor " + GetFE_EDITOR_VersionInfo().GetFullVersionString();
}

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

	if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_PRESS)
	{
		INSPECTOR_WINDOW.bLeftMousePressed = true;
	}
	else if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
	{
		INSPECTOR_WINDOW.bLeftMousePressed = false;
	}

	EDITOR_SCENE_WINDOW_MANAGER.MouseButtonCallback(Button, Action, Mods);
}

void FEEditor::KeyButtonCallback(int Key, int Scancode, int Action, int Mods)
{
	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
	{
		if (PROJECT_MANAGER.GetCurrent() == nullptr)
			ENGINE.Terminate();
		ProjectWasModifiedPopUp::GetInstance().Show(PROJECT_MANAGER.GetCurrent(), true);
	}

	EDITOR_SCENE_WINDOW_MANAGER.KeyButtonCallback(Key, Scancode, Action, Mods);

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

	SetUpImGui();
}

void FEEditor::MouseMoveCallback(double Xpos, double Ypos)
{
	EDITOR.SetLastMouseX(EDITOR.GetMouseX());
	EDITOR.SetLastMouseY(EDITOR.GetMouseY());

	EDITOR.SetMouseX(Xpos);
	EDITOR.SetMouseY(Ypos);

	DRAG_AND_DROP_MANAGER.MouseMove();

	EDITOR_SCENE_WINDOW_MANAGER.MouseMoveCallback(Xpos, Ypos, EDITOR.GetLastMouseX(), EDITOR.GetLastMouseY());
}

void FEEditor::AfterEngineUpdate()
{
	SELECTED.OnCameraUpdate();
	GIZMO_MANAGER.Update();
}

void FEEditor::RenderTemporaryDebugWindow()
{
	if (ImGui::Begin("Temporary debug window"))
	{
		// FE_FIX_ME: Find a proper place for this UI.
		static float AverageTime = 0.0f;
		static std::vector<float> RecentTimeSamples;
		static int Counter = 0;

		ImGui::Text((std::string("Time : ") + std::to_string(RENDERER.LastTestTime)).c_str());

		if (RecentTimeSamples.size() < 100)
		{
			RecentTimeSamples.push_back(RENDERER.LastTestTime);
		}
		else if (RecentTimeSamples.size() >= 100)
		{
			RecentTimeSamples[Counter++ % 100] = RENDERER.LastTestTime;
		}

		for (size_t i = 0; i < RecentTimeSamples.size(); i++)
			AverageTime += RecentTimeSamples[i];
		
		AverageTime /= RecentTimeSamples.size();

		if (Counter > 1000000)
			Counter = 0;

		ImGui::Text((std::string("avg Time : ") + std::to_string(AverageTime)).c_str());

		bool bFreezeCulling = RENDERER.bFreezeCulling;
		ImGui::Checkbox("bFreezeCulling", &bFreezeCulling);
		RENDERER.bFreezeCulling = bFreezeCulling;

		bool bFreezeOcclusionCulling = !RENDERER.IsOcclusionCullingEnabled();
		ImGui::Checkbox("freezeOcclusionCulling", &bFreezeOcclusionCulling);
		RENDERER.SetOcclusionCullingEnabled(!bFreezeOcclusionCulling);

		static bool bDisplaySelectedObjAABB = false;
		ImGui::Checkbox("Display AABB of selected object", &bDisplaySelectedObjAABB);

		FEScene* CurrentScene = EDITOR.GetFocusedScene();
		if (CurrentScene != nullptr)
		{
			// Draw AABB
			FEEntity* SelectedEntity = SELECTED.GetSelected(CurrentScene);
			if (SelectedEntity != nullptr &&
				(SelectedEntity->HasComponent<FEGameModelComponent>() || SelectedEntity->HasComponent<FETerrainComponent>() || SelectedEntity->HasComponent<FEPointCloudComponent>()) &&
				bDisplaySelectedObjAABB)
			{
				FEAABB SelectedAABB;
				SelectedAABB = SelectedEntity->GetParentScene()->GetEntityAABB(SelectedEntity);
				RENDERER.DebugDrawAABB(SelectedAABB);

				if (SelectedEntity->HasComponent<FEInstancedComponent>())
				{
					static bool bDisplaySubObjAABB = false;
					ImGui::Checkbox("Display AABB of instanced entity subobjects", &bDisplaySubObjAABB);

					if (bDisplaySubObjAABB)
					{
						FEInstancedComponent& InstancedComponent = SelectedEntity->GetComponent<FEInstancedComponent>();
						const int MaxIterations = InstancedComponent.IndividualInstancedAABB.size() * 8 >= FE_MAX_DEBUG_LINES ? FE_MAX_DEBUG_LINES : int(InstancedComponent.IndividualInstancedAABB.size());

						for (size_t j = 0; j < MaxIterations; j++)
						{
							RENDERER.DebugDrawAABB(InstancedComponent.IndividualInstancedAABB[j]);
						}
					}
				}
			}

			static bool bDisplaySceneAABB = false;
			ImGui::Checkbox("Display AABB of scene", &bDisplaySceneAABB);
			if (bDisplaySceneAABB)
			{
				FEAABB SceneAABB = CurrentScene->GetSceneAABB([](FEEntity* Entity) -> bool {
					if (Entity->GetTag() == EDITOR_RESOURCE_TAG)
						return false;

					if (Entity->HasComponent<FESkyDomeComponent>())
						return false;

					if (Entity->HasComponent<FECameraComponent>())
						return false;

					return true;
					});
				RENDERER.DebugDrawAABB(SceneAABB);
			}

			// Draw camera frustum
			if (SelectedEntity != nullptr && SelectedEntity->HasComponent<FECameraComponent>())
			{
				static bool bDisplayCameraFrustum = false;
				ImGui::Checkbox("Display camera frustum", &bDisplayCameraFrustum);
				if (bDisplayCameraFrustum)
					RENDERER.DebugDrawFrustum(SelectedEntity);
			}
		}
	}
	ImGui::End();
}

void FEEditor::SetUpDocking()
{
	ImGuiID DockspaceID = APPLICATION.GetMainWindow()->GetDefaultDockspaceID();
	if (!bHadImGuiIniFileAtStartup && DockspaceID != 0)
	{
		bHadImGuiIniFileAtStartup = true;
		// Wipe any pre-existing layout and recreate the root node
		ImGui::DockBuilderRemoveNode(DockspaceID);
		ImGui::DockBuilderAddNode(DockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(DockspaceID, ImGui::GetMainViewport()->Size);

		ImGuiID BottomID;
		ImGuiID CenterID;
		ImGuiID LeftID;
		ImGuiID RightID;

		ImGui::DockBuilderSplitNode(DockspaceID, ImGuiDir_Down, 0.25f, &BottomID, &CenterID);
		ImGui::DockBuilderSplitNode(CenterID, ImGuiDir_Left, 0.15f, &LeftID, &CenterID);
		ImGui::DockBuilderSplitNode(CenterID, ImGuiDir_Right, 0.15f, &RightID, &CenterID);

		ImGui::DockBuilderDockWindow("Scene Graph", LeftID);
		ImGui::DockBuilderDockWindow("Inspector", RightID);
		ImGui::DockBuilderDockWindow("Content Browser", BottomID);
		ImGui::DockBuilderDockWindow("Editor Cameras", BottomID);
		ImGui::DockBuilderDockWindow("Log", BottomID);

		ImGui::DockBuilderFinish(DockspaceID);
	}
}

void FEEditor::Render()
{
	SetUpDocking();

	PREVIEW_MANAGER.Update();
	EDITOR_SCRIPTING_SYSTEM.Update();

	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::Renderable);
	if (ActiveScenes.empty())
		EDITOR_SCENE_WINDOW_MANAGER.Clear();

	DRAG_AND_DROP_MANAGER.Render();

	if (PROJECT_MANAGER.GetCurrent())
	{
		//APPLICATION.GetMainWindow()->EnableDefaultDockspace();

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
						CloseProjectAndCleanup();

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
						CloseProjectAndCleanup();
						ENGINE.Terminate();
						return;
					}
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Scene Graph", nullptr, SCENE_GRAPH_WINDOW.bVisible))
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

			if (ImGui::BeginMenu("Info"))
			{
				if (ImGui::MenuItem("About..."))
					ShowAboutDialog();

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
		ImGui::PopStyleVar();

		bool bFocusedSceneCouldBeUsedForGameMode = false;
		if (EDITOR.GetFocusedScene() != nullptr)
		{
			FEEditorSceneWindow* SceneWindow = EDITOR_SCENE_WINDOW_MANAGER.GetSceneWindow(EDITOR.GetFocusedScene()->GetObjectID());
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

		RenderTemporaryDebugWindow();
		SCENE_GRAPH_WINDOW.Render();
		CONTENT_BROWSER_WINDOW.Render();
		INSPECTOR_WINDOW.Render();
		DisplayEditorCamerasWindow();
		DisplayLogWindow();
		if (!GizmosSettingsWindowObject.IsVisible())
			GizmosSettingsWindowObject.Show();
		GizmosSettingsWindowObject.Render();

		EDITOR_SCENE_WINDOW_MANAGER.Update();

		RenderAllSubWindows();
		RenderAboutWindow();
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
		EDITOR.CloseProjectAndCleanup();
		ENGINE.Terminate();
		return;
	}
}

void FEEditor::ShowAboutDialog()
{
	bShouldOpenAboutWindow = true;
}

void FEEditor::RenderAboutWindow()
{
	if (bShouldOpenAboutWindow)
	{
		ImGui::OpenPopup("About");
		bShouldOpenAboutWindow = false;
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		int WindowW = 0;
		int WindowH = 0;
		APPLICATION.GetMainWindow()->GetSize(&WindowW, &WindowH);

		ImGui::SetWindowPos(ImVec2(WindowW / 2.0f - ImGui::GetWindowWidth() / 2.0f, WindowH / 2.0f - ImGui::GetWindowHeight() / 2.0f));

		float ContentW = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
		auto CenteredText = [ContentW](const std::string& Text) {
			ImVec2 TextSize = ImGui::CalcTextSize(Text.c_str());
			ImGui::SetCursorPosX((ContentW - TextSize.x) / 2.0f + ImGui::GetWindowContentRegionMin().x);
			ImGui::Text("%s", Text.c_str());
		};
		CenteredText(GetEditorFullVersion());

		ImGui::Separator();
		ImGui::Text("Modules:");

		CenteredText(ENGINE.GetFullVersion());
		CenteredText(SCENE_GRAPH_WINDOW.GetSceneGraphUI()->GetFullVersion());
		CenteredText(NODE_SYSTEM.GetFullVersion());
		CenteredText(APPLICATION.GetFullVersion());

		ImGui::Separator();

		CenteredText("To submit a bug report or provide feedback,");
		CenteredText("please email me at ");

		ImGui::Separator();

		float ButtonW = 210.0f;
		ImGui::SetCursorPosX((ContentW - ButtonW) / 2.0f + ImGui::GetWindowContentRegionMin().x);
		if (ImGui::Button("Close", ImVec2(ButtonW, 25.0f)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void FEEditor::OnViewportResize(std::string ViewportID)
{
	
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
		FEScene* Scene = SCENE_MANAGER.GetSceneByID(EditorCameraIterator->first);
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
			ImGui::Indent();

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				FETransformComponent& Transform = CameraEntity->GetComponent<FETransformComponent>();
				INSPECTOR_WINDOW.ShowTransformConfiguration(CameraEntity->GetName(), &Transform);
			}
			
			if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
			{
				INSPECTOR_WINDOW.DisplayCameraProperties(CameraEntity);
			}

			if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen))
			{
				INSPECTOR_WINDOW.DisplayNativeScriptProperties(CameraEntity);
			}
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
	DeletePointCloudPopup::GetInstance().Render();
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

void FEEditor::SetImGuiStyle()
{
	ImGuiStyle* Style = &ImGui::GetStyle();
	ImVec4* Colors = Style->Colors;

	Style->WindowRounding = 2.0f;
	Style->ScrollbarRounding = 3.0f;
	Style->GrabRounding = 2.0f;
	Style->AntiAliasedLines = true;
	Style->AntiAliasedFill = true;
	Style->WindowRounding = 2;
	Style->ChildRounding = 2;
	Style->ScrollbarSize = 16;
	Style->ScrollbarRounding = 3;
	Style->GrabRounding = 2;
	Style->ItemSpacing.x = 10;
	Style->ItemSpacing.y = 4;
	Style->IndentSpacing = 22;
	Style->FramePadding.x = 6;
	Style->FramePadding.y = 4;
	Style->Alpha = 1.0f;
	Style->FrameRounding = 3.0f;

	Colors[ImGuiCol_Text] = ImVec4(1.0f, 243.0f / 255.0f, 1.0f, 1.00f);
	Colors[ImGuiCol_TextDisabled] = ImVec4(158.0f / 255.0f, 158.0f / 255.0f, 158.0f / 255.0f, 1.00f);
	Colors[ImGuiCol_WindowBg] = ImVec4(43.0f / 255.0f, 43.0f / 255.0f, 43.0f / 255.0f, 1.00f);
	Colors[ImGuiCol_PopupBg] = ImVec4(60.0f / 255.0f, 60.0f / 255.0f, 60.0f / 255.0f, 0.98f);

	Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	Colors[ImGuiCol_Border] = ImVec4(0.71f, 0.71f, 0.71f, 0.08f);
	Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
	Colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.55f);
	Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
	Colors[ImGuiCol_FrameBgActive] = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
	Colors[ImGuiCol_TitleBg] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.41f, 0.68f, 0.89f, 1.00f);
	Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.47f, 0.83f, 1.00f);

	Colors[ImGuiCol_MenuBarBg] = ImVec4(92.0f / 255.0f, 92.0f / 255.0f, 92.0f / 255.0f, 1.00f);
	Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
	Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
	Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
	Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	Colors[ImGuiCol_CheckMark] = ImVec4(0.184f, 0.407f, 0.193f, 1.00f);
	Colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	Colors[ImGuiCol_Button] = ImVec4(0.71f, 0.78f, 0.69f, 0.40f);
	Colors[ImGuiCol_ButtonHovered] = ImVec4(0.725f, 0.805f, 0.702f, 1.00f);
	Colors[ImGuiCol_ButtonActive] = ImVec4(0.793f, 0.900f, 0.836f, 1.00f);
	Colors[ImGuiCol_Header] = ImVec4(0.71f, 0.78f, 0.69f, 0.31f);
	Colors[ImGuiCol_HeaderHovered] = ImVec4(0.71f, 0.78f, 0.69f, 0.80f);
	Colors[ImGuiCol_HeaderActive] = ImVec4(0.71f, 0.78f, 0.69f, 1.00f);
	Colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	Colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
	Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	Colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	Colors[ImGuiCol_NavHighlight] = Colors[ImGuiCol_HeaderHovered];
	Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
}

void FEEditor::SetUpImGui()
{
	ImGuiIO& IO = ImGui::GetIO();

	std::string ResourcesFolder = "Resources\\";

	const size_t PathLength = strlen((ResourcesFolder + "imgui.ini").c_str()) + 1;
	//FILE_SYSTEM.DeleteFile((ResourcesFolder + "imgui.ini").c_str());
	bHadImGuiIniFileAtStartup = FILE_SYSTEM.DoesFileExist((ResourcesFolder + "imgui.ini").c_str());
	char* ImguiIniFile = new char[PathLength];
	strcpy_s(ImguiIniFile, PathLength, (ResourcesFolder + "imgui.ini").c_str());
	IO.IniFilename = ImguiIniFile;
	IO.Fonts->AddFontFromFileTTF((ResourcesFolder + "Cousine-Regular.ttf").c_str(), 20);
	IO.Fonts->AddFontFromFileTTF((ResourcesFolder + "Cousine-Regular.ttf").c_str(), 32);
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	unsigned char* TextPixels = nullptr;
	int TextWidth, TextHeight;
	IO.Fonts->GetTexDataAsRGBA32(&TextPixels, &TextWidth, &TextHeight);

	IO.DisplaySize = ImVec2(static_cast<float>(APPLICATION.GetMainWindow()->GetWidth()), static_cast<float>(APPLICATION.GetMainWindow()->GetHeight()));
	ImGui::StyleColorsDark();

	SetImGuiStyle();

	APPLICATION.GetMainWindow()->EnableDefaultDockspace();
}

void FEEditor::CloseProjectAndCleanup()
{
	EDITOR_MATERIAL_WINDOW.Stop();
	EDITOR_SCENE_WINDOW_MANAGER.Clear();

	PROJECT_MANAGER.CloseCurrentProject();
	CONTENT_BROWSER_WINDOW.Clear();
	SCENE_GRAPH_WINDOW.Clear();
	PREFAB_EDITOR_MANAGER.Clear();
}

FEEditorSceneWindow* FEEditor::GetEditorSceneWindow(std::string SceneID)
{
	return EDITOR_SCENE_WINDOW_MANAGER.GetSceneWindow(SceneID);
}

void FEEditor::CreateEditorWindowForScene(const std::string& SceneID, FEProject* CurrentProject)
{
	EDITOR_SCENE_WINDOW_MANAGER.CreateSceneWindow(SceneID, CurrentProject);
}

void FEEditor::RegisterEditorSceneWindow(FEEditorSceneWindow* SceneWindow)
{
	EDITOR_SCENE_WINDOW_MANAGER.RegisterSceneWindow(SceneWindow);
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

	EDITOR.CreateEditorWindowForScene(GameModeScene->GetObjectID());

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
			EDITOR_SCENE_WINDOW_MANAGER.DeleteSceneAndCleanup(SceneIterator->second->GetObjectID());
			SceneIterator = ParentIDToScenesInGameMode.erase(SceneIterator);
		}
	}

	return true;
}

std::vector<std::string> FEEditor::GetEditorOpenedScenesIDs() const
{
	return EDITOR_SCENE_WINDOW_MANAGER.GetOpenedScenesIDs();
}

FEScene* FEEditor::GetFocusedScene() const
{
	return EDITOR_SCENE_WINDOW_MANAGER.GetFocusedScene();
}

bool FEEditor::SetFocusedScene(FEScene* NewSceneInFocus)
{
	return EDITOR_SCENE_WINDOW_MANAGER.SetFocusedScene(NewSceneInFocus);
}

bool FEEditor::SetFocusedScene(std::string NewSceneInFocusID)
{
	return EDITOR_SCENE_WINDOW_MANAGER.SetFocusedScene(NewSceneInFocusID);
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
					CAMERA_SYSTEM.SetMainCamera(Scenes[i]->GetEntity(EditorCameraID));
				}
			}
		}
	}
}

bool FEEditor::HadImGuiIniFileAtStartup() const
{
	return bHadImGuiIniFileAtStartup;
}