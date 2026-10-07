#include "FEEditorSceneWindow.h"
#include "../../FEEditor.h"

FEEditorSceneWindow::FEEditorSceneWindow(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	this->Scene = Scene;
	SceneID = Scene->GetID();
	Flags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	std::string WindowName = Scene->GetName();
	// Window name must be unique.
	WindowName += "##" + UNIQUE_ID.ToString(Scene->GetID());

	SetCaption(WindowName);
	SetBorderSize(0.0f);
	SetPadding(glm::vec2(0.0f, 0.0f));

	AcceptedTypes.push_back(FE_GAMEMODEL);
	AcceptedTypes.push_back(FE_PREFAB);
	AcceptedTypes.push_back(FE_POINT_CLOUD);
	ToolTipTexts.push_back("Drop to add to scene");
	ToolTipTexts.push_back("Drop to add to scene");
	ToolTipTexts.push_back("Drop to add to scene");
	CurrentDragAndDropCallback = DragAndDropCallBack;

	bHaveCloseButton = true;
}

bool FEEditorSceneWindow::DragAndDropCallBack(FEObject* Object, void** UserData)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return false;

	if (CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene()) == nullptr)
		return false;

	if (UserData == nullptr)
		return false;

	FEEditorSceneWindow* EditorSceneWindow = reinterpret_cast<FEEditorSceneWindow*>(UserData);
	if (EditorSceneWindow->GetScene() == nullptr)
		return false;

	if (Object->GetType() == FE_GAMEMODEL)
	{
		FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Object->GetID());
		if (GameModel == nullptr)
			return false;

		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();

		FEEntity* Entity = EditorSceneWindow->GetScene()->CreateEntity(Object->GetName());
		Entity->GetComponent<FETransformComponent>().SetPosition(CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
		Entity->AddComponent<FEGameModelComponent>(GameModel);

		SELECTED.SetSelected(Entity);
		PROJECT_MANAGER.GetCurrent()->SetModified(true);

		return true;

	}
	else if (Object->GetType() == FE_PREFAB)
	{
		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();

		FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(Object->GetID());
		if (Prefab == nullptr)
			return false;

		FEScene* PrefabScene = Prefab->GetScene();
		FENaiveSceneGraphNode* RootNode = PrefabScene->SceneGraph.GetRoot();

		std::vector<FEEntity*> AddedEntities = SCENE_MANAGER.InstantiatePrefab(Prefab, EditorSceneWindow->GetScene());
		if (!AddedEntities.empty())
		{
			AddedEntities[0]->GetComponent<FETransformComponent>().SetPosition(CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
			SELECTED.SetSelected(AddedEntities[0]);
			PROJECT_MANAGER.GetCurrent()->SetModified(true);

			return true;
		}
	}
	else if (Object->GetType() == FE_POINT_CLOUD)
	{
		FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(Object->GetID());
		if (PointCloud == nullptr)
			return false;

		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();
		FEEntity* Entity = EditorSceneWindow->GetScene()->CreateEntity(Object->GetName());
		Entity->GetComponent<FETransformComponent>().SetPosition(CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
		Entity->AddComponent<FEPointCloudComponent>(PointCloud);
		SELECTED.SetSelected(Entity);
		PROJECT_MANAGER.GetCurrent()->SetModified(true);

		return true;
	}

	return false;
}

FEEditorSceneWindow::~FEEditorSceneWindow()
{

}

void FEEditorSceneWindow::Show()
{
	FEImGuiWindow::Show();
}

void FEEditorSceneWindow::Render()
{
	if (!IsVisible())
		return;

	Scene = SCENE_MANAGER.GetSceneByID(SceneID);
	if (Scene == nullptr)
		return;

	if (bJustAdded)
	{
		ImGui::SetWindowFocus();
		Size = ImVec2(910, 512);
		bJustAdded = false;
	}

	ImGuiID DockspaceID = APPLICATION.GetMainWindow()->GetDefaultDockspaceID();
	if (DockspaceID != 0 && bShouldDockToCentralNode)
	{
		ImGuiDockNode* CentralNode = ImGui::DockBuilderGetCentralNode(DockspaceID);
		if (CentralNode != nullptr)
		{
			ImGui::SetNextWindowDockID(CentralNode->ID, ImGuiCond_Appearing);
			bShouldDockToCentralNode = false;
		}
	}

	FEImGuiWindow::Render();

	if (SceneWindowTarget != nullptr)
		SceneWindowTarget->StickToCurrentWindow();

	if (ImGui::IsWindowFocused())
		EDITOR.SetFocusedScene(Scene);

	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject == nullptr)
		return;

	FEEntity* CameraEntity = nullptr;
	if (Scene->HasFlag(FESceneFlag::EditorMode))
	{
		FEUUID EditorCameraID = CurrentProject->GetEditorCameraIDBySceneID(Scene->GetID());
		CameraEntity = Scene->GetEntity(EditorCameraID);
	}
	else if (Scene->HasFlag(FESceneFlag::GameMode))
	{
		CameraEntity = CAMERA_SYSTEM.GetMainCamera(Scene);
	}

	if (CameraEntity != nullptr)
	{
		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		if (CameraComponent.GetViewport() == nullptr || CameraComponent.GetViewport()->GetType() == FE_VIEWPORT_VIRTUAL)
		{
			FEUUID NewViewportID = ENGINE.CreateViewport(GetWindow());
			CAMERA_SYSTEM.SetCameraViewport(CameraEntity, NewViewportID);
			SELECTED.AddSceneData(Scene->GetID());
			GIZMO_MANAGER.AddSceneData(Scene->GetID());
			SceneWindowTarget = DRAG_AND_DROP_MANAGER.AddTarget(AcceptedTypes,
				CurrentDragAndDropCallback,
				reinterpret_cast<void**>(this),
				ToolTipTexts);
		}

		// It could be the case that viewport or camera was not ready when we set the viewport above, so we need to check again.
		if (SELECTED.GetSceneData(Scene->GetID()) == nullptr)
			SELECTED.AddSceneData(Scene->GetID());
		
		if (GIZMO_MANAGER.GetSceneData(Scene->GetID()) == nullptr)
			GIZMO_MANAGER.AddSceneData(Scene->GetID());

		ImGuiStyle& Style = ImGui::GetStyle();
		Style.WindowBorderSize = 0.0f;
		Style.WindowPadding = ImVec2(0.0f, 0.0f);

		FETexture* CameraResult = RENDERER.GetCameraResult(CAMERA_SYSTEM.GetMainCamera(Scene));
		if (CameraResult != nullptr)
		{
			ImGui::Image(CameraResult->GetTextureID(), ImVec2(GetWindow()->ContentRegionRect.GetWidth(), GetWindow()->ContentRegionRect.GetHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
	}
	else
	{
		ImGui::Text("No camera in scene");
	}
	
	if (bSelfContained)
		OnRenderEnd();
}

void FEEditorSceneWindow::OnRenderEnd()
{
	FEImGuiWindow::OnRenderEnd();
}

FEScene* FEEditorSceneWindow::GetScene() const
{
	return SCENE_MANAGER.GetSceneByID(SceneID);
}

glm::vec2 FEEditorSceneWindow::GetSize() const
{
	return glm::vec2(Size.x, Size.y);
}

void FEEditorSceneWindow::SetSize(glm::vec2 NewSize)
{
	Size = ImVec2(NewSize.x, NewSize.y);
}