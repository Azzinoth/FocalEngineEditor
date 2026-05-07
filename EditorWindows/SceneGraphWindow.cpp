#include "SceneGraphWindow.h"
#include "../FEEditor.h"
using namespace SceneGraphUI;

FEEditorSceneGraphWindow::FEEditorSceneGraphWindow()
{
	SceneGraphBackend = new FESceneGraphBackend();
	SceneGraphUI = new TreeView(SceneGraphBackend);
	SceneGraphUI->AddHiddenEntityTag(EDITOR_RESOURCE_TAG);

	SceneGraphUI->AddOnNodeClickedCallback(FEEditorSceneGraphWindow::OnNodeClicked);
	SceneGraphUI->SetNodeSelectionPredicate(FEEditorSceneGraphWindow::IsSelected);
	SceneGraphUI->SetNodeDisplayNameProvider(FEEditorSceneGraphWindow::GetDisplayedName);
	SceneGraphUI->AddOnNodeHoveredCallback(FEEditorSceneGraphWindow::OnNodeHovered);
	SceneGraphUI->AddAfterNodeRenderCallback(FEEditorSceneGraphWindow::AfterNodeRender);
	SceneGraphUI->SetContextMenuRenderingFunction(FEEditorSceneGraphWindow::ContextMenuRenderingFunction);
	SceneGraphUI->SetRenameNodeFunction(FEEditorSceneGraphWindow::RenameNodeFunction);
}

void FEEditorSceneGraphWindow::InitializeResources()
{
	GameModelSceneGraphIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/GameModelSceneGraphIcon.png", "GameModelSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(GameModelSceneGraphIcon, EDITOR_RESOURCE_TAG);

	EntityIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/entitySceneBrowserIcon.png", "entitySceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(EntityIcon, EDITOR_RESOURCE_TAG);
	InstancedEntityIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/InstancedEntitySceneGraphIcon.png", "InstancedEntitySceneGraphIcon");
	RESOURCE_MANAGER.SetTag(InstancedEntityIcon, EDITOR_RESOURCE_TAG);

	DirectionalLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/DirectionalLightSceneGraphIcon.png", "DirectionalLightSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(DirectionalLightIcon, EDITOR_RESOURCE_TAG);
	SpotLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/SpotLightSceneGraphIcon.png", "SpotLightSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(SpotLightIcon, EDITOR_RESOURCE_TAG);
	PointLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/PointLightSceneGraphIcon.png", "PointLightSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(PointLightIcon, EDITOR_RESOURCE_TAG);

	TerrainIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/TerrainSceneGraphIcon.png", "TerrainSceneGraphIcon.png");
	RESOURCE_MANAGER.SetTag(TerrainIcon, EDITOR_RESOURCE_TAG);

	CameraIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/CameraSceneGraphIcon.png", "CameraSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(CameraIcon, EDITOR_RESOURCE_TAG);

	PrefabSceneGraphIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/PrefabSceneGraphIcon.png", "PrefabSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(PrefabSceneGraphIcon, EDITOR_RESOURCE_TAG);

	SkyDomeIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/SkyDomeSceneGraphIcon.png", "SkyDomeSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(SkyDomeIcon, EDITOR_RESOURCE_TAG);

	LineIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/LineSceneGraphIcon.png", "LineSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(LineIcon, EDITOR_RESOURCE_TAG);

	VirtualUIIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/VirtualUISceneGraphIcon.png", "VirtualUISceneGraphIcon");
	RESOURCE_MANAGER.SetTag(VirtualUIIcon, EDITOR_RESOURCE_TAG);

	PointCloudIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/PointCloudSceneGraphIcon.png", "PointCloudSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(PointCloudIcon, EDITOR_RESOURCE_TAG);

	NativeScriptIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/NativeScriptSceneGraphIcon.png", "NativeScriptSceneGraphIcon");
	RESOURCE_MANAGER.SetTag(NativeScriptIcon, EDITOR_RESOURCE_TAG);

	VisibilityOnIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/VisibilityOn.png", "VisibilityOnIcon");
	RESOURCE_MANAGER.SetTag(VisibilityOnIcon, EDITOR_RESOURCE_TAG);
	VisibilityOffIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/VisibilityOff.png", "VisibilityOffIcon");
	RESOURCE_MANAGER.SetTag(VisibilityOffIcon, EDITOR_RESOURCE_TAG);

	CameraComponentIndicator.Icon = CameraIcon->GetTextureID();
	CameraComponentIndicator.bIsInteractive = false;
	// FE_TO_DO_MAYBE: Maybe after clicking it should focus on camera component in component list or something similar.
	//CameraComponentIndicator.OnClickCallback = [](FENaiveSceneGraphNode* Node) {};
	CameraComponentIndicator.bIsVisibleByDefault = false;
	CameraComponentIndicator.TooltipText = "Camera component";

	CameraComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FECameraComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(CameraComponentIndicator);

	LightComponentIndicator.Icon = 0;
	LightComponentIndicator.bIsInteractive = false;
	// FE_TO_DO_MAYBE: Maybe after clicking it should focus on camera component in component list or something similar.
	//CameraComponentIndicator.OnClickCallback = [](FENaiveSceneGraphNode* Node) {};
	LightComponentIndicator.bIsVisibleByDefault = false;
	LightComponentIndicator.TooltipText = "Light component";

	LightComponentIndicator.IsVisiblePredicate = [this](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FELightComponent>())
			return true;

		return false;
	};

	LightComponentIndicator.DynamicIconProvider = [this](SceneGraphUI::NodeHandle Node) -> ImTextureID {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return 0;

		if (!CurrentEntity->HasComponent<FELightComponent>())
			return 0;

		FELightComponent& LightComponent = CurrentEntity->GetComponent<FELightComponent>();
		if (LightComponent.GetType() == FE_DIRECTIONAL_LIGHT)
			return DirectionalLightIcon->GetTextureID();

		if (LightComponent.GetType() == FE_SPOT_LIGHT)
			return SpotLightIcon->GetTextureID();

		if (LightComponent.GetType() == FE_POINT_LIGHT)
			return PointLightIcon->GetTextureID();

		return 0;
	};
	SceneGraphUI->AddNodeWidget(LightComponentIndicator);

	GameModelComponentIndicator.Icon = GameModelSceneGraphIcon->GetTextureID();
	GameModelComponentIndicator.bIsInteractive = false;
	GameModelComponentIndicator.bIsVisibleByDefault = false;
	GameModelComponentIndicator.TooltipText = "Game model component";

	GameModelComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FEGameModelComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(GameModelComponentIndicator);

	TerrainComponentIndicator.Icon = TerrainIcon->GetTextureID();
	TerrainComponentIndicator.bIsInteractive = false;
	TerrainComponentIndicator.bIsVisibleByDefault = false;
	TerrainComponentIndicator.TooltipText = "Terrain component";

	TerrainComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FETerrainComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(TerrainComponentIndicator);

	InstancedEntityComponentIndicator.Icon = InstancedEntityIcon->GetTextureID();
	InstancedEntityComponentIndicator.bIsInteractive = false;
	InstancedEntityComponentIndicator.bIsVisibleByDefault = false;
	InstancedEntityComponentIndicator.TooltipText = "Instanced entity component";

	InstancedEntityComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FEInstancedComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(InstancedEntityComponentIndicator);

	PrefabSceneGraphIndicator.Icon = PrefabSceneGraphIcon->GetTextureID();
	PrefabSceneGraphIndicator.bIsInteractive = false;
	PrefabSceneGraphIndicator.bIsVisibleByDefault = false;
	PrefabSceneGraphIndicator.TooltipText = "Prefab Instance component";

	PrefabSceneGraphIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FEPrefabInstanceComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(PrefabSceneGraphIndicator);

	SkyDomeComponentIndicator.Icon = SkyDomeIcon->GetTextureID();
	SkyDomeComponentIndicator.bIsInteractive = false;
	SkyDomeComponentIndicator.bIsVisibleByDefault = false;
	SkyDomeComponentIndicator.TooltipText = "Sky dome component";

	SkyDomeComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FESkyDomeComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(SkyDomeComponentIndicator);

	LineComponentIndicator.Icon = LineIcon->GetTextureID();
	LineComponentIndicator.bIsInteractive = false;
	LineComponentIndicator.bIsVisibleByDefault = false;
	LineComponentIndicator.TooltipText = "Line component";

	LineComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FELineComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(LineComponentIndicator);

	VirtualUIComponentIndicator.Icon = VirtualUIIcon->GetTextureID();
	VirtualUIComponentIndicator.bIsInteractive = false;
	VirtualUIComponentIndicator.bIsVisibleByDefault = false;
	VirtualUIComponentIndicator.TooltipText = "Virtual UI component";

	VirtualUIComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FEVirtualUIComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(VirtualUIComponentIndicator);

	PointCloudComponentIndicator.Icon = PointCloudIcon->GetTextureID();
	PointCloudComponentIndicator.bIsInteractive = false;
	PointCloudComponentIndicator.bIsVisibleByDefault = false;
	PointCloudComponentIndicator.TooltipText = "Point cloud component";

	PointCloudComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FEPointCloudComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(PointCloudComponentIndicator);

	NativeScriptComponentIndicator.Icon = NativeScriptIcon->GetTextureID();
	NativeScriptComponentIndicator.bIsInteractive = false;
	NativeScriptComponentIndicator.bIsVisibleByDefault = false;
	NativeScriptComponentIndicator.TooltipText = "Native script component";

	NativeScriptComponentIndicator.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		if (CurrentEntity->HasComponent<FENativeScriptComponent>())
			return true;

		return false;
	};
	SceneGraphUI->AddNodeWidget(NativeScriptComponentIndicator);

	VisibilityToggleWidget.Icon = VisibilityOnIcon->GetTextureID();
	VisibilityToggleWidget.DynamicIconProvider = [this](SceneGraphUI::NodeHandle Node) -> ImTextureID {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return 0;

		bool bIsVisible = CurrentEntity->IsVisible();
		return bIsVisible ? VisibilityOnIcon->GetTextureID() : VisibilityOffIcon->GetTextureID();
	};

	VisibilityToggleWidget.bIsInteractive = true;
	VisibilityToggleWidget.OnClickCallback = [](SceneGraphUI::NodeHandle Node) {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return;

		bool bIsVisible = CurrentEntity->IsVisible();
		CurrentEntity->SetVisible(!bIsVisible);
	};
	VisibilityToggleWidget.bIsVisibleByDefault = true;
	VisibilityToggleWidget.IsVisiblePredicate = [](SceneGraphUI::NodeHandle Node) -> bool {
		FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
		if (CurrentEntity == nullptr)
			return false;

		return true;
	};
	VisibilityToggleWidget.TooltipText = "Show/Hide";

	SceneGraphUI->AddNodeWidget(VisibilityToggleWidget);
}

void FEEditorSceneGraphWindow::Clear()
{
	bLastFrameWasInvisible = true;
}

static void CreateInstancedEntityCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_PREFAB)
	{
		FEGameModel* SelectedGameModel = RESOURCE_MANAGER.GetGameModel(SelectionsResult[0]->GetObjectID());
		if (SelectedGameModel == nullptr)
			return;

		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();
		FEEntity* Entity = EDITOR.GetFocusedScene()->CreateEntity();
		Entity->GetComponent<FETransformComponent>().SetPosition(CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
		Entity->AddComponent<FEGameModelComponent>(SelectedGameModel);
		Entity->AddComponent<FEInstancedComponent>();
		SELECTED.SetSelected(Entity);

		PROJECT_MANAGER.GetCurrent()->SetModified(true);
	}
}

static void CreateEntityCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_PREFAB)
	{
		FEPrefab* SelectedPrefab = RESOURCE_MANAGER.GetPrefab(SelectionsResult[0]->GetObjectID());
		if (SelectedPrefab == nullptr)
			return;

		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();
		FEEntity* Entity = EDITOR.GetFocusedScene()->CreateEntity();
		Entity->GetComponent<FETransformComponent>().SetPosition(CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
		SELECTED.SetSelected(Entity);

		PROJECT_MANAGER.GetCurrent()->SetModified(true);
	}
}

DragAndDropTarget* FEEditorSceneGraphWindow::GetSceneNodeDragAndDropTarget(FENaiveSceneGraphNode* NodeToFind)
{
	std::string NewNodeID = NodeToFind->GetObjectID();
	int64_t UniqueID = static_cast<int64_t>(std::hash<std::string>{}(NewNodeID.c_str()));

	if (SceneNodeDragAndDropTargets.find(UniqueID) == SceneNodeDragAndDropTargets.end())
	{
		SceneNodeDragAndDropTargets[UniqueID] = DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_SCENE_GRAPH_NODE },
			SceneNodeDragAndDropCallback, reinterpret_cast<void**>(NodeToFind),
			std::vector<std::string> { "Drop move to that parent" });
	}

	return SceneNodeDragAndDropTargets[UniqueID];
}

void FEEditorSceneGraphWindow::Render()
{
	if (!bVisible)
	{
		bLastFrameWasInvisible = true;
		return;
	}

	FEScene* CurrentScene = EDITOR.GetFocusedScene();
	if (CurrentScene == nullptr)
	{
		bLastFrameWasInvisible = true;
		return;
	}

	if (ImGui::Button("ExpandAll"))
	{
		SceneGraphUI->ExpandAllNodes();
	}

	if (ImGui::Button("CollapseAll"))
	{
		SceneGraphUI->CollapseAllNodes();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::Begin("Scene Graph", nullptr, ImGuiWindowFlags_None))
	{
		SceneGraphBackend->SetSceneID(CurrentScene->GetObjectID());
		SceneGraphUI->Render(SceneGraphUI::NodeHandle(CurrentScene->SceneGraph.GetRoot(), SceneGraphBackend));
		if (LastFrameRootNodeID != CurrentScene->SceneGraph.GetRoot()->GetObjectID())
		{
			LastFrameRootNodeID = CurrentScene->SceneGraph.GetRoot()->GetObjectID();
			SceneGraphUI->SetNodeExpanded(SceneGraphUI::NodeHandle(CurrentScene->SceneGraph.GetRoot(), SceneGraphBackend), true);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void FEEditorSceneGraphWindow::OnNodeClicked(SceneGraphUI::NodeHandle Node, ImGuiMouseButton_ MouseButton)
{
	if (MouseButton != ImGuiMouseButton_Left)
		return;

	FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
	if (CurrentEntity == nullptr)
		return;

	SELECTED.SetSelected(CurrentEntity);
}

bool FEEditorSceneGraphWindow::IsSelected(SceneGraphUI::NodeHandle Node)
{
	FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
	if (CurrentEntity == nullptr)
		return false;

	if (SELECTED.GetSelected(EDITOR.GetFocusedScene()) != nullptr && CurrentEntity != nullptr)
	{
		if (SELECTED.GetSelected(EDITOR.GetFocusedScene())->GetObjectID() == CurrentEntity->GetObjectID())
			return true;
	}

	return false;
}

std::string FEEditorSceneGraphWindow::GetDisplayedName(SceneGraphUI::NodeHandle Node)
{
	size_t Depth = Node.As<FENaiveSceneGraphNode>()->GetDepth();

	FEEntity* CurrentEntity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
	std::string DisplayedName = CurrentEntity == nullptr ? Node.As<FENaiveSceneGraphNode>()->GetName() : CurrentEntity->GetName();

	if (CurrentEntity == nullptr && Depth == 0)
	{
		FEScene* ParentScene = EDITOR.GetFocusedScene();
		if (ParentScene != nullptr)
			DisplayedName = ParentScene->GetName();
	}

	return DisplayedName;
}

void FEEditorSceneGraphWindow::OnNodeHovered(SceneGraphUI::NodeHandle Node)
{
	FENaiveSceneGraphNode* NaiveNode = Node.As<FENaiveSceneGraphNode>();
	if (!DRAG_AND_DROP_MANAGER.ObjectIsDraged())
	{
		if (ImGui::IsMouseDragging(0))
			DRAG_AND_DROP_MANAGER.SetObjectToDrag(NaiveNode, nullptr, ImVec2(), ImVec2());
	}
}

void FEEditorSceneGraphWindow::AfterNodeRender(SceneGraphUI::NodeHandle Node)
{
	FENaiveSceneGraphNode* NaiveNode = Node.As<FENaiveSceneGraphNode>();
	SCENE_GRAPH_WINDOW.GetSceneNodeDragAndDropTarget(NaiveNode)->StickToItem();
}

void FEEditorSceneGraphWindow::ContextMenuRenderingFunction(SceneGraphUI::NodeHandle Node)
{
	FENaiveSceneGraphNode* NaiveNode = Node.As<FENaiveSceneGraphNode>();
	FEScene* CurrentScene = EDITOR.GetFocusedScene();
	if (CurrentScene == nullptr)
		return;

	if (NaiveNode == nullptr)
	{
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Empty entity"))
			{
				CurrentScene->CreateEntity("Unnamed entity");
			}

			ImGui::EndMenu();
		}
	}
	else
	{
		FEEntity* HoveredEntity = NaiveNode->GetEntity();
		if (HoveredEntity != nullptr)
		{
			if (ImGui::MenuItem("Rename"))
			{
				SCENE_GRAPH_WINDOW.SceneGraphUI->SetNodeForRenaming(Node);
			}

			if (ImGui::MenuItem("Delete"))
			{
				if (SELECTED.GetSelected(CurrentScene) == HoveredEntity)
					SELECTED.Clear(CurrentScene);

				CurrentScene->DeleteEntity(HoveredEntity);
			}
		}
	}
}

TreeView* FEEditorSceneGraphWindow::GetSceneGraphUI() const
{
	return SceneGraphUI;
}

void FEEditorSceneGraphWindow::RenameNodeFunction(SceneGraphUI::NodeHandle Node, std::string NewName)
{
	FENaiveSceneGraphNode* NaiveNode = Node.As<FENaiveSceneGraphNode>();
	if (NaiveNode == nullptr)
		return;

	FEEntity* CurrentEntity = NaiveNode->GetEntity();
	if (CurrentEntity == nullptr)
		return;

	NaiveNode->SetName(NewName);
	CurrentEntity->SetName(NewName);
}