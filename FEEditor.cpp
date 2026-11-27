#include "FEEditor.h"
#include "SubSystems/PluginSystem/FEEditorPluginManager.h"
#include "EditorWindows/PluginsWindow.h"

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

std::string FEEditor::GetEditorCommitHash()
{
	return GetFE_EDITOR_VersionInfo().GitHash;
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

FEUUID FEEditor::GetSceneEntityIDInClipboard()
{
	return SceneEntityIDInClipboard;
}

void FEEditor::SetSceneEntityIDInClipboard(const FEUUID& NewValue)
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

	// Not while text is typed, a text field can have its own use for these keys.
	if (!ImGui::GetIO().WantTextInput && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_S && Action == GLFW_PRESS)
	{
		if (PROJECT_MANAGER.GetCurrent() != nullptr)
			PROJECT_MANAGER.GetCurrent()->SaveProject();
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_V && Action == GLFW_RELEASE)
	{
		if (!UNIQUE_ID.IsNull(EDITOR.GetSceneEntityIDInClipboard()))
		{
			if (EDITOR.GetFocusedScene() != nullptr)
			{
				FEEntity* EntityToDuplicate = EDITOR.GetFocusedScene()->GetEntity(EDITOR.GetSceneEntityIDInClipboard());
				// Skip if entity was deleted or belongs to another scene
				if (EntityToDuplicate != nullptr)
				{
					FENaiveSceneGraphNode* NodeToDuplicate = EDITOR.GetFocusedScene()->SceneGraph.GetNodeByEntityID(EntityToDuplicate->GetID());
					FENaiveSceneGraphNode* DuplicatedNode = EDITOR.GetFocusedScene()->SceneGraph.DuplicateNode(NodeToDuplicate->GetID(), NodeToDuplicate->GetParent()->GetID());
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
	if (!bTemporaryDebugWindowVisible)
		return;

	if (ImGui::Begin("Temporary debug window", &bTemporaryDebugWindowVisible))
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

		static bool bDisplaySelectedObjectAABB = false;
		ImGui::Checkbox("Display AABB of selected object", &bDisplaySelectedObjectAABB);

		FEScene* CurrentScene = EDITOR.GetFocusedScene();
		if (CurrentScene != nullptr)
		{
			// Draw AABB
			FEEntity* SelectedEntity = SELECTED.GetSelected(CurrentScene);
			if (SelectedEntity != nullptr &&
				(SelectedEntity->HasComponent<FEGameModelComponent>() || SelectedEntity->HasComponent<FETerrainComponent>() || SelectedEntity->HasComponent<FEPointCloudComponent>()) &&
				bDisplaySelectedObjectAABB)
			{
				FEAABB SelectedAABB;
				SelectedAABB = SelectedEntity->GetParentScene()->GetEntityAABB(SelectedEntity);
				RENDERER.DebugDrawAABB(SelectedAABB);

				if (SelectedEntity->HasComponent<FEInstancedComponent>())
				{
					static bool bDisplaySubObjectAABB = false;
					ImGui::Checkbox("Display AABB of instanced entity subobjects", &bDisplaySubObjectAABB);

					if (bDisplaySubObjectAABB)
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

bool FEEditor::IsDebugWindowVisible() const
{
	return bDebugWindowVisible;
}

// Small window at the top center of the main window, with a triangle to run game mode and a square to exit it.
// Name of the project and whether it has changes that are not saved, at the right end of the menu bar.
static void RenderProjectStatusInMenuBar(FEProject* Project)
{
	const float SmallFontSize = 16.0f;
	const bool bModified = Project->IsModified();
	const std::string Name = Project->GetName();
	const std::string Status = bModified ? "not saved" : "saved";
	const ImU32 StatusColor = bModified ? FEEditorStyle::Warning : FEEditorStyle::Success;
	const float CenterY = ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2.0f;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	float X = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - 16.0f - ImGui::GetFont()->CalcTextSizeA(SmallFontSize, FLT_MAX, 0.0f, Status.c_str()).x;
	DrawList->AddText(ImGui::GetFont(), SmallFontSize, ImVec2(floorf(X), floorf(CenterY - SmallFontSize / 2.0f)), bModified ? FEEditorStyle::Warning : FEEditorStyle::TextMuted, Status.c_str());
	X -= 18.0f;
	DrawList->AddCircleFilled(ImVec2(floorf(X + 5.0f), floorf(CenterY)), 5.0f, StatusColor);
	X -= 12.0f + ImGui::CalcTextSize(Name.c_str()).x;
	DrawList->AddText(ImVec2(floorf(X), floorf(CenterY - ImGui::GetFontSize() / 2.0f)), FEEditorStyle::Text, Name.c_str());
}

// Play or Stop in the middle of the menu bar.
void FEEditor::RenderGameModeButton()
{
	bool bFocusedSceneCouldBeUsedForGameMode = false;
	if (EDITOR.GetFocusedScene() != nullptr)
	{
		FEEditorSceneWindow* SceneWindow = EDITOR_SCENE_WINDOW_MANAGER.GetSceneWindow(EDITOR.GetFocusedScene()->GetID());
		// Focused scene could be prefab scene
		if (!PREFAB_EDITOR_MANAGER.IsEditorWindowIsPrefabWindow(SceneWindow))
			bFocusedSceneCouldBeUsedForGameMode = true;
	}

	const bool bInGameMode = EDITOR.IsInGameMode();
	std::string ToolTipText = "Run game mode";
	if (bInGameMode)
	{
		ToolTipText = "Exit game mode";
	}
	else if (!bFocusedSceneCouldBeUsedForGameMode)
	{
		ToolTipText = "Can not run game mode (focused scene is prefab scene).";
	}

	const char* Name = bInGameMode ? "Stop" : "Play";
	const ImVec2 BarMin = ImGui::GetWindowPos();
	const float BarWidth = ImGui::GetWindowWidth();
	const float BarHeight = ImGui::GetWindowHeight();
	const ImVec2 ButtonSize = ImVec2(ImGui::CalcTextSize(Name).x + 54.0f, BarHeight - 4.0f);
	const ImVec2 ButtonMin = ImVec2(floorf(BarMin.x + (BarWidth - ButtonSize.x) / 2.0f), BarMin.y + 2.0f);
	const ImVec2 ButtonMax = ImVec2(ButtonMin.x + ButtonSize.x, ButtonMin.y + ButtonSize.y);

	const bool bDisabled = !bInGameMode && !bFocusedSceneCouldBeUsedForGameMode;
	if (bDisabled)
		ImGui::BeginDisabled();

	ImGui::SetCursorScreenPos(ButtonMin);
	const bool bClicked = ImGui::InvisibleButton("##GameModeButton", ButtonSize);
	if (bDisabled)
		ImGui::EndDisabled();

	const bool bHovered = ImGui::IsItemHovered();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("%s", ToolTipText.c_str());

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	const float IconX = ButtonMin.x + 18.0f;
	const float IconY = floorf(ButtonMin.y + ButtonSize.y / 2.0f);
	ImU32 ContentColor = FEEditorStyle::Text;
	if (bInGameMode)
	{
		ContentColor = IM_COL32(255, 255, 255, 255);
		DrawList->AddRectFilled(ButtonMin, ButtonMax, bHovered ? FEEditorStyle::DangerButtonHovered : FEEditorStyle::DangerButton, 4.0f);
		DrawList->AddRectFilled(ImVec2(IconX, IconY - 6.0f), ImVec2(IconX + 12.0f, IconY + 6.0f), ContentColor);

		// Line under the whole bar while the game runs.
		DrawList->AddRectFilled(ImVec2(BarMin.x, BarMin.y + BarHeight - 3.0f), ImVec2(BarMin.x + BarWidth, BarMin.y + BarHeight), FEEditorStyle::DangerButton);
	}
	else
	{
		if (bDisabled)
			ContentColor = FEEditorStyle::TextDisabled;

		DrawList->AddRectFilled(ButtonMin, ButtonMax, bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button, 4.0f);
		DrawList->AddRect(ButtonMin, ButtonMax, FEEditorStyle::ButtonBorder, 4.0f);
		DrawList->AddTriangleFilled(ImVec2(IconX, IconY - 7.0f), ImVec2(IconX + 13.0f, IconY), ImVec2(IconX, IconY + 7.0f), bDisabled ? ContentColor : FEEditorStyle::Success);
	}

	DrawList->AddText(ImVec2(ButtonMin.x + 40.0f, floorf(ButtonMin.y + (ButtonSize.y - ImGui::GetFontSize()) / 2.0f)), ContentColor, Name);

	if (bClicked)
		EDITOR.SetGameMode(!bInGameMode);
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
	if (bSaveProjectAndRestartRequested)
	{
		bSaveProjectAndRestartRequested = false;
		FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
		if (CurrentProject != nullptr)
		{
			const std::string ProjectFolder = CurrentProject->GetProjectFolder();
			CurrentProject->SaveProject();
			CloseProjectAndCleanup();
			RestartWithProject(ProjectFolder);
			return;
		}
	}

	SetUpDocking();

	PREVIEW_MANAGER.Update();
	EDITOR_SCRIPTING_SYSTEM.Update();
	EDITOR_PLUGIN_MANAGER.Update(ENGINE.GetDeltaTime());

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
				if (ImGui::MenuItem("Save project", "Ctrl+S"))
				{
					PROJECT_MANAGER.GetCurrent()->SaveProject();
				}

				if (ImGui::MenuItem("Save project as..."))
				{
					FILE_SYSTEM.OpenFolderDialog([](const std::string& Path) {
						if (Path.empty())
							return;

						PROJECT_MANAGER.GetCurrent()->SaveProjectTo(Path + "\\");
						PROJECT_MANAGER.AddToProjectList(Path);
					});
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Build executable"))
					EDITOR_PROJECT_BUILD_SYSTEM.BuildExecutable(PROJECT_MANAGER.GetCurrent());

				ImGui::Separator();
				if (ImGui::MenuItem("Close project"))
				{
					if (PROJECT_MANAGER.GetCurrent()->IsModified())
					{
						ProjectWasModifiedPopUp::GetInstance().Show(PROJECT_MANAGER.GetCurrent(), false);
					}
					else
					{
						CloseProjectAndCleanup();
						if (IsRestartNeededForProject(""))
							RestartWithProject("");

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

						ImGui::PopStyleVar();
						ImGui::EndMenu();
						ImGui::EndMainMenuBar();

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

				if (ImGui::MenuItem("Editor Cameras", nullptr, bEditorCamerasWindowVisible))
				{
					bEditorCamerasWindowVisible = !bEditorCamerasWindowVisible;
				}

				if (ImGui::MenuItem("Log", nullptr, bLogWindowVisible))
				{
					bLogWindowVisible = !bLogWindowVisible;
				}

				if (ImGui::MenuItem("Plugins", nullptr, PLUGINS_WINDOW.bVisible))
				{
					PLUGINS_WINDOW.bVisible = !PLUGINS_WINDOW.bVisible;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Debug window", nullptr, bDebugWindowVisible))
					bDebugWindowVisible = !bDebugWindowVisible;

				if (ImGui::MenuItem("Temporary debug window", nullptr, bTemporaryDebugWindowVisible))
					bTemporaryDebugWindowVisible = !bTemporaryDebugWindowVisible;

				auto PossibleWindows = RENDERER.GetDebugOutputTextures();

				bool bSeparatorAdded = false;
				auto WindowIterator = PossibleWindows.begin();
				while (WindowIterator != PossibleWindows.end())
				{
					if (WindowIterator->second != nullptr)
					{
						if (!bSeparatorAdded)
						{
							ImGui::Separator();
							bSeparatorAdded = true;
						}

						FEImGuiWindow* CurrentWindow = FE_IMGUI_WINDOW_MANAGER.GetWindowByCaption(WindowIterator->first.c_str());
						bool bVisible = false;
						if (CurrentWindow != nullptr)
							bVisible = CurrentWindow->IsVisible();

						if (ImGui::MenuItem(WindowIterator->first.c_str(), nullptr, bVisible))
						{
							bVisible = !bVisible;

							if (bVisible)
							{
								if (CurrentWindow == nullptr)
								{
									CurrentWindow = new DebugTextureViewWindow(WindowIterator->second);
									CurrentWindow->SetCaption(WindowIterator->first);
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
					WindowIterator++;
				}

				ImGui::EndMenu();
			}

			EDITOR_PLUGIN_MANAGER.RenderMenu();

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About..."))
					ShowAboutDialog();

				ImGui::EndMenu();
			}

			RenderGameModeButton();
			RenderProjectStatusInMenuBar(PROJECT_MANAGER.GetCurrent());

			ImGui::EndMainMenuBar();
		}
		ImGui::PopStyleVar();

		RenderTemporaryDebugWindow();
		SCENE_GRAPH_WINDOW.Render();
		CONTENT_BROWSER_WINDOW.Render();
		INSPECTOR_WINDOW.Render();
		DisplayEditorCamerasWindow();
		DisplayLogWindow();
		PLUGINS_WINDOW.Render();
		EDITOR_PLUGIN_MANAGER.RenderWindows();
		if (!GizmosSettingsWindowObject.IsVisible())
			GizmosSettingsWindowObject.Show();

		EDITOR_SCENE_WINDOW_MANAGER.Update();

		RenderAllSubWindows();
		RenderAboutWindow();
	}
	else
	{
		PROJECT_MANAGER.DisplayProjectSelection();
		PLUGINS_WINDOW.Render();
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

	const bool bOpened = BeginStyledPopup("About");
	if (bOpened)
	{
		RenderPopupHeading("Focal Engine Editor");
		RenderPopupText("Version " + GetFE_EDITOR_VersionInfo().GetFullVersionString(), FEEditorStyle::TextMuted, 0.0f);
		RenderPopupCard(nullptr, "Modules", { ENGINE.GetFullVersion(), SCENE_GRAPH_WINDOW.GetSceneGraphUI()->GetFullVersion(), NODE_SYSTEM.GetFullVersion(), APPLICATION.GetFullVersion() });

		RenderPopupText("Bug reports and feedback are welcome on GitHub:");
		const std::string GitHubURL = "https://github.com/Azzinoth/FocalEngineEditor";
		const std::string LinkText = "github.com/Azzinoth/FocalEngineEditor";
		const ImVec2 LinkMin = ImGui::GetCursorScreenPos();
		const ImVec2 LinkSize = ImGui::CalcTextSize(LinkText.c_str());
		if (ImGui::InvisibleButton("##GitHubLink", LinkSize))
			PLUGINS_WINDOW.OpenURL(GitHubURL);

		const bool bLinkHovered = ImGui::IsItemHovered();
		ImDrawList* DrawList = ImGui::GetWindowDrawList();
		DrawList->AddText(LinkMin, bLinkHovered ? FEEditorStyle::AccentHovered : FEEditorStyle::Accent, LinkText.c_str());
		if (bLinkHovered)
		{
			DrawList->AddLine(ImVec2(LinkMin.x, LinkMin.y + LinkSize.y), ImVec2(LinkMin.x + LinkSize.x, LinkMin.y + LinkSize.y), FEEditorStyle::AccentHovered);
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		ShowToolTip("Opens the page in the browser.");

		if (RenderPopupButtons({ { "Close", FEPopupButtonKind::Primary } }) != -1)
			ImGui::CloseCurrentPopup();
	}
	EndStyledPopup(bOpened);
}

void FEEditor::OnViewportResize(FEUUID ViewportID)
{
	
}

void FEEditor::DropCallback(const int Count, const char** Paths)
{
	for (size_t i = 0; i < size_t(Count); i++)
	{
		if (FILE_SYSTEM.DoesDirectoryExist(Paths[i]) && PROJECT_MANAGER.GetCurrent() == nullptr)
			PROJECT_MANAGER.AddProjectsFromFolderToList(Paths[i]);

		if (PROJECT_MANAGER.GetCurrent() != nullptr)
		{
			if (EDITOR.GetFocusedScene() != nullptr)
			{
				std::vector<FEObject*> LoadedObjects = EDITOR.GetFocusedScene()->ImportAsset(Paths[i]);
				VIRTUAL_FILE_SYSTEM.CreateMissingFiles(LoadedObjects, VIRTUAL_FILE_SYSTEM.GetCurrentPath(), true);
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
							PROJECT_MANAGER.GetCurrent()->SetModified(true);
							PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(LoadedObjects[j]);
						}
					}
				}
			}
		}
	}
}

// The prefab editor works on a copy of the prefab scene, the copy has the editor tag and the name "Scene: <prefab name>".
static bool IsPrefabEditorScene(FEScene* Scene)
{
	return Scene->GetTag() == EDITOR_RESOURCE_TAG;
}

static std::string GetEditorCameraSceneName(FEScene* Scene)
{
	const std::string Name = Scene->GetName();
	const std::string PrefabEditorPrefix = "Scene: ";
	if (IsPrefabEditorScene(Scene) && Name.rfind(PrefabEditorPrefix, 0) == 0)
		return Name.substr(PrefabEditorPrefix.size());

	return Name;
}

static void RenderEditorCameraColumnTitle(const char* Title)
{
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Accent);
	ImGui::TextUnformatted(Title);
	ImGui::PopStyleColor();
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
}

// Transform and script are short, so they share the first column, the camera pages take the second one.
void FEEditor::RenderEditorCameraSettings(FEEntity* CameraEntity, const float Height) const
{
	const float ColumnWidth = 640.0f;
	ImGui::PushID(CameraEntity);
	ImGui::BeginChild("##TransformAndScript", ImVec2(ColumnWidth, Height));
	RenderEditorCameraColumnTitle("Transform");
	INSPECTOR_WINDOW.ShowTransformConfiguration(CameraEntity->GetName(), &CameraEntity->GetComponent<FETransformComponent>());
	if (CameraEntity->HasComponent<FENativeScriptComponent>())
	{
		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		RenderEditorCameraColumnTitle("Script");
		INSPECTOR_WINDOW.DisplayNativeScriptProperties(CameraEntity);
	}
	ImGui::EndChild();

	ImGui::SameLine(0.0f, 32.0f);
	ImGui::BeginChild("##Camera", ImVec2(ColumnWidth, Height));
	RenderEditorCameraColumnTitle("Camera");
	INSPECTOR_WINDOW.DisplayCameraProperties(CameraEntity);
	ImGui::EndChild();
	ImGui::PopID();
}

void FEEditor::DisplayEditorCamerasWindow() const
{
	if (!bEditorCamerasWindowVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (!ImGui::Begin("Editor Cameras", nullptr, ImGuiWindowFlags_None))
	{
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}

	struct EditorCameraRecord
	{
		FEScene* Scene = nullptr;
		FEEntity* Camera = nullptr;
	};

	std::vector<EditorCameraRecord> Cameras;
	auto EditorCameraIterator = PROJECT_MANAGER.GetCurrent()->SceneIDToEditorCameraID.begin();
	while (EditorCameraIterator != PROJECT_MANAGER.GetCurrent()->SceneIDToEditorCameraID.end())
	{
		FEScene* Scene = SCENE_MANAGER.GetSceneByID(EditorCameraIterator->first);
		FEEntity* CameraEntity = Scene == nullptr ? nullptr : Scene->GetEntity(EditorCameraIterator->second);
		if (CameraEntity != nullptr && CameraEntity->HasComponent<FECameraComponent>())
		{
			EditorCameraRecord Record;
			Record.Scene = Scene;
			Record.Camera = CameraEntity;
			Cameras.push_back(Record);
		}

		EditorCameraIterator++;
	}

	// The focused scene is the first one, the rest are in the order of their names.
	FEScene* FocusedScene = GetFocusedScene();
	std::sort(Cameras.begin(), Cameras.end(), [FocusedScene](const EditorCameraRecord& First, const EditorCameraRecord& Second) -> bool {
		if ((First.Scene == FocusedScene) != (Second.Scene == FocusedScene))
			return First.Scene == FocusedScene;

		return GetEditorCameraSceneName(First.Scene) < GetEditorCameraSceneName(Second.Scene);
	});

	// These pointers are only compared with scenes that exist now.
	static FEScene* SelectedScene = nullptr;
	static FEScene* LastFocusedScene = nullptr;
	if (FocusedScene != LastFocusedScene)
	{
		// The camera of a scene is selected when that scene gets the focus.
		LastFocusedScene = FocusedScene;
		SelectedScene = FocusedScene;
	}

	FEEntity* SelectedCamera = nullptr;
	for (size_t i = 0; i < Cameras.size(); i++)
	{
		if (Cameras[i].Scene == SelectedScene)
			SelectedCamera = Cameras[i].Camera;
	}

	if (SelectedCamera == nullptr && !Cameras.empty())
	{
		SelectedScene = Cameras[0].Scene;
		SelectedCamera = Cameras[0].Camera;
	}

	if (Cameras.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
		ImGui::TextUnformatted("No scene is open.");
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}

	const float SmallFontSize = 16.0f;
	const float ListWidth = 420.0f;
	const float Height = ImGui::GetContentRegionAvail().y;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, FEEditorStyle::Background);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
	ImGui::BeginChild("##EditorCameraList", ImVec2(ListWidth, Height), ImGuiChildFlags_AlwaysUseWindowPadding);
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	for (size_t i = 0; i < Cameras.size(); i++)
	{
		const bool bSelected = Cameras[i].Scene == SelectedScene;
		const bool bFocused = Cameras[i].Scene == FocusedScene;
		const ImVec2 CardMin = ImGui::GetCursorScreenPos();
		const ImVec2 CardSize = ImVec2(ImGui::GetContentRegionAvail().x, 92.0f);
		const ImVec2 CardMax = ImVec2(CardMin.x + CardSize.x, CardMin.y + CardSize.y);
		ImGui::PushID(static_cast<int>(i));
		if (ImGui::InvisibleButton("##EditorCameraCard", CardSize))
			SelectedScene = Cameras[i].Scene;
		ImGui::PopID();

		ImU32 CardColor = ImGui::IsItemHovered() ? FEEditorStyle::CardHovered : FEEditorStyle::Card;
		if (bSelected)
			CardColor = FEEditorStyle::SelectedItem;

		DrawList->AddRectFilled(CardMin, CardMax, CardColor, 4.0f);
		DrawList->AddRect(CardMin, CardMax, bSelected ? FEEditorStyle::Accent : FEEditorStyle::CardBorder, 4.0f);

		// What the camera sees.
		const ImVec2 PictureMin = ImVec2(CardMin.x + 10.0f, CardMin.y + 10.0f);
		const ImVec2 PictureMax = ImVec2(PictureMin.x + 128.0f, PictureMin.y + 72.0f);
		DrawList->AddRectFilled(PictureMin, PictureMax, FEEditorStyle::Background);
		FETexture* CameraView = RENDERER.GetCameraResult(Cameras[i].Camera);
		if (CameraView != nullptr)
			DrawList->AddImage(CameraView->GetTextureID(), PictureMin, PictureMax, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		DrawList->AddRect(PictureMin, PictureMax, FEEditorStyle::CardBorder);

		std::string Kind = IsPrefabEditorScene(Cameras[i].Scene) ? "Prefab" : "Scene";
		if (bFocused)
			Kind += ", in focus";

		const glm::vec3 CameraPosition = Cameras[i].Camera->GetComponent<FETransformComponent>().GetPosition();
		char PositionText[96] = {};
		sprintf_s(PositionText, "at %.1f, %.1f, %.1f", CameraPosition.x, CameraPosition.y, CameraPosition.z);

		const float TextX = PictureMax.x + 12.0f;
		DrawList->PushClipRect(ImVec2(TextX, CardMin.y), ImVec2(CardMax.x - 8.0f, CardMax.y), true);
		DrawList->AddText(ImVec2(TextX, CardMin.y + 10.0f), FEEditorStyle::Text, GetEditorCameraSceneName(Cameras[i].Scene).c_str());
		ImGui::PushFont(nullptr, SmallFontSize);
		DrawList->AddText(ImVec2(TextX, CardMin.y + 38.0f), bFocused ? FEEditorStyle::Accent : FEEditorStyle::TextMuted, Kind.c_str());
		DrawList->AddText(ImVec2(TextX, CardMin.y + 60.0f), FEEditorStyle::TextMuted, PositionText);
		ImGui::PopFont();
		DrawList->PopClipRect();
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	ImGui::SameLine(0.0f, 24.0f);
	RenderEditorCameraSettings(SelectedCamera, Height);

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
	JustTextWindowObject.Render();

	MessagePopUp::GetInstance().Render();
	
	ProjectWasModifiedPopUp::GetInstance().Render();

	FE_IMGUI_WINDOW_MANAGER.RenderAllWindows();
}

void FEEditor::SetImGuiStyle()
{
	ImGuiStyle* Style = &ImGui::GetStyle();
	ImVec4* Colors = Style->Colors;
	using namespace FEEditorStyle;

	Style->AntiAliasedLines = true;
	Style->AntiAliasedFill = true;
	Style->Alpha = 1.0f;
	Style->WindowRounding = 4.0f;
	Style->ChildRounding = 4.0f;
	Style->FrameRounding = 4.0f;
	Style->PopupRounding = 4.0f;
	Style->GrabRounding = 3.0f;
	Style->TabRounding = 4.0f;
	Style->ScrollbarRounding = 3.0f;
	Style->ScrollbarSize = 16.0f;
	Style->WindowBorderSize = 1.0f;
	Style->PopupBorderSize = 1.0f;
	Style->FrameBorderSize = 1.0f;
	Style->TabBarOverlineSize = 3.0f;
	Style->ItemSpacing = ImVec2(10.0f, 4.0f);
	Style->IndentSpacing = 22.0f;
	Style->FramePadding = ImVec2(6.0f, 4.0f);

	Colors[ImGuiCol_Text] = ToVec4(Text);
	Colors[ImGuiCol_TextDisabled] = ToVec4(TextMuted);
	Colors[ImGuiCol_WindowBg] = ToVec4(Panel);
	Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	Colors[ImGuiCol_PopupBg] = ToVec4(Card);
	Colors[ImGuiCol_Border] = ToVec4(ButtonBorder);
	Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	Colors[ImGuiCol_FrameBg] = ToVec4(Background);
	Colors[ImGuiCol_FrameBgHovered] = ToVec4(Card);
	Colors[ImGuiCol_FrameBgActive] = ToVec4(CardHovered);
	Colors[ImGuiCol_TitleBg] = ToVec4(Background);
	Colors[ImGuiCol_TitleBgActive] = ToVec4(Background);
	Colors[ImGuiCol_TitleBgCollapsed] = ToVec4(Background);
	Colors[ImGuiCol_MenuBarBg] = ToVec4(Background);
	Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	Colors[ImGuiCol_ScrollbarGrab] = ToVec4(ButtonHovered);
	Colors[ImGuiCol_ScrollbarGrabHovered] = ToVec4(ButtonBorder);
	Colors[ImGuiCol_ScrollbarGrabActive] = ToVec4(ButtonBorder);
	Colors[ImGuiCol_CheckMark] = ToVec4(Accent);
	Colors[ImGuiCol_SliderGrab] = ToVec4(Accent);
	Colors[ImGuiCol_SliderGrabActive] = ToVec4(AccentHovered);
	Colors[ImGuiCol_Button] = ToVec4(Button);
	Colors[ImGuiCol_ButtonHovered] = ToVec4(ButtonHovered);
	Colors[ImGuiCol_ButtonActive] = ToVec4(ButtonBorder);
	Colors[ImGuiCol_Header] = ToVec4(SelectedRow);
	Colors[ImGuiCol_HeaderHovered] = ToVec4(Button);
	Colors[ImGuiCol_HeaderActive] = ToVec4(ButtonHovered);
	Colors[ImGuiCol_Separator] = ToVec4(CardBorder);
	Colors[ImGuiCol_SeparatorHovered] = ToVec4(Accent);
	Colors[ImGuiCol_SeparatorActive] = ToVec4(Accent);
	Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	Colors[ImGuiCol_ResizeGripHovered] = ToVec4(AccentHovered);
	Colors[ImGuiCol_ResizeGripActive] = ToVec4(Accent);

	// Selected tab is lighter than the bar of tabs and has a line of accent color above it, the line is brighter in focused window.
	Colors[ImGuiCol_Tab] = ToVec4(Background);
	Colors[ImGuiCol_TabHovered] = ToVec4(ButtonHovered);
	Colors[ImGuiCol_TabSelected] = ToVec4(ButtonHovered);
	Colors[ImGuiCol_TabSelectedOverline] = ToVec4(Accent);
	Colors[ImGuiCol_TabDimmed] = ToVec4(Background);
	Colors[ImGuiCol_TabDimmedSelected] = ToVec4(Button);
	Colors[ImGuiCol_TabDimmedSelectedOverline] = ToVec4(AccentMuted);

	Colors[ImGuiCol_DockingPreview] = ImVec4(0.0f, 0.635f, 0.91f, 0.45f);
	Colors[ImGuiCol_DockingEmptyBg] = ToVec4(Background);
	Colors[ImGuiCol_PlotLines] = ToVec4(TextMuted);
	Colors[ImGuiCol_PlotLinesHovered] = ToVec4(AccentHovered);
	Colors[ImGuiCol_PlotHistogram] = ToVec4(Accent);
	Colors[ImGuiCol_PlotHistogramHovered] = ToVec4(AccentHovered);
	Colors[ImGuiCol_TableHeaderBg] = ToVec4(Card);
	Colors[ImGuiCol_TableBorderStrong] = ToVec4(CardBorder);
	Colors[ImGuiCol_TableBorderLight] = ToVec4(CardBorder);
	Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.025f);
	Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.635f, 0.91f, 0.35f);
	Colors[ImGuiCol_DragDropTarget] = ToVec4(Accent);
	Colors[ImGuiCol_NavCursor] = ToVec4(Accent);
	Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	Colors[ImGuiCol_ModalWindowDimBg] = ToVec4(Dim);
}

void FEEditor::SetUpImGui()
{
	ImGuiIO& IO = ImGui::GetIO();

	std::string ResourcesFolder = "Resources\\";

	const size_t PathLength = strlen((ResourcesFolder + "imgui.ini").c_str()) + 1;
	//FILE_SYSTEM.RemoveFile((ResourcesFolder + "imgui.ini").c_str());
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

std::vector<FEUUID> FEEditor::GetPluginsForProject(const std::string& ProjectFolder)
{
	std::vector<FEUUID> Result = EDITOR_PLUGIN_MANAGER.GetAlwaysLoadedPlugins();
	if (ProjectFolder.empty())
		return Result;

	const std::vector<FEUUID> ProjectPlugins = FEProject::ReadPluginIDs(ProjectFolder);
	for (size_t i = 0; i < ProjectPlugins.size(); i++)
	{
		if (std::find(Result.begin(), Result.end(), ProjectPlugins[i]) == Result.end())
			Result.push_back(ProjectPlugins[i]);
	}

	return Result;
}

bool FEEditor::IsRestartNeededForProject(const std::string& ProjectFolder) const
{
	// With automatic loading all plugins are loaded, so any project could be opened.
	if (EDITOR_PLUGIN_MANAGER.IsAutomaticLoadingEnabled())
		return false;

	const std::vector<FEUUID> NeededPlugins = GetPluginsForProject(ProjectFolder);
	const std::vector<FEUUID>& LoadedPlugins = EDITOR_PLUGIN_MANAGER.GetRequestedPlugins();
	if (NeededPlugins.size() != LoadedPlugins.size())
		return true;

	for (size_t i = 0; i < NeededPlugins.size(); i++)
	{
		if (std::find(LoadedPlugins.begin(), LoadedPlugins.end(), NeededPlugins[i]) == LoadedPlugins.end())
			return true;
	}

	return false;
}

bool FEEditor::RestartWithProject(const std::string& ProjectFolder)
{
	const std::string ExecutablePath = FILE_SYSTEM.GetExecutablePath();
	std::string CommandLine = "\"" + ExecutablePath + "\"";
	if (!ProjectFolder.empty())
		CommandLine += " -load_project path=\"" + ProjectFolder + "\"";

	// New process waits for this one to exit, so plugin folders and settings files are used by one editor at a time.
	CommandLine += " -wait_for_process id=" + std::to_string(GetCurrentProcessId());
	if (IsDebuggerPresent())
		CommandLine += " -wait_for_debugger";

	STARTUPINFOA StartupInfo = {};
	StartupInfo.cb = sizeof(StartupInfo);
	PROCESS_INFORMATION ProcessInfo = {};
	if (!CreateProcessA(ExecutablePath.c_str(), CommandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo))
	{
		LOG.Add("FEEditor::RestartWithProject: Failed to start \"" + ExecutablePath + "\", error code " + std::to_string(GetLastError()), "FE_EDITOR", FE_LOG_ERROR);
		return false;
	}

	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);
	ENGINE.Terminate();
	return true;
}

void FEEditor::RequestSaveProjectAndRestart()
{
	bSaveProjectAndRestartRequested = true;
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

FEEditorSceneWindow* FEEditor::GetEditorSceneWindow(const FEUUID& SceneID)
{
	return EDITOR_SCENE_WINDOW_MANAGER.GetSceneWindow(SceneID);
}

void FEEditor::CreateEditorWindowForScene(const FEUUID& SceneID, FEProject* CurrentProject)
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
	FEScene* GameModeScene = SCENE_MANAGER.DuplicateScene(EDITOR.GetFocusedScene()->GetID(), "GameMode", [](FEEntity* EntityToCheck) {
		return !(EntityToCheck->GetTag() == EDITOR_RESOURCE_TAG);
	}, FESceneFlag::Active | FESceneFlag::GameMode | FESceneFlag::Renderable);

	if (GameModeScene == nullptr)
	{
		LOG.Add("FEEditor::DuplicateScenesForGameMode: Failed to duplicate scene for game mode.", "FE_EDITOR_GAME_MODE", FE_LOG_ERROR);
		return false;
	}

	ParentIDToScenesInGameMode[EDITOR.GetFocusedScene()->GetID()] = GameModeScene;

	EDITOR.CreateEditorWindowForScene(GameModeScene->GetID());

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
			EDITOR_SCENE_WINDOW_MANAGER.DeleteSceneAndCleanup(SceneIterator->second->GetID());
			SceneIterator = ParentIDToScenesInGameMode.erase(SceneIterator);
		}
	}

	return true;
}

std::vector<FEUUID> FEEditor::GetEditorOpenedScenesIDs() const
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

bool FEEditor::SetFocusedScene(const FEUUID& NewSceneInFocusID)
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
			FEUUID EditorCameraID = CurrentProject->GetEditorCameraIDBySceneID(Scenes[i]->GetID());
			if (!UNIQUE_ID.IsNull(EditorCameraID))
			{
				if (CurrentMainCameraEntity != nullptr && CurrentMainCameraEntity->GetID() != EditorCameraID)
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