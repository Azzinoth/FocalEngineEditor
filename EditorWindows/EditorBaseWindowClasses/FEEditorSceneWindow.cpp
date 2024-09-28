#include "FEEditorSceneWindow.h"
#include "../../FEEditor.h"

FEEditorSceneWindow::FEEditorSceneWindow(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	this->Scene = Scene;
	SceneID = Scene->GetObjectID();
	Flags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	std::string WindowName = Scene->GetName();
	// Window name must be unique.
	WindowName += "##" + Scene->GetObjectID();

	SetCaption(WindowName);
	SetBorderSize(0.0f);
	SetPadding(glm::vec2(0.0f, 0.0f));

	AcceptedTypes.push_back(FE_GAMEMODEL);
	AcceptedTypes.push_back(FE_PREFAB);
	ToolTipTexts.push_back("Drop to add to scene");
	ToolTipTexts.push_back("Drop to add to scene");
	CurrentDragAndDropCallback = DragAndDropCallBack;
}

bool FEEditorSceneWindow::DragAndDropCallBack(FEObject* Object, void** UserData)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return false;

	if (CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene()) == nullptr)
		return false;

	if (UserData == nullptr)
		return false;

	FEEditorSceneWindow* EditorSceneWindow = reinterpret_cast<FEEditorSceneWindow*>(UserData);
	if (EditorSceneWindow->GetScene() == nullptr)
		return false;

	if (Object->GetType() == FE_GAMEMODEL)
	{
		FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Object->GetObjectID());

		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();

		FEEntity* Entity = EditorSceneWindow->GetScene()->CreateEntity(Object->GetName());
		Entity->GetComponent<FETransformComponent>().SetPosition(CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
		Entity->AddComponent<FEGameModelComponent>(GameModel);

		SELECTED.SetSelected(Entity);
		PROJECT_MANAGER.GetCurrent()->SetModified(true);

		return true;

	}
	else if (Object->GetType() == FE_PREFAB)
	{
		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();

		FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(Object->GetObjectID());
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

	return false;
}

FEEditorSceneWindow::~FEEditorSceneWindow()
{

}

void FEEditorSceneWindow::Show()
{
	
}

void FEEditorSceneWindow::Render()
{
	if (bJustAdded)
	{
		ImGui::SetWindowFocus();
		Size = ImVec2(910, 512);
		bJustAdded = false;
	}

	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	if (SceneWindowTarget != nullptr)
		SceneWindowTarget->StickToCurrentWindow();

	if (ImGui::IsWindowFocused())
		EDITOR.SetFocusedScene(Scene);

	FEEntity* CameraEntity = CAMERA_SYSTEM.GetMainCameraEntity(Scene);
	if (CameraEntity != nullptr)
	{
		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

		if (CameraComponent.GetViewport() == nullptr)
		{
			std::string NewViewportID = ENGINE.AddViewport(GetWindow());
			CAMERA_SYSTEM.SetCameraViewport(CameraEntity, NewViewportID);
			SELECTED.AddSceneData(Scene->GetObjectID());
			GIZMO_MANAGER.AddSceneData(Scene->GetObjectID());
			SceneWindowTarget = DRAG_AND_DROP_MANAGER.AddTarget(AcceptedTypes,
				CurrentDragAndDropCallback,
				reinterpret_cast<void**>(this),
				ToolTipTexts);
		}

		ImGuiStyle& Style = ImGui::GetStyle();
		Style.WindowBorderSize = 0.0f;
		Style.WindowPadding = ImVec2(0.0f, 0.0f);

		FETexture* CameraResult = RENDERER.GetCameraResult(CAMERA_SYSTEM.GetMainCameraEntity(Scene));
		if (CameraResult != nullptr)
		{
			ImGui::Image((void*)(intptr_t)CameraResult->GetTextureID(), ImVec2(GetWindow()->ContentRegionRect.GetWidth(), GetWindow()->ContentRegionRect.GetHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
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
	return Scene;
}