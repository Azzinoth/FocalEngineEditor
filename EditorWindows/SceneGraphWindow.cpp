#include "SceneGraphWindow.h"

FEEditorSceneGraphWindow* FEEditorSceneGraphWindow::Instance = nullptr;
FEEntity* FEEditorSceneGraphWindow::EntityToModify = nullptr;

FEEditorSceneGraphWindow::FEEditorSceneGraphWindow()
{
	strcpy_s(FilterForEntities, "");
}

void FEEditorSceneGraphWindow::InitializeResources()
{
	EntityIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/entitySceneBrowserIcon.png", "entitySceneBrowserIcon");
	RESOURCE_MANAGER.MakeTextureStandard(EntityIcon);
	InstancedEntityIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/instancedEntitySceneBrowserIcon.png", "instancedEntitySceneBrowserIcon");
	RESOURCE_MANAGER.MakeTextureStandard(InstancedEntityIcon);

	DirectionalLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/directionalLightSceneBrowserIcon.png", "directionalLightSceneBrowserIcon");
	RESOURCE_MANAGER.MakeTextureStandard(DirectionalLightIcon);
	SpotLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/spotLightSceneBrowserIcon.png", "spotLightSceneBrowserIcon");
	RESOURCE_MANAGER.MakeTextureStandard(SpotLightIcon);
	PointLightIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/pointLightSceneBrowserIcon.png", "pointLightSceneBrowserIcon");
	RESOURCE_MANAGER.MakeTextureStandard(PointLightIcon);

	TerrainIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/terrainSceneBrowserIcon.png", "terrainSceneBrowserIcon");
	RESOURCE_MANAGER.MakeTextureStandard(TerrainIcon);

	CameraIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/cameraSceneBrowserIcon.png", "cameraSceneBrowserIcon");
	RESOURCE_MANAGER.MakeTextureStandard(CameraIcon);
}

void FEEditorSceneGraphWindow::Clear()
{
	strcpy_s(FilterForEntities, "");
}

void FEEditorSceneGraphWindow::SetCorrectItemColor(FEObject* SceneObject) const
{
	if (SceneObject->GetType() == FE_DIRECTIONAL_LIGHT ||
		SceneObject->GetType() == FE_SPOT_LIGHT ||
		SceneObject->GetType() == FE_POINT_LIGHT)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, LightItemColor);
	}
	else if (SceneObject->GetType() == FE_CAMERA)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, CameraItemColor);
	}
	else if (SceneObject->GetType() == FE_TERRAIN)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, TerrainItemColor);
	}
	else if (SceneObject->GetType() == FE_ENTITY)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, EntityItemColor);
	}
	else if (SceneObject->GetType() == FE_ENTITY_INSTANCED)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, InstancedEntityItemColor);
	}
}

void FEEditorSceneGraphWindow::PopCorrectItemColor(FEObject* SceneObject)
{
	if (SceneObject->GetType() == FE_DIRECTIONAL_LIGHT ||
		SceneObject->GetType() == FE_SPOT_LIGHT ||
		SceneObject->GetType() == FE_POINT_LIGHT ||
		SceneObject->GetType() == FE_CAMERA ||
		SceneObject->GetType() == FE_TERRAIN ||
		SceneObject->GetType() == FE_ENTITY ||
		SceneObject->GetType() == FE_ENTITY_INSTANCED)
	{
		ImGui::PopStyleColor();
	}
}

static void CreateNewInstancedEntityCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_PREFAB)
	{
		FEPrefab* SelectedPrefab = RESOURCE_MANAGER.GetPrefab(SelectionsResult[0]->GetObjectID());
		if (SelectedPrefab == nullptr)
			return;

		FEEntityInstanced* NewEntity = SCENE.AddEntityInstanced(SelectedPrefab);
		NewEntity->Transform.SetPosition(ENGINE.GetCamera()->GetPosition() + ENGINE.GetCamera()->GetForward() * 10.0f);
		// FIX ME!
		//SELECTED.SetSelected(NewEntity);

		PROJECT_MANAGER.GetCurrent()->SetModified(true);
	}
}

static void CreateNewEntityCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_PREFAB)
	{
		FEPrefab* SelectedPrefab = RESOURCE_MANAGER.GetPrefab(SelectionsResult[0]->GetObjectID());
		if (SelectedPrefab == nullptr)
			return;

		// FIX ME!
		FEEntity* NewEntity = SCENE.AddEntity(SelectedPrefab);
		FENewEntity* NewNewEntity = SCENE.GetNewStyleEntityByOldStyleID(NewEntity->GetObjectID());
		NewNewEntity->GetComponent<FETransformComponent>().SetPosition(ENGINE.GetCamera()->GetPosition() + ENGINE.GetCamera()->GetForward() * 10.0f);
		//NewEntity->Transform.SetPosition(ENGINE.GetCamera()->GetPosition() + ENGINE.GetCamera()->GetForward() * 10.0f);
		SELECTED.SetSelected(NewNewEntity);

		PROJECT_MANAGER.GetCurrent()->SetModified(true);
	}
}

void FEEditorSceneGraphWindow::ChangePrefabOfEntityCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (EntityToModify == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_PREFAB)
	{
		FEPrefab* SelectedPrefab = RESOURCE_MANAGER.GetPrefab(SelectionsResult[0]->GetObjectID());
		if (SelectedPrefab == nullptr)
			return;

		EntityToModify->Prefab = SelectedPrefab;
	}
}

void FEEditorSceneGraphWindow::DrawCorrectIcon(const FEObject* SceneObject) const
{
	ImGui::SetCursorPosX(20);

	if (SceneObject->GetType() == FE_ENTITY || SceneObject->GetType() == FE_ENTITY_INSTANCED)
	{
		const FEEntity* entity = SCENE.GetEntity(SceneObject->GetObjectID());

		if (EDITOR_INTERNAL_RESOURCES.IsInInternalEditorList(entity))
			return;

		if (entity->GetType() == FE_ENTITY_INSTANCED)
		{

			ImGui::Image((void*)(intptr_t)InstancedEntityIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
		else
		{
			ImGui::Image((void*)(intptr_t)EntityIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
	}

	if (SceneObject->GetType() == FE_DIRECTIONAL_LIGHT)
	{
		ImGui::Image((void*)(intptr_t)DirectionalLightIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	}

	if (SceneObject->GetType() == FE_SPOT_LIGHT)
	{
		ImGui::Image((void*)(intptr_t)SpotLightIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	}

	if (SceneObject->GetType() == FE_POINT_LIGHT)
	{
		ImGui::Image((void*)(intptr_t)PointLightIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	}

	if (SceneObject->GetType() == FE_TERRAIN)
	{
		ImGui::Image((void*)(intptr_t)TerrainIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	if (SceneObject->GetType() == FE_CAMERA)
	{
		ImGui::Image((void*)(intptr_t)CameraIcon->GetTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	ImGui::SameLine();
	return;
}

DragAndDropTarget* FEEditorSceneGraphWindow::GetSceneNodeDragAndDropTarget(FENaiveSceneGraphNode* NodeToFind)
{
	int64_t UniqueID = 0;
	//// If it is Root.
	//if (NodeToFind->GetOldStyleEntity() == nullptr)
	//{
	//	UniqueID = 1;
	//}
	//else
	//{
		UniqueID = static_cast<intptr_t>(std::hash<std::string>{}(NodeToFind->GetNewStyleEntity()->GetObjectID().c_str()));
	//}

	if (SceneNodeDragAndDropTargets.find(UniqueID) == SceneNodeDragAndDropTargets.end())
	{
		SceneNodeDragAndDropTargets[UniqueID] = DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_SCENE_ENTITY },
			SceneNodeDragAndDropCallback, reinterpret_cast<void**>(NodeToFind),
			std::vector<std::string> { "Drop move to that parent" });
	}

	return SceneNodeDragAndDropTargets[UniqueID];
}

void FEEditorSceneGraphWindow::RenderSubTree(FENaiveSceneGraphNode* SubTreeRoot)
{
	SceneNodeDragAndDropTargetIndex++;
	int64_t UniqueID = 0;
	bool bIsLeaf = SubTreeRoot->GetChildren().size() == 0;
	ImGuiTreeNodeFlags NodeFlags = bIsLeaf ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen : ImGuiTreeNodeFlags_OpenOnArrow;
	std::string Name = SubTreeRoot->GetParent() == nullptr ? PROJECT_MANAGER.GetCurrent()->GetName() : SubTreeRoot->GetName();

	if (SELECTED.GetSelected() != nullptr && SubTreeRoot->GetNewStyleEntity() != nullptr)
	{
		if (SELECTED.GetSelected()->GetObjectID() == SubTreeRoot->GetNewStyleEntity()->GetObjectID())
		{
			NodeFlags |= ImGuiTreeNodeFlags_Selected;
		}
	}

	// If it is Root.
	/*if (SubTreeRoot->GetOldStyleEntity() == nullptr)
	{
		UniqueID = 1;
	}
	else
	{*/
		UniqueID = static_cast<intptr_t>(std::hash<std::string>{}(SubTreeRoot->GetNewStyleEntity()->GetObjectID().c_str()));
	//}

	bool bOpened = ImGui::TreeNodeEx((void*)UniqueID, NodeFlags, Name.c_str(), 0);
	GetSceneNodeDragAndDropTarget(SubTreeRoot)->StickToItem();

	if (ImGui::IsItemClicked())
	{
		if (SubTreeRoot->GetParent() != nullptr)
		{
			SELECTED.SetSelected(SubTreeRoot->GetNewStyleEntity());
			SELECTED.SetDirtyFlag(false);
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
				RenderSubTree(Children[i]);

			ImGui::TreePop();
		}
	}
}

void FEEditorSceneGraphWindow::RenderNewSceneGraph()
{
	FENaiveSceneGraphNode* Root = SCENE.SceneGraph.GetRoot();

	if (bSceneNodeTargetsDirty)
		SceneNodeDragAndDropTargets.clear();

	SceneNodeDragAndDropTargetIndex = -1;
	RenderSubTree(Root);

	if (bSceneNodeTargetsDirty)
		bSceneNodeTargetsDirty = false;
}

void FEEditorSceneGraphWindow::Render()
{
	if (!bVisible)
		return;

	static int SceneObjectHoveredIndex = -1;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Scene Entities", nullptr, ImGuiWindowFlags_None);

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

	RenderNewSceneGraph();

	/*if (ImGui::Button("Enter game mode", ImVec2(220, 0)))
	{
		FEEditor::getInstance().SetGameMode(true);
	}*/

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	const std::vector<std::string> EntityList = SCENE.GetEntityList();
	std::vector<std::string> SceneObjectsList;
	for (size_t i = 0; i < EntityList.size(); i++)
	{
		if (EDITOR_INTERNAL_RESOURCES.IsInInternalEditorList(SCENE.GetEntity(EntityList[i])))
			continue;
		SceneObjectsList.push_back(EntityList[i]);
	}

	const std::vector<std::string> LightList = SCENE.GetLightsList();
	for (size_t i = 0; i < LightList.size(); i++)
	{
		SceneObjectsList.push_back(LightList[i]);
	}

	const std::vector<std::string> TerrainList = SCENE.GetTerrainList();
	for (size_t i = 0; i < TerrainList.size(); i++)
	{
		SceneObjectsList.push_back(TerrainList[i]);
	}

	SceneObjectsList.push_back(ENGINE.GetCamera()->GetObjectID());

	// Filtering.
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);
	ImGui::Text("Filter: ");
	ImGui::SameLine();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	ImGui::InputText("##selectFEObjectPopUpFilter", FilterForEntities, IM_ARRAYSIZE(FilterForEntities));

	std::vector<std::string> FilteredSceneObjectsList;
	if (strlen(FilterForEntities) == 0)
	{
		FilteredSceneObjectsList = SceneObjectsList;
	}
	else
	{
		FilteredSceneObjectsList.clear();
		for (size_t i = 0; i < SceneObjectsList.size(); i++)
		{
			if (OBJECT_MANAGER.GetFEObject(SceneObjectsList[i])->GetName().find(FilterForEntities) != -1)
			{
				FilteredSceneObjectsList.push_back(SceneObjectsList[i]);
			}
		}
	}

	if (!bShouldOpenContextMenu)
		SceneObjectHoveredIndex = -1;

	for (size_t i = 0; i < FilteredSceneObjectsList.size(); i++)
	{
		DrawCorrectIcon(OBJECT_MANAGER.GetFEObject(FilteredSceneObjectsList[i]));

		ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (SELECTED.GetSelected() != nullptr)
		{
			if (SELECTED.GetSelected()->GetObjectID() == FilteredSceneObjectsList[i])
			{
				NodeFlags |= ImGuiTreeNodeFlags_Selected;
			}
		}

		SetCorrectItemColor(OBJECT_MANAGER.GetFEObject(FilteredSceneObjectsList[i]));
		ImGui::TreeNodeEx((void*)(intptr_t)i, NodeFlags, OBJECT_MANAGER.GetFEObject(FilteredSceneObjectsList[i])->GetName().c_str(), i);
		PopCorrectItemColor(OBJECT_MANAGER.GetFEObject(FilteredSceneObjectsList[i]));

		if (ImGui::IsItemClicked())
		{
			// FIX ME!
			FENewEntity* NewNewEntity = SCENE.GetNewStyleEntityByOldStyleID(OBJECT_MANAGER.GetFEObject(FilteredSceneObjectsList[i])->GetObjectID());
			SELECTED.SetSelected(NewNewEntity);
			SELECTED.SetDirtyFlag(false);
		}

		if (ImGui::IsItemHovered())
		{
			SceneObjectHoveredIndex = int(i);
		}
	}

	bool bOpenContextMenu = false;
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		bOpenContextMenu = true;

	if (bOpenContextMenu)
		ImGui::OpenPopup("##context_menu");

	bShouldOpenContextMenu = false;

	if (ImGui::BeginPopup("##context_menu"))
	{
		bShouldOpenContextMenu = true;

		if (SceneObjectHoveredIndex == -1)
		{
			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("Entity"))
				{
					SelectFEObjectPopUp::getInstance().Show(FE_PREFAB, CreateNewEntityCallBack);
					//selectGameModelPopUp::getInstance().show(nullptr, true);
				}

				if (ImGui::MenuItem("Instanced entity"))
				{
					SelectFEObjectPopUp::getInstance().Show(FE_PREFAB, CreateNewInstancedEntityCallBack);
					//selectGameModelPopUp::getInstance().show(nullptr, true, true);
				}

				if (ImGui::MenuItem("Terrain"))
				{
					const std::vector<std::string> TerrainList = SCENE.GetTerrainList();
					const size_t NextId = TerrainList.size();
					size_t index = 0;
					std::string NewName = "terrain_" + std::to_string(NextId + index);

					while (true)
					{
						bool bCorrectName = true;
						for (size_t i = 0; i < TerrainList.size(); i++)
						{
							if (TerrainList[i] == NewName)
							{
								bCorrectName = false;
								break;
							}
						}

						if (bCorrectName)
							break;

						index++;
						NewName = "terrain_" + std::to_string(NextId + index);
					}

					FETerrain* NewTerrain = RESOURCE_MANAGER.CreateTerrain(true, NewName);
					SCENE.AddTerrain(NewTerrain);
					NewTerrain->HeightMap->SetDirtyFlag(true);
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
				}

				if (ImGui::BeginMenu("Light"))
				{
					if (ImGui::MenuItem("Directional"))
					{
						SCENE.AddLight(FE_DIRECTIONAL_LIGHT, "");
					}

					if (ImGui::MenuItem("Spot"))
					{
						SCENE.AddLight(FE_SPOT_LIGHT, "");
					}

					if (ImGui::MenuItem("Point"))
					{
						SCENE.AddLight(FE_POINT_LIGHT, "");
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}
		}
		else
		{
			if (ImGui::MenuItem("Rename"))
			{
				if (SCENE.GetEntity(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					RenamePopUp::getInstance().Show(SCENE.GetEntity(FilteredSceneObjectsList[SceneObjectHoveredIndex]));
				}
				else if (SCENE.GetTerrain(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					RenamePopUp::getInstance().Show(SCENE.GetTerrain(FilteredSceneObjectsList[SceneObjectHoveredIndex]));
				}
				else if (SCENE.GetLight(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					RenamePopUp::getInstance().Show(SCENE.GetLight(FilteredSceneObjectsList[SceneObjectHoveredIndex]));
					//renameLightWindow.show(SCENE.getLight(filteredSceneObjectsList[sceneObjectHoveredIndex]));
				}
			}

			if (ImGui::MenuItem("Delete"))
			{
				if (SCENE.GetEntity(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					const FEEntity* Entity = SCENE.GetEntity(FilteredSceneObjectsList[SceneObjectHoveredIndex]);
					if (SELECTED.GetEntity() == Entity)
						SELECTED.Clear();

					SCENE.DeleteEntity(Entity->GetObjectID());
				}
				else if (SCENE.GetTerrain(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					const FETerrain* Terrain = SCENE.GetTerrain(FilteredSceneObjectsList[SceneObjectHoveredIndex]);
					if (SELECTED.GetTerrain() == Terrain)
						SELECTED.Clear();

					SCENE.DeleteTerrain(Terrain->GetObjectID());
				}
				else if (SCENE.GetLight(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					const FELight* Light = SCENE.GetLight(FilteredSceneObjectsList[SceneObjectHoveredIndex]);
					if (SELECTED.GetLight() == Light)
						SELECTED.Clear();

					SCENE.DeleteLight(Light->GetObjectID());
				}
			}

			/*if (ImGui::MenuItem("*DEBUG* Test model camera on this"))
			{
				FEModelViewCamera* NewCamera = new FEModelViewCamera("New ModelViewCamera");
				NewCamera->SetAspectRatio(static_cast<float>(ENGINE.GetRenderTargetWidth()) / static_cast<float>(ENGINE.GetRenderTargetHeight()));

				glm::vec3 Position = glm::vec3(0.0f);
				if (SCENE.GetEntity(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					const FEEntity* Entity = SCENE.GetEntity(FilteredSceneObjectsList[SceneObjectHoveredIndex]);
					Position = Entity->Transform.GetPosition();
				}
				else if (SCENE.GetTerrain(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					const FETerrain* Terrain = SCENE.GetTerrain(FilteredSceneObjectsList[SceneObjectHoveredIndex]);
					Position = Terrain->Transform.GetPosition();
				}
				else if (SCENE.GetLight(FilteredSceneObjectsList[SceneObjectHoveredIndex]) != nullptr)
				{
					const FELight* Light = SCENE.GetLight(FilteredSceneObjectsList[SceneObjectHoveredIndex]);
					Position = Light->Transform.GetPosition();
				}

				NewCamera->SetTrackingObjectPosition(Position);
				NewCamera->SetOnUpdate(OnCameraUpdate);
				ENGINE.SetCamera(NewCamera);
			}*/
		}

		ImGui::EndPopup();
	}

	// It should not be here.
	static bool bDisplayGrid = true;
	ImGui::Checkbox("Display grid", &bDisplayGrid);

	static glm::vec3 color = glm::vec3(0.2f, 0.3f, 0.4f);

	const float BasicW = 0.1f;
	float width = BasicW * 4.0f;
	if (bDisplayGrid)
	{
		const int GridSize = 200;
		for (int i = -GridSize / 2; i < GridSize / 2; i++)
		{
			color = glm::vec3(0.4f, 0.65f, 0.73f);
			width = BasicW * 4.0f;
			if (i % 2 != 0 && i != 0)
			{
				color = color / 4.0f;
				width = width / 4.0f;
			}
			else if (i == 0)
			{
				color = glm::vec3(0.9f, 0.9f, 0.9f);
				width = BasicW * 4.0f;
			}

			RENDERER.DrawLine(glm::vec3(i, 0.0f, -GridSize / 2), glm::vec3(i, 0.0f, GridSize / 2), color, width);
			RENDERER.DrawLine(glm::vec3(-GridSize / 2, 0.0f, i), glm::vec3(GridSize / 2, 0.0f, i), color, width);
		}
	}

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
	if (SELECTED.GetSelected() != nullptr &&
		(SELECTED.GetSelected()->GetType() == FE_ENTITY || SELECTED.GetSelected()->GetType() == FE_ENTITY_INSTANCED || SELECTED.GetSelected()->GetType() == FE_TERRAIN) &&
		bDisplaySelectedObjAABB)
	{
		if (SELECTED.GetSelected()->HasComponent<FERenderableComponent>())
		{
			FEEntity* OldStyleEntity = SELECTED.GetSelected()->GetComponent<FERenderableComponent>().OldStyleEntity;
			FEAABB SelectedAABB = OldStyleEntity->GetAABB().Transform(SELECTED.GetSelected()->GetComponent<FETransformComponent>().GetTransformMatrix());
			//const FEAABB SelectedAABB = SELECTED.GetEntity() != nullptr ? SELECTED.GetEntity()->GetAABB() : SELECTED.GetTerrain()->GetAABB();

			RENDERER.DrawAABB(SelectedAABB);

			if (SELECTED.GetSelected()->GetType() == FE_ENTITY_INSTANCED)
			{
				static bool bDisplaySubObjAABB = false;
				ImGui::Checkbox("Display AABB of instanced entity subobjects", &bDisplaySubObjAABB);

				if (bDisplaySubObjAABB)
				{
					const FEEntityInstanced* EntityInstanced = reinterpret_cast<FEEntityInstanced*> (SELECTED.GetSelected());
					const int MaxIterations = EntityInstanced->InstancedAABB.size() * 8 >= FE_MAX_LINES ? FE_MAX_LINES : int(EntityInstanced->InstancedAABB.size());

					for (size_t j = 0; j < MaxIterations; j++)
					{
						RENDERER.DrawAABB(EntityInstanced->InstancedAABB[j]);
					}
				}
			}
		}
	}

	ImGui::PopStyleVar();
	ImGui::End();
}