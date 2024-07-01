#include "InspectorWindow.h"

FEEditorInspectorWindow* FEEditorInspectorWindow::Instance = nullptr;
FEEntity* FEEditorInspectorWindow::EntityToModify = nullptr;

FEEditorInspectorWindow::FEEditorInspectorWindow()
{
	
}

void FEEditorInspectorWindow::InitializeResources()
{
	// ************** Terrain Settings **************
	ExportHeightMapButton = new ImGuiButton("Export HeightMap");
	ExportHeightMapButton->SetSize(ImVec2(200, 0));

	ImportHeightMapButton = new ImGuiButton("Import HeightMap");
	ImportHeightMapButton->SetSize(ImVec2(200, 0));

	SculptBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/sculptBrush.png", "sculptBrushIcon");
	RESOURCE_MANAGER.MakeTextureStandard(SculptBrushIcon);
	SculptBrushButton = new ImGuiImageButton(SculptBrushIcon);
	SculptBrushButton->SetSize(ImVec2(24, 24));

	LevelBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/levelBrush.png", "levelBrushIcon");
	RESOURCE_MANAGER.MakeTextureStandard(LevelBrushIcon);
	LevelBrushButton = new ImGuiImageButton(LevelBrushIcon);
	LevelBrushButton->SetSize(ImVec2(24, 24));

	SmoothBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/smoothBrush.png", "smoothBrushIcon");
	RESOURCE_MANAGER.MakeTextureStandard(SmoothBrushIcon);
	SmoothBrushButton = new ImGuiImageButton(SmoothBrushIcon);
	SmoothBrushButton->SetSize(ImVec2(24, 24));

	DrawBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/paintbrush.png", "drawBrushIcon");
	RESOURCE_MANAGER.MakeTextureStandard(DrawBrushIcon);
	LayerBrushButton = new ImGuiImageButton(DrawBrushIcon);
	LayerBrushButton->SetSize(ImVec2(48, 48));

	EntityChangePrefabTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_PREFAB, EntityChangePrefabTargetCallBack, nullptr, "Drop to assign prefab");
	// ************** Terrain Settings END **************

	MouseCursorIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/mouseCursorIcon.png", "mouseCursorIcon");
	RESOURCE_MANAGER.MakeTextureStandard(MouseCursorIcon);
	ArrowToGroundIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/arrowToGroundIcon.png", "arrowToGroundIcon");
	RESOURCE_MANAGER.MakeTextureStandard(ArrowToGroundIcon);
}

void FEEditorInspectorWindow::Clear()
{
	
}

void FEEditorInspectorWindow::ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const
{
	// ********************* POSITION *********************
	glm::vec3 TemporaryPosition = Transform->GetPosition();
	bool bModified = false;

	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##X pos : ") + Object->GetName()).c_str(), &TemporaryPosition[0], 0.1f))
		bModified = true;
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##Y pos : ") + Object->GetName()).c_str(), &TemporaryPosition[1], 0.1f))
		bModified = true;
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##Z pos : ") + Object->GetName()).c_str(), &TemporaryPosition[2], 0.1f))
		bModified = true;
	ShowToolTip("Z position");

	if (bModified)
		Transform->SetPosition(TemporaryPosition);

	// ********************* ROTATION *********************
	glm::vec3 TemporaryRotation = Transform->GetRotation();
	bModified = false;

	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##X rot : ") + Object->GetName()).c_str(), &TemporaryRotation[0], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##Y rot : ") + Object->GetName()).c_str(), &TemporaryRotation[1], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##Z rot : ") + Object->GetName()).c_str(), &TemporaryRotation[2], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Z rotation");

	if (bModified)
		Transform->SetRotation(TemporaryRotation);

	// ********************* SCALE *********************
	bool bUniformScaling = Transform->IsUniformScalingSet();
	bModified = false;

	if (ImGui::Checkbox("Uniform scaling", &bUniformScaling))
		Transform->SetUniformScaling(bUniformScaling);

	glm::vec3 TemporaryScale = Transform->GetScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##X scale : ") + Object->GetName()).c_str(), &TemporaryScale[0], 0.01f, 0.01f, 1000.0f))
		bModified = true;
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##Y scale : ") + Object->GetName()).c_str(), &TemporaryScale[1], 0.01f, 0.01f, 1000.0f))
		bModified = true;
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	if (ImGui::DragFloat((std::string("##Z scale : ") + Object->GetName()).c_str(), &TemporaryScale[2], 0.01f, 0.01f, 1000.0f))
		bModified = true;
	ShowToolTip("Z scale");

	if (bModified)
	{
		glm::vec3 OldScale = Transform->GetScale();
		Transform->ChangeXScaleBy(TemporaryScale[0] - OldScale[0]);
		Transform->ChangeYScaleBy(TemporaryScale[1] - OldScale[1]);
		Transform->ChangeZScaleBy(TemporaryScale[2] - OldScale[2]);
	}

	// ********************* REAL WORLD COMPARISON SCALE *********************
	if (Object->GetType() == FE_ENTITY || Object->GetType() == FE_ENTITY_INSTANCED)
	{
		FEEntity* Entity = SCENE.GetEntity(Object->GetObjectID());
		FENewEntity* NewStyleEntity = SCENE.GetNewStyleEntityByOldStyleID(Object->GetObjectID());

		//// Temporary solution, becuase of the lack of proper ECS system
		//if (Object->GetType() == FE_ENTITY)
		//{
		//	Entity = reinterpret_cast<FEEntity*>(Object);
		//}

		FEAABB RealAABB = Entity->GetAABB().Transform(NewStyleEntity->GetComponent<FETransformComponent>().GetTransformMatrix());
		const glm::vec3 Min = RealAABB.GetMin();
		const glm::vec3 Max = RealAABB.GetMax();

		const float XSize = sqrt((Max.x - Min.x) * (Max.x - Min.x));
		const float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
		const float ZSize = sqrt((Max.z - Min.z) * (Max.z - Min.z));

		std::string SizeInM = "Approximate object size: ";
		SizeInM += std::to_string(std::max(XSize, std::max(YSize, ZSize)));
		SizeInM += " m";

		/*std::string dementionsInM = "Xlength: ";
		dementionsInM += std::to_string(xSize);
		dementionsInM += " m Ylength: ";
		dementionsInM += std::to_string(ySize);
		dementionsInM += " m Zlength: ";
		dementionsInM += std::to_string(zSize);
		dementionsInM += " m";*/

		ImGui::Text(SizeInM.c_str());
	}
}

bool FEEditorInspectorWindow::EntityChangePrefabTargetCallBack(FEObject* Object, void** EntityPointer)
{
	FEEntity* Entity = SELECTED.GetEntity();
	if (Entity == nullptr)
		return false;

	Entity->Prefab = (RESOURCE_MANAGER.GetPrefab(Object->GetObjectID()));
	return true;
}

bool FEEditorInspectorWindow::TerrainChangeMaterialTargetCallBack(FEObject* Object, void** LayerIndex)
{
	FETerrain* Terrain = SELECTED.GetTerrain();
	if (Terrain == nullptr)
		return false;

	FEMaterial* MaterialToAssign = RESOURCE_MANAGER.GetMaterial(Object->GetObjectID());
	if (!MaterialToAssign->IsCompackPacking())
		return false;

	const int TempLayerIndex = *(int*)LayerIndex;
	if (TempLayerIndex >= 0 && TempLayerIndex < FE_TERRAIN_MAX_LAYERS)
		Terrain->GetLayerInSlot(TempLayerIndex)->SetMaterial(MaterialToAssign);

	return true;
}

void FEEditorInspectorWindow::ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform) const
{
	// ********************* POSITION *********************
	glm::vec3 position = Transform->GetPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + Name).c_str(), &position[0], 0.1f);
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + Name).c_str(), &position[1], 0.1f);
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + Name).c_str(), &position[2], 0.1f);
	ShowToolTip("Z position");
	Transform->SetPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = Transform->GetRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + Name).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + Name).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + Name).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	ShowToolTip("Z rotation");
	Transform->SetRotation(rotation);

	// ********************* SCALE *********************
	bool bUniformScaling = Transform->IsUniformScalingSet();
	ImGui::Checkbox("Uniform scaling", &bUniformScaling);
	Transform->SetUniformScaling(bUniformScaling);

	glm::vec3 scale = Transform->GetScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + Name).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + Name).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + Name).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("Z scale");

	glm::vec3 OldScale = Transform->GetScale();
	Transform->ChangeXScaleBy(scale[0] - OldScale[0]);
	Transform->ChangeYScaleBy(scale[1] - OldScale[1]);
	Transform->ChangeZScaleBy(scale[2] - OldScale[2]);
}

void FEEditorInspectorWindow::DisplayLightProperties(FELight* Light) const
{
	ShowTransformConfiguration(Light, &Light->Transform);

	if (Light->GetType() == FE_DIRECTIONAL_LIGHT)
	{
		FEDirectionalLight* DirectionalLight = reinterpret_cast<FEDirectionalLight*>(Light);
		ImGui::Separator();
		ImGui::Text("-------------Shadow settings--------------");

		ImGui::Text("Cast shadows:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		bool bCastShadows = DirectionalLight->IsCastShadows();
		ImGui::Checkbox("##Cast shadows", &bCastShadows);
		DirectionalLight->SetCastShadows(bCastShadows);
		ShowToolTip("Will this light cast shadows.");

		if (!DirectionalLight->IsCastShadows())
			ImGui::BeginDisabled();

		ImGui::Text("Number of cascades :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		int cascades = DirectionalLight->GetActiveCascades();
		ImGui::SliderInt("##cascades", &cascades, 1, 4);
		DirectionalLight->SetActiveCascades(cascades);
		ShowToolTip("How much steps of shadow quality will be used.");

		ImGui::Text("Shadow coverage in M :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float FirstCascadeSize = DirectionalLight->GetShadowCoverage();
		ImGui::DragFloat("##shadowCoverage", &FirstCascadeSize, 0.1f, 0.1f, 500.0f);
		DirectionalLight->SetShadowCoverage(FirstCascadeSize);
		ShowToolTip("Distance from camera at which shadows would be present.");

		ImGui::Text("Z depth of shadow map :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float CSMZDepth = DirectionalLight->GetCSMZDepth();
		ImGui::DragFloat("##CSMZDepth", &CSMZDepth, 0.01f, 0.1f, 100.0f);
		DirectionalLight->SetCSMZDepth(CSMZDepth);
		ShowToolTip("If you have problems with shadow disapearing when camera is at close distance to shadow reciver, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		ImGui::Text("XY depth of shadow map :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float CSMXYDepth = DirectionalLight->GetCSMXYDepth();
		ImGui::DragFloat("##CSMXYDepth", &CSMXYDepth, 0.01f, 0.0f, 100.0f);
		DirectionalLight->SetCSMXYDepth(CSMXYDepth);
		ShowToolTip("If you have problems with shadow on edges of screen, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		ImGui::Text("Shadows blur factor:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		float ShadowsBlurFactor = DirectionalLight->GetShadowBlurFactor();
		ImGui::DragFloat("##Shadows blur factor", &ShadowsBlurFactor, 0.001f, 0.0f, 10.0f);
		DirectionalLight->SetShadowBlurFactor(ShadowsBlurFactor);

		bool bStaticShadowBias = DirectionalLight->IsStaticShadowBias();
		ImGui::Checkbox("Static shadow bias :", &bStaticShadowBias);
		DirectionalLight->SetIsStaticShadowBias(bStaticShadowBias);

		if (DirectionalLight->IsStaticShadowBias())
		{
			ImGui::Text("Static shadow bias value :");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			float ShadowBias = DirectionalLight->GetShadowBias();
			ImGui::DragFloat("##shadowBias", &ShadowBias, 0.0001f, 0.00001f, 0.1f);
			DirectionalLight->SetShadowBias(ShadowBias);
		}
		else
		{
			ImGui::Text("Intensity of variable shadow bias :");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			float ShadowBiasIntensity = DirectionalLight->GetShadowBiasVariableIntensity();
			ImGui::DragFloat("##shadowBiasIntensity", &ShadowBiasIntensity, 0.01f, 0.01f, 10.0f);
			DirectionalLight->SetShadowBiasVariableIntensity(ShadowBiasIntensity);
		}

		if (!DirectionalLight->IsCastShadows())
			ImGui::EndDisabled();
	}
	else if (Light->GetType() == FE_POINT_LIGHT)
	{
	}
	else if (Light->GetType() == FE_SPOT_LIGHT)
	{
		FESpotLight* SpotLight = reinterpret_cast<FESpotLight*>(Light);
		glm::vec3 direction = SpotLight->GetDirection();
		ImGui::DragFloat("##x", &direction[0], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##y", &direction[1], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##z", &direction[2], 0.01f, 0.0f, 1.0f);

		float SpotAngle = SpotLight->GetSpotAngle();
		ImGui::SliderFloat((std::string("Inner angle##") + SpotLight->GetName()).c_str(), &SpotAngle, 0.0f, 90.0f);
		SpotLight->SetSpotAngle(SpotAngle);

		float SpotAngleOuter = SpotLight->GetSpotAngleOuter();
		ImGui::SliderFloat((std::string("Outer angle ##") + SpotLight->GetName()).c_str(), &SpotAngleOuter, 0.0f, 90.0f);
		SpotLight->SetSpotAngleOuter(SpotAngleOuter);
	}

	glm::vec3 color = Light->GetColor();
	ImGui::ColorEdit3((std::string("Color##") + Light->GetName()).c_str(), &color.x);
	Light->SetColor(color);

	float intensity = Light->GetIntensity();
	ImGui::SliderFloat((std::string("Intensity##") + Light->GetName()).c_str(), &intensity, 0.0f, 100.0f);
	Light->SetIntensity(intensity);
}

void FEEditorInspectorWindow::ChangePrefabOfEntityCallBack(const std::vector<FEObject*> SelectionsResult)
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

void FEEditorInspectorWindow::DisplayLightsProperties() const
{
	const std::vector<std::string> LightList = SCENE.GetLightsList();

	for (size_t i = 0; i < LightList.size(); i++)
	{
		if (ImGui::TreeNode(LightList[i].c_str()))
		{
			DisplayLightProperties(SCENE.GetLight(LightList[i]));
			ImGui::TreePop();
		}
	}
}

void FEEditorInspectorWindow::Render()
{
	if (!bVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);

	// Test new ECS.
	if (SELECTED.GetSelected() == nullptr)
	{
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}
	else
	{
		FENewEntity* NewStyleEntity = SELECTED.GetSelected();
		if (NewStyleEntity->HasComponent<FETagComponent>())
		{
			FETagComponent& Tag = NewStyleEntity->GetComponent<FETagComponent>();
			char Buffer[256];
			memset(Buffer, 0, 256);
			strcpy_s(Buffer, Tag.Tag.c_str());
			if (ImGui::InputText("Tag", Buffer, 256))
			{
				Tag.Tag = std::string(Buffer);
			}
		}

		if (NewStyleEntity->HasComponent<FETransformComponent>())
		{
			FETransformComponent& Transform = NewStyleEntity->GetComponent<FETransformComponent>();
			ShowTransformConfiguration(NewStyleEntity->GetName(), &Transform);
		}

		/*if (NewStyleEntity->HasComponent<FERenderableComponent>())
		{
			FERenderableComponent& Renderable = NewStyleEntity->GetComponent<FERenderableComponent>();
			if (Renderable.OldStyleEntity != nullptr)
			{
				FEEntity* OldStyleEntity = Renderable.OldStyleEntity;
				ShowTransformConfiguration(OldStyleEntity, &OldStyleEntity->Transform);
			}
		}*/


		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}
	

	if (SELECTED.GetSelected() == nullptr)
	{
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}

	if (SELECTED.GetEntity() != nullptr)
	{
		FEEntity* Entity = SELECTED.GetEntity();

		if (Entity->GetType() == FE_ENTITY)
		{
			ShowTransformConfiguration(Entity, &Entity->Transform);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::InputInt("Force old matrix order", &Entity->Transform.OldWayForce);
			ImGui::Checkbox("Use old matrix order globaly", &Entity->Transform.OldWayGlobal);

			bool bActive = Entity->IsWireframeMode();
			ImGui::Checkbox("WireframeMode", &bActive);
			Entity->SetWireframeMode(bActive);

			ImGui::Separator();
			ImGui::Text("Prefab : ");
			// Temporary solution, becuase of the lack of proper ECS system
			if (Entity->Prefab != nullptr)
			{
				FETexture* PreviewTexture = PREVIEW_MANAGER.GetPrefabPreview(Entity->Prefab->GetObjectID());

				if (ImGui::ImageButton((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					EntityToModify = Entity;
					SelectFEObjectPopUp::getInstance().Show(FE_PREFAB, ChangePrefabOfEntityCallBack, Entity->Prefab);

				}
				EntityChangePrefabTarget->StickToItem();
			}
			else
			{
				ImGui::Text("No prefab assigned.");
			}

			bool bOpenContextMenu = false;
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
				bOpenContextMenu = true;

			if (bOpenContextMenu)
				ImGui::OpenPopup("##Inspector_context_menu");

			CONTENT_BROWSER_WINDOW.bShouldOpenContextMenu = false;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			if (ImGui::BeginPopup("##Inspector_context_menu"))
			{
				CONTENT_BROWSER_WINDOW.bShouldOpenContextMenu = true;

				if (ImGui::MenuItem("Show in folder"))
				{
					CONTENT_BROWSER_WINDOW.OpenItemParentFolder(Entity->Prefab);
				}

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			ImGui::Separator();

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
		else if (Entity->GetType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* InstancedEntity = reinterpret_cast<FEEntityInstanced*>(Entity);

			if (SELECTED.InstancedSubObjectIndexSelected != -1)
			{
				std::string InstancedSubObjectInfo = "index: ";

				ImGui::Text("Selected instance info:");
				InstancedSubObjectInfo = "index: " + std::to_string(SELECTED.InstancedSubObjectIndexSelected);
				ImGui::Text(InstancedSubObjectInfo.c_str());

				FETransformComponent TempTransform = FETransformComponent(InstancedEntity->GetTransformedInstancedMatrix(SELECTED.InstancedSubObjectIndexSelected));
				ShowTransformConfiguration("selected instance", &TempTransform);
				InstancedEntity->ModifyInstance(SELECTED.InstancedSubObjectIndexSelected, TempTransform.GetTransformMatrix());

				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.55f, 0.55f, 0.95f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));

				if (ImGui::ImageButton((void*)(intptr_t)ArrowToGroundIcon->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					InstancedEntity->TryToSnapInstance(SELECTED.InstancedSubObjectIndexSelected);
				}
				ShowToolTip("Selected instance will attempt to snap to the terrain.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
			else
			{
				ShowTransformConfiguration(Entity, &Entity->Transform);

				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

				ImGui::Separator();

				ImGui::Text("Prefab : ");
				FETexture* PreviewTexture = PREVIEW_MANAGER.GetPrefabPreview(Entity->Prefab->GetObjectID());
				if (ImGui::ImageButton((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					EntityToModify = Entity;
					SelectFEObjectPopUp::getInstance().Show(FE_PREFAB, ChangePrefabOfEntityCallBack, Entity->Prefab);
				}
				EntityChangePrefabTarget->StickToItem();

				bool bOpenContextMenu = false;
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
					bOpenContextMenu = true;

				if (bOpenContextMenu)
					ImGui::OpenPopup("##Inspector_context_menu");

				CONTENT_BROWSER_WINDOW.bShouldOpenContextMenu = false;
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
				if (ImGui::BeginPopup("##Inspector_context_menu"))
				{
					CONTENT_BROWSER_WINDOW.bShouldOpenContextMenu = true;

					if (ImGui::MenuItem("Show in folder"))
					{
						CONTENT_BROWSER_WINDOW.OpenItemParentFolder(Entity->Prefab);
					}

					ImGui::EndPopup();
				}
				ImGui::PopStyleVar();
				ImGui::Separator();

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				ImGui::Text("Snapped to: ");
				ImGui::SameLine();

				const std::vector<std::string> TerrainList = SCENE.GetTerrainList();
				static std::string CurrentTerrain = "none";

				if (InstancedEntity->GetSnappedToTerrain() == nullptr)
				{
					CurrentTerrain = "none";
				}
				else
				{
					CurrentTerrain = InstancedEntity->GetSnappedToTerrain()->GetName();
				}

				ImGui::SetNextItemWidth(220);
				if (ImGui::BeginCombo("##Terrain", CurrentTerrain.c_str(), ImGuiWindowFlags_None))
				{
					const bool is_selected = (CurrentTerrain == "none");
					if (ImGui::Selectable("none", is_selected))
					{
						if (InstancedEntity->GetSnappedToTerrain() != nullptr)
							InstancedEntity->GetSnappedToTerrain()->UnSnapInstancedEntity(InstancedEntity);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();

					for (size_t i = 0; i < TerrainList.size(); i++)
					{
						const bool is_selected = (CurrentTerrain == TerrainList[i]);
						if (ImGui::Selectable(SCENE.GetTerrain(TerrainList[i])->GetName().c_str(), is_selected))
						{
							SCENE.GetTerrain(TerrainList[i])->SnapInstancedEntity(InstancedEntity);
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (InstancedEntity->GetSnappedToTerrain() != nullptr)
				{
					ImGui::Text("Terrain layer: ");
					ImGui::SameLine();

					const int CurrentLayer = InstancedEntity->GetTerrainLayer();
					FETerrain* CurrentTerrain = InstancedEntity->GetSnappedToTerrain();

					std::string caption = "none";
					const auto layer = CurrentTerrain->GetLayerInSlot(CurrentLayer);
					if (layer != nullptr)
						caption = layer->GetName();

					ImGui::SetNextItemWidth(220);
					if (ImGui::BeginCombo("##TerrainLayers", caption.c_str(), ImGuiWindowFlags_None))
					{
						const bool is_selected = (CurrentLayer == -1);
						ImGui::PushID("none_TerrainLayers_entity");
						if (ImGui::Selectable("none", is_selected))
						{
							if (CurrentTerrain != nullptr)
								CurrentTerrain->UnConnectInstancedEntityFromLayer(InstancedEntity);
						}
						ImGui::PopID();

						if (is_selected)
							ImGui::SetItemDefaultFocus();

						for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
						{
							FETerrainLayer* layer = CurrentTerrain->GetLayerInSlot(i);
							if (layer == nullptr)
								break;

							const bool is_selected = (CurrentLayer == i);
							ImGui::PushID(layer->GetObjectID().c_str());
							if (ImGui::Selectable(layer->GetName().c_str(), is_selected))
							{
								CurrentTerrain->ConnectInstancedEntityToLayer(InstancedEntity, int(i));
							}
							ImGui::PopID();

							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (CurrentLayer != -1)
					{
						ImGui::Text("Minimal layer intensity:");
						float MinLevel = InstancedEntity->GetMinimalLayerIntensity();
						ImGui::SameLine();
						ImGui::SetNextItemWidth(80);
						ImGui::DragFloat("##minLevel", &MinLevel);
						InstancedEntity->SetMinimalLayerIntensity(MinLevel);
					}
				}

				ImGui::Separator();

				ImGui::Text("Seed:");
				int seed = InstancedEntity->SpawnInfo.Seed;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::DragInt("##Seed", &seed);
				InstancedEntity->SpawnInfo.Seed = seed;

				ImGui::Text("Object count:");
				int ObjectCount = InstancedEntity->SpawnInfo.ObjectCount;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::DragInt("##Object count", &ObjectCount);
				if (ObjectCount <= 0)
					ObjectCount = 1;
				InstancedEntity->SpawnInfo.ObjectCount = ObjectCount;

				ImGui::Text("Radius:");
				float radius = InstancedEntity->SpawnInfo.Radius;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::DragFloat("##Radius", &radius);
				if (radius < 0.0f)
					radius = 0.1f;
				InstancedEntity->SpawnInfo.Radius = radius;

				// Scale deviation.
				ImGui::Text("Scale: ");

				ImGui::SameLine();
				ImGui::Text("min ");

				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				float MinScale = InstancedEntity->SpawnInfo.GetMinScale();
				ImGui::DragFloat("##minScale", &MinScale, 0.01f);
				InstancedEntity->SpawnInfo.SetMinScale(MinScale);

				ImGui::SameLine();
				ImGui::Text("max ");

				ImGui::SameLine();
				float MaxScale = InstancedEntity->SpawnInfo.GetMaxScale();
				ImGui::SetNextItemWidth(100);
				ImGui::DragFloat("##maxScale", &MaxScale, 0.01f);
				InstancedEntity->SpawnInfo.SetMaxScale(MaxScale);

				ImGui::Text("Rotation deviation:");
				float RotationDeviationX = InstancedEntity->SpawnInfo.RotationDeviation.x;
				ImGui::Text("X:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation X", &RotationDeviationX, 0.01f);
				if (RotationDeviationX < 0.01f)
					RotationDeviationX = 0.01f;
				if (RotationDeviationX > 1.0f)
					RotationDeviationX = 1.0f;
				InstancedEntity->SpawnInfo.RotationDeviation.x = RotationDeviationX;

				float RotationDeviationY = InstancedEntity->SpawnInfo.RotationDeviation.y;
				ImGui::Text("Y:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation Y", &RotationDeviationY, 0.01f);
				if (RotationDeviationY < 0.01f)
					RotationDeviationY = 0.01f;
				if (RotationDeviationY > 1.0f)
					RotationDeviationY = 1.0f;
				InstancedEntity->SpawnInfo.RotationDeviation.y = RotationDeviationY;

				float RotationDeviationZ = InstancedEntity->SpawnInfo.RotationDeviation.z;
				ImGui::Text("Z:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation z", &RotationDeviationZ, 0.01f);
				if (RotationDeviationZ < 0.01f)
					RotationDeviationZ = 0.01f;
				if (RotationDeviationZ > 1.0f)
					RotationDeviationZ = 1.0f;
				InstancedEntity->SpawnInfo.RotationDeviation.z = RotationDeviationZ;

				if (ImGui::Button("Spawn/Re-Spawn"))
				{
					InstancedEntity->Clear();
					InstancedEntity->Populate(InstancedEntity->SpawnInfo);
				}

				if (ImGui::Button("Add instance"))
				{
					glm::mat4 NewInstanceMatrix = glm::identity<glm::mat4>();
					NewInstanceMatrix = glm::translate(NewInstanceMatrix, ENGINE.GetCamera()->GetPosition() + ENGINE.GetCamera()->GetForward() * 10.0f);
					InstancedEntity->AddInstance(NewInstanceMatrix);

					PROJECT_MANAGER.GetCurrent()->SetModified(true);
				}

				if (InstancedEntity->IsSelectMode())
				{
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.0f, 0.75f, 0.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.0f, 1.0f, 0.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.0f, 1.0f, 0.0f));
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.55f, 0.55f, 0.95f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
				}

				ImGui::Separator();
				if (ImGui::ImageButton((void*)(intptr_t)MouseCursorIcon->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					SCENE.SetSelectMode(InstancedEntity, !InstancedEntity->IsSelectMode());
					if (!InstancedEntity->IsSelectMode())
					{
						SELECTED.Clear();
						// FIX ME!
						//SELECTED.SetSelected(InstancedEntity);
					}
				}
				ShowToolTip("Individual selection mode - Used to select individual instances.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
		}
	}
	else if (SELECTED.GetTerrain() != nullptr)
	{
		FETerrain* CurrentTerrain = SELECTED.GetTerrain();
		DisplayTerrainSettings(CurrentTerrain);

		if (CurrentTerrain->GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
		{
			// FIX ME!
			// To hide gizmos.
			/*if (SELECTED.GetTerrain() != nullptr)
				SELECTED.SetSelected(SELECTED.GetTerrain());*/

			CurrentTerrain->SetBrushActive(bLeftMousePressed);

			if (bShiftPressed)
			{
				if (CurrentTerrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW)
					CurrentTerrain->SetBrushMode(FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED);
			}
			else
			{
				if (CurrentTerrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED)
					CurrentTerrain->SetBrushMode(FE_TERRAIN_BRUSH_SCULPT_DRAW);
			}

			/*if (EDITOR.bLeftMousePressed)
			{
				if (EDITOR.bShiftPressed)
				{
					currentTerrain->setBrushMode(FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED);
				}
				else
				{
					currentTerrain->setBrushMode(FE_TERRAIN_BRUSH_SCULPT_DRAW);
				}
			}
			else
			{
				currentTerrain->setBrushMode(FE_TERRAIN_BRUSH_NONE);
			}*/

			//currentTerrain->setBrushActive(EDITOR.bLeftMousePressed);
			//currentTerrain->setBrushInversed(EDITOR.bShiftPressed);
		}
		else
		{
			// FIX ME!
			// To show gizmos.
			/*if (SELECTED.GetTerrain() != nullptr)
				SELECTED.SetSelected(SELECTED.GetTerrain());*/
		}
	}
	else if (SELECTED.GetLight() != nullptr)
	{
		DisplayLightProperties(SELECTED.GetLight());
	}
	else if (SELECTED.GetSelected()->GetType() == FE_CAMERA)
	{
		FEBasicCamera* Camera = ENGINE.GetCamera();

		// ********* POSITION *********
		glm::vec3 CameraPosition = Camera->GetPosition();

		ImGui::Text("Position : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90);
		ImGui::DragFloat("##X pos", &CameraPosition[0], 0.1f);
		ShowToolTip("X position");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90);
		ImGui::DragFloat("##Y pos", &CameraPosition[1], 0.1f);
		ShowToolTip("Y position");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90);
		ImGui::DragFloat("##Z pos", &CameraPosition[2], 0.1f);
		ShowToolTip("Z position");

		Camera->SetPosition(CameraPosition);

		// ********* ROTATION *********
		glm::vec3 CameraRotation = glm::vec3(Camera->GetYaw(), Camera->GetPitch(), Camera->GetRoll());

		ImGui::Text("Rotation : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90);
		ImGui::DragFloat("##X rot", &CameraRotation[0], 0.1f);
		ShowToolTip("X rotation");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90);
		ImGui::DragFloat("##Y rot", &CameraRotation[1], 0.1f);
		ShowToolTip("Y rotation");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(90);
		ImGui::DragFloat("##Z rot", &CameraRotation[2], 0.1f);
		ShowToolTip("Z rotation");

		Camera->SetYaw(CameraRotation[0]);
		Camera->SetPitch(CameraRotation[1]);
		Camera->SetRoll(CameraRotation[2]);

		float CameraSpeed = Camera->GetMovementSpeed();
		ImGui::Text("Camera speed in m/s : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Camera_speed", &CameraSpeed, 0.01f, 0.01f, 100.0f);
		Camera->SetMovementSpeed(CameraSpeed);
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

static FETerrain* TerrainToWorkWith = nullptr;
static void CreateNewTerrainLayerWithMaterialCallBack(std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		if (TerrainToWorkWith == nullptr)
			return;

		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		RESOURCE_MANAGER.ActivateTerrainVacantLayerSlot(TerrainToWorkWith, SelectedMaterial);
	}

	TerrainToWorkWith = nullptr;
}

static size_t TempLayerIndex = -1;
static void ChangeMaterialInTerrainLayerCallBack(std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		if (TempLayerIndex == -1)
			return;

		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		TerrainToWorkWith->GetLayerInSlot(TempLayerIndex)->SetMaterial(SelectedMaterial);
	}

	TerrainToWorkWith = nullptr;
	TempLayerIndex = -1;
}

void FEEditorInspectorWindow::DisplayTerrainSettings(FETerrain* Terrain)
{
	if (TerrainChangeLayerMaterialTargets.size() != Terrain->LayersUsed())
	{
		for (size_t i = 0; i < TerrainChangeLayerMaterialTargets.size(); i++)
		{
			delete TerrainChangeLayerMaterialTargets[i];
		}

		TerrainChangeLayerMaterialTargets.resize(Terrain->LayersUsed());
		TerrainChangeMaterialIndecies.resize(Terrain->LayersUsed());
		for (size_t i = 0; i < size_t(Terrain->LayersUsed()); i++)
		{
			TerrainChangeMaterialIndecies[i] = int(i);
			TerrainChangeLayerMaterialTargets[i] = DRAG_AND_DROP_MANAGER.AddTarget(FE_MATERIAL, TerrainChangeMaterialTargetCallBack, (void**)&TerrainChangeMaterialIndecies[i], "Drop to assing material to " + Terrain->GetLayerInSlot(i)->GetName());
		}
	}

	if (ImGui::BeginTabBar("##terrainSettings", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("General"))
		{
			bool bActive = Terrain->IsWireframeMode();
			ImGui::Checkbox("WireframeMode", &bActive);
			Terrain->SetWireframeMode(bActive);

			int IData = *(int*)RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->GetParameterData("debugFlag");
			ImGui::SliderInt("debugFlag", &IData, 0, 10);
			RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("debugFlag", IData);

			float DisplacementScale = Terrain->GetDisplacementScale();
			ImGui::DragFloat("displacementScale", &DisplacementScale, 0.02f, -10.0f, 10.0f);
			Terrain->SetDisplacementScale(DisplacementScale);

			float LODLevel = Terrain->GetLODLevel();
			ImGui::DragFloat("LODlevel", &LODLevel, 2.0f, 2.0f, 128.0f);
			Terrain->SetLODLevel(LODLevel);
			ShowToolTip("Bigger LODlevel more details terraine will have and less performance you will get.");

			float ChunkPerSide = Terrain->GetChunkPerSide();
			ImGui::DragFloat("chunkPerSide", &ChunkPerSide, 2.0f, 1.0f, 16.0f);
			Terrain->SetChunkPerSide(ChunkPerSide);

			// ********************* REAL WORLD COMPARISON SCALE *********************
			FEAABB RealAABB = Terrain->GetAABB();
			glm::vec3 min = RealAABB.GetMin();
			glm::vec3 max = RealAABB.GetMax();

			float XSize = sqrt((max.x - min.x) * (max.x - min.x));
			float YSize = sqrt((max.y - min.y) * (max.y - min.y));
			float ZSize = sqrt((max.z - min.z) * (max.z - min.z));

			std::string SizeInM = "Approximate terrain size: ";
			SizeInM += std::to_string(std::max(XSize, std::max(YSize, ZSize)));
			SizeInM += " m";
			ImGui::Text(SizeInM.c_str());
			// ********************* REAL WORLD COMPARISON SCALE END *********************

			ShowTransformConfiguration(Terrain, &Terrain->Transform);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Sculpt"))
		{
			ExportHeightMapButton->Render();
			if (ExportHeightMapButton->IsClicked())
			{
				std::string filePath = "";
				FILE_SYSTEM.ShowFileSaveDialog(filePath, TEXTURE_LOAD_FILTER, 1);

				if (!filePath.empty())
				{
					filePath += ".png";
					RESOURCE_MANAGER.ExportFETextureToPNG(Terrain->HeightMap, filePath.c_str());
				}
			}

			ImGui::SameLine();
			ImportHeightMapButton->Render();
			if (ImportHeightMapButton->IsClicked())
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, TEXTURE_LOAD_FILTER, 1);

				if (!FilePath.empty())
				{
					FETexture* LoadedTexture = RESOURCE_MANAGER.LoadPNGHeightmap(FilePath.c_str(), Terrain);
					if (LoadedTexture == RESOURCE_MANAGER.NoTexture)
					{
						LOG.Add(std::string("can't load height map: ") + FilePath, "FE_LOG_LOADING", FE_LOG_ERROR);
					}
					else
					{
						LoadedTexture->SetDirtyFlag(true);
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
					}
				}
			}

			float HighScale = Terrain->GetHightScale();
			ImGui::DragFloat("hight range in m", &HighScale);
			Terrain->SetHightScale(HighScale);

			float CurrentBrushSize = Terrain->GetBrushSize();
			ImGui::DragFloat("brushSize", &CurrentBrushSize, 0.1f, 0.01f, 100.0f);
			Terrain->SetBrushSize(CurrentBrushSize);

			float CurrentBrushIntensity = Terrain->GetBrushIntensity();
			ImGui::DragFloat("brushIntensity", &CurrentBrushIntensity, 0.0001f, 0.0001f, 10.0f);
			Terrain->SetBrushIntensity(CurrentBrushIntensity);

			SetDefaultStyle(SculptBrushButton);
			if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW ||
				Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED)
				SetSelectedStyle(SculptBrushButton);

			SculptBrushButton->Render();
			ShowToolTip("Sculpt Brush. Left mouse to increase height, hold shift to decrease height.");

			if (SculptBrushButton->IsClicked())
			{
				if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW ||
					Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED)
				{
					Terrain->SetBrushMode(FE_TERRAIN_BRUSH_NONE);
				}
				else
				{
					Terrain->SetBrushMode(FE_TERRAIN_BRUSH_SCULPT_DRAW);
				}
			}

			SetDefaultStyle(LevelBrushButton);
			if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_LEVEL)
				SetSelectedStyle(LevelBrushButton);

			ImGui::SameLine();
			LevelBrushButton->Render();
			ShowToolTip("Level Brush.");

			if (LevelBrushButton->IsClicked())
			{
				if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_LEVEL)
				{
					Terrain->SetBrushMode(FE_TERRAIN_BRUSH_NONE);
				}
				else
				{
					Terrain->SetBrushMode(FE_TERRAIN_BRUSH_SCULPT_LEVEL);
				}
			}

			SetDefaultStyle(SmoothBrushButton);
			if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_SMOOTH)
				SetSelectedStyle(SmoothBrushButton);

			ImGui::SameLine();
			SmoothBrushButton->Render();
			ShowToolTip("Smooth Brush.");

			if (SmoothBrushButton->IsClicked())
			{
				if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_SMOOTH)
				{
					Terrain->SetBrushMode(FE_TERRAIN_BRUSH_NONE);
				}
				else
				{
					Terrain->SetBrushMode(FE_TERRAIN_BRUSH_SCULPT_SMOOTH);
				}
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Paint"))
		{
			glm::vec2 TileMult = Terrain->GetTileMult();
			ImGui::DragFloat2("all layers tile factors", &TileMult[0], 0.1f, 1.0f, 100.0f);
			Terrain->SetTileMult(TileMult);

			float CurrentBrushSize = Terrain->GetBrushSize();
			ImGui::DragFloat("brushSize", &CurrentBrushSize, 0.1f, 0.01f, 100.0f);
			Terrain->SetBrushSize(CurrentBrushSize);

			float CurrentBrushIntensity = Terrain->GetBrushIntensity();
			ImGui::DragFloat("brushIntensity", &CurrentBrushIntensity, 0.0001f, 0.0001f, 10.0f);
			Terrain->SetBrushIntensity(CurrentBrushIntensity);

			SetDefaultStyle(LayerBrushButton);
			if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_LAYER_DRAW)
				SetSelectedStyle(LayerBrushButton);

			LayerBrushButton->Render();
			ShowToolTip("Layer draw brush. Left mouse to paint currently selected layer, hold shift to decrease layer influence.");
			static int SelectedLayer = -1;
			if (SelectedLayer != -1 && Terrain->GetLayerInSlot(SelectedLayer) == nullptr)
				SelectedLayer = -1;

			if (LayerBrushButton->IsClicked())
			{
				if (SelectedLayer != -1)
				{
					if (Terrain->GetBrushMode() == FE_TERRAIN_BRUSH_LAYER_DRAW)
					{
						Terrain->SetBrushMode(FE_TERRAIN_BRUSH_NONE);
					}
					else
					{
						Terrain->SetBrushMode(FE_TERRAIN_BRUSH_LAYER_DRAW);
					}
				}

			}

			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			static bool bContextMenuOpened = false;

			ImGui::Text("Layers:");

			ImGui::BeginChildFrame(ImGui::GetID("Layers ListBox Child"), ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 500.0f), ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			bool bListBoxHovered = false;
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
				bListBoxHovered = true;

			static bool bShouldOpenContextMenu = false;
			if (ImGui::IsMouseClicked(1))
			{
				if (bListBoxHovered)
				{
					bShouldOpenContextMenu = true;
				}
			}

			ImGui::BeginListBox("##Layers ListBox", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 500.0f));

			for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
			{
				FETerrainLayer* layer = Terrain->GetLayerInSlot(i);
				if (layer == nullptr)
					break;

				ImVec2 PostionBeforeDraw = ImGui::GetCursorPos();

				ImVec2 TextSize = ImGui::CalcTextSize(layer->GetName().c_str());
				ImGui::SetCursorPos(PostionBeforeDraw + ImVec2(ImGui::GetContentRegionAvail().x / 2.0f - TextSize.x / 2.0f, 16));

				if (TerrainLayerRenameIndex == i)
				{
					if (!bLastFrameTerrainLayerRenameEditWasVisiable)
					{
						ImGui::SetKeyboardFocusHere(0);
						ImGui::SetFocusID(ImGui::GetID("##newNameTerrainLayerEditor"), ImGui::GetCurrentWindow());
						ImGui::SetItemDefaultFocus();
						bLastFrameTerrainLayerRenameEditWasVisiable = true;
					}

					ImGui::SetNextItemWidth(350.0f);
					ImGui::SetCursorPos(ImVec2(PostionBeforeDraw.x + 64.0f + (ImGui::GetContentRegionAvail().x - 64.0f) / 2.0f - 350.0f / 2.0f, PostionBeforeDraw.y + 12));
					if (ImGui::InputText("##newNameTerrainLayerEditor", TerrainLayerRename, IM_ARRAYSIZE(TerrainLayerRename), ImGuiInputTextFlags_EnterReturnsTrue) ||
						ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##newNameTerrainLayerEditor"))
					{
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						layer->SetName(TerrainLayerRename);

						TerrainLayerRenameIndex = -1;
					}
				}
				else
				{
					ImGui::Text(layer->GetName().c_str());
				}
				ImGui::SetCursorPos(PostionBeforeDraw);

				ImGui::PushID(int(i));
				if (ImGui::Selectable("##item", SelectedLayer == i ? true : false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x - 0, 64)))
				{
					SelectedLayer = int(i);
					Terrain->SetBrushLayerIndex(SelectedLayer);
				}
				TerrainChangeLayerMaterialTargets[i]->StickToItem();
				ImGui::PopID();

				if (ImGui::IsItemHovered())
					HoveredTerrainLayerItem = int(i);

				ImGui::SetCursorPos(PostionBeforeDraw);
				ImColor ImageTint = ImGui::IsItemHovered() ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
				FETexture* PreviewTexture = PREVIEW_MANAGER.GetMaterialPreview(layer->GetMaterial()->GetObjectID());
				ImGui::Image((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImageTint);
			}

			ImGui::EndListBox();
			ImGui::PopFont();

			ImGui::EndChildFrame();
			ImGui::EndTabItem();

			if (bShouldOpenContextMenu)
			{
				bShouldOpenContextMenu = false;
				ImGui::OpenPopup("##layers_listBox_context_menu");
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			if (ImGui::BeginPopup("##layers_listBox_context_menu"))
			{
				bContextMenuOpened = true;

				if (Terrain->GetLayerInSlot(FE_TERRAIN_MAX_LAYERS - 1) != nullptr)
					ImGui::BeginDisabled();

				if (ImGui::MenuItem("Add layer..."))
				{
					std::vector<std::string> TempMaterialList = RESOURCE_MANAGER.GetMaterialList();
					std::vector<FEObject*> FinalMaterialList;
					for (size_t i = 0; i < TempMaterialList.size(); i++)
					{
						if (RESOURCE_MANAGER.GetMaterial(TempMaterialList[i])->IsCompackPacking())
						{
							FinalMaterialList.push_back(RESOURCE_MANAGER.GetMaterial(TempMaterialList[i]));
						}
					}

					if (FinalMaterialList.empty())
					{
						MessagePopUp::getInstance().Show("No suitable material", "There are no materials with compack packing.");
					}
					else
					{
						TerrainToWorkWith = Terrain;
						SelectFEObjectPopUp::getInstance().Show(FE_MATERIAL, CreateNewTerrainLayerWithMaterialCallBack, nullptr, FinalMaterialList);
					}
				}

				if (Terrain->GetLayerInSlot(FE_TERRAIN_MAX_LAYERS - 1) != nullptr)
					ImGui::EndDisabled();

				if (HoveredTerrainLayerItem != -1)
				{
					FETerrainLayer* layer = Terrain->GetLayerInSlot(HoveredTerrainLayerItem);
					if (layer != nullptr)
					{
						ImGui::Separator();
						std::string LayerName = layer->GetName();
						ImGui::Text((std::string("Actions with ") + LayerName).c_str());
						ImGui::Separator();

						if (ImGui::MenuItem("Rename"))
						{
							TerrainLayerRenameIndex = HoveredTerrainLayerItem;

							strcpy_s(TerrainLayerRename, layer->GetName().size() + 1, layer->GetName().c_str());
							bLastFrameTerrainLayerRenameEditWasVisiable = false;
						}

						if (ImGui::MenuItem("Fill"))
						{
							RESOURCE_MANAGER.FillTerrainLayerMask(Terrain, HoveredTerrainLayerItem);
						}

						if (ImGui::MenuItem("Clear"))
						{
							RESOURCE_MANAGER.ClearTerrainLayerMask(Terrain, HoveredTerrainLayerItem);
						}

						if (ImGui::MenuItem("Delete"))
						{
							RESOURCE_MANAGER.DeleteTerrainLayerMask(Terrain, HoveredTerrainLayerItem);
						}

						ImGui::Separator();

						if (ImGui::MenuItem("Change material..."))
						{
							std::vector<std::string> TempMaterialList = RESOURCE_MANAGER.GetMaterialList();
							std::vector<FEObject*> FinalMaterialList;
							for (size_t i = 0; i < TempMaterialList.size(); i++)
							{
								if (RESOURCE_MANAGER.GetMaterial(TempMaterialList[i])->IsCompackPacking())
								{
									FinalMaterialList.push_back(RESOURCE_MANAGER.GetMaterial(TempMaterialList[i]));
								}
							}

							if (FinalMaterialList.empty())
							{
								MessagePopUp::getInstance().Show("No suitable material", "There are no materials with compack packing.");
							}
							else
							{
								TerrainToWorkWith = Terrain;
								TempLayerIndex = HoveredTerrainLayerItem;
								SelectFEObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeMaterialInTerrainLayerCallBack, Terrain->GetLayerInSlot(HoveredTerrainLayerItem)->GetMaterial(), FinalMaterialList);
							}
						}

						if (ImGui::MenuItem("Export mask..."))
						{
							std::string filePath = "";
							FILE_SYSTEM.ShowFileSaveDialog(filePath, TEXTURE_LOAD_FILTER, 1);
							if (!filePath.empty())
							{
								filePath += ".png";
								RESOURCE_MANAGER.SaveTerrainLayerMask(filePath.c_str(), Terrain, HoveredTerrainLayerItem);
							}
						}

						if (ImGui::MenuItem("Import mask..."))
						{
							std::string filePath = "";
							FILE_SYSTEM.ShowFileOpenDialog(filePath, TEXTURE_LOAD_FILTER, 1);
							if (!filePath.empty())
							{
								RESOURCE_MANAGER.LoadTerrainLayerMask(filePath.c_str(), Terrain, HoveredTerrainLayerItem);
								PROJECT_MANAGER.GetCurrent()->SetModified(true);
							}
						}
					}
				}

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			if (!bContextMenuOpened)
				HoveredTerrainLayerItem = -1;

			bContextMenuOpened = false;
		}
		ImGui::EndTabBar();
	}
}