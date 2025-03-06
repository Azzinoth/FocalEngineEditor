#include "SceneGraphWindow.h"
#include "../FEEditor.h"

FEEditorSceneGraphWindow::FEEditorSceneGraphWindow()
{
	strcpy_s(FilterForEntities, "Filter entities...");
}

void FEEditorSceneGraphWindow::InitializeResources()
{
	EntityIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/entitySceneBrowserIcon.png", "entitySceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(EntityIcon, EDITOR_RESOURCE_TAG);
	InstancedEntityIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/instancedEntitySceneBrowserIcon.png", "instancedEntitySceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(InstancedEntityIcon, EDITOR_RESOURCE_TAG);

	DirectionalLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/directionalLightSceneBrowserIcon.png", "directionalLightSceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(DirectionalLightIcon, EDITOR_RESOURCE_TAG);
	SpotLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/spotLightSceneBrowserIcon.png", "spotLightSceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(SpotLightIcon, EDITOR_RESOURCE_TAG);
	PointLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/pointLightSceneBrowserIcon.png", "pointLightSceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(PointLightIcon, EDITOR_RESOURCE_TAG);

	TerrainIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/terrainSceneBrowserIcon.png", "terrainSceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(TerrainIcon, EDITOR_RESOURCE_TAG);

	CameraIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/cameraSceneBrowserIcon.png", "cameraSceneBrowserIcon");
	RESOURCE_MANAGER.SetTag(CameraIcon, EDITOR_RESOURCE_TAG);
}

void FEEditorSceneGraphWindow::Clear()
{
	strcpy_s(FilterForEntities, "");
	bLastFrameWasInvisible = true;
}

static void CreateInstancedEntityCallBack(const std::vector<FEObject*> SelectionsResult)
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

static void CreateEntityCallBack(const std::vector<FEObject*> SelectionsResult)
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

// FIXME: Make icons colored and place them in the right place.
// Currently this function is not working properly.
void FEEditorSceneGraphWindow::DrawCorrectIcon(FEEntity* SceneEntity) const
{
	if (SceneEntity == nullptr)
		return;

	float CurrentCursorPosX = ImGui::GetCursorPosX();
	ImGui::SameLine();

	if (SceneEntity->HasComponent<FEInstancedComponent>())
	{
		ImGui::Image((void*)(intptr_t)InstancedEntityIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	if (SceneEntity->HasComponent<FELightComponent>())
	{
		if (SceneEntity->GetComponent<FELightComponent>().GetType() == FE_DIRECTIONAL_LIGHT)
		{
			ImGui::Image((void*)(intptr_t)DirectionalLightIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
		if (SceneEntity->GetComponent<FELightComponent>().GetType() == FE_SPOT_LIGHT)
		{
			ImGui::Image((void*)(intptr_t)SpotLightIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
		if (SceneEntity->GetComponent<FELightComponent>().GetType() == FE_POINT_LIGHT)
		{
			ImGui::Image((void*)(intptr_t)PointLightIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
	}

	if (SceneEntity->HasComponent<FETerrainComponent>())
	{
		ImGui::Image((void*)(intptr_t)TerrainIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	if (SceneEntity->HasComponent<FECameraComponent>())
	{
		ImGui::Image((void*)(intptr_t)CameraIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	ImGui::SetCursorPosX(CurrentCursorPosX);
}

DragAndDropTarget* FEEditorSceneGraphWindow::GetSceneNodeDragAndDropTarget(FENaiveSceneGraphNode* NodeToFind)
{
	int64_t UniqueID = 0;
	// If node is root.
	if (NodeToFind->GetEntity() == nullptr)
	{
		UniqueID = -1;
	}
	else
	{
		UniqueID = static_cast<intptr_t>(std::hash<std::string>{}(NodeToFind->GetEntity()->GetObjectID().c_str()));
	}

	if (SceneNodeDragAndDropTargets.find(UniqueID) == SceneNodeDragAndDropTargets.end())
	{
		SceneNodeDragAndDropTargets[UniqueID] = DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_SCENE_GRAPH_NODE },
			SceneNodeDragAndDropCallback, reinterpret_cast<void**>(NodeToFind),
			std::vector<std::string> { "Drop move to that parent" });
	}

	return SceneNodeDragAndDropTargets[UniqueID];
}

void FEEditorSceneGraphWindow::RenderNodeBackground()
{
	// Calculate the expected height of the node.
	float NodeHeight = ImGui::GetFrameHeight();
	NodeHeight += BackgroundHeightModifier;

	ImRect BackgroundRect = ImRect(ImVec2(bIndintationAwareNodeBackground ? ImGui::GetCursorScreenPos().x : ImGui::GetWindowContentRegionMin().x, ImGui::GetCursorScreenPos().y),
								   ImVec2(ImGui::GetWindowContentRegionMax().x, ImGui::GetCursorScreenPos().y + NodeHeight));

	// Shift the background rectangle
	BackgroundRect.Min.y += BackgroundColorYShift;
	BackgroundRect.Max.y += BackgroundColorYShift;

	// Render
	ImColor BackgroundColor = bBackgroundColorSwitch ? ImColor(EvenNodeBackgroundColor) : ImColor(OddNodeBackgroundColor);
	ImGui::GetWindowDrawList()->AddRectFilled(BackgroundRect.Min, BackgroundRect.Max, BackgroundColor);
	bBackgroundColorSwitch = !bBackgroundColorSwitch;
}

ImRect FEEditorSceneGraphWindow::RenderSubTree(FENaiveSceneGraphNode* SubTreeRoot)
{
	if (SubTreeRoot == nullptr)
		return ImRect();

	FEEntity* CurrentEntity = SubTreeRoot->GetEntity();
	if (CurrentEntity != nullptr)
	{
		if (CurrentEntity->GetTag() == EDITOR_RESOURCE_TAG)
			return ImRect();
	}

	SceneNodeDragAndDropTargetIndex++;
	int64_t UniqueID = 0;
	bool bIsLeaf = SubTreeRoot->GetChildren().size() == 0;
	ImGuiTreeNodeFlags NodeFlags = bIsLeaf ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen : ImGuiTreeNodeFlags_OpenOnArrow;
	std::string Name = SubTreeRoot->GetParent() == nullptr ? PROJECT_MANAGER.GetCurrent()->GetName() : SubTreeRoot->GetName();

	if (SELECTED.GetSelected(EDITOR.GetFocusedScene()) != nullptr && CurrentEntity != nullptr)
	{
		if (SELECTED.GetSelected(EDITOR.GetFocusedScene())->GetObjectID() == CurrentEntity->GetObjectID())
		{
			NodeFlags |= ImGuiTreeNodeFlags_Selected;
		}
	}

	// If node is root.
	if (CurrentEntity == nullptr)
	{
		UniqueID = -1;
	}
	else
	{
		UniqueID = static_cast<intptr_t>(std::hash<std::string>{}(CurrentEntity->GetObjectID().c_str()));
	}

	// If last frame scene graph was invisible, open root node.
	if (bLastFrameWasInvisible && CurrentEntity == nullptr)
		ImGui::SetNextItemOpen(true);

	if (bUseNodeBackground)
		RenderNodeBackground();

	bool bOpened = ImGui::TreeNodeEx((void*)UniqueID, NodeFlags, Name.c_str(), 0);
	GetSceneNodeDragAndDropTarget(SubTreeRoot)->StickToItem();
	const ImRect NodeRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	//DrawCorrectIcon(SubTreeRoot->GetEntity());

	ImVec2 VerticalLineStart = ImGui::GetCursorScreenPos();
	VerticalLineStart.x += VerticalTreeLineXOffset;
	VerticalLineStart.y += VerticalTreeLineYOffset;
	ImVec2 VerticalLineEnd = VerticalLineStart;

	if (ImGui::IsItemClicked())
	{
		if (SubTreeRoot->GetParent() != nullptr)
		{
			SELECTED.SetSelected(CurrentEntity);
		}
	}

	if (ImGui::IsItemHovered())
	{
		if (!bShouldOpenContextMenu && !DRAG_AND_DROP_MANAGER.ObjectIsDraged())
		{
			/*std::string AdditionalTypeInfo;
			if (FilteredResources[i]->GetType() == FE_TEXTURE)
			{
				AdditionalTypeInfo += "\nTexture type: ";
				AdditionalTypeInfo += FETexture::TextureInternalFormatToString(RESOURCE_MANAGER.GetTexture(FilteredResources[i]->GetObjectID())->GetInternalFormat());
			}

			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(("Name: " + FilteredResources[i]->GetName() +
				"\nType: " + FEObjectTypeToString(FilteredResources[i]->GetType()) +
				AdditionalTypeInfo +
				"\nPath: " + VIRTUAL_FILE_SYSTEM.GetCurrentPath()
				).c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();*/

			SceneGraphNodeHoveredID = SubTreeRoot->GetObjectID();

			ItemUnderMouse = UniqueID;

			if (ImGui::IsMouseDragging(0))
				DRAG_AND_DROP_MANAGER.SetObjectToDrag(SubTreeRoot, nullptr, ImVec2(), ImVec2());
		}
	}

	if (bOpened)
	{
		auto Children = SubTreeRoot->GetChildren();
		if (!Children.empty())
		{
			for (size_t i = 0; i < Children.size(); i++)
			{
				// Double check if we need to draw child to make sure we don't draw internal editor resources.
				FEEntity* ChildEntity = Children[i]->GetEntity();
				if (ChildEntity != nullptr)
				{
					if (ChildEntity->GetTag() == EDITOR_RESOURCE_TAG)
						continue;
				}

				const ImRect ChildRect = RenderSubTree(Children[i]);
				
				// Draw horizontal line.
				const float MiddlePoint = (ChildRect.Min.y + ChildRect.Max.y) / 2.0f;
				bool bChildHaveChildren = Children[i]->GetChildren().size() > 0;
				ImVec2 HorizontalLineStart = ImVec2(VerticalLineStart.x, MiddlePoint);
				ImVec2 HorizontalLineEnd = ImVec2(VerticalLineStart.x + HorizontalTreeLineLength, MiddlePoint);
				if (bChildHaveChildren)
					HorizontalLineEnd.x += HorizontalTreeLineLengthParentOffset;
				ImGui::GetWindowDrawList()->AddLine(HorizontalLineStart, HorizontalLineEnd, ImColor(VerticalTreeLineColor));

				VerticalLineEnd.y = MiddlePoint;
			}

			ImGui::TreePop();
		}

		// Draw vertical line.
		ImGui::GetWindowDrawList()->AddLine(VerticalLineStart, VerticalLineEnd, ImColor(VerticalTreeLineColor));
	}

	SceneGraphBackgroundRect.Max.x = SceneGraphBackgroundRect.Max.x > NodeRect.Max.x ? SceneGraphBackgroundRect.Max.x : NodeRect.Max.x;
	SceneGraphBackgroundRect.Max.y = SceneGraphBackgroundRect.Max.y > NodeRect.Max.y ? SceneGraphBackgroundRect.Max.y : NodeRect.Max.y;

	return NodeRect;
}

int FEEditorSceneGraphWindow::FilterInputTextCallback(ImGuiInputTextCallbackData* Data)
{
	if (Data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
	{
		bool isFocused = ImGui::IsItemActive();
		if (SCENE_GRAPH_WINDOW.bIsPlaceHolderTextUsed)
		{
			// Check if the input just gained focus
			if (isFocused && !SCENE_GRAPH_WINDOW.bFilterInputWasFocused)
			{
				strcpy_s(SCENE_GRAPH_WINDOW.FilterForEntities, "");
				
				// Update ImGui's buffer.
				Data->BufDirty = true;
				Data->DeleteChars(0, Data->BufTextLen);
				Data->InsertChars(0, SCENE_GRAPH_WINDOW.FilterForEntities);

				SCENE_GRAPH_WINDOW.bIsPlaceHolderTextUsed = false;
			}
		}

		SCENE_GRAPH_WINDOW.bFilterInputWasFocused = isFocused;
	}

	return 0;
}


void FEEditorSceneGraphWindow::RenderFilterInput()
{
	const bool bIsPlaceHolderTextUsedWasOn = bIsPlaceHolderTextUsed;

	if (bIsPlaceHolderTextUsedWasOn)
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 150));

	// Setting up the callback for the input text.
	std::function<int(ImGuiInputTextCallbackData*)> Callback = std::bind(&FEEditorSceneGraphWindow::FilterInputTextCallback, std::placeholders::_1);
	auto StaticCallback = [](ImGuiInputTextCallbackData* Data) -> int {
		const auto& Callback = *static_cast<std::function<int(ImGuiInputTextCallbackData*)>*>(Data->UserData);
		return Callback(Data);
	};

	if (ImGui::InputText("##SceneGraphWindowFilter", FilterForEntities, FilterInputBufferSize, ImGuiInputTextFlags_CallbackAlways, StaticCallback, &Callback))
	{
		
	}

	if (!ImGui::IsItemActive())
	{
		if (strlen(FilterForEntities) == 0)
		{
			strcpy_s(FilterForEntities, PlaceHolderTextString.c_str());
			bIsPlaceHolderTextUsed = true;
			bFilterInputWasFocused = false;
		}
	}

	if (bIsPlaceHolderTextUsedWasOn)
		ImGui::PopStyleColor();
}

void FEEditorSceneGraphWindow::RenderSceneGraph()
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return;

	FENaiveSceneGraphNode* Root = EDITOR.GetFocusedScene()->SceneGraph.GetRoot();

	if (bSceneNodeTargetsDirty)
		SceneNodeDragAndDropTargets.clear();

	SceneGraphBackgroundRect.Min = ImGui::GetCursorScreenPos();
	SceneGraphBackgroundRect.Max = SceneGraphBackgroundRect.Min;

	RenderFilterInput();

	SceneNodeDragAndDropTargetIndex = -1;
	bBackgroundColorSwitch = true;
	RenderSubTree(Root);

	SceneGraphBackgroundRect.Max.x = ImGui::GetWindowContentRegionMax().x;
	SceneGraphBackgroundRect.Min -= ImVec2(10.0f, 10.0f);
	SceneGraphBackgroundRect.Max += ImVec2(10.0f, 10.0f);
	// Render box around the scene graph.
	ImGui::GetWindowDrawList()->AddRect(SceneGraphBackgroundRect.Min, SceneGraphBackgroundRect.Max, ImColor(ImVec4(0.0f, 0.0f, 0.0f, 0.7f)), 2.0f);

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.0f);

	// Uncomment this to tweak the scene graph rendering.
	/*ImGui::DragFloat("VerticalTreeLineXOffset", &VerticalTreeLineXOffset, 0.1f);
	ImGui::DragFloat("VerticalTreeLineYOffset", &VerticalTreeLineYOffset, 0.1f);
	ImGui::DragFloat("HorizontalTreeLineLenght", &HorizontalTreeLineLength, 0.1f);
	ImGui::DragFloat("HorizontalTreeLineLenghtOffset", &HorizontalTreeLineLengthParentOffset, 0.1f);
	ImGui::DragFloat("BackgroundColorYShift", &BackgroundColorYShift, 0.1f);
	ImGui::DragFloat("BackgroundHeightModifier", &BackgroundHeightModifier, 0.1f);
	ImGui::ColorEdit4("VerticalTreeLineColor", (float*)&VerticalTreeLineColor);
	ImGui::ColorEdit4("EvenNodeBackgroundColor", (float*)&EvenNodeBackgroundColor);
	ImGui::ColorEdit4("OddNodeBackgroundColor", (float*)&OddNodeBackgroundColor);
	ImGui::Checkbox("Draw node background", &bUseNodeBackground);
	ImGui::Checkbox("IndintationAwareNodeBackground", &bIndintationAwareNodeBackground);*/
	
	if (bSceneNodeTargetsDirty)
		bSceneNodeTargetsDirty = false;

	bLastFrameWasInvisible = false;
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

	if (!bShouldOpenContextMenu)
		SceneGraphNodeHoveredID = "";

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Scene Entities", nullptr, ImGuiWindowFlags_None);

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

	RenderSceneGraph();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	bool bOpenContextMenu = false;
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		bOpenContextMenu = true;

	if (bOpenContextMenu)
		ImGui::OpenPopup("##context_menu");

	bShouldOpenContextMenu = false;

	if (ImGui::BeginPopup("##context_menu"))
	{
		bShouldOpenContextMenu = true;

		if (SceneGraphNodeHoveredID.empty())
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
			FENaiveSceneGraphNode* HoveredNode = CurrentScene->SceneGraph.GetNode(SceneGraphNodeHoveredID);
			if (HoveredNode != nullptr)
			{
				FEEntity* HoveredEntity = CurrentScene->SceneGraph.GetNode(SceneGraphNodeHoveredID)->GetEntity();
				if (HoveredEntity != nullptr)
				{
					if (ImGui::MenuItem("Rename"))
					{
						RenamePopUp::GetInstance().Show(HoveredNode);
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

		ImGui::EndPopup();
	}

	// FIXME: Grid settings should be moved to other window, not scene graph window.
	//static bool bDisplayGrid = true;
	//ImGui::Checkbox("Display grid", &bDisplayGrid);

	//static glm::vec3 color = glm::vec3(0.2f, 0.3f, 0.4f);

	//const float BasicW = 0.1f;
	//float width = BasicW * 4.0f;
	//if (bDisplayGrid)
	//{
	//	const int GridSize = 200;
	//	for (int i = -GridSize / 2; i < GridSize / 2; i++)
	//	{
	//		color = glm::vec3(0.4f, 0.65f, 0.73f);
	//		width = BasicW * 4.0f;
	//		if (i % 2 != 0 && i != 0)
	//		{
	//			color = color / 4.0f;
	//			width = width / 4.0f;
	//		}
	//		else if (i == 0)
	//		{
	//			color = glm::vec3(0.9f, 0.9f, 0.9f);
	//			width = BasicW * 4.0f;
	//		}

	//		RENDERER.DrawLine(glm::vec3(i, 0.0f, -GridSize / 2), glm::vec3(i, 0.0f, GridSize / 2), color, width);
	//		RENDERER.DrawLine(glm::vec3(-GridSize / 2, 0.0f, i), glm::vec3(GridSize / 2, 0.0f, i), color, width);
	//	}
	//}

	static float FavgTime = 0.0f;
	static std::vector<float> AvgTime;
	static int counter = 0;

	ImGui::Text((std::string("Time : ") + std::to_string(RENDERER.LastTestTime)).c_str());

	if (AvgTime.size() < 100)
	{
		AvgTime.push_back(RENDERER.LastTestTime);
	}
	else if (AvgTime.size() >= 100)
	{
		AvgTime[counter++ % 100] = RENDERER.LastTestTime;
	}

	for (size_t i = 0; i < AvgTime.size(); i++)
	{
		FavgTime += AvgTime[i];
	}
	FavgTime /= AvgTime.size();


	if (counter > 1000000)
		counter = 0;

	ImGui::Text((std::string("avg Time : ") + std::to_string(FavgTime)).c_str());

	bool bFreezeCulling = RENDERER.bFreezeCulling;
	ImGui::Checkbox("bFreezeCulling", &bFreezeCulling);
	RENDERER.bFreezeCulling = bFreezeCulling;

	bool bFreezeOcclusionCulling = !RENDERER.IsOcclusionCullingEnabled();
	ImGui::Checkbox("freezeOcclusionCulling", &bFreezeOcclusionCulling);
	RENDERER.SetOcclusionCullingEnabled(!bFreezeOcclusionCulling);

	static bool bDisplaySelectedObjAABB = false;
	ImGui::Checkbox("Display AABB of selected object", &bDisplaySelectedObjAABB);

	// Draw AABB
	FEEntity* SelectedEntity = SELECTED.GetSelected(CurrentScene);
	if (SelectedEntity != nullptr &&
		(SelectedEntity->HasComponent<FEGameModelComponent>() || SelectedEntity->HasComponent<FETerrainComponent>()) &&
		bDisplaySelectedObjAABB)
	{
		FEAABB SelectedAABB;
		SelectedAABB = SelectedEntity->GetParentScene()->GetEntityAABB(SelectedEntity);
		RENDERER.DrawAABB(SelectedAABB);

		if (SelectedEntity->HasComponent<FEInstancedComponent>())
		{
			static bool bDisplaySubObjAABB = false;
			ImGui::Checkbox("Display AABB of instanced entity subobjects", &bDisplaySubObjAABB);

			if (bDisplaySubObjAABB)
			{
				FEInstancedComponent& InstancedComponent = SelectedEntity->GetComponent<FEInstancedComponent>();
				const int MaxIterations = InstancedComponent.IndividualInstancedAABB.size() * 8 >= FE_MAX_LINES ? FE_MAX_LINES : int(InstancedComponent.IndividualInstancedAABB.size());

				for (size_t j = 0; j < MaxIterations; j++)
				{
					RENDERER.DrawAABB(InstancedComponent.IndividualInstancedAABB[j]);
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
		RENDERER.DrawAABB(SceneAABB);
	}

	ImGui::PopStyleVar();
	ImGui::End();
}