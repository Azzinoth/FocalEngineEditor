#include "InspectorWindow.h"
#include "../FEEditor.h"

FEEntity* FEEditorInspectorWindow::EntityToModify = nullptr;
FEEntity* FEEditorInspectorWindow::TerrainToWorkWith = nullptr;

FEEditorInspectorWindow::FEEditorInspectorWindow()
{
	std::vector<FEComponentTypeInfo> RegisteredComponentList = COMPONENTS_TOOL.GetComponentInfoList();
	AddComponentHandlers[typeid(FELightComponent)] = &FEEditorInspectorWindow::AddLightComponent;
	AddComponentHandlers[typeid(FECameraComponent)] = &FEEditorInspectorWindow::AddCameraComponent;
	AddComponentHandlers[typeid(FEGameModelComponent)] = &FEEditorInspectorWindow::AddGameModelComponent;
	AddComponentHandlers[typeid(FEInstancedComponent)] = &FEEditorInspectorWindow::AddInstancedComponent;
	AddComponentHandlers[typeid(FETerrainComponent)] = &FEEditorInspectorWindow::AddTerrainComponent;
	AddComponentHandlers[typeid(FEVirtualUIComponent)] = &FEEditorInspectorWindow::AddVirtualUIComponent;
	AddComponentHandlers[typeid(FENativeScriptComponent)] = &FEEditorInspectorWindow::AddNativeScriptComponent;

	RemoveComponentHandlers[typeid(FECameraComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FECameraComponent>();
	};
	RemoveComponentHandlers[typeid(FELightComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FELightComponent>();
	};
	RemoveComponentHandlers[typeid(FEGameModelComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FEGameModelComponent>();
	};
	RemoveComponentHandlers[typeid(FEInstancedComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FEInstancedComponent>();
	};
	RemoveComponentHandlers[typeid(FETerrainComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FETerrainComponent>();
	};
	RemoveComponentHandlers[typeid(FESkyDomeComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FESkyDomeComponent>();
	};
	RemoveComponentHandlers[typeid(FEPrefabInstanceComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FEPrefabInstanceComponent>();
	};
	RemoveComponentHandlers[typeid(FEVirtualUIComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FEVirtualUIComponent>();
	};
	RemoveComponentHandlers[typeid(FENativeScriptComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FENativeScriptComponent>();
	};
}

void FEEditorInspectorWindow::InitializeResources()
{
	// ************** Terrain Settings **************
	ExportHeightMapButton = new ImGuiButton("Export HeightMap");
	ExportHeightMapButton->SetSize(ImVec2(200, 0));

	ImportHeightMapButton = new ImGuiButton("Import HeightMap");
	ImportHeightMapButton->SetSize(ImVec2(200, 0));

	SculptBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/sculptBrush.png", "sculptBrushIcon");
	RESOURCE_MANAGER.SetTag(SculptBrushIcon, EDITOR_RESOURCE_TAG);
	SculptBrushButton = new ImGuiImageButton(SculptBrushIcon);
	SculptBrushButton->SetSize(ImVec2(24, 24));

	LevelBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/levelBrush.png", "levelBrushIcon");
	RESOURCE_MANAGER.SetTag(LevelBrushIcon, EDITOR_RESOURCE_TAG);
	LevelBrushButton = new ImGuiImageButton(LevelBrushIcon);
	LevelBrushButton->SetSize(ImVec2(24, 24));

	SmoothBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/smoothBrush.png", "smoothBrushIcon");
	RESOURCE_MANAGER.SetTag(SmoothBrushIcon, EDITOR_RESOURCE_TAG);
	SmoothBrushButton = new ImGuiImageButton(SmoothBrushIcon);
	SmoothBrushButton->SetSize(ImVec2(24, 24));

	DrawBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/paintbrush.png", "drawBrushIcon");
	RESOURCE_MANAGER.SetTag(DrawBrushIcon, EDITOR_RESOURCE_TAG);
	LayerBrushButton = new ImGuiImageButton(DrawBrushIcon);
	LayerBrushButton->SetSize(ImVec2(48, 48));

	EntityChangePrefabTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_PREFAB, EntityChangePrefabTargetCallBack, nullptr, "Drop to assign prefab");
	// ************** Terrain Settings END **************

	MouseCursorIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/mouseCursorIcon.png", "mouseCursorIcon");
	RESOURCE_MANAGER.SetTag(MouseCursorIcon, EDITOR_RESOURCE_TAG);
	ArrowToGroundIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/arrowToGroundIcon.png", "arrowToGroundIcon");
	RESOURCE_MANAGER.SetTag(ArrowToGroundIcon, EDITOR_RESOURCE_TAG);
}

void FEEditorInspectorWindow::Clear()
{
	
}

void FEEditorInspectorWindow::ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const
{
	ShowTransformConfiguration(Object->GetName(), Transform);
}

bool FEEditorInspectorWindow::EntityChangePrefabTargetCallBack(FEObject* Object, void** EntityPointer)
{
	// FIX ME! Prefabs
	/*FEEntity* Entity = SELECTED.GetEntity();
	if (Entity == nullptr)
		return false;

	Entity->Prefab = (RESOURCE_MANAGER.GetPrefab(Object->GetObjectID()));
	return true;*/
	return false;
}

bool FEEditorInspectorWindow::TerrainChangeMaterialTargetCallBack(FEObject* Object, void** LayerIndex)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return false;

	FEScene* CurrentScene = EDITOR.GetFocusedScene();

	if (SELECTED.GetSelected(CurrentScene) == nullptr || !SELECTED.GetSelected(CurrentScene)->HasComponent<FETerrainComponent>())
		return false;

	FEMaterial* MaterialToAssign = RESOURCE_MANAGER.GetMaterial(Object->GetObjectID());
	if (!MaterialToAssign->IsCompackPacking())
		return false;

	FETerrainComponent& Terrain = SELECTED.GetSelected(CurrentScene)->GetComponent<FETerrainComponent>();
	const int TempLayerIndex = *(int*)LayerIndex;
	if (TempLayerIndex >= 0 && TempLayerIndex < FE_TERRAIN_MAX_LAYERS)
		Terrain.GetLayerInSlot(TempLayerIndex)->SetMaterial(MaterialToAssign);

	return true;
}

void FEEditorInspectorWindow::ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform) const
{
	static float EditWidth = 70.0f;
	bool bModified = false;
	// ********************* POSITION *********************
	glm::vec3 Position = Transform->GetPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X pos : ") + Name).c_str(), &Position[0], 0.1f))
		bModified = true;
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y pos : ") + Name).c_str(), &Position[1], 0.1f))
		bModified = true;
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z pos : ") + Name).c_str(), &Position[2], 0.1f))
		bModified = true;
	ShowToolTip("Z position");
	
	if (bModified)
		Transform->SetPosition(Position);

	bModified = false;

	// ********************* WORLD POSITION *********************
	glm::vec3 WorldPosition = Transform->GetPosition(FE_WORLD_SPACE);
	ImGui::Text("World Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X pos : ") + Name).c_str(), &WorldPosition[0], 0.1f))
		bModified = true;
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y pos : ") + Name).c_str(), &WorldPosition[1], 0.1f))
		bModified = true;
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z pos : ") + Name).c_str(), &WorldPosition[2], 0.1f))
		bModified = true;
	ShowToolTip("Z position");

	if (bModified)
		Transform->SetPosition(WorldPosition, FE_WORLD_SPACE);

	bModified = false;

	// ********************* ROTATION *********************
	glm::vec3 Rotation = Transform->GetRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X rot : ") + Name).c_str(), &Rotation[0], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y rot : ") + Name).c_str(), &Rotation[1], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z rot : ") + Name).c_str(), &Rotation[2], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Z rotation");

	if (bModified)
		Transform->SetRotation(Rotation);

	bModified = false;

	// ********************* WORLD ROTATION *********************
	glm::vec3 WorldRotation = Transform->GetRotation(FE_WORLD_SPACE);
	ImGui::Text("World Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X rot : ") + Name).c_str(), &WorldRotation[0], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y rot : ") + Name).c_str(), &WorldRotation[1], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z rot : ") + Name).c_str(), &WorldRotation[2], 0.1f, -360.0f, 360.0f))
		bModified = true;
	ShowToolTip("Z rotation");

	if (bModified)
		Transform->SetRotation(WorldRotation, FE_WORLD_SPACE);

	bModified = false;

	// ********************* SCALE *********************
	bool bUniformScaling = Transform->IsUniformScalingSet();
	ImGui::Checkbox("Uniform scaling", &bUniformScaling);
	Transform->SetUniformScaling(bUniformScaling);

	glm::vec3 Scale = Transform->GetScale();
	float ScaleChangeSpeed = Scale.x * 0.01f;
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##X scale : ") + Name).c_str(), &Scale[0], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[1] = Scale[0];
			Scale[2] = Scale[0];
		}
	}
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Y scale : ") + Name).c_str(), &Scale[1], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[0] = Scale[1];
			Scale[2] = Scale[1];
		}
	}
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##Z scale : ") + Name).c_str(), &Scale[2], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			Scale[0] = Scale[2];
			Scale[1] = Scale[2];
		}
	}
	ShowToolTip("Z scale");

	if (bModified)
		Transform->SetScale(Scale);

	bModified = false;

	// ********************* WORLD SCALE *********************
	glm::vec3 WorldScale = Transform->GetScale(FE_WORLD_SPACE);
	ScaleChangeSpeed = WorldScale.x * 0.01f;
	ImGui::Text("World Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World X scale : ") + Name).c_str(), &WorldScale[0], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[1] = WorldScale[0];
			WorldScale[2] = WorldScale[0];
		}
	}
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Y scale : ") + Name).c_str(), &WorldScale[1], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[0] = WorldScale[1];
			WorldScale[2] = WorldScale[1];
		}
	}
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(EditWidth);
	if (ImGui::DragFloat((std::string("##World Z scale : ") + Name).c_str(), &WorldScale[2], ScaleChangeSpeed, 0.001f, 1000.0f))
	{
		bModified = true;
		if (bUniformScaling)
		{
			WorldScale[0] = WorldScale[2];
			WorldScale[1] = WorldScale[2];
		}
	}
	ShowToolTip("Z scale");

	if (bModified)
		Transform->SetScale(WorldScale, FE_WORLD_SPACE);
}

void FEEditorInspectorWindow::DisplayLightProperties(FEEntity* LightEntity) const
{
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();

	if (LightComponent.GetType() == FE_DIRECTIONAL_LIGHT)
	{
		ImGui::Text("Cast shadows:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		bool bCastShadows = LightComponent.IsCastShadows();
		ImGui::Checkbox("##Cast shadows", &bCastShadows);
		LightComponent.SetCastShadows(bCastShadows);
		ShowToolTip("Will this light cast shadows.");

		if (!LightComponent.IsCastShadows())
			ImGui::BeginDisabled();

		ImGui::Text("Number of cascades :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		int CascadesCount = LightComponent.GetActiveCascades();
		ImGui::SliderInt("##cascades", &CascadesCount, 1, 4);
		LightComponent.SetActiveCascades(CascadesCount);
		ShowToolTip("How much steps of shadow quality will be used.");

		ImGui::Text("Shadow coverage in M :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float FirstCascadeSize = LightComponent.GetShadowCoverage();
		ImGui::DragFloat("##shadowCoverage", &FirstCascadeSize, 0.1f, 0.1f, 500.0f);
		LightComponent.SetShadowCoverage(FirstCascadeSize);
		ShowToolTip("Distance from camera at which shadows would be present.");

		ImGui::Text("Z depth of shadow map :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float CSMZDepth = LightComponent.GetCSMZDepth();
		ImGui::DragFloat("##CSMZDepth", &CSMZDepth, 0.01f, 0.1f, 100.0f);
		LightComponent.SetCSMZDepth(CSMZDepth);
		ShowToolTip("If you have problems with shadow disapearing when camera is at close distance to shadow reciver, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		ImGui::Text("XY depth of shadow map :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float CSMXYDepth = LightComponent.GetCSMXYDepth();
		ImGui::DragFloat("##CSMXYDepth", &CSMXYDepth, 0.01f, 0.0f, 100.0f);
		LightComponent.SetCSMXYDepth(CSMXYDepth);
		ShowToolTip("If you have problems with shadow on edges of screen, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		ImGui::Text("Shadows blur factor:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		float ShadowsBlurFactor = LightComponent.GetShadowBlurFactor();
		ImGui::DragFloat("##Shadows blur factor", &ShadowsBlurFactor, 0.001f, 0.0f, 10.0f);
		LightComponent.SetShadowBlurFactor(ShadowsBlurFactor);

		bool bStaticShadowBias = LightComponent.IsStaticShadowBias();
		ImGui::Checkbox("Static shadow bias :", &bStaticShadowBias);
		LightComponent.SetIsStaticShadowBias(bStaticShadowBias);

		if (LightComponent.IsStaticShadowBias())
		{
			ImGui::Text("Static shadow bias value :");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			float ShadowBias = LightComponent.GetShadowBias();
			ImGui::DragFloat("##shadowBias", &ShadowBias, 0.0001f, 0.00001f, 0.1f);
			LightComponent.SetShadowBias(ShadowBias);
		}
		else
		{
			ImGui::Text("Intensity of variable shadow bias :");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			float ShadowBiasIntensity = LightComponent.GetShadowBiasVariableIntensity();
			ImGui::DragFloat("##shadowBiasIntensity", &ShadowBiasIntensity, 0.01f, 0.01f, 10.0f);
			LightComponent.SetShadowBiasVariableIntensity(ShadowBiasIntensity);
		}

		if (!LightComponent.IsCastShadows())
			ImGui::EndDisabled();
	}
	else if (LightComponent.GetType() == FE_POINT_LIGHT)
	{
		ImGui::Text("Light range :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float LightRange = LightComponent.GetRange();
		ImGui::DragFloat("##LightRadius", &LightRange, 0.1f, 0.1f, 100.0f);
		LightComponent.SetRange(LightRange);
	
	}
	else if (LightComponent.GetType() == FE_SPOT_LIGHT)
	{
		glm::vec3 Direction = LIGHT_SYSTEM.GetDirection(LightEntity);
		ImGui::DragFloat("##x", &Direction[0], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##y", &Direction[1], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##z", &Direction[2], 0.01f, 0.0f, 1.0f);

		float SpotAngle = LightComponent.GetSpotAngle();
		ImGui::SliderFloat((std::string("Inner angle##") + LightEntity->GetName()).c_str(), &SpotAngle, 0.0f, 90.0f);
		LightComponent.SetSpotAngle(SpotAngle);

		float SpotAngleOuter = LightComponent.GetSpotAngleOuter();
		ImGui::SliderFloat((std::string("Outer angle ##") + LightEntity->GetName()).c_str(), &SpotAngleOuter, 0.0f, 90.0f);
		LightComponent.SetSpotAngleOuter(SpotAngleOuter);
	}

	glm::vec3 Color = LightComponent.GetColor();
	ImGui::ColorEdit3((std::string("Color##") + LightEntity->GetName()).c_str(), &Color.x);
	LightComponent.SetColor(Color);

	float Intensity = LightComponent.GetIntensity();
	ImGui::SliderFloat((std::string("Intensity##") + LightEntity->GetName()).c_str(), &Intensity, 0.0f, 100.0f);
	LightComponent.SetIntensity(Intensity);
}

void FEEditorInspectorWindow::DisplayCameraProperties(FEEntity* CameraEntity) const
{
	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

	if (EDITOR.GetFocusedScene() == nullptr)
		return;

	// Because of editor camera system, we need to check if it is main camera in a different way.
	std::string MainCameraID = PROJECT_MANAGER.GetCurrent()->GetProperMainCameraIDBySceneID(EDITOR.GetFocusedScene()->GetObjectID());
	bool bIsMainCamera = MainCameraID == CameraEntity->GetObjectID();
	if (ImGui::Checkbox("Main camera", &bIsMainCamera))
	{
		if (bIsMainCamera)
		{
			PROJECT_MANAGER.GetCurrent()->SetProperMainCameraIDBySceneID(EDITOR.GetFocusedScene()->GetObjectID(), CameraEntity->GetObjectID());
		}
		else
		{
			PROJECT_MANAGER.GetCurrent()->SetProperMainCameraIDBySceneID(EDITOR.GetFocusedScene()->GetObjectID(), "");
		}
	}

	// Camera Preview
	ImGui::Text("Camera Preview:");

	FEScene* CurrentScene = CameraEntity->GetParentScene();
	FEEntity* OldMainCameraEntity = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene());
	CAMERA_SYSTEM.SetMainCamera(CameraEntity);

	CameraComponent.SetRenderTargetSize(452, 256);

	// It would not render, because it does not have proper FEViewPort
	RENDERER.Render(CurrentScene);
	FETexture* PreviewTexture = RENDERER.GetCameraResult(CameraEntity);
	if (PreviewTexture != nullptr)
		ImGui::Image((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(452, 256), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	CAMERA_SYSTEM.SetMainCamera(OldMainCameraEntity);

	// Rendering pipeline settings
	ImGui::Text("Rendering pipeline:");
	static const char* RenderingPipelineOptions[2] = { "Deferred", "Forward_Simplified" };
	FERenderingPipeline SelectedRenderingPipeline = CameraComponent.GetRenderingPipeline();
	static int CurrentIndex = 0;
	if (SelectedRenderingPipeline == FERenderingPipeline::Deferred)
	{
		CurrentIndex = 0;
	}
	else if (SelectedRenderingPipeline == FERenderingPipeline::Forward_Simplified)
	{
		CurrentIndex = 1;
	}

	ImGui::SetNextItemWidth(200);
	if (ImGui::Combo("##RenderingPipeline", &CurrentIndex, RenderingPipelineOptions, 2))
	{
		CAMERA_SYSTEM.SetCameraRenderingPipeline(CameraEntity, CurrentIndex == 0 ? FERenderingPipeline::Deferred : FERenderingPipeline::Forward_Simplified);
	}

	float FOV = CameraComponent.GetFOV();
	ImGui::Text("Field of view : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##FOV", &FOV, 0.1f, 1.0f, 179.0f);
	CameraComponent.SetFOV(FOV);

	float NearPlane = CameraComponent.GetNearPlane();
	ImGui::Text("Near plane : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##NearPlane", &NearPlane, 0.1f, 0.01f, 1000.0f);
	CameraComponent.SetNearPlane(NearPlane);

	float FarPlane = CameraComponent.GetFarPlane();
	ImGui::Text("Far plane : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##FarPlane", &FarPlane, 0.1f, 0.01f, 10000.0f);
	CameraComponent.SetFarPlane(FarPlane);

	glm::vec4 CurrentClearColor = CameraComponent.GetClearColor();
	ImGui::ColorEdit4("Clear color", &CurrentClearColor.x);
	CameraComponent.SetClearColor(CurrentClearColor);

	float Gamma = CameraComponent.GetGamma();
	ImGui::Text("Gamma : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Gamma", &Gamma, 0.1f, 0.1f, 10.0f);
	CameraComponent.SetGamma(Gamma);

	float Exposure = CameraComponent.GetExposure();
	ImGui::Text("Exposure : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Exposure", &Exposure, 0.1f, 0.1f, 10.0f);
	CameraComponent.SetExposure(Exposure);

	// *********** Bloom ***********
	ImGui::Separator();
	ImGui::Text("Bloom:");
	ImGui::Text("Threshold:");
	float Threshold = CameraComponent.GetBloomThreshold();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##Threshold", &Threshold, 0.01f, 0.001f, 30.0f);
	CameraComponent.SetBloomThreshold(Threshold);

	/*ImGui::PushID(GUIID++);
	ImGui::SameLine();
	ResetButton->Render();
	if (ResetButton->IsClicked())
	{
		CameraComponent.SetBloomThreshold(1.5f);
	}
	ImGui::PopID();*/

	ImGui::Text("Size:");
	float Size = CameraComponent.GetBloomSize();
	ImGui::SetNextItemWidth(70);
	ImGui::DragFloat("##BloomSize", &Size, 0.01f, 0.001f, 100.0f);
	CameraComponent.SetBloomSize(Size);

	/*ImGui::PushID(GUIID++);
	ImGui::SameLine();
	ResetButton->Render();
	if (ResetButton->IsClicked())
	{
		CameraComponent.SetBloomSize(5.0f);
	}
	ImGui::PopID();*/

	// *********** Anti-Aliasing(FXAA) ***********
	ImGui::Separator();
	ImGui::Text("Anti-Aliasing(FXAA)");
	static const char* options[5] = { "none", "1x", "2x", "4x", "8x" };
	static std::string SelectedOption = "1x";

	// FIX ME! Was used with one camera only.
	static bool bFirstLook = true;
	if (bFirstLook)
	{
		const float FXAASpanMax = CameraComponent.GetFXAASpanMax();
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
		const float FXAASpanMax = CameraComponent.GetFXAASpanMax();
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
					CameraComponent.SetFXAASpanMax(float(pow(2.0, (i - 1))));
					if (i == 0)
						CameraComponent.SetFXAASpanMax(0.0f);
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
		ImGui::SetNextItemWidth(70);
		float FXAASpanMax = CameraComponent.GetFXAASpanMax();
		ImGui::DragFloat("##FXAASpanMax", &FXAASpanMax, 0.0f, 0.001f, 32.0f);
		CameraComponent.SetFXAASpanMax(FXAASpanMax);

		/*ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			CameraComponent.SetFXAASpanMax(8.0f);
		}
		ImGui::PopID();*/

		ImGui::Text("FXAAReduceMin:");
		ImGui::SetNextItemWidth(70);
		float FXAAReduceMin = CameraComponent.GetFXAAReduceMin();
		ImGui::DragFloat("##FXAAReduceMin", &FXAAReduceMin, 0.01f, 0.001f, 100.0f);
		CameraComponent.SetFXAAReduceMin(FXAAReduceMin);

		/*ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			CameraComponent.SetFXAAReduceMin(0.008f);
		}
		ImGui::PopID();*/

		ImGui::Text("FXAAReduceMul:");
		ImGui::SetNextItemWidth(70);
		float FXAAReduceMul = CameraComponent.GetFXAAReduceMul();
		ImGui::DragFloat("##FXAAReduceMul", &FXAAReduceMul, 0.01f, 0.001f, 100.0f);
		CameraComponent.SetFXAAReduceMul(FXAAReduceMul);

		/*ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			CameraComponent.SetFXAAReduceMul(0.400f);
		}
		ImGui::PopID();*/
	}

	ImGui::Separator();
	ImGui::Text("Depth of Field:");
	// *********** Depth of Field ***********
	ImGui::Text("Near distance:");
	ImGui::SetNextItemWidth(70);
	float DepthThreshold = CameraComponent.GetDOFNearDistance();
	ImGui::DragFloat("##depthThreshold", &DepthThreshold, 0.0f, 0.001f, 100.0f);
	CameraComponent.SetDOFNearDistance(DepthThreshold);

	ImGui::Text("Far distance:");
	ImGui::SetNextItemWidth(70);
	float DepthThresholdFar = CameraComponent.GetDOFFarDistance();
	ImGui::DragFloat("##depthThresholdFar", &DepthThresholdFar, 0.0f, 0.001f, 100.0f);
	CameraComponent.SetDOFFarDistance(DepthThresholdFar);

	ImGui::Text("Strength:");
	ImGui::SetNextItemWidth(70);
	float Strength = CameraComponent.GetDOFStrength();
	ImGui::DragFloat("##Strength", &Strength, 0.0f, 0.001f, 10.0f);
	CameraComponent.SetDOFStrength(Strength);

	ImGui::Text("Distance dependent strength:");
	ImGui::SetNextItemWidth(70);
	float IntMult = CameraComponent.GetDOFDistanceDependentStrength();
	ImGui::DragFloat("##Distance dependent strength", &IntMult, 0.0f, 0.001f, 100.0f);
	CameraComponent.SetDOFDistanceDependentStrength(IntMult);

	ImGui::Separator();
	ImGui::Text("Chromatic Aberration:");
	ImGui::Text("Shift strength:");
	ImGui::SetNextItemWidth(70);
	float intensity = CameraComponent.GetChromaticAberrationIntensity();
	ImGui::DragFloat("##intensity", &intensity, 0.01f, 0.0f, 30.0f);
	CameraComponent.SetChromaticAberrationIntensity(intensity);

	/*ImGui::PushID(GUIID++);
	ImGui::SameLine();
	ResetButton->Render();
	if (ResetButton->IsClicked())
	{
		CameraComponent.SetChromaticAberrationIntensity(1.0f);
	}
	ImGui::PopID();*/

	ImGui::Separator();
	ImGui::Text("SSAO:");
	// *********** SSAO ***********
	static const char* SSAO_Options[5] = { "Off", "Low", "Medium", "High", "Custom" };
	static std::string SSAO_SelectedOption = "Medium";

	// FIX ME! Was used with one camera only.
	static bool bSSAO_FirstLook = true;
	if (bFirstLook)
	{
		const int SampleCount = CameraComponent.GetSSAOSampleCount();

		if (!CameraComponent.IsSSAOEnabled())
		{
			SSAO_SelectedOption = SSAO_Options[0];
		}
		else if (SampleCount == 4)
		{
			SSAO_SelectedOption = SSAO_Options[1];
		}
		else if (SampleCount == 16 && CameraComponent.GetSSAORadiusSmallDetails())
		{
			SSAO_SelectedOption = SSAO_Options[2];
		}
		else if (SampleCount == 32 && CameraComponent.GetSSAORadiusSmallDetails())
		{
			SSAO_SelectedOption = SSAO_Options[3];
		}
		else
		{
			SSAO_SelectedOption = SSAO_Options[4];
		}

		//bFirstLook = false;
	}

	static bool bSSAO_DebugSettings = false;
	if (ImGui::Checkbox("Debug view", &bSSAO_DebugSettings))
	{

	}

	if (!bSSAO_DebugSettings)
	{
		ImGui::Text("SSAO Quality:");
		ImGui::SetNextItemWidth(150);
		if (ImGui::BeginCombo("##SSAO Quality", SSAO_SelectedOption.c_str(), ImGuiWindowFlags_None))
		{
			for (size_t i = 0; i < 5; i++)
			{
				const bool is_selected = (SSAO_SelectedOption == SSAO_Options[i]);
				if (ImGui::Selectable(SSAO_Options[i], is_selected))
				{
					CameraComponent.SetSSAOResultBlured(true);
					CameraComponent.SetSSAOBias(0.013f);
					CameraComponent.SetSSAORadius(10.0f);
					CameraComponent.SetSSAORadiusSmallDetails(0.4f);
					CameraComponent.SetSSAOSmallDetailsWeight(0.2f);

					if (i == 0)
					{
						CameraComponent.SetSSAOEnabled(false);
					}
					else if (i == 1)
					{
						CameraComponent.SetSSAOEnabled(true);

						CameraComponent.SetSSAOSampleCount(4);
						CameraComponent.SetSSAOSmallDetailsEnabled(false);
					}
					else if (i == 2)
					{
						CameraComponent.SetSSAOEnabled(true);

						CameraComponent.SetSSAOSampleCount(16);
						CameraComponent.SetSSAOSmallDetailsEnabled(true);
					}
					else if (i == 3)
					{
						CameraComponent.SetSSAOEnabled(true);

						CameraComponent.SetSSAOSampleCount(32);
						CameraComponent.SetSSAOSmallDetailsEnabled(true);
					}

					SSAO_SelectedOption = SSAO_Options[i];
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		bool TempBool = CameraComponent.IsSSAOEnabled();
		ImGui::Checkbox("SSAO active", &TempBool);
		CameraComponent.SetSSAOEnabled(TempBool);

		TempBool = CameraComponent.IsSSAOSmallDetailsEnabled();
		ImGui::Checkbox("SSAO small details", &TempBool);
		CameraComponent.SetSSAOSmallDetailsEnabled(TempBool);

		TempBool = CameraComponent.IsSSAOResultBlured();
		ImGui::Checkbox("SSAO blured", &TempBool);
		CameraComponent.SetSSAOResultBlured(TempBool);

		int TempInt = CameraComponent.GetSSAOSampleCount();
		ImGui::SetNextItemWidth(100);
		ImGui::DragInt("SSAO sample count", &TempInt);
		CameraComponent.SetSSAOSampleCount(TempInt);

		float TempFloat = CameraComponent.GetSSAOBias();
		ImGui::SetNextItemWidth(100);
		ImGui::DragFloat("SSAO bias", &TempFloat, 0.1f);
		CameraComponent.SetSSAOBias(TempFloat);

		TempFloat = CameraComponent.GetSSAORadius();
		ImGui::SetNextItemWidth(100);
		ImGui::DragFloat("SSAO radius", &TempFloat, 0.1f);
		CameraComponent.SetSSAORadius(TempFloat);

		TempFloat = CameraComponent.GetSSAORadiusSmallDetails();
		ImGui::SetNextItemWidth(100);
		ImGui::DragFloat("SSAO radius small details", &TempFloat, 0.1f);
		CameraComponent.SetSSAORadiusSmallDetails(TempFloat);

		TempFloat = CameraComponent.GetSSAOSmallDetailsWeight();
		ImGui::SetNextItemWidth(100);
		ImGui::DragFloat("SSAO small details weight", &TempFloat, 0.01f);
		CameraComponent.SetSSAOSmallDetailsWeight(TempFloat);
	}
	
	// *********** Distance Fog ***********
	ImGui::Separator();
	ImGui::Text("Distance Fog:");
	bool bEnabledFog = CameraComponent.IsDistanceFogEnabled();
	if (ImGui::Checkbox("Enable fog", &bEnabledFog))
	{
		CameraComponent.SetDistanceFogEnabled(bEnabledFog);
	}

	if (bEnabledFog)
	{
		ImGui::Text("Density:");
		ImGui::SetNextItemWidth(70);
		float FogDensity = CameraComponent.GetDistanceFogDensity();
		ImGui::DragFloat("##fogDensity", &FogDensity, 0.0001f, 0.0f, 5.0f);
		CameraComponent.SetDistanceFogDensity(FogDensity);

		/*ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			CameraComponent.SetDistanceFogDensity(0.007f);
		}
		ImGui::PopID();*/

		ImGui::Text("Gradient:");
		ImGui::SetNextItemWidth(70);
		float FogGradient = CameraComponent.GetDistanceFogGradient();
		ImGui::DragFloat("##fogGradient", &FogGradient, 0.001f, 0.0f, 5.0f);
		CameraComponent.SetDistanceFogGradient(FogGradient);

		/*ImGui::PushID(GUIID++);
		ImGui::SameLine();
		ResetButton->Render();
		if (ResetButton->IsClicked())
		{
			CameraComponent.SetDistanceFogGradient(2.5f);
		}
		ImGui::PopID();*/
	}
}

// FIX ME!
void FEEditorInspectorWindow::ChangePrefabOfEntityCallBack(const std::vector<FEObject*> SelectionsResult)
{
	/*if (EntityToModify == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_PREFAB)
	{
		FEPrefab* SelectedPrefab = RESOURCE_MANAGER.GetPrefab(SelectionsResult[0]->GetObjectID());
		if (SelectedPrefab == nullptr)
			return;

		EntityToModify->Prefab = SelectedPrefab;
	}*/
}

void FEEditorInspectorWindow::AddLightComponent(FEEntity* Entity)
{
	Entity->AddComponent<FELightComponent>(FE_POINT_LIGHT);
}

void FEEditorInspectorWindow::AddCameraComponent(FEEntity* Entity)
{
	Entity->AddComponent<FECameraComponent>();
}

std::vector<std::string> FEEditorInspectorWindow::GetAvailableComponentsToAdd(FEEntity* Entity) const
{
	std::vector<std::string> Result;
	if (Entity == nullptr)
		return Result;

	auto RegisteredComponentList = COMPONENTS_TOOL.GetComponentInfoList();
	for (size_t i = 0; i < RegisteredComponentList.size(); i++)
	{
		if (RegisteredComponentList[i].CanBeAddedToEntity(Entity))
			Result.push_back(RegisteredComponentList[i].Name);
	}

	return Result;
}

bool FEEditorInspectorWindow::AddComponent(FEEntity* Entity, std::string ComponentName)
{
	auto RegisteredComponentList = COMPONENTS_TOOL.GetComponentInfoList();

	for (size_t i = 0; i < RegisteredComponentList.size(); i++)
	{
		if (RegisteredComponentList[i].Name == ComponentName)
		{
			auto ComponentHashCode = RegisteredComponentList[i].Type->hash_code();

			auto ComponentIterator = AddComponentHandlers.begin();
			while (ComponentIterator != AddComponentHandlers.end())
			{
				if (ComponentIterator->first.hash_code() == ComponentHashCode)
				{
					ComponentIterator->second(Entity);
					return true;
				}

				ComponentIterator++;
			}
		}
	}

	return false;
}

bool FEEditorInspectorWindow::RenderComponentDeleteButton(FEEntity* Entity, FEComponentTypeInfo* ComponentInfo) const
{
	if (!ComponentInfo->CanBeRemovedFromEntity(Entity))
		return false;

	float HeaderHeight = ImGui::GetFrameHeight();
	float HeaderWidth = ImGui::GetContentRegionAvail().x;

	float ButtonSize = 24;
	ImVec2 PreviosCursorPos = ImGui::GetCursorPos();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.5f, 0.5f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.90f, 0.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 1.0f, 0.1f, 1.0f));
	ImGui::SetCursorPos(ImVec2(HeaderWidth - ButtonSize / 8.0f - 2.0f, PreviosCursorPos.y + 2.0f));
	if (ImGui::Button(std::string("x##" + ComponentInfo->Name).c_str(), ImVec2(ButtonSize, ButtonSize)))
	{
		size_t ComponentHashCode = ComponentInfo->Type->hash_code();

		auto ComponentIterator = RemoveComponentHandlers.begin();
		while (ComponentIterator != RemoveComponentHandlers.end())
		{
			if (ComponentIterator->first.hash_code() == ComponentHashCode)
			{
				ComponentIterator->second(Entity);

				ImGui::PopStyleColor(3);
				ImGui::SetCursorPos(PreviosCursorPos);
				return true;
			}

			ComponentIterator++;
		}
	}

	ImGui::PopStyleColor(3);
	ImGui::SetCursorPos(PreviosCursorPos);

	return false;
}

void FEEditorInspectorWindow::Render()
{
	if (!bVisible)
		return;

	if (EDITOR.GetFocusedScene() == nullptr)
		return;

	FEScene* CurrentScene = EDITOR.GetFocusedScene();
	FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(CurrentScene);
	if (MainCameraEntity == nullptr)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);

	if (SELECTED.GetSelected(CurrentScene) == nullptr)
	{
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}

	FEEntity* EntitySelected = SELECTED.GetSelected(CurrentScene);

	ImGui::Text("ID : %s", EntitySelected->GetObjectID().c_str());
	ImGui::Text("Name : %s", EntitySelected->GetName().c_str());

	if (EntitySelected->HasComponent<FETagComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FETagComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Tag", ImGuiTreeNodeFlags_DefaultOpen))
		{
			FETagComponent& TagComponent = EntitySelected->GetComponent<FETagComponent>();
			char Buffer[1024];
			memset(Buffer, 0, 1024);
			strcpy_s(Buffer, TagComponent.GetTag().c_str());
			if (ImGui::InputText("##Tag Edit", Buffer, 1024))
			{
				std::string NewTag = Buffer;
				TagComponent.SetTag(NewTag);
			}
		}
	}

	if (EntitySelected->HasComponent<FETransformComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FETransformComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			FETransformComponent& Transform = EntitySelected->GetComponent<FETransformComponent>();
			ShowTransformConfiguration(EntitySelected->GetName(), &Transform);
		}
	}

	if (EntitySelected->HasComponent<FEGameModelComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEGameModelComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Game Model", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			FEGameModelComponent& GameModelComponent = EntitySelected->GetComponent<FEGameModelComponent>();
			bool bActive = GameModelComponent.IsWireframeMode();
			ImGui::Checkbox("WireframeMode", &bActive);
			GameModelComponent.SetWireframeMode(bActive);

			ImGui::Text("Game Model : ");
			FETexture* PreviewTexture = PREVIEW_MANAGER.GetGameModelPreview(GameModelComponent.GetGameModel()->GetObjectID());

			if (ImGui::ImageButton((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
			{
				//EntityToModify = EntitySelected;
				//SELECT_FEOBJECT_POPUP.Show(FE_PREFAB, ChangePrefabOfEntityCallBack, Entity->Prefab);
			}
			//EntityChangePrefabTarget->StickToItem();

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
					CONTENT_BROWSER_WINDOW.OpenItemParentFolder(GameModelComponent.GetGameModel());
				}

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
	}

	if (EntitySelected->HasComponent<FEPrefabInstanceComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEPrefabInstanceComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Prefab Instance", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Prefab ID : %s", EntitySelected->GetComponent<FEPrefabInstanceComponent>().GetPrefab()->GetObjectID().c_str());
		}
	}

	if (EntitySelected->HasComponent<FEInstancedComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEInstancedComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Instanced", ImGuiTreeNodeFlags_DefaultOpen))
		{
			FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(CurrentScene->GetObjectID());
			FEInstancedComponent& InstancedComponent = EntitySelected->GetComponent<FEInstancedComponent>();

			if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1)
			{
				std::string InstancedSubObjectInfo = "index: ";

				ImGui::Text("Selected instance info:");
				InstancedSubObjectInfo = "index: " + std::to_string(CurrentSelectionData->InstancedSubObjectIndexSelected);
				ImGui::Text(InstancedSubObjectInfo.c_str());

				FETransformComponent TempTransform = FETransformComponent(InstancedComponent.GetTransformedInstancedMatrix(CurrentSelectionData->InstancedSubObjectIndexSelected));
				TempTransform.SetSceneIndependent(true);
				ShowTransformConfiguration("Selected instance", &TempTransform);

				INSTANCED_RENDERING_SYSTEM.ModifyIndividualInstance(EntitySelected, CurrentSelectionData->InstancedSubObjectIndexSelected, TempTransform.GetWorldMatrix());

				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.55f, 0.55f, 0.95f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));

				if (ImGui::ImageButton((void*)(intptr_t)ArrowToGroundIcon->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					INSTANCED_RENDERING_SYSTEM.TryToSnapIndividualInstance(EntitySelected, CurrentSelectionData->InstancedSubObjectIndexSelected);
				}
				ShowToolTip("Selected instance will attempt to snap to the terrain.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
			else
			{
				ImGui::Text("Snapped to: ");
				ImGui::SameLine();

				const std::vector<std::string> TerrainList = CurrentScene->GetEntityIDListWithComponent<FETerrainComponent>();
				static std::string CurrentTerrain = "none";

				if (InstancedComponent.GetSnappedToTerrain() == nullptr)
				{
					CurrentTerrain = "none";
				}
				else
				{
					CurrentTerrain = InstancedComponent.GetSnappedToTerrain()->GetName();
				}

				ImGui::SetNextItemWidth(220);
				if (ImGui::BeginCombo("##Terrain", CurrentTerrain.c_str(), ImGuiWindowFlags_None))
				{
					const bool bIsSelected = (CurrentTerrain == "none");
					if (ImGui::Selectable("none", bIsSelected))
					{
						if (InstancedComponent.GetSnappedToTerrain() != nullptr)
						{
							TERRAIN_SYSTEM.UnSnapInstancedEntity(InstancedComponent.GetSnappedToTerrain(), EntitySelected);
						}
					}

					if (bIsSelected)
						ImGui::SetItemDefaultFocus();

					for (size_t i = 0; i < TerrainList.size(); i++)
					{
						const bool bIsSelected = (CurrentTerrain == TerrainList[i]);
						if (ImGui::Selectable(CurrentScene->GetEntity(TerrainList[i])->GetName().c_str(), bIsSelected))
						{
							TERRAIN_SYSTEM.SnapInstancedEntity(CurrentScene->GetEntity(TerrainList[i]), EntitySelected);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (InstancedComponent.GetSnappedToTerrain() != nullptr)
				{
					ImGui::Text("Terrain layer: ");
					ImGui::SameLine();

					const int CurrentLayer = InstancedComponent.GetTerrainLayer();
					FEEntity* TerrainEntity = InstancedComponent.GetSnappedToTerrain();
					FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

					std::string Caption = "none";
					FETerrainLayer* Layer = TerrainComponent.GetLayerInSlot(CurrentLayer);
					if (Layer != nullptr)
						Caption = Layer->GetName();

					ImGui::SetNextItemWidth(220);
					if (ImGui::BeginCombo("##TerrainLayers", Caption.c_str(), ImGuiWindowFlags_None))
					{
						const bool bIsSelected = (CurrentLayer == -1);
						ImGui::PushID("none_TerrainLayers_entity");
						if (ImGui::Selectable("none", bIsSelected))
						{
							TERRAIN_SYSTEM.UnConnectInstancedEntityFromLayer(EntitySelected);
						}
						ImGui::PopID();

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();

						for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
						{
							FETerrainLayer* Layer = TerrainComponent.GetLayerInSlot(i);
							if (Layer == nullptr)
								break;

							const bool bIsSelected = (CurrentLayer == i);
							ImGui::PushID(/*Layer->GetObjectID().c_str()*/static_cast<int>(i));
							if (ImGui::Selectable(Layer->GetName().c_str(), bIsSelected))
							{
								TERRAIN_SYSTEM.ConnectInstancedEntityToLayer(TerrainEntity, EntitySelected, static_cast<int>(i));
							}
							ImGui::PopID();

							if (bIsSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (CurrentLayer != -1)
					{
						ImGui::Text("Minimal layer intensity to spawn:");
						float MinLevel = InstancedComponent.GetMinimalLayerIntensityToSpawn();
						ImGui::SameLine();
						ImGui::SetNextItemWidth(80);
						ImGui::DragFloat("##minLevel", &MinLevel);
						InstancedComponent.SetMinimalLayerIntensityToSpawn(MinLevel);
					}
				}

				ImGui::Separator();

				ImGui::Text("Seed:");
				int seed = InstancedComponent.SpawnInfo.Seed;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::DragInt("##Seed", &seed);
				InstancedComponent.SpawnInfo.Seed = seed;

				ImGui::Text("Object count:");
				int ObjectCount = InstancedComponent.SpawnInfo.ObjectCount;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::DragInt("##Object count", &ObjectCount);
				if (ObjectCount <= 0)
					ObjectCount = 1;
				InstancedComponent.SpawnInfo.ObjectCount = ObjectCount;

				ImGui::Text("Radius:");
				float radius = InstancedComponent.SpawnInfo.Radius;
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200);
				ImGui::DragFloat("##Radius", &radius);
				if (radius < 0.0f)
					radius = 0.1f;
				InstancedComponent.SpawnInfo.Radius = radius;

				// Scale deviation.
				ImGui::Text("Scale: ");

				ImGui::SameLine();
				ImGui::Text("min ");

				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				float MinScale = InstancedComponent.SpawnInfo.GetMinScale();
				ImGui::DragFloat("##minScale", &MinScale, 0.01f);
				InstancedComponent.SpawnInfo.SetMinScale(MinScale);

				ImGui::SameLine();
				ImGui::Text("max ");

				ImGui::SameLine();
				float MaxScale = InstancedComponent.SpawnInfo.GetMaxScale();
				ImGui::SetNextItemWidth(100);
				ImGui::DragFloat("##maxScale", &MaxScale, 0.01f);
				InstancedComponent.SpawnInfo.SetMaxScale(MaxScale);

				ImGui::Text("Rotation deviation:");
				float RotationDeviationX = InstancedComponent.SpawnInfo.RotationDeviation.x;
				ImGui::Text("X:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation X", &RotationDeviationX, 0.01f);
				if (RotationDeviationX < 0.01f)
					RotationDeviationX = 0.01f;
				if (RotationDeviationX > 1.0f)
					RotationDeviationX = 1.0f;
				InstancedComponent.SpawnInfo.RotationDeviation.x = RotationDeviationX;

				float RotationDeviationY = InstancedComponent.SpawnInfo.RotationDeviation.y;
				ImGui::Text("Y:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation Y", &RotationDeviationY, 0.01f);
				if (RotationDeviationY < 0.01f)
					RotationDeviationY = 0.01f;
				if (RotationDeviationY > 1.0f)
					RotationDeviationY = 1.0f;
				InstancedComponent.SpawnInfo.RotationDeviation.y = RotationDeviationY;

				float RotationDeviationZ = InstancedComponent.SpawnInfo.RotationDeviation.z;
				ImGui::Text("Z:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation z", &RotationDeviationZ, 0.01f);
				if (RotationDeviationZ < 0.01f)
					RotationDeviationZ = 0.01f;
				if (RotationDeviationZ > 1.0f)
					RotationDeviationZ = 1.0f;
				InstancedComponent.SpawnInfo.RotationDeviation.z = RotationDeviationZ;

				if (ImGui::Button("Spawn/Re-Spawn"))
				{
					INSTANCED_RENDERING_SYSTEM.ClearInstance(EntitySelected);
					INSTANCED_RENDERING_SYSTEM.PopulateInstance(EntitySelected, InstancedComponent.SpawnInfo);
				}

				if (ImGui::Button("Add instance"))
				{
					glm::mat4 NewInstanceMatrix = glm::identity<glm::mat4>();
					FETransformComponent& CameraTransformComponent = MainCameraEntity->GetComponent<FETransformComponent>();
					FECameraComponent& CameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
					NewInstanceMatrix = glm::translate(NewInstanceMatrix, CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
					INSTANCED_RENDERING_SYSTEM.AddIndividualInstance(EntitySelected, NewInstanceMatrix);

					PROJECT_MANAGER.GetCurrent()->SetModified(true);
				}

				if (INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(EntitySelected))
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
					INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(EntitySelected, !INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(EntitySelected));
					if (!INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(EntitySelected))
					{
						SELECTED.Clear(CurrentScene);
						SELECTED.SetSelected(EntitySelected);
					}
				}
				ShowToolTip("Individual selection mode - Used to select individual instances.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
		}
	}

	if (EntitySelected->HasComponent<FETerrainComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FETerrainComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_DefaultOpen))
		{
			FETerrainComponent& TerrainComponent = EntitySelected->GetComponent<FETerrainComponent>();
			DisplayTerrainSettings(EntitySelected);

			if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
			{
				// To hide gizmos.
				SELECTED.SetSelected(EntitySelected);

				TERRAIN_SYSTEM.SetBrushActive(bLeftMousePressed);

				if (bShiftPressed)
				{
					if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW)
						TERRAIN_SYSTEM.SetBrushMode(EntitySelected, FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED);
				}
				else
				{
					if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED)
						TERRAIN_SYSTEM.SetBrushMode(EntitySelected, FE_TERRAIN_BRUSH_SCULPT_DRAW);
				}
			}
			else
			{
				// To show gizmos.
				SELECTED.SetSelected(EntitySelected);
			}
		}
	}

	if (EntitySelected->HasComponent<FELightComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FELightComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayLightProperties(SELECTED.GetSelected(CurrentScene));
		}
	}

	if (EntitySelected->HasComponent<FECameraComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FECameraComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayCameraProperties(SELECTED.GetSelected(CurrentScene));
		}
	}

	if (EntitySelected->HasComponent<FEVirtualUIComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEVirtualUIComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Virtual UI", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayVirtualUIProperties(SELECTED.GetSelected(CurrentScene));
		}
	}

	if (EntitySelected->HasComponent<FENativeScriptComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FENativeScriptComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (ImGui::CollapsingHeader("Native Script", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayNativeScriptProperties(SELECTED.GetSelected(CurrentScene));
		}
	}

	std::vector<std::string> AvailableComponentTypes = GetAvailableComponentsToAdd(EntitySelected);

	ImGui::SetNextItemWidth(220.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);

	float RegionWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetCursorPosX(RegionWidth * 0.5f - 220.0f / 2.0f + 20.0f);

	if (ImGui::BeginCombo("##Add Component", "   Add Component", ImGuiComboFlags_NoArrowButton))
	{
		for (size_t i = 0; i < AvailableComponentTypes.size(); i++)
		{
			if (ImGui::Selectable(AvailableComponentTypes[i].c_str(), false))
			{
				AddComponent(EntitySelected, AvailableComponentTypes[i]);
			}
		}
		ImGui::EndCombo();
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditorInspectorWindow::CreateNewTerrainLayerWithMaterialCallBack(std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		if (TerrainToWorkWith == nullptr)
			return;

		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		TERRAIN_SYSTEM.ActivateVacantLayerSlot(TerrainToWorkWith, SelectedMaterial);
	}

	TerrainToWorkWith = nullptr;
}

static size_t TempLayerIndex = -1;
void FEEditorInspectorWindow::ChangeMaterialInTerrainLayerCallBack(std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		if (TempLayerIndex == -1)
			return;

		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		FETerrainComponent& TerrainComponent = TerrainToWorkWith->GetComponent<FETerrainComponent>();
		TerrainComponent.GetLayerInSlot(TempLayerIndex)->SetMaterial(SelectedMaterial);
	}

	TerrainToWorkWith = nullptr;
	TempLayerIndex = -1;
}

void FEEditorInspectorWindow::DisplayTerrainSettings(FEEntity* TerrainEntity)
{
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();
	if (TerrainChangeLayerMaterialTargets.size() != TerrainComponent.LayersUsed())
	{
		for (size_t i = 0; i < TerrainChangeLayerMaterialTargets.size(); i++)
		{
			delete TerrainChangeLayerMaterialTargets[i];
		}

		TerrainChangeLayerMaterialTargets.resize(TerrainComponent.LayersUsed());
		TerrainChangeMaterialIndecies.resize(TerrainComponent.LayersUsed());
		for (size_t i = 0; i < size_t(TerrainComponent.LayersUsed()); i++)
		{
			TerrainChangeMaterialIndecies[i] = int(i);
			TerrainChangeLayerMaterialTargets[i] = DRAG_AND_DROP_MANAGER.AddTarget(FE_MATERIAL, TerrainChangeMaterialTargetCallBack, (void**)&TerrainChangeMaterialIndecies[i], "Drop to assing material to " + TerrainComponent.GetLayerInSlot(i)->GetName());
		}
	}

	if (ImGui::BeginTabBar("##terrainSettings", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("General"))
		{
			bool bActive = TerrainComponent.IsWireframeMode();
			ImGui::Checkbox("WireframeMode", &bActive);
			TerrainComponent.SetWireframeMode(bActive);

			FEShaderUniformValue CurrentValue;
			if (RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->GetUniformData("debugFlag", CurrentValue))
			{

				int IData = CurrentValue.GetValue<int>();
				ImGui::SliderInt("debugFlag", &IData, 0, 10);
				RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("debugFlag", IData);
			}

			float DisplacementScale = TerrainComponent.GetDisplacementScale();
			ImGui::DragFloat("displacementScale", &DisplacementScale, 0.02f, -10.0f, 10.0f);
			TerrainComponent.SetDisplacementScale(DisplacementScale);

			float LODLevel = TerrainComponent.GetLODLevel();
			ImGui::DragFloat("LODlevel", &LODLevel, 2.0f, 2.0f, 128.0f);
			TerrainComponent.SetLODLevel(LODLevel);
			ShowToolTip("Bigger LODlevel more details terraine will have and less performance you will get.");

			float ChunkPerSide = TerrainComponent.GetChunkPerSide();
			ImGui::DragFloat("chunkPerSide", &ChunkPerSide, 2.0f, 1.0f, 16.0f);
			TerrainComponent.SetChunkPerSide(ChunkPerSide);

			// ********************* REAL WORLD COMPARISON SCALE *********************
			FEAABB RealAABB = TERRAIN_SYSTEM.GetAABB(TerrainEntity);
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

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Sculpt"))
		{
			ExportHeightMapButton->Render();
			if (ExportHeightMapButton->IsClicked())
			{
				std::string FilePath = "";
				FILE_SYSTEM.ShowFileSaveDialog(FilePath, TEXTURE_LOAD_FILTER, 1);

				if (!FilePath.empty())
				{
					FilePath += ".png";
					RESOURCE_MANAGER.ExportFETextureToPNG(TerrainComponent.GetHeightMap(), FilePath.c_str());
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
					TERRAIN_SYSTEM.LoadHeightMap(FilePath.c_str(), TerrainEntity);
					FETexture* LoadedTexture = TerrainComponent.GetHeightMap();
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

			float HighScale = TerrainComponent.GetHightScale();
			ImGui::DragFloat("hight range in m", &HighScale);
			TerrainComponent.SetHightScale(HighScale);

			float CurrentBrushSize = TERRAIN_SYSTEM.GetBrushSize();
			ImGui::DragFloat("brushSize", &CurrentBrushSize, 0.1f, 0.01f, 100.0f);
			TERRAIN_SYSTEM.SetBrushSize(CurrentBrushSize);

			float CurrentBrushIntensity = TERRAIN_SYSTEM.GetBrushIntensity();
			ImGui::DragFloat("brushIntensity", &CurrentBrushIntensity, 0.0001f, 0.0001f, 10.0f);
			TERRAIN_SYSTEM.SetBrushIntensity(CurrentBrushIntensity);

			SetDefaultStyle(SculptBrushButton);
			if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW ||
				TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED)
				SetSelectedStyle(SculptBrushButton);

			SculptBrushButton->Render();
			ShowToolTip("Sculpt Brush. Left mouse to increase height, hold shift to decrease height.");

			if (SculptBrushButton->IsClicked())
			{
				if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW ||
					TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED)
				{
					TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_NONE);
				}
				else
				{
					TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_SCULPT_DRAW);
				}
			}

			SetDefaultStyle(LevelBrushButton);
			if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_LEVEL)
				SetSelectedStyle(LevelBrushButton);

			ImGui::SameLine();
			LevelBrushButton->Render();
			ShowToolTip("Level Brush.");

			if (LevelBrushButton->IsClicked())
			{
				if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_LEVEL)
				{
					TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_NONE);
				}
				else
				{
					TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_SCULPT_LEVEL);
				}
			}

			SetDefaultStyle(SmoothBrushButton);
			if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_SMOOTH)
				SetSelectedStyle(SmoothBrushButton);

			ImGui::SameLine();
			SmoothBrushButton->Render();
			ShowToolTip("Smooth Brush.");

			if (SmoothBrushButton->IsClicked())
			{
				if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_SCULPT_SMOOTH)
				{
					TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_NONE);
				}
				else
				{
					TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_SCULPT_SMOOTH);
				}
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Paint"))
		{
			glm::vec2 TileMult = TerrainComponent.GetTileMult();
			ImGui::DragFloat2("all layers tile factors", &TileMult[0], 0.1f, 1.0f, 100.0f);
			TerrainComponent.SetTileMult(TileMult);

			float CurrentBrushSize = TERRAIN_SYSTEM.GetBrushSize();
			ImGui::DragFloat("brushSize", &CurrentBrushSize, 0.1f, 0.01f, 100.0f);
			TERRAIN_SYSTEM.SetBrushSize(CurrentBrushSize);

			float CurrentBrushIntensity = TERRAIN_SYSTEM.GetBrushIntensity();
			ImGui::DragFloat("brushIntensity", &CurrentBrushIntensity, 0.0001f, 0.0001f, 10.0f);
			TERRAIN_SYSTEM.SetBrushIntensity(CurrentBrushIntensity);

			SetDefaultStyle(LayerBrushButton);
			if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_LAYER_DRAW)
				SetSelectedStyle(LayerBrushButton);

			LayerBrushButton->Render();
			ShowToolTip("Layer draw brush. Left mouse to paint currently selected layer, hold shift to decrease layer influence.");
			static int SelectedLayer = -1;
			if (SelectedLayer != -1 && TerrainComponent.GetLayerInSlot(SelectedLayer) == nullptr)
				SelectedLayer = -1;

			if (LayerBrushButton->IsClicked())
			{
				if (SelectedLayer != -1)
				{
					if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_LAYER_DRAW)
					{
						TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_NONE);
					}
					else
					{
						TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_LAYER_DRAW);
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
				FETerrainLayer* Layer = TerrainComponent.GetLayerInSlot(i);
				if (Layer == nullptr)
					break;

				ImVec2 PostionBeforeDraw = ImGui::GetCursorPos();

				ImVec2 TextSize = ImGui::CalcTextSize(Layer->GetName().c_str());
				ImGui::SetCursorPos(PostionBeforeDraw + ImVec2(ImGui::GetContentRegionAvail().x / 2.0f - TextSize.x / 2.0f, 16));

				if (TerrainLayerRenameIndex == i)
				{
					if (!bLastFrameTerrainLayerRenameEditWasVisiable)
					{
						ImGui::SetKeyboardFocusHere(0);
						ImGui::SetFocusID(ImGui::GetID("##newNameTerrainLayerEditor"), FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl());
						ImGui::SetItemDefaultFocus();
						bLastFrameTerrainLayerRenameEditWasVisiable = true;
					}

					ImGui::SetNextItemWidth(350.0f);
					ImGui::SetCursorPos(ImVec2(PostionBeforeDraw.x + 64.0f + (ImGui::GetContentRegionAvail().x - 64.0f) / 2.0f - 350.0f / 2.0f, PostionBeforeDraw.y + 12));
					if (ImGui::InputText("##newNameTerrainLayerEditor", TerrainLayerRename, IM_ARRAYSIZE(TerrainLayerRename), ImGuiInputTextFlags_EnterReturnsTrue) ||
						ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::IsItemFocused()/*FE_IMGUI_WINDOW_MANAGER.GetCurrentFocusID() != ImGui::GetID("##newNameTerrainLayerEditor")*/)
					{
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						Layer->SetName(TerrainLayerRename);

						TerrainLayerRenameIndex = -1;
					}
				}
				else
				{
					ImGui::Text(Layer->GetName().c_str());
				}
				ImGui::SetCursorPos(PostionBeforeDraw);

				ImGui::PushID(int(i));
				if (ImGui::Selectable("##item", SelectedLayer == i ? true : false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x - 0, 64)))
				{
					SelectedLayer = int(i);
					TERRAIN_SYSTEM.SetBrushLayerIndex(SelectedLayer);
				}
				TerrainChangeLayerMaterialTargets[i]->StickToItem();
				ImGui::PopID();

				if (ImGui::IsItemHovered())
					HoveredTerrainLayerItem = int(i);

				ImGui::SetCursorPos(PostionBeforeDraw);
				ImColor ImageTint = ImGui::IsItemHovered() ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
				FETexture* PreviewTexture = PREVIEW_MANAGER.GetMaterialPreview(Layer->GetMaterial()->GetObjectID());
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

				if (TerrainComponent.GetLayerInSlot(FE_TERRAIN_MAX_LAYERS - 1) != nullptr)
					ImGui::BeginDisabled();

				if (ImGui::MenuItem("Add layer..."))
				{
					std::vector<std::string> TempMaterialList = RESOURCE_MANAGER.GetMaterialIDList();
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
						MessagePopUp::GetInstance().Show("No suitable material", "There are no materials with compack packing.");
					}
					else
					{
						TerrainToWorkWith = TerrainEntity;
						SELECT_FEOBJECT_POPUP.Show(FE_MATERIAL, CreateNewTerrainLayerWithMaterialCallBack, nullptr, FinalMaterialList);
					}
				}

				if (TerrainComponent.GetLayerInSlot(FE_TERRAIN_MAX_LAYERS - 1) != nullptr)
					ImGui::EndDisabled();

				if (HoveredTerrainLayerItem != -1)
				{
					FETerrainLayer* Layer = TerrainComponent.GetLayerInSlot(HoveredTerrainLayerItem);
					if (Layer != nullptr)
					{
						ImGui::Separator();
						std::string LayerName = Layer->GetName();
						ImGui::Text((std::string("Actions with ") + LayerName).c_str());
						ImGui::Separator();

						if (ImGui::MenuItem("Rename"))
						{
							TerrainLayerRenameIndex = HoveredTerrainLayerItem;

							strcpy_s(TerrainLayerRename, Layer->GetName().size() + 1, Layer->GetName().c_str());
							bLastFrameTerrainLayerRenameEditWasVisiable = false;
						}

						if (ImGui::MenuItem("Fill"))
						{
							TERRAIN_SYSTEM.FillTerrainLayerMask(TerrainEntity, HoveredTerrainLayerItem);
						}

						if (ImGui::MenuItem("Clear"))
						{
							TERRAIN_SYSTEM.ClearTerrainLayerMask(TerrainEntity, HoveredTerrainLayerItem);
						}

						if (ImGui::MenuItem("Delete"))
						{
							TERRAIN_SYSTEM.DeleteTerrainLayerMask(TerrainEntity, HoveredTerrainLayerItem);
						}

						ImGui::Separator();

						if (ImGui::MenuItem("Change material..."))
						{
							std::vector<std::string> TempMaterialList = RESOURCE_MANAGER.GetMaterialIDList();
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
								MessagePopUp::GetInstance().Show("No suitable material", "There are no materials with compack packing.");
							}
							else
							{
								TerrainToWorkWith = TerrainEntity;
								TempLayerIndex = HoveredTerrainLayerItem;
								SELECT_FEOBJECT_POPUP.Show(FE_MATERIAL, ChangeMaterialInTerrainLayerCallBack, TerrainComponent.GetLayerInSlot(HoveredTerrainLayerItem)->GetMaterial(), FinalMaterialList);
							}
						}

						if (ImGui::MenuItem("Export mask..."))
						{
							std::string FilePath = "";
							FILE_SYSTEM.ShowFileSaveDialog(FilePath, TEXTURE_LOAD_FILTER, 1);
							if (!FilePath.empty())
							{
								FilePath += ".png";
								TERRAIN_SYSTEM.SaveTerrainLayerMask(TerrainEntity, FilePath, HoveredTerrainLayerItem);
							}
						}

						if (ImGui::MenuItem("Import mask..."))
						{
							std::string FilePath = "";
							FILE_SYSTEM.ShowFileOpenDialog(FilePath, TEXTURE_LOAD_FILTER, 1);
							if (!FilePath.empty())
							{
								TERRAIN_SYSTEM.LoadTerrainLayerMask(TerrainEntity, FilePath, HoveredTerrainLayerItem);
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

void FEEditorInspectorWindow::AddVirtualUIComponent(FEEntity* Entity)
{
	Entity->AddComponent<FEVirtualUIComponent>();
	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();
	VirtualUIComponent.SetWindowToListen(APPLICATION.GetMainWindow());
}

void FEEditorInspectorWindow::DisplayVirtualUIProperties(FEEntity* VirtualUIEntity) const
{
	FEVirtualUIComponent& VirtualUIComponent = VirtualUIEntity->GetComponent<FEVirtualUIComponent>();

	// TO-DO: Give user ability to change window.
	FEWindow* CurrentWindow = VirtualUIComponent.GetWindowToListen();

	// TO-DO: Give user ability to change canvas mesh.
	FEMesh* CurrentCanvasMesh = VirtualUIComponent.GetCanvasMesh();

	// Internal resolution.
	ImGui::Text("Internal Resolution:");
	glm::vec2 InternalResolution = VirtualUIComponent.GetCanvasResolution();
	ImGui::DragFloat2("##InternalResolution", &InternalResolution[0], 1.0f, 1.0f, 4096.0f);
	VirtualUIComponent.SetCanvasResolution(InternalResolution);

	bool bActive = VirtualUIComponent.IsInputActive();
	ImGui::Checkbox("Input Active", &bActive);
	VirtualUIComponent.SetInputActive(bActive);

	bool bMouseButtonPassThrough = VirtualUIComponent.IsMouseButtonPassThroughActive();
	ImGui::Checkbox("Mouse Button Pass Through", &bMouseButtonPassThrough);
	VirtualUIComponent.SetMouseButtonPassThrough(bMouseButtonPassThrough);

	bool bMouseMovePassThrough = VirtualUIComponent.IsMouseMovePassThroughActive();
	ImGui::Checkbox("Mouse Move Pass Through", &bMouseMovePassThrough);
	VirtualUIComponent.SetMouseMovePassThrough(bMouseMovePassThrough);

	bool bMouseScrollPassThrough = VirtualUIComponent.IsScrollPassThroughActive();
	ImGui::Checkbox("Mouse Scroll Pass Through", &bMouseScrollPassThrough);
	VirtualUIComponent.SetScrollPassThrough(bMouseScrollPassThrough);

	bool bCharPassThrough = VirtualUIComponent.IsCharPassThroughActive();
	ImGui::Checkbox("Char Pass Through", &bCharPassThrough);
	VirtualUIComponent.SetCharPassThrough(bCharPassThrough);

	bool bKeyboardPassThrough = VirtualUIComponent.IsKeyPassThroughActive();
	ImGui::Checkbox("Key Pass Through", &bKeyboardPassThrough);
	VirtualUIComponent.SetKeyPassThrough(bKeyboardPassThrough);

	bool bDropPassThrough = VirtualUIComponent.IsDropPassThroughActive();
	ImGui::Checkbox("Drop Pass Through", &bDropPassThrough);
	VirtualUIComponent.SetDropPassThrough(bDropPassThrough);
}

void FEEditorInspectorWindow::AddGameModelComponent(FEEntity* Entity)
{
	INSPECTOR_WINDOW.EntityToWorkWith = Entity;
	SELECT_FEOBJECT_POPUP.Show(FE_GAMEMODEL, AddNewGameModelComponentCallBack);
}

void FEEditorInspectorWindow::AddNewGameModelComponentCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (!SelectionsResult.empty() && SelectionsResult[0]->GetType() == FE_GAMEMODEL)
	{
		FEGameModel* SelectedGameModel = RESOURCE_MANAGER.GetGameModel(SelectionsResult[0]->GetObjectID());
		if (SelectedGameModel == nullptr)
		{
			LOG.Add("Can't add game model component. Game model is not loaded.", "FE_LOG_LOADING", FE_LOG_ERROR);
			return;
		}

		if (INSPECTOR_WINDOW.EntityToWorkWith == nullptr)
		{
			LOG.Add("Can't add game model component. No entity selected.", "FE_LOG_LOADING", FE_LOG_ERROR);
			return;
		}

		INSPECTOR_WINDOW.EntityToWorkWith->AddComponent<FEGameModelComponent>(SelectedGameModel);

		PROJECT_MANAGER.GetCurrent()->SetModified(true);
	}
}

void FEEditorInspectorWindow::AddTerrainComponent(FEEntity* Entity)
{
	Entity->AddComponent<FETerrainComponent>();
	TERRAIN_SYSTEM.SetHeightMap(RESOURCE_MANAGER.CreateBlankHightMapTexture(1024, 1024), Entity);
}

void FEEditorInspectorWindow::AddInstancedComponent(FEEntity* Entity)
{
	Entity->AddComponent<FEInstancedComponent>();
}

void FEEditorInspectorWindow::AddNativeScriptComponent(FEEntity* Entity)
{
	Entity->AddComponent<FENativeScriptComponent>();
}

// TO-DO: Make it more general with more templated magic.
template<typename T>
void HandleScriptVariable(FENativeScriptComponent& Component, const std::string VariableName)
{
	T Value;
	if (Component.GetVariableValue(VariableName, Value))
	{
		if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
		{
			if (ImGui::InputScalar(VariableName.c_str(), ImGuiDataType_Float, &Value))
				Component.SetVariableValue(VariableName, Value);
		}
		else if constexpr (std::is_same_v<T, int>)
		{
			if (ImGui::InputInt(VariableName.c_str(), &Value))
				Component.SetVariableValue(VariableName, Value);
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			if (ImGui::Checkbox(VariableName.c_str(), &Value))
				Component.SetVariableValue(VariableName, Value);
		}
		else if constexpr (std::is_same_v<T, std::string>)
		{
			char Buffer[1024];
			strcpy_s(Buffer, sizeof(Buffer), Value.c_str());
			if (ImGui::InputText(VariableName.c_str(), Buffer, sizeof(Buffer)))
			{
				Value = Buffer;
				Component.SetVariableValue(VariableName, Value);
			}
		}
		else if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>)
		{
			int VectorCardinality = std::is_same_v<T, glm::vec2> ? 2 : std::is_same_v<T, glm::vec3> ? 3 : 4;
			if (ImGui::InputScalarN(VariableName.c_str(), ImGuiDataType_Float, glm::value_ptr(Value), VectorCardinality))
				Component.SetVariableValue(VariableName, Value);
		}
		else if constexpr (std::is_same_v<T, FEPrefab*>)
		{
			std::string PrefabName = "";
			if (Value != nullptr)
				PrefabName = Value->GetName();
			if (ImGui::BeginCombo(VariableName.c_str(), PrefabName.c_str()))
			{
				std::vector<std::string> PrefabIDList = RESOURCE_MANAGER.GetPrefabIDList();
				for (size_t i = 0; i < PrefabIDList.size(); i++)
				{
					FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabIDList[i]);
					bool bIsSelected = PrefabName == CurrentPrefab->GetName();
					if (ImGui::Selectable(CurrentPrefab->GetName().c_str(), bIsSelected))
					{
						Value = RESOURCE_MANAGER.GetPrefab(PrefabIDList[i]);
						Component.SetVariableValue(VariableName, Value);
					}

					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
	}
}

// TO-DO: Make it more general with more templated magic.
template<typename T>
void HandleScriptArrayVariable(FENativeScriptComponent& Component, const std::string VariableName)
{
	std::vector<T> Value;
	if (Component.GetVariableValue(VariableName, Value))
	{
		for (size_t i = 0; i < Value.size(); i++)
		{
			ImGui::Text((VariableName + "[" + std::to_string(i) + "]").c_str());
			ImGui::SameLine();

			std::string ElementName = "##" + VariableName + "[" + std::to_string(i) + "]";
			if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
			{
				if (ImGui::InputScalar(ElementName.c_str(), ImGuiDataType_Float, &Value[i]))
					Component.SetVariableValue(VariableName, Value);
			}
			else if constexpr (std::is_same_v<T, int>)
			{
				if (ImGui::InputInt(ElementName.c_str(), &Value[i]))
					Component.SetVariableValue(VariableName, Value);
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				// std::vector<bool> is a special case.
				bool bTemporaryValue = Value[i];
				if (ImGui::Checkbox(ElementName.c_str(), &bTemporaryValue))
				{
					Value[i] = bTemporaryValue;
					Component.SetVariableValue(VariableName, Value);
				}
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				char Buffer[1024];
				strcpy_s(Buffer, sizeof(Buffer), Value[i].c_str());
				if (ImGui::InputText(ElementName.c_str(), Buffer, sizeof(Buffer)))
				{
					Value[i] = Buffer;
					Component.SetVariableValue(VariableName, Value);
				}
			}
			else if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>)
			{
				int VectorCardinality = std::is_same_v<T, glm::vec2> ? 2 : std::is_same_v<T, glm::vec3> ? 3 : 4;
				if (ImGui::InputScalarN(ElementName.c_str(), ImGuiDataType_Float, glm::value_ptr(Value[i]), VectorCardinality))
					Component.SetVariableValue(VariableName, Value);
			}
			else if constexpr (std::is_same_v<T, FEPrefab*>)
			{
				std::string PrefabName = "";
				if (Value[i] != nullptr)
					PrefabName = Value[i]->GetName();
				if (ImGui::BeginCombo((VariableName + "[" + std::to_string(i) + "]").c_str(), PrefabName.c_str()))
				{
					std::vector<std::string> PrefabIDList = RESOURCE_MANAGER.GetPrefabIDList();
					for (size_t j = 0; j < PrefabIDList.size(); j++)
					{
						FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabIDList[j]);
						bool bIsSelected = PrefabName == CurrentPrefab->GetName();
						if (ImGui::Selectable(CurrentPrefab->GetName().c_str(), bIsSelected))
						{
							Value[i] = RESOURCE_MANAGER.GetPrefab(PrefabIDList[j]);
							Component.SetVariableValue(VariableName, Value);
						}

						if (bIsSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}
			}
		}

		if (ImGui::Button(("Add##" + VariableName).c_str()))
		{
			Value.push_back(T());
			Component.SetVariableValue(VariableName, Value);
		}
	}
}

void FEEditorInspectorWindow::DisplayNativeScriptProperties(FEEntity* NativeScriptEntity) const
{
	FENativeScriptComponent& NativeScriptComponent = NativeScriptEntity->GetComponent<FENativeScriptComponent>();

	if (!NativeScriptComponent.IsInitialized())
	{
		if (NativeScriptComponent.IsFailedToLoad())
		{
			ImGui::Text("Script failed to load.");
			FENativeScriptFailedToLoadData* FailedToLoadData = NativeScriptComponent.GetFailedToLoadData();
			ImGui::Text(("Module ID: " + FailedToLoadData->GetModuleID()).c_str());
			// And show raw data in multiline text box.
			Json::Value Data = FailedToLoadData->GetRawData();
			std::string RawData = Data.toStyledString();

			static char buffer[10240] = {};

			strncpy_s(buffer, RawData.c_str(), sizeof(buffer) - 1);

			ImGui::InputTextMultiline("##FailedToLoadData", buffer, sizeof(buffer), ImVec2(500, 500), ImGuiInputTextFlags_ReadOnly);

		}
		else
		{
			std::vector<std::string> ModuleList = NATIVE_SCRIPT_SYSTEM.GetActiveModuleIDList();

			for (size_t i = 0; i < ModuleList.size(); i++)
			{
				ImGui::Text(("Module ID: " + ModuleList[i]).c_str());

				ImGui::Text("Script list: ");
				std::vector<std::string> ScriptList = NATIVE_SCRIPT_SYSTEM.GetActiveModuleScriptNameList(ModuleList[i]);
				for (size_t j = 0; j < ScriptList.size(); j++)
				{
					ImGui::Text(ScriptList[j].c_str());
					ImGui::SameLine();
					if (ImGui::Button(("Add##" + ModuleList[i] + "_" + ScriptList[j]).c_str()))
					{
						NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(NativeScriptEntity, ModuleList[i], ScriptList[j]);
					}
				}
			}
		}
	}
	else
	{
		std::string ModuleID = NativeScriptComponent.GetModuleID();
		const FEScriptData* ScriptData = NativeScriptComponent.GetScriptData();

		// Showing general information.
		ImGui::Text(("Module: " + ModuleID).c_str());
		ImGui::Text(("Script name: " + ScriptData->Name).c_str());
		ImGui::Text((std::string("Run in editor: ") + std::string(ScriptData->bRunInEditor ? "Yes" : "No")).c_str());

		// Showing script variables.
		std::unordered_map<std::string, FEScriptVariableInfo> VariablesRegistry = ScriptData->VariablesRegistry;
		auto VariablesIterator = VariablesRegistry.begin();
		while (VariablesIterator != VariablesRegistry.end())
		{
			std::string VariableName = VariablesIterator->first;
			std::string VariableType = VariablesIterator->second.Type;

			// TO-DO: Make it more general with more templated magic.
			if (VariableType.find("vector<") != std::string::npos || VariableType.find("std::vector<") != std::string::npos)
			{
				if (VariableType.find("float>") != std::string::npos) HandleScriptArrayVariable<float>(NativeScriptComponent, VariableName);
				else if (VariableType.find("int>") != std::string::npos) HandleScriptArrayVariable<int>(NativeScriptComponent, VariableName);
				else if (VariableType.find("bool>") != std::string::npos) HandleScriptArrayVariable<bool>(NativeScriptComponent, VariableName);
				else if (VariableType.find("std::string>") != std::string::npos) HandleScriptArrayVariable<std::string>(NativeScriptComponent, VariableName);
				else if (VariableType.find("glm::vec2>") != std::string::npos) HandleScriptArrayVariable<glm::vec2>(NativeScriptComponent, VariableName);
				else if (VariableType.find("glm::vec3>") != std::string::npos) HandleScriptArrayVariable<glm::vec3>(NativeScriptComponent, VariableName);
				else if (VariableType.find("glm::vec4>") != std::string::npos) HandleScriptArrayVariable<glm::vec4>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEShader*>") != std::string::npos) HandleScriptArrayVariable<FEShader*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEMesh*>") != std::string::npos) HandleScriptArrayVariable<FEMesh*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FETexture*>") != std::string::npos) HandleScriptArrayVariable<FETexture*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEMaterial*>") != std::string::npos) HandleScriptArrayVariable<FEMaterial*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEGameModel*>") != std::string::npos) HandleScriptArrayVariable<FEGameModel*>(NativeScriptComponent, VariableName);
				// TO-DO: Think how to make it work with enitity. Right now it is not consistent between editor and game mode scenes.
				//else if (VariableType.find("FEEntity*>") != std::string::npos) HandleScriptArrayVariable<FEEntity*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEFramebuffer*>") != std::string::npos) HandleScriptArrayVariable<FEFramebuffer*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEPostProcess*>") != std::string::npos) HandleScriptArrayVariable<FEPostProcess*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEPrefab*>") != std::string::npos) HandleScriptArrayVariable<FEPrefab*>(NativeScriptComponent, VariableName);
				// TO-DO: Check if it is working.
				else if (VariableType.find("FEScene*>") != std::string::npos) HandleScriptArrayVariable<FEScene*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FEAssetPackage*>") != std::string::npos) HandleScriptArrayVariable<FEAssetPackage*>(NativeScriptComponent, VariableName);
				else if (VariableType.find("FENativeScriptModule*>") != std::string::npos) HandleScriptArrayVariable<FENativeScriptModule*>(NativeScriptComponent, VariableName);
			}
			else
			{
				if (VariableType == "float") HandleScriptVariable<float>(NativeScriptComponent, VariableName);
				else if (VariableType == "int") HandleScriptVariable<int>(NativeScriptComponent, VariableName);
				else if (VariableType == "bool") HandleScriptVariable<bool>(NativeScriptComponent, VariableName);
				else if (VariableType == "std::string") HandleScriptVariable<std::string>(NativeScriptComponent, VariableName);
				else if (VariableType == "glm::vec2") HandleScriptVariable<glm::vec2>(NativeScriptComponent, VariableName);
				else if (VariableType == "glm::vec3") HandleScriptVariable<glm::vec3>(NativeScriptComponent, VariableName);
				else if (VariableType == "glm::vec4") HandleScriptVariable<glm::vec4>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEShader*") HandleScriptVariable<FEShader*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEMesh*") HandleScriptVariable<FEMesh*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FETexture*") HandleScriptVariable<FETexture*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEMaterial*") HandleScriptVariable<FEMaterial*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEGameModel*") HandleScriptVariable<FEGameModel*>(NativeScriptComponent, VariableName);
				// TO-DO: Think how to make it work with enitity. Right now it is not consistent between editor and game mode scenes.
				//else if (VariableType == "FEEntity*") HandleScriptVariable<FEEntity*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEFramebuffer*") HandleScriptVariable<FEFramebuffer*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEPostProcess*") HandleScriptVariable<FEPostProcess*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEPrefab*") HandleScriptVariable<FEPrefab*>(NativeScriptComponent, VariableName);
				// TO-DO: Check if it is working.
				else if (VariableType == "FEScene*") HandleScriptVariable<FEScene*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FEAssetPackage*") HandleScriptVariable<FEAssetPackage*>(NativeScriptComponent, VariableName);
				else if (VariableType == "FENativeScriptModule*") HandleScriptVariable<FENativeScriptModule*>(NativeScriptComponent, VariableName);
			}

			VariablesIterator++;
		}
	}
}