#include "FEEditor.h"

FEEditor* FEEditor::Instance = nullptr;
ImGuiWindow* FEEditor::SceneWindow = nullptr;
FEEntity* FEEditor::EntityToModify = nullptr;

bool SceneWindowDragAndDropCallBack(FEObject* Object, void** UserData)
{
	if (Object->GetType() == FE_PREFAB)
	{
		FEEntity* NewEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefab(Object->GetObjectID()));
		NewEntity->Transform.SetPosition(ENGINE.GetCamera()->GetPosition() + ENGINE.GetCamera()->GetForward() * 10.0f);
		SELECTED.SetSelected(NewEntity);
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

std::string FEEditor::GetObjectNameInClipboard()
{
	return ObjectNameInClipboard;
}

void FEEditor::SetObjectNameInClipboard(const std::string NewValue)
{
	ObjectNameInClipboard = NewValue;
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
		
		EDITOR.bLeftMousePressed = true;
	}
	else if (Button == GLFW_MOUSE_BUTTON_1 && Action == GLFW_RELEASE)
	{
		EDITOR.bLeftMousePressed = false;
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
		if (SELECTED.GetEntity() != nullptr)
			EDITOR.SetObjectNameInClipboard(SELECTED.GetEntity()->GetObjectID());
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && Mods == GLFW_MOD_CONTROL && Key == GLFW_KEY_V && Action == GLFW_RELEASE)
	{
		if (!EDITOR.GetObjectNameInClipboard().empty())
		{
			FEEntity* NewEntity = SCENE.AddEntity(SCENE.GetEntity(EDITOR.GetObjectNameInClipboard())->Prefab, "");
			NewEntity->Transform = SCENE.GetEntity(EDITOR.GetObjectNameInClipboard())->Transform;
			NewEntity->Transform.SetPosition(NewEntity->Transform.GetPosition() * 1.1f);
			SELECTED.SetSelected(NewEntity);
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
		EDITOR.bShiftPressed = false;
	}
	else if (!ImGui::GetIO().WantCaptureKeyboard && (Key == GLFW_KEY_RIGHT_SHIFT || Key == GLFW_KEY_LEFT_SHIFT) && Action == GLFW_PRESS)
	{
		EDITOR.bShiftPressed = true;
	}
}

void FEEditor::ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const
{
	// ********************* POSITION *********************
	glm::vec3 position = Transform->GetPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + Object->GetName()).c_str(), &position[0], 0.1f);
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + Object->GetName()).c_str(), &position[1], 0.1f);
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + Object->GetName()).c_str(), &position[2], 0.1f);
	ShowToolTip("Z position");
	Transform->SetPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = Transform->GetRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + Object->GetName()).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + Object->GetName()).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + Object->GetName()).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
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
	ImGui::DragFloat((std::string("##X scale : ") + Object->GetName()).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + Object->GetName()).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + Object->GetName()).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("Z scale");

	glm::vec3 OldScale = Transform->GetScale();
	Transform->ChangeXScaleBy(scale[0] - OldScale[0]);
	Transform->ChangeYScaleBy(scale[1] - OldScale[1]);
	Transform->ChangeZScaleBy(scale[2] - OldScale[2]);

	// ********************* REAL WORLD COMPARISON SCALE *********************
	if (Object->GetType() == FE_ENTITY || Object->GetType() == FE_ENTITY_INSTANCED)
	{
		FEEntity* entity = SCENE.GetEntity(Object->GetObjectID());

		FEAABB RealAabb = entity->GetAABB();
		const glm::vec3 min = RealAabb.GetMin();
		const glm::vec3 max = RealAabb.GetMax();

		const float XSize = sqrt((max.x - min.x) * (max.x - min.x));
		const float YSize = sqrt((max.y - min.y) * (max.y - min.y));
		const float ZSize = sqrt((max.z - min.z) * (max.z - min.z));

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

void FEEditor::ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform) const
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

void FEEditor::DisplayLightProperties(FELight* Light) const
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

void FEEditor::DisplayLightsProperties() const
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

void FEEditor::ChangePrefabOfEntityCallBack(const std::vector<FEObject*> SelectionsResult)
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

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXyGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXyGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYzGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationYzGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXzGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.TransformationXzGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.ScaleZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(GIZMO_MANAGER.RotateZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.AddResourceToInternalEditorList(PREVIEW_MANAGER.PreviewEntity);

	MouseCursorIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/mouseCursorIcon.png", "mouseCursorIcon");
	RESOURCE_MANAGER.MakeTextureStandard(MouseCursorIcon);
	ArrowToGroundIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/arrowToGroundIcon.png", "arrowToGroundIcon");
	RESOURCE_MANAGER.MakeTextureStandard(ArrowToGroundIcon);

	SCENE_GRAPH_WINDOW.InitializeResources();

	CONTENT_BROWSER_WINDOW.InitializeResources();

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

				if (ImGui::MenuItem("Inspector", nullptr, bInspectorVisible))
				{
					bInspectorVisible = !bInspectorVisible;
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
		DisplayInspector();
		DisplayEffectsWindow();
		DisplayLogWindow();
		if (!GyzmosSettingsWindowObject.IsVisible())
			GyzmosSettingsWindowObject.Show();
		GyzmosSettingsWindowObject.Render();

		const int index = SELECTED.GetIndexOfObjectUnderMouse(EDITOR.GetMouseX(), EDITOR.GetMouseY());
		if (index >= 0)
		{
			if (!GIZMO_MANAGER.WasSelected(index))
			{
				SELECTED.SetSelectedByIndex(index);
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

bool FEEditor::EntityChangePrefabTargetCallBack(FEObject* Object, void** EntityPointer)
{
	FEEntity* entity = SELECTED.GetEntity();
	if (entity == nullptr)
		return false;

	entity->Prefab = (RESOURCE_MANAGER.GetPrefab(Object->GetObjectID()));
	return true;
}

bool FEEditor::TerrainChangeMaterialTargetCallBack(FEObject* Object, void** LayerIndex)
{
	FETerrain* terrain = SELECTED.GetTerrain();
	if (terrain == nullptr)
		return false;

	FEMaterial* MaterialToAssign = RESOURCE_MANAGER.GetMaterial(Object->GetObjectID());
	if (!MaterialToAssign->IsCompackPacking())
		return false;

	const int TempLayerIndex = *(int*)LayerIndex;
	if (TempLayerIndex >= 0 && TempLayerIndex < FE_TERRAIN_MAX_LAYERS)
		terrain->GetLayerInSlot(TempLayerIndex)->SetMaterial(MaterialToAssign);

	return true;
}

void FEEditor::DisplayInspector()
{
	if (!bInspectorVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
	
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

			bool bActive = Entity->IsWireframeMode();
			ImGui::Checkbox("WireframeMode", &bActive);
			Entity->SetWireframeMode(bActive);

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
						SELECTED.SetSelected(InstancedEntity);
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
			// to hide gizmos
			if (SELECTED.GetTerrain() != nullptr)
				SELECTED.SetSelected(SELECTED.GetTerrain());

			CurrentTerrain->SetBrushActive(EDITOR.bLeftMousePressed);

			if (EDITOR.bShiftPressed)
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
			// to show gizmos
			if (SELECTED.GetTerrain() != nullptr)
				SELECTED.SetSelected(SELECTED.GetTerrain());
		}
	}
	else if (SELECTED.GetLight() != nullptr)
	{
		DisplayLightProperties(SELECTED.GetLight());
	}
	else if (SELECTED.GetSelected()->GetType() == FE_CAMERA)
	{
		FEBasicCamera* camera = ENGINE.GetCamera();

		// ********* POSITION *********
		glm::vec3 CameraPosition = camera->GetPosition();

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

		camera->SetPosition(CameraPosition);

		// ********* ROTATION *********
		glm::vec3 CameraRotation = glm::vec3(camera->GetYaw(), camera->GetPitch(), camera->GetRoll());

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

		camera->SetYaw(CameraRotation[0]);
		camera->SetPitch(CameraRotation[1]);
		camera->SetRoll(CameraRotation[2]);

		float CameraSpeed = camera->GetMovementSpeed();
		ImGui::Text("Camera speed in m/s : ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##Camera_speed", &CameraSpeed, 0.01f, 0.01f, 100.0f);
		camera->SetMovementSpeed(CameraSpeed);
	}

	ImGui::PopStyleVar();
	ImGui::End();
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

void FEEditor::DisplayLogWindow() const
{
	if (!bLogWindowVisible)
		return;

	const auto TopicList = LOG.GetTopicList();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Log", nullptr, ImGuiWindowFlags_None);

	static std::string SelectedChannel = "FE_LOG_GENERAL";
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
	ImGui::Text("Channel:");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
	if (ImGui::BeginCombo("##Channel", (SelectedChannel == "" ? "ALL" : SelectedChannel).c_str(), ImGuiWindowFlags_None))
	{
		for (int i = -1; i < int(TopicList.size()); i++)
		{
			ImGui::PushID(i);

			if (i == -1)
			{
				const bool is_selected = (SelectedChannel == "");

				if (ImGui::Selectable("ALL", is_selected))
				{
					SelectedChannel = "";
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			else
			{
				const bool is_selected = (SelectedChannel == TopicList[i]);
				if (ImGui::Selectable(TopicList[i].c_str(), is_selected))
				{
					SelectedChannel = TopicList[i];
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	std::string LogMessages;
	std::vector<LogItem> LogItems;

	if (SelectedChannel == "")
	{
		std::vector<LogItem> TempItems;
		for (int i = 0; i < int(TopicList.size()); i++)
		{
			TempItems = LOG.GetLogItems(TopicList[i]);
			for (size_t j = 0; j < TempItems.size(); j++)
			{
				LogItems.push_back(TempItems[j]);
			}
		}
	}
	else
	{
		LogItems = LOG.GetLogItems(SelectedChannel);
	}

	std::sort(LogItems.begin(), LogItems.end(),
	[](const LogItem& A, const LogItem& B) -> bool
	{
		return A.TimeStamp < B.TimeStamp;
	});

	for (size_t i = 0; i < LogItems.size(); i++)
	{
		LogMessages += LogItems[i].Text;

		if (LogItems[i].Count < 1000)
		{
			LogMessages += " | COUNT: " + std::to_string(LogItems[i].Count);
		}
		else
		{
			LogMessages += " | COUNT: 1000+(Suppressed)";
		}

		LogMessages += " | SEVERITY: " + LOG.SeverityLevelToString(LogItems[i].Severity);

		if (SelectedChannel == "")
		{
			LogMessages += " | CHANNEL: " + LogItems[i].Topic;
		}

		if (i < LogItems.size() - 1)
			LogMessages += "\n";
	}

	static TextEditor LogEditor;
	LogEditor.SetReadOnly(true);
	LogEditor.SetShowWhitespaces(false);
	LogEditor.SetText(LogMessages);
	LogEditor.Render("Log messages");

	ImGui::PopStyleVar();
	ImGui::End();
}

//static FEMaterial* tempMaterial = nullptr;
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

void FEEditor::DisplayTerrainSettings(FETerrain* Terrain)
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
					ImGui::SetCursorPos(ImVec2(PostionBeforeDraw.x + 64.0f + (ImGui::GetContentRegionAvail().x- 64.0f) / 2.0f - 350.0f / 2.0f, PostionBeforeDraw.y + 12));
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