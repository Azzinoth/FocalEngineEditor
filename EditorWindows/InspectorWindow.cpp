#include "InspectorWindow.h"
#include "../FEEditor.h"
#include "../SubSystems/PluginSystem/FEEditorPluginManager.h"

FEEntity* FEEditorInspectorWindow::EntityToModify = nullptr;
FEEntity* FEEditorInspectorWindow::TerrainToWorkWith = nullptr;

FEEditorInspectorWindow::FEEditorInspectorWindow()
{
	std::vector<FEComponentTypeInfo> RegisteredComponentList = COMPONENTS_TOOL.GetComponentInfoList();
	AddComponentHandlers[typeid(FELightComponent)] = &FEEditorInspectorWindow::AddLightComponent;
	AddComponentHandlers[typeid(FECameraComponent)] = &FEEditorInspectorWindow::AddCameraComponent;
	AddComponentHandlers[typeid(FEGameModelComponent)] = &FEEditorInspectorWindow::AddGameModelComponent;
	AddComponentHandlers[typeid(FEPointCloudComponent)] = &FEEditorInspectorWindow::AddPointCloudComponent;
	AddComponentHandlers[typeid(FEInstancedComponent)] = &FEEditorInspectorWindow::AddInstancedComponent;
	AddComponentHandlers[typeid(FETerrainComponent)] = &FEEditorInspectorWindow::AddTerrainComponent;
	AddComponentHandlers[typeid(FEVirtualUIComponent)] = &FEEditorInspectorWindow::AddVirtualUIComponent;
	AddComponentHandlers[typeid(FENativeScriptComponent)] = &FEEditorInspectorWindow::AddNativeScriptComponent;
	AddComponentHandlers[typeid(FEVolumeComponent)] = &FEEditorInspectorWindow::AddVolumeComponent;

	RemoveComponentHandlers[typeid(FECameraComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FECameraComponent>();
	};
	RemoveComponentHandlers[typeid(FELightComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FELightComponent>();
	};
	RemoveComponentHandlers[typeid(FEGameModelComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FEGameModelComponent>();
	};
	RemoveComponentHandlers[typeid(FEPointCloudComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FEPointCloudComponent>();
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
	RemoveComponentHandlers[typeid(FEVolumeComponent)] = [](FEEntity* ParentEntity) -> void {
		ParentEntity->RemoveComponent<FEVolumeComponent>();
	};
}

void FEEditorInspectorWindow::InitializeResources()
{
	EntityChangeGameModelTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_GAMEMODEL, EntityChangeGameModelTargetCallback, nullptr, "Drop to assign game model");
	EntityChangePointCloudTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_POINT_CLOUD, EntityChangePointCloudTargetCallback, nullptr, "Drop to assign point cloud");

	LockIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/lockIcon.png", "lockIcon");
	RESOURCE_MANAGER.SetTag(LockIcon, EDITOR_RESOURCE_TAG);
}

void FEEditorInspectorWindow::Clear()
{
	
}

void FEEditorInspectorWindow::ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const
{
	ShowTransformConfiguration(Object->GetName(), Transform);
}

bool FEEditorInspectorWindow::EntityChangeGameModelTargetCallback(FEObject* Object, void** EntityPointer)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return false;

	FEEntity* Entity = SELECTED.GetSelected(EDITOR.GetFocusedScene());
	if (Entity == nullptr)
		return false;

	if (!Entity->HasComponent<FEGameModelComponent>())
		return false;

	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
	FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Object->GetID());
	if (GameModel == nullptr)
		return false;

	GameModelComponent.SetGameModel(GameModel);

	return true;
}

bool FEEditorInspectorWindow::EntityChangePointCloudTargetCallback(FEObject* Object, void** EntityPointer)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return false;

	FEEntity* Entity = SELECTED.GetSelected(EDITOR.GetFocusedScene());
	if (Entity == nullptr)
		return false;

	if (!Entity->HasComponent<FEPointCloudComponent>())
		return false;

	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();
	FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(Object->GetID());
	if (PointCloud == nullptr)
		return false;

	PointCloudComponent.SetPointCloud(PointCloud);

	return true;
}

bool FEEditorInspectorWindow::TerrainChangeMaterialTargetCallback(FEObject* Object, void** LayerIndex)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return false;

	FEScene* CurrentScene = EDITOR.GetFocusedScene();

	if (SELECTED.GetSelected(CurrentScene) == nullptr || !SELECTED.GetSelected(CurrentScene)->HasComponent<FETerrainComponent>())
		return false;

	FEMaterial* MaterialToAssign = RESOURCE_MANAGER.GetMaterial(Object->GetID());
	if (!MaterialToAssign->IsCompactPacking())
		return false;

	FETerrainComponent& Terrain = SELECTED.GetSelected(CurrentScene)->GetComponent<FETerrainComponent>();
	const int TemporaryLayerIndex = *(int*)LayerIndex;
	if (TemporaryLayerIndex >= 0 && TemporaryLayerIndex < FE_TERRAIN_MAX_LAYERS)
		Terrain.GetLayerInSlot(TemporaryLayerIndex)->SetMaterial(MaterialToAssign);

	return true;
}

// Label of a row with the width that is used by rows of entity and transform.
static void RenderRowLabel(const char* Label)
{
	RenderRowLabel(Label, ImGui::CalcTextSize("Rotation").x + 14.0f);
}

// ID in a small font, it is cut when it does not fit in the rest of the row. Click copies the whole ID.
static void RenderIDText(const std::string& ID)
{
	const float FontSize = 16.0f;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetFontSize() - FontSize) / 2.0f);
	ImGui::PushFont(nullptr, FontSize);

	std::string ShownID = ID;
	const float AvailableWidth = ImGui::GetContentRegionAvail().x;
	if (ImGui::CalcTextSize(ShownID.c_str()).x > AvailableWidth)
	{
		while (ShownID.size() > 1 && ImGui::CalcTextSize((ShownID + "...").c_str()).x > AvailableWidth)
			ShownID.pop_back();

		ShownID += "...";
	}

	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
	ImGui::TextUnformatted(ShownID.c_str());
	ImGui::PopStyleColor();
	ImGui::PopFont();

	if (ImGui::IsItemClicked())
		ImGui::SetClipboardText(ID.c_str());
	ShowToolTip((ID + "\nClick to copy").c_str());
}

// Returns index of the changed axis or -1.
static int RenderTransformRow(const char* Label, glm::vec3& Value, const float Speed, const float Min, const float Max, const float WidthToLeave = 0.0f)
{
	const ImU32 AxisColors[] = { FEEditorStyle::AxisX, FEEditorStyle::AxisY, FEEditorStyle::AxisZ };
	const char* AxisNames[] = { "X", "Y", "Z" };
	const float FieldSpacing = 6.0f;

	RenderRowLabel(Label);

	std::string LabelInToolTip = Label;
	LabelInToolTip[0] = static_cast<char>(std::tolower(LabelInToolTip[0]));

	int ChangedAxis = -1;
	const float FieldWidth = std::max(floorf((ImGui::GetContentRegionAvail().x - WidthToLeave - FieldSpacing * 2.0f) / 3.0f), 40.0f);
	ImGui::PushID(Label);
	for (int i = 0; i < 3; i++)
	{
		ImGui::PushID(i);
		ImGui::SetNextItemWidth(FieldWidth);
		if (ImGui::DragFloat("##Value", &Value[i], Speed, Min, Max))
			ChangedAxis = i;
		ShowToolTip((std::string(AxisNames[i]) + " " + LabelInToolTip).c_str());

		const ImVec2 FieldMin = ImGui::GetItemRectMin();
		const ImVec2 FieldMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddRectFilled(FieldMin, ImVec2(FieldMin.x + 4.0f, FieldMax.y), AxisColors[i], 4.0f, ImDrawFlags_RoundCornersLeft);
		ImGui::PopID();

		if (i < 2)
			ImGui::SameLine(0.0f, FieldSpacing);
	}
	ImGui::PopID();

	return ChangedAxis;
}

// Icon should be white, it is tinted.
static bool RenderIconToggle(const char* ID, FETexture* Icon, const bool bSelected)
{
	const float Size = ImGui::GetFrameHeight();
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + Size, Min.y + Size);
	const bool bPressed = ImGui::InvisibleButton(ID, ImVec2(Size, Size));
	const bool bHovered = ImGui::IsItemHovered();

	ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
	if (bSelected)
		Fill = bHovered ? FEEditorStyle::AccentHovered : FEEditorStyle::Accent;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, Fill, 4.0f);
	if (!bSelected)
		DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f);

	const float IconPadding = 5.0f;
	DrawList->AddImage(Icon->GetTextureID(), ImVec2(Min.x + IconPadding, Min.y + IconPadding), ImVec2(Max.x - IconPadding, Max.y - IconPadding), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), bSelected ? FEEditorStyle::TextOnAccent : FEEditorStyle::TextMuted);
	return bPressed;
}

void FEEditorInspectorWindow::ShowTransformConfiguration(const std::string Name, FETransformComponent* Transform) const
{
	static bool bWorldSpace = false;

	ImGui::PushID(Name.c_str());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));

	ImGui::AlignTextToFramePadding();
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
	ImGui::TextUnformatted("Values in");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	if (RenderSwitchButton("Local", !bWorldSpace, ImDrawFlags_RoundCornersLeft))
		bWorldSpace = false;

	ImGui::SameLine(0.0f, 0.0f);
	if (RenderSwitchButton("World", bWorldSpace, ImDrawFlags_RoundCornersRight))
		bWorldSpace = true;

	const FE_COORDINATE_SPACE_TYPE Space = bWorldSpace ? FE_WORLD_SPACE : FE_LOCAL_SPACE;

	glm::vec3 Position = Transform->GetPosition(Space);
	if (RenderTransformRow("Position", Position, 0.1f, 0.0f, 0.0f) != -1)
		Transform->SetPosition(Position, Space);

	glm::vec3 Rotation = Transform->GetRotation(Space);
	if (RenderTransformRow("Rotation", Rotation, 0.1f, -360.0f, 360.0f) != -1)
		Transform->SetRotation(Rotation, Space);

	const bool bUniformScaling = Transform->IsUniformScalingSet();
	const float LockButtonSpacing = 6.0f;
	glm::vec3 Scale = Transform->GetScale(Space);
	const int ChangedAxis = RenderTransformRow("Scale", Scale, Scale.x * 0.01f, 0.001f, 1000.0f, ImGui::GetFrameHeight() + LockButtonSpacing);
	if (ChangedAxis != -1)
	{
		if (bUniformScaling)
			Scale = glm::vec3(Scale[ChangedAxis]);

		Transform->SetScale(Scale, Space);
	}

	ImGui::SameLine(0.0f, LockButtonSpacing);
	if (RenderIconToggle("##UniformScaling", LockIcon, bUniformScaling))
		Transform->SetUniformScaling(!bUniformScaling);
	ShowToolTip("Uniform scaling");

	ImGui::PopStyleVar();
	ImGui::PopID();
}

// Title of a group of rows that can be collapsed. Note is shown at the right end of the title.
static bool RenderGroupTitle(const std::string Title, const std::string Note = "", const bool bOpenAtFirst = true)
{
	ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Accent);
	ImGui::SetNextItemOpen(bOpenAtFirst, ImGuiCond_FirstUseEver);
	const bool bOpen = ImGui::TreeNodeEx(Title.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	ImGui::PopStyleColor(2);

	if (!Note.empty())
	{
		const float NoteFontSize = 16.0f;
		const ImVec2 TitleMin = ImGui::GetItemRectMin();
		const ImVec2 TitleMax = ImGui::GetItemRectMax();
		ImGui::PushFont(nullptr, NoteFontSize);
		const float NoteWidth = ImGui::CalcTextSize(Note.c_str()).x;
		ImGui::GetWindowDrawList()->AddText(ImVec2(TitleMax.x - NoteWidth - 6.0f, TitleMin.y + (TitleMax.y - TitleMin.y - NoteFontSize) / 2.0f), FEEditorStyle::TextMuted, Note.c_str());
		ImGui::PopFont();
	}

	return bOpen;
}

void FEEditorInspectorWindow::DisplayLightProperties(FEEntity* LightEntity) const
{
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();
	const bool bDirectional = LightComponent.GetType() == FE_DIRECTIONAL_LIGHT;

	const char* TypeName = "Unknown";
	if (LightComponent.GetType() == FE_DIRECTIONAL_LIGHT)
	{
		TypeName = "Directional";
	}
	else if (LightComponent.GetType() == FE_POINT_LIGHT)
	{
		TypeName = "Point";
	}
	else if (LightComponent.GetType() == FE_SPOT_LIGHT)
	{
		TypeName = "Spot";
	}

	const float LabelWidth = ImGui::CalcTextSize(bDirectional ? "Cast shadows" : "Outer angle").x + 14.0f;
	const float FieldSpacing = 6.0f;

	ImGui::PushID(UNIQUE_ID.ToString(LightEntity->GetID()).c_str());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));

	RenderRowLabel("Type", LabelWidth);
	ImGui::TextUnformatted(TypeName);

	RenderRowLabel("Color", LabelWidth);
	glm::vec3 Color = LightComponent.GetColor();
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::ColorEdit3("##Color", &Color.x))
		LightComponent.SetColor(Color);

	RenderRowLabel("Intensity", LabelWidth);
	float Intensity = LightComponent.GetIntensity();
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::SliderFloat("##Intensity", &Intensity, 0.0f, 100.0f))
		LightComponent.SetIntensity(Intensity);

	if (LightComponent.GetType() == FE_POINT_LIGHT)
	{
		RenderRowLabel("Range", LabelWidth);
		float LightRange = LightComponent.GetRange();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::DragFloat("##LightRadius", &LightRange, 0.1f, 0.1f, 100.0f))
			LightComponent.SetRange(LightRange);
	}
	else if (LightComponent.GetType() == FE_SPOT_LIGHT)
	{
		RenderRowLabel("Inner angle", LabelWidth);
		float SpotAngle = LightComponent.GetSpotAngle();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::SliderFloat("##InnerAngle", &SpotAngle, 0.0f, 90.0f, "%.1f deg"))
			LightComponent.SetSpotAngle(SpotAngle);

		RenderRowLabel("Outer angle", LabelWidth);
		float SpotAngleOuter = LightComponent.GetSpotAngleOuter();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::SliderFloat("##OuterAngle", &SpotAngleOuter, 0.0f, 90.0f, "%.1f deg"))
			LightComponent.SetSpotAngleOuter(SpotAngleOuter);

		// Direction comes from rotation of the entity, so it is only shown.
		const glm::vec3 Direction = LIGHT_SYSTEM.GetDirection(LightEntity);
		RenderRowLabel("Direction", LabelWidth);
		ImGui::Text("%.2f, %.2f, %.2f", Direction.x, Direction.y, Direction.z);
		ShowToolTip("Direction follows the rotation of the entity.");
	}

	if (bDirectional && RenderGroupTitle("Shadows###LightShadows"))
	{
		RenderRowLabel("Cast shadows", LabelWidth);
		bool bCastShadows = LightComponent.IsCastShadows();
		if (ImGui::Checkbox("##Cast shadows", &bCastShadows))
			LightComponent.SetCastShadows(bCastShadows);
		ShowToolTip("Will this light cast shadows.");

		if (!LightComponent.IsCastShadows())
			ImGui::BeginDisabled();

		RenderRowLabel("Cascades", LabelWidth);
		const int MaxCascades = 4;
		ImGui::PushID("Cascades");
		for (int i = 1; i <= MaxCascades; i++)
		{
			ImDrawFlags RoundFlags = ImDrawFlags_RoundCornersNone;
			if (i == 1)
			{
				RoundFlags = ImDrawFlags_RoundCornersLeft;
			}
			else if (i == MaxCascades)
			{
				RoundFlags = ImDrawFlags_RoundCornersRight;
			}

			if (RenderSwitchButton(std::to_string(i).c_str(), LightComponent.GetActiveCascades() == i, RoundFlags))
				LightComponent.SetActiveCascades(i);
			ShowToolTip("How much steps of shadow quality will be used.");

			if (i < MaxCascades)
				ImGui::SameLine(0.0f, 0.0f);
		}
		ImGui::PopID();

		RenderRowLabel("Coverage", LabelWidth);
		float FirstCascadeSize = LightComponent.GetShadowCoverage();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::DragFloat("##shadowCoverage", &FirstCascadeSize, 0.1f, 0.1f, 500.0f, "%.1f m"))
			LightComponent.SetShadowCoverage(FirstCascadeSize);
		ShowToolTip("Distance from camera at which shadows would be present.");

		RenderRowLabel("Map depth", LabelWidth);
		const float DepthFieldWidth = floorf((ImGui::GetContentRegionAvail().x - FieldSpacing) / 2.0f);
		float CSMZDepth = LightComponent.GetCSMZDepth();
		ImGui::SetNextItemWidth(DepthFieldWidth);
		if (ImGui::DragFloat("##CSMZDepth", &CSMZDepth, 0.01f, 0.1f, 100.0f, "Z %.2f"))
			LightComponent.SetCSMZDepth(CSMZDepth);
		ShowToolTip("If you have problems with shadow disapearing when camera is at close distance to shadow reciver, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		ImGui::SameLine(0.0f, FieldSpacing);
		float CSMXYDepth = LightComponent.GetCSMXYDepth();
		ImGui::SetNextItemWidth(DepthFieldWidth);
		if (ImGui::DragFloat("##CSMXYDepth", &CSMXYDepth, 0.01f, 0.0f, 100.0f, "XY %.2f"))
			LightComponent.SetCSMXYDepth(CSMXYDepth);
		ShowToolTip("If you have problems with shadow on edges of screen, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		RenderRowLabel("Blur", LabelWidth);
		float ShadowsBlurFactor = LightComponent.GetShadowBlurFactor();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::DragFloat("##Shadows blur factor", &ShadowsBlurFactor, 0.001f, 0.0f, 10.0f))
			LightComponent.SetShadowBlurFactor(ShadowsBlurFactor);

		RenderRowLabel("Bias", LabelWidth);
		ImGui::PushID("Bias");
		if (RenderSwitchButton("Static", LightComponent.IsStaticShadowBias(), ImDrawFlags_RoundCornersLeft))
			LightComponent.SetIsStaticShadowBias(true);

		ImGui::SameLine(0.0f, 0.0f);
		if (RenderSwitchButton("Variable", !LightComponent.IsStaticShadowBias(), ImDrawFlags_RoundCornersRight))
			LightComponent.SetIsStaticShadowBias(false);
		ImGui::PopID();

		ImGui::SameLine(0.0f, FieldSpacing);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (LightComponent.IsStaticShadowBias())
		{
			float ShadowBias = LightComponent.GetShadowBias();
			if (ImGui::DragFloat("##shadowBias", &ShadowBias, 0.0001f, 0.00001f, 0.1f, "%.5f"))
				LightComponent.SetShadowBias(ShadowBias);
			ShowToolTip("Static shadow bias value.");
		}
		else
		{
			float ShadowBiasIntensity = LightComponent.GetShadowBiasVariableIntensity();
			if (ImGui::DragFloat("##shadowBiasIntensity", &ShadowBiasIntensity, 0.01f, 0.01f, 10.0f))
				LightComponent.SetShadowBiasVariableIntensity(ShadowBiasIntensity);
			ShowToolTip("Intensity of variable shadow bias.");
		}

		if (!LightComponent.IsCastShadows())
			ImGui::EndDisabled();
	}

	ImGui::PopStyleVar();
	ImGui::PopID();
}

static bool RenderFloatRow(const char* Label, const char* ID, float& Value, const float Speed, const float Min, const float Max, const float LabelWidth, const char* Format = "%.3f")
{
	RenderRowLabel(Label, LabelWidth);
	ImGui::SetNextItemWidth(-FLT_MIN);
	return ImGui::DragFloat(ID, &Value, Speed, Min, Max, Format);
}

// Row with "Enabled" checkbox of an effect, returns true if the value was changed.
static bool RenderEnabledRow(const char* ID, bool& bEnabled, const float LabelWidth)
{
	RenderRowLabel("Enabled", LabelWidth);
	return ImGui::Checkbox(ID, &bEnabled);
}

static const char* GetFXAAStrengthName(const float FXAASpanMax)
{
	if (FXAASpanMax == 0.0f)
		return "none";

	if (FXAASpanMax > 0.1f && FXAASpanMax < 1.1f)
		return "1x";

	if (FXAASpanMax > 1.1f && FXAASpanMax < 2.1f)
		return "2x";

	if (FXAASpanMax > 2.1f && FXAASpanMax < 4.1f)
		return "4x";

	if (FXAASpanMax > 4.1f && FXAASpanMax < 8.1f)
		return "8x";

	return "custom";
}

static const char* GetSSAOQualityName(FECameraComponent& CameraComponent)
{
	const int SampleCount = CameraComponent.GetSSAOSampleCount();
	if (SampleCount == 4)
		return "Low";

	if (SampleCount == 16 && CameraComponent.GetSSAORadiusSmallDetails())
		return "Medium";

	if (SampleCount == 32 && CameraComponent.GetSSAORadiusSmallDetails())
		return "High";

	return "Custom";
}

void FEEditorInspectorWindow::DisplayCameraProperties(FEEntity* CameraEntity) const
{
	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

	if (EDITOR.GetFocusedScene() == nullptr)
		return;

	const float LabelWidth = ImGui::CalcTextSize("Details weight").x + 14.0f;
	const float FieldSpacing = 6.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));

	// Settings are split into pages, only one page is shown.
	static int CurrentPage = 0;
	const char* PageNames[] = { "Camera", "Image", "Effects" };
	const int PageCount = 3;
	ImGui::PushID("CameraPages");
	for (int i = 0; i < PageCount; i++)
	{
		ImDrawFlags RoundFlags = ImDrawFlags_RoundCornersNone;
		if (i == 0)
		{
			RoundFlags = ImDrawFlags_RoundCornersLeft;
		}
		else if (i == PageCount - 1)
		{
			RoundFlags = ImDrawFlags_RoundCornersRight;
		}

		if (RenderSwitchButton(PageNames[i], CurrentPage == i, RoundFlags))
			CurrentPage = i;

		if (i < PageCount - 1)
			ImGui::SameLine(0.0f, 0.0f);
	}
	ImGui::PopID();

	if (CurrentPage == 0)
	{
		// Because of editor camera system, we need to check if it is main camera in a different way.
		FEUUID MainCameraID = PROJECT_MANAGER.GetCurrent()->GetProperMainCameraIDBySceneID(EDITOR.GetFocusedScene()->GetID());
		bool bIsMainCamera = MainCameraID == CameraEntity->GetID();
		RenderRowLabel("Main camera", LabelWidth);
		if (ImGui::Checkbox("##MainCamera", &bIsMainCamera))
		{
			if (bIsMainCamera)
			{
				PROJECT_MANAGER.GetCurrent()->SetProperMainCameraIDBySceneID(EDITOR.GetFocusedScene()->GetID(), CameraEntity->GetID());
			}
			else
			{
				PROJECT_MANAGER.GetCurrent()->SetProperMainCameraIDBySceneID(EDITOR.GetFocusedScene()->GetID(), FEUUID());
			}
		}

		// Rendering pipeline settings
		RenderRowLabel("Pipeline", LabelWidth);
		static const char* RenderingPipelineOptions[2] = { "Deferred", "Forward simplified" };
		int CurrentIndex = CameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified ? 1 : 0;
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::Combo("##RenderingPipeline", &CurrentIndex, RenderingPipelineOptions, 2))
			CAMERA_SYSTEM.SetCameraRenderingPipeline(CameraEntity, CurrentIndex == 0 ? FERenderingPipeline::Deferred : FERenderingPipeline::Forward_Simplified);

		// Rendering scale
		float RenderScale = CameraComponent.GetRenderScale();
		if (RenderFloatRow("Render scale", "##RenderScale", RenderScale, 0.01f, 0.05f, 2.0f, LabelWidth, "%.2f"))
			CAMERA_SYSTEM.SetCameraRenderScale(CameraEntity, RenderScale);

		float FOV = CameraComponent.GetFOV();
		if (RenderFloatRow("Field of view", "##FOV", FOV, 0.1f, 1.0f, 179.0f, LabelWidth, "%.1f deg"))
			CameraComponent.SetFOV(FOV);

		RenderRowLabel("Clip planes", LabelWidth);
		const float PlaneFieldWidth = floorf((ImGui::GetContentRegionAvail().x - FieldSpacing) / 2.0f);
		float NearPlane = CameraComponent.GetNearPlane();
		ImGui::SetNextItemWidth(PlaneFieldWidth);
		if (ImGui::DragFloat("##NearPlane", &NearPlane, 0.1f, 0.01f, 1000.0f, "Near %.2f"))
			CameraComponent.SetNearPlane(NearPlane);

		ImGui::SameLine(0.0f, FieldSpacing);
		float FarPlane = CameraComponent.GetFarPlane();
		ImGui::SetNextItemWidth(PlaneFieldWidth);
		if (ImGui::DragFloat("##FarPlane", &FarPlane, 0.1f, 0.01f, 10000.0f, "Far %.1f"))
			CameraComponent.SetFarPlane(FarPlane);

		// Camera Preview
		const int PreviewWidth = 452;
		const int PreviewHeight = 256;
		FEScene* CurrentScene = CameraEntity->GetParentScene();
		FETexture* CameraPreviewTexture = nullptr;

		// If it is an editor camera, we do not need to render scene again.
		if (CameraEntity->GetID() == PROJECT_MANAGER.GetCurrent()->GetEditorCameraIDBySceneID(CurrentScene->GetID()))
		{
			CameraPreviewTexture = RENDERER.GetCameraResult(CameraEntity);
		}
		else
		{
			FEEntity* OldMainCameraEntity = CAMERA_SYSTEM.GetMainCamera(EDITOR.GetFocusedScene());
			CAMERA_SYSTEM.SetMainCamera(CameraEntity);

			CameraComponent.TryToSetViewportSize(PreviewWidth, PreviewHeight);
			RENDERER.Render(CurrentScene);
			CameraPreviewTexture = RENDERER.GetCameraResult(CameraEntity);

			CAMERA_SYSTEM.SetMainCamera(OldMainCameraEntity);
		}

		// Preview takes the whole width and keeps its proportions.
		if (CameraPreviewTexture != nullptr)
		{
			const float ShownWidth = ImGui::GetContentRegionAvail().x;
			ImGui::Image(CameraPreviewTexture->GetTextureID(), ImVec2(ShownWidth, floorf(ShownWidth * PreviewHeight / PreviewWidth)), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
	}
	else if (CurrentPage == 1)
	{
		RenderRowLabel("Clear color", LabelWidth);
		glm::vec4 CurrentClearColor = CameraComponent.GetClearColor();
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::ColorEdit4("##ClearColor", &CurrentClearColor.x, ImGuiColorEditFlags_DisplayHex))
			CameraComponent.SetClearColor(CurrentClearColor);

		float Gamma = CameraComponent.GetGamma();
		if (RenderFloatRow("Gamma", "##Gamma", Gamma, 0.1f, 0.1f, 10.0f, LabelWidth, "%.2f"))
			CameraComponent.SetGamma(Gamma);

		float Exposure = CameraComponent.GetExposure();
		if (RenderFloatRow("Exposure", "##Exposure", Exposure, 0.1f, 0.1f, 10.0f, LabelWidth, "%.2f"))
			CameraComponent.SetExposure(Exposure);

		// *********** Bloom ***********
		if (RenderGroupTitle("Bloom###CameraBloom", CameraComponent.IsBloomEnabled() ? "" : "off"))
		{
			bool bBloomEnabled = CameraComponent.IsBloomEnabled();
			if (RenderEnabledRow("##BloomEnabled", bBloomEnabled, LabelWidth))
				CameraComponent.SetBloomEnabled(bBloomEnabled);

			ImGui::BeginDisabled(!bBloomEnabled);
			float Threshold = CameraComponent.GetBloomThreshold();
			if (RenderFloatRow("Threshold", "##Threshold", Threshold, 0.01f, 0.001f, 30.0f, LabelWidth))
				CameraComponent.SetBloomThreshold(Threshold);

			float Size = CameraComponent.GetBloomSize();
			if (RenderFloatRow("Size", "##BloomSize", Size, 0.01f, 0.001f, 100.0f, LabelWidth))
				CameraComponent.SetBloomSize(Size);
			ImGui::EndDisabled();
		}

		// *********** Anti-Aliasing(FXAA) ***********
		if (RenderGroupTitle("Anti-aliasing###CameraAntiAliasing"))
		{
			static const char* Options[5] = { "none", "1x", "2x", "4x", "8x" };
			const int OptionCount = 5;
			const std::string SelectedOption = GetFXAAStrengthName(CameraComponent.GetFXAASpanMax());

			RenderRowLabel("FXAA", LabelWidth);
			ImGui::PushID("FXAAStrength");
			for (int i = 0; i < OptionCount; i++)
			{
				ImDrawFlags RoundFlags = ImDrawFlags_RoundCornersNone;
				if (i == 0)
				{
					RoundFlags = ImDrawFlags_RoundCornersLeft;
				}
				else if (i == OptionCount - 1)
				{
					RoundFlags = ImDrawFlags_RoundCornersRight;
				}

				if (RenderSwitchButton(Options[i], SelectedOption == Options[i], RoundFlags))
					CameraComponent.SetFXAASpanMax(i == 0 ? 0.0f : static_cast<float>(pow(2.0, (i - 1))));

				if (i < OptionCount - 1)
					ImGui::SameLine(0.0f, 0.0f);
			}
			ImGui::PopID();

			// TO-DO: it should be per camera.
			static bool bDebugSettings = false;
			RenderRowLabel("Custom values", LabelWidth);
			ImGui::Checkbox("##FXAACustomValues", &bDebugSettings);
			if (bDebugSettings)
			{
				float FXAASpanMax = CameraComponent.GetFXAASpanMax();
				if (RenderFloatRow("Span max", "##FXAASpanMax", FXAASpanMax, 0.01f, 0.0f, 32.0f, LabelWidth))
					CameraComponent.SetFXAASpanMax(FXAASpanMax);

				float FXAAReduceMin = CameraComponent.GetFXAAReduceMin();
				if (RenderFloatRow("Reduce min", "##FXAAReduceMin", FXAAReduceMin, 0.01f, 0.001f, 100.0f, LabelWidth))
					CameraComponent.SetFXAAReduceMin(FXAAReduceMin);

				float FXAAReduceMul = CameraComponent.GetFXAAReduceMul();
				if (RenderFloatRow("Reduce mul", "##FXAAReduceMul", FXAAReduceMul, 0.01f, 0.001f, 100.0f, LabelWidth))
					CameraComponent.SetFXAAReduceMul(FXAAReduceMul);
			}

			RenderRowLabel("Jitter", LabelWidth);
			bool bTemporalJitterEnabled = CameraComponent.IsTemporalJitterEnabled();
			ImGui::Checkbox("##SubPixelJitter", &bTemporalJitterEnabled);
			CameraComponent.SetTemporalJitterEnabled(bTemporalJitterEnabled);
			ShowToolTip("Sub pixel jitter");

			const std::vector<size_t> SEQUENCE_OPTIONS = { 72, 32, 24, 18, 8 };
			static size_t TemporalJitterSequenceLength = 64;

			RenderRowLabel("Jitter phases", LabelWidth);
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##JitterPhaseSequenceCount", std::to_string(TemporalJitterSequenceLength).c_str()))
			{
				for (const auto& SequenceLength : SEQUENCE_OPTIONS)
				{
					bool bIsSelected = (TemporalJitterSequenceLength == SequenceLength);
					if (ImGui::Selectable(std::to_string(SequenceLength).c_str(), bIsSelected))
						TemporalJitterSequenceLength = SequenceLength;
					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ShowToolTip("Jitter phase sequence count");
			CameraComponent.SetTemporalJitterSequenceLength(TemporalJitterSequenceLength);
		}
	}
	else
	{
		// *********** Depth of Field ***********
		if (RenderGroupTitle("Depth of field###CameraDepthOfField", CameraComponent.IsDOFEnabled() ? "" : "off"))
		{
			bool bDOFEnabled = CameraComponent.IsDOFEnabled();
			if (RenderEnabledRow("##DOFEnabled", bDOFEnabled, LabelWidth))
				CameraComponent.SetDOFEnabled(bDOFEnabled);

			ImGui::BeginDisabled(!bDOFEnabled);
			float DepthThreshold = CameraComponent.GetDOFNearDistance();
			if (RenderFloatRow("Near distance", "##depthThreshold", DepthThreshold, 0.01f, 0.001f, 100.0f, LabelWidth))
				CameraComponent.SetDOFNearDistance(DepthThreshold);
			ShowToolTip("What is closer than this distance is blurred.");

			float DepthThresholdFar = CameraComponent.GetDOFFarDistance();
			if (RenderFloatRow("Far distance", "##depthThresholdFar", DepthThresholdFar, 1.0f, 0.001f, 10000.0f, LabelWidth))
				CameraComponent.SetDOFFarDistance(DepthThresholdFar);
			ShowToolTip("What is further than this distance is blurred.");

			float Strength = CameraComponent.GetDOFStrength();
			if (RenderFloatRow("Strength", "##Strength", Strength, 0.01f, 0.001f, 10.0f, LabelWidth))
				CameraComponent.SetDOFStrength(Strength);

			float IntMult = CameraComponent.GetDOFDistanceDependentStrength();
			if (RenderFloatRow("By distance", "##Distance dependent strength", IntMult, 0.01f, 0.001f, 100.0f, LabelWidth))
				CameraComponent.SetDOFDistanceDependentStrength(IntMult);
			ShowToolTip("Distance dependent strength");
			ImGui::EndDisabled();
		}

		// *********** Chromatic Aberration ***********
		if (RenderGroupTitle("Chromatic aberration###CameraChromaticAberration", CameraComponent.IsChromaticAberrationEnabled() ? "" : "off"))
		{
			bool bChromaticAberrationEnabled = CameraComponent.IsChromaticAberrationEnabled();
			if (RenderEnabledRow("##ChromaticAberrationEnabled", bChromaticAberrationEnabled, LabelWidth))
				CameraComponent.SetChromaticAberrationEnabled(bChromaticAberrationEnabled);

			ImGui::BeginDisabled(!bChromaticAberrationEnabled);
			float Intensity = CameraComponent.GetChromaticAberrationIntensity();
			if (RenderFloatRow("Shift strength", "##intensity", Intensity, 0.01f, 0.0f, 30.0f, LabelWidth))
				CameraComponent.SetChromaticAberrationIntensity(Intensity);
			ImGui::EndDisabled();
		}

		// *********** SSAO ***********
		if (RenderGroupTitle("SSAO###CameraSSAO", CameraComponent.IsSSAOEnabled() ? "" : "off"))
		{
			bool bSSAOEnabled = CameraComponent.IsSSAOEnabled();
			if (RenderEnabledRow("##SSAOEnabled", bSSAOEnabled, LabelWidth))
				CameraComponent.SetSSAOEnabled(bSSAOEnabled);

			ImGui::BeginDisabled(!bSSAOEnabled);
			static const char* SSAO_Options[3] = { "Low", "Medium", "High" };
			static const int SSAO_SampleCounts[3] = { 4, 16, 32 };
			const std::string SSAO_SelectedOption = GetSSAOQualityName(CameraComponent);

			RenderRowLabel("Quality", LabelWidth);
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##SSAO Quality", SSAO_SelectedOption.c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < 3; i++)
				{
					const bool bIsSelected = (SSAO_SelectedOption == SSAO_Options[i]);
					if (ImGui::Selectable(SSAO_Options[i], bIsSelected))
					{
						CameraComponent.SetSSAOResultBlurred(true);
						CameraComponent.SetSSAOBias(0.013f);
						CameraComponent.SetSSAORadius(10.0f);
						CameraComponent.SetSSAORadiusSmallDetails(0.4f);
						CameraComponent.SetSSAOSmallDetailsWeight(0.2f);

						CameraComponent.SetSSAOSampleCount(SSAO_SampleCounts[i]);
						CameraComponent.SetSSAOSmallDetailsEnabled(i != 0);
					}

					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			static bool bSSAO_DebugSettings = false;
			RenderRowLabel("Custom values", LabelWidth);
			ImGui::Checkbox("##SSAOCustomValues", &bSSAO_DebugSettings);
			if (bSSAO_DebugSettings)
			{
				RenderRowLabel("Small details", LabelWidth);
				bool TemporaryBool = CameraComponent.IsSSAOSmallDetailsEnabled();
				if (ImGui::Checkbox("##SSAO small details", &TemporaryBool))
					CameraComponent.SetSSAOSmallDetailsEnabled(TemporaryBool);

				RenderRowLabel("Blurred", LabelWidth);
				TemporaryBool = CameraComponent.IsSSAOResultBlurred();
				if (ImGui::Checkbox("##SSAO blurred", &TemporaryBool))
					CameraComponent.SetSSAOResultBlurred(TemporaryBool);

				RenderRowLabel("Samples", LabelWidth);
				int TemporaryInt = CameraComponent.GetSSAOSampleCount();
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::DragInt("##SSAO sample count", &TemporaryInt))
					CameraComponent.SetSSAOSampleCount(TemporaryInt);

				float TemporaryFloat = CameraComponent.GetSSAOBias();
				if (RenderFloatRow("Bias", "##SSAO bias", TemporaryFloat, 0.1f, 0.0f, 0.0f, LabelWidth))
					CameraComponent.SetSSAOBias(TemporaryFloat);

				TemporaryFloat = CameraComponent.GetSSAORadius();
				if (RenderFloatRow("Radius", "##SSAO radius", TemporaryFloat, 0.1f, 0.0f, 0.0f, LabelWidth))
					CameraComponent.SetSSAORadius(TemporaryFloat);

				TemporaryFloat = CameraComponent.GetSSAORadiusSmallDetails();
				if (RenderFloatRow("Details radius", "##SSAO radius small details", TemporaryFloat, 0.1f, 0.0f, 0.0f, LabelWidth))
					CameraComponent.SetSSAORadiusSmallDetails(TemporaryFloat);

				TemporaryFloat = CameraComponent.GetSSAOSmallDetailsWeight();
				if (RenderFloatRow("Details weight", "##SSAO small details weight", TemporaryFloat, 0.01f, 0.0f, 0.0f, LabelWidth))
					CameraComponent.SetSSAOSmallDetailsWeight(TemporaryFloat);
			}
			ImGui::EndDisabled();
		}

		// *********** Distance Fog ***********
		if (RenderGroupTitle("Distance fog###CameraDistanceFog", CameraComponent.IsDistanceFogEnabled() ? "" : "off"))
		{
			bool bEnabledFog = CameraComponent.IsDistanceFogEnabled();
			if (RenderEnabledRow("##FogEnabled", bEnabledFog, LabelWidth))
				CameraComponent.SetDistanceFogEnabled(bEnabledFog);

			ImGui::BeginDisabled(!bEnabledFog);
			float FogDensity = CameraComponent.GetDistanceFogDensity();
			if (RenderFloatRow("Density", "##fogDensity", FogDensity, 0.0001f, 0.0f, 5.0f, LabelWidth, "%.4f"))
				CameraComponent.SetDistanceFogDensity(FogDensity);

			float FogGradient = CameraComponent.GetDistanceFogGradient();
			if (RenderFloatRow("Gradient", "##fogGradient", FogGradient, 0.001f, 0.0f, 5.0f, LabelWidth))
				CameraComponent.SetDistanceFogGradient(FogGradient);
			ImGui::EndDisabled();
		}
	}

	ImGui::PopStyleVar();
}

void FEEditorInspectorWindow::ChangeGameModelOfEntityCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (EntityToModify == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_GAMEMODEL)
	{
		FEGameModel* SelectedGameModel = RESOURCE_MANAGER.GetGameModel(SelectionsResult[0]->GetID());
		if (SelectedGameModel == nullptr)
			return;

		FEGameModelComponent& GameModelComponent = EntityToModify->GetComponent<FEGameModelComponent>();
		GameModelComponent.SetGameModel(SelectedGameModel);
	}
}

void FEEditorInspectorWindow::ChangePointCloudOfEntityCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (EntityToModify == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_POINT_CLOUD)
	{
		FEPointCloud* SelectedPointCloud = RESOURCE_MANAGER.GetPointCloud(SelectionsResult[0]->GetID());
		if (SelectedPointCloud == nullptr)
			return;

		FEPointCloudComponent& PointCloudComponent = EntityToModify->GetComponent<FEPointCloudComponent>();
		PointCloudComponent.SetPointCloud(SelectedPointCloud);
	}
}

void FEEditorInspectorWindow::AddLightComponent(FEEntity* Entity)
{
	Entity->AddComponent<FELightComponent>(FE_POINT_LIGHT);
}

void FEEditorInspectorWindow::AddCameraComponent(FEEntity* Entity)
{
	Entity->AddComponent<FECameraComponent>();
}

void FEEditorInspectorWindow::RenderAddComponentMenu(FEEntity* Entity)
{
	static std::string Filter;

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

	const ImVec2 ButtonSize = ImVec2(ImGui::GetContentRegionAvail().x, 36.0f);
	const ImVec2 ButtonMin = ImGui::GetCursorScreenPos();
	const ImVec2 ButtonMax = ImVec2(ButtonMin.x + ButtonSize.x, ButtonMin.y + ButtonSize.y);
	const bool bPressed = ImGui::InvisibleButton("##AddComponentButton", ButtonSize);
	const bool bHovered = ImGui::IsItemHovered();
	const bool bMenuOpen = ImGui::IsPopupOpen("AddComponentMenu");

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(ButtonMin, ButtonMax, (bHovered || bMenuOpen) ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button, 4.0f);
	DrawList->AddRect(ButtonMin, ButtonMax, bMenuOpen ? FEEditorStyle::Accent : FEEditorStyle::ButtonBorder, 4.0f);

	const char* ButtonText = "Add Component";
	const float PlusSize = 12.0f;
	const float PlusToTextSpacing = 10.0f;
	const float ContentWidth = PlusSize + PlusToTextSpacing + ImGui::CalcTextSize(ButtonText).x;
	const ImVec2 PlusCenter = ImVec2(floorf(ButtonMin.x + (ButtonSize.x - ContentWidth) / 2.0f + PlusSize / 2.0f), floorf(ButtonMin.y + ButtonSize.y / 2.0f));
	DrawList->AddRectFilled(ImVec2(PlusCenter.x - PlusSize / 2.0f, PlusCenter.y - 1.0f), ImVec2(PlusCenter.x + PlusSize / 2.0f, PlusCenter.y + 1.0f), FEEditorStyle::Accent);
	DrawList->AddRectFilled(ImVec2(PlusCenter.x - 1.0f, PlusCenter.y - PlusSize / 2.0f), ImVec2(PlusCenter.x + 1.0f, PlusCenter.y + PlusSize / 2.0f), FEEditorStyle::Accent);
	DrawList->AddText(ImVec2(PlusCenter.x + PlusSize / 2.0f + PlusToTextSpacing, ButtonMin.y + (ButtonSize.y - ImGui::GetFontSize()) / 2.0f), FEEditorStyle::Text, ButtonText);

	if (bPressed)
	{
		Filter = "";
		ImGui::OpenPopup("AddComponentMenu");
	}

	// Menu is under the button, or above it when there is not enough space below.
	const float MenuMaxHeight = 460.0f;
	const ImGuiViewport* Viewport = ImGui::GetMainViewport();
	const float SpaceBelow = Viewport->WorkPos.y + Viewport->WorkSize.y - ButtonMax.y;
	const float SpaceAbove = ButtonMin.y - Viewport->WorkPos.y;
	if (SpaceBelow < MenuMaxHeight && SpaceAbove > SpaceBelow)
	{
		ImGui::SetNextWindowPos(ImVec2(ButtonMin.x, ButtonMin.y - 4.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
	}
	else
	{
		ImGui::SetNextWindowPos(ImVec2(ButtonMin.x, ButtonMax.y + 4.0f));
	}

	std::string ComponentToAdd;
	ImGui::SetNextWindowSizeConstraints(ImVec2(ButtonSize.x, 0.0f), ImVec2(ButtonSize.x, MenuMaxHeight));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
	if (ImGui::BeginPopup("AddComponentMenu"))
	{
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##AddComponentFilter", "Filter components...", &Filter);
		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		struct ComponentRow
		{
			std::string Name;
			std::string Label;
			FETexture* Icon = nullptr;
			ImU32 IconColor = FEEditorStyle::Text;
			bool bCanBeAdded = false;
			std::string Note;
			std::string Reason;
		};

		// Only components that the inspector knows how to add. Ones that can not be added now are listed in grey.
		std::vector<ComponentRow> Rows;
		const std::vector<FEComponentTypeInfo> EntityComponents = Entity->GetComponentsInfoList();
		std::vector<FEComponentTypeInfo> RegisteredComponents = COMPONENTS_TOOL.GetComponentInfoList();
		for (size_t i = 0; i < RegisteredComponents.size(); i++)
		{
			if (AddComponentHandlers.find(std::type_index(*RegisteredComponents[i].Type)) == AddComponentHandlers.end())
				continue;

			ComponentRow Row;
			Row.Name = RegisteredComponents[i].Name;
			Row.Label = Row.Name == "Volume" ? "Volumetric" : Row.Name;
			Row.Icon = SCENE_GRAPH_WINDOW.GetComponentIcon(*RegisteredComponents[i].Type);
			Row.bCanBeAdded = RegisteredComponents[i].CanBeAddedToEntity(Entity, &Row.Reason);
			// Icons are white and take the color of their component, dimmed when it can not be added.
			Row.IconColor = FEEditorStyle::GetComponentColor(*RegisteredComponents[i].Type);
			if (!Row.bCanBeAdded)
			{
				Row.IconColor = FEEditorStyle::Mix(FEEditorStyle::Panel, Row.IconColor, 0.45f);
				Row.Note = "not available";
				for (size_t j = 0; j < EntityComponents.size(); j++)
				{
					if (EntityComponents[j].Name == Row.Name)
						Row.Note = "already added";
				}
			}

			Rows.push_back(Row);
		}

		std::sort(Rows.begin(), Rows.end(), [](const ComponentRow& First, const ComponentRow& Second) {
			return First.Label < Second.Label;
		});

		const float RowHeight = 34.0f;
		const float IconSize = 26.0f;
		const float NoteFontSize = 16.0f;
		const std::string LowerCaseFilter = ToLowerCase(Filter);
		bool bAnyRowShown = false;
		ImDrawList* MenuDrawList = ImGui::GetWindowDrawList();
		for (size_t i = 0; i < Rows.size(); i++)
		{
			if (!LowerCaseFilter.empty() && ToLowerCase(Rows[i].Label).find(LowerCaseFilter) == std::string::npos)
				continue;

			bAnyRowShown = true;
			const ImVec2 RowMin = ImGui::GetCursorScreenPos();
			const float RowWidth = ImGui::GetContentRegionAvail().x;
			if (Rows[i].bCanBeAdded)
			{
				if (ImGui::Selectable(("##" + Rows[i].Name).c_str(), false, ImGuiSelectableFlags_None, ImVec2(0.0f, RowHeight)))
					ComponentToAdd = Rows[i].Name;
			}
			else
			{
				ImGui::Dummy(ImVec2(RowWidth, RowHeight));
				if (!Rows[i].Reason.empty())
					ShowToolTip(Rows[i].Reason.c_str());
			}

			const ImU32 ContentColor = Rows[i].bCanBeAdded ? FEEditorStyle::Text : FEEditorStyle::TextDisabled;
			const float IconY = RowMin.y + (RowHeight - IconSize) / 2.0f;
			if (Rows[i].Icon != nullptr)
				MenuDrawList->AddImage(Rows[i].Icon->GetTextureID(), ImVec2(RowMin.x + 6.0f, IconY), ImVec2(RowMin.x + 6.0f + IconSize, IconY + IconSize), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), Rows[i].IconColor);

			MenuDrawList->AddText(ImVec2(RowMin.x + 6.0f + IconSize + 10.0f, RowMin.y + (RowHeight - ImGui::GetFontSize()) / 2.0f), ContentColor, Rows[i].Label.c_str());
			if (!Rows[i].bCanBeAdded)
			{
				ImGui::PushFont(nullptr, NoteFontSize);
				const float NoteWidth = ImGui::CalcTextSize(Rows[i].Note.c_str()).x;
				MenuDrawList->AddText(ImVec2(RowMin.x + RowWidth - NoteWidth - 6.0f, RowMin.y + (RowHeight - NoteFontSize) / 2.0f), FEEditorStyle::TextDisabled, Rows[i].Note.c_str());
				ImGui::PopFont();
			}
		}

		if (!bAnyRowShown)
			ImGui::TextDisabled("No component with this name");

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	if (!ComponentToAdd.empty())
		AddComponent(Entity, ComponentToAdd);
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

// Header of a component, with a line in the color of the component at its left edge.
static bool RenderComponentHeader(const char* Label, const std::type_info& ComponentType)
{
	const bool bOpen = ImGui::CollapsingHeader(Label, ImGuiTreeNodeFlags_DefaultOpen);
	const ImU32 Color = FEEditorStyle::GetComponentColor(ComponentType);
	// A component without a color of its own has no line.
	if (Color != FEEditorStyle::Text)
	{
		const ImVec2 Min = ImGui::GetItemRectMin();
		ImGui::GetWindowDrawList()->AddRectFilled(Min, ImVec2(Min.x + 3.0f, ImGui::GetItemRectMax().y), Color, 3.0f, ImDrawFlags_RoundCornersLeft);
	}

	return bOpen;
}

bool FEEditorInspectorWindow::RenderComponentDeleteButton(FEEntity* Entity, FEComponentTypeInfo* ComponentInfo) const
{
	if (!ComponentInfo->CanBeRemovedFromEntity(Entity))
		return false;

	float HeaderWidth = ImGui::GetContentRegionAvail().x;
	ImVec2 PreviousCursorPosition = ImGui::GetCursorPos();

	// Button is inside the header, with the same space above, under and at the right of it.
	const float HeaderPadding = 3.0f;
	float ButtonSize = ImGui::GetFrameHeight() - HeaderPadding * 2.0f;
	ImGui::SetCursorPos(ImVec2(PreviousCursorPosition.x + HeaderWidth - ButtonSize - HeaderPadding, PreviousCursorPosition.y + HeaderPadding));
	// Button is in a child window, so header of the component that is rendered after it does not cover it.
	ImGui::BeginChild(std::string("##DeleteButton" + ComponentInfo->Name).c_str(), ImVec2(ButtonSize, ButtonSize), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + ButtonSize, Min.y + ButtonSize);
	const ImVec2 Center = ImVec2(floorf(Min.x + ButtonSize / 2.0f), floorf(Min.y + ButtonSize / 2.0f));
	const bool bPressed = ImGui::InvisibleButton("##Button", ImVec2(ButtonSize, ButtonSize));
	const bool bHovered = ImGui::IsItemHovered();
	const bool bHeld = ImGui::IsItemActive();
	ShowToolTip(("Remove " + ComponentInfo->Name + " component").c_str());

	// Trash can: handle, lid and body.
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	if (bHovered || bHeld)
		DrawList->AddRectFilled(Min, Max, bHeld ? FEEditorStyle::DangerButtonHovered : FEEditorStyle::DangerButton, 4.0f);

	const ImU32 IconColor = bHovered || bHeld ? IM_COL32_WHITE : FEEditorStyle::TextMuted;
	DrawList->AddRectFilled(ImVec2(Center.x - 2.0f, Center.y - 8.0f), ImVec2(Center.x + 2.0f, Center.y - 6.0f), IconColor);
	DrawList->AddRectFilled(ImVec2(Center.x - 6.0f, Center.y - 6.0f), ImVec2(Center.x + 6.0f, Center.y - 4.0f), IconColor);
	DrawList->AddQuadFilled(ImVec2(Center.x - 5.0f, Center.y - 3.0f), ImVec2(Center.x + 5.0f, Center.y - 3.0f), ImVec2(Center.x + 4.0f, Center.y + 7.0f), ImVec2(Center.x - 4.0f, Center.y + 7.0f), IconColor);
	ImGui::EndChild();
	if (bPressed)
	{
		size_t ComponentHashCode = ComponentInfo->Type->hash_code();

		auto ComponentIterator = RemoveComponentHandlers.begin();
		while (ComponentIterator != RemoveComponentHandlers.end())
		{
			if (ComponentIterator->first.hash_code() == ComponentHashCode)
			{
				ComponentIterator->second(Entity);

				ImGui::SetCursorPos(PreviousCursorPosition);
				return true;
			}

			ComponentIterator++;
		}
	}

	ImGui::SetCursorPos(PreviousCursorPosition);

	return false;
}

void FEEditorInspectorWindow::DisplayVolumeProperties(FEEntity* VolumeEntity)
{
	FEVolumeComponent& VolumeComponent = VolumeEntity->GetComponent<FEVolumeComponent>();
	FENewMaterial* VolumeMaterial = VolumeComponent.GetMaterial();
	if (VolumeMaterial == nullptr)
	{
		if (ImGui::Button("Create new volume material"))
		{
			FENewMaterial* DefaultStartingMaterial = RESOURCE_MANAGER.CreateNewMaterial();
			DefaultStartingMaterial->SetMaterialType(FEMaterialType::Volumetric);
			DefaultStartingMaterial->SetBlendMode(FEMaterialBlendMode::Additive);
			DefaultStartingMaterial->SetShader(VOLUME_SYSTEM.GetVolumetricShaders()[0]);
			VolumeComponent.SetMaterial(DefaultStartingMaterial);
		}

		std::vector<FEUUID> ListOfVolumetricMaterials = RESOURCE_MANAGER.GetNewMaterialIDList();
		for (size_t i = 0; i < ListOfVolumetricMaterials.size(); i++)
		{
			FENewMaterial* CurrentMaterial = RESOURCE_MANAGER.GetNewMaterial(ListOfVolumetricMaterials[i]);
			if (CurrentMaterial == nullptr)
			{
				ListOfVolumetricMaterials.erase(ListOfVolumetricMaterials.begin() + i);
				i--;
				continue;
			}

			if (CurrentMaterial->GetMaterialType() != FEMaterialType::Volumetric)
			{
				ListOfVolumetricMaterials.erase(ListOfVolumetricMaterials.begin() + i);
				i--;
				continue;
			}
		}

		// FE_TO_DO: Add combo box to select a volumetric material.
		return;
	}

	// Only scalar/vector uniforms are editable here; samplers and matrices are skipped by type.
	std::vector<std::string> ParameterNames;
	const std::vector<std::string> UniformOverrideNames = VolumeMaterial->GetUniformOverrideNameList();
	for (size_t i = 0; i < UniformOverrideNames.size(); i++)
	{
		FEShaderUniformValue* UniformValue = VolumeMaterial->GetUniformOverride(UniformOverrideNames[i]);
		if (UniformValue == nullptr)
			continue;

		if (VolumeMaterial->GetShader()->IsUniformProvidedByEngine(UniformOverrideNames[i]))
			continue;

		const bool bIsEditableType = UniformValue->IsType<float>() || UniformValue->IsType<int>() ||
									 UniformValue->IsType<bool>() || UniformValue->IsType<glm::vec2>() ||
									 UniformValue->IsType<glm::vec3>() || UniformValue->IsType<glm::vec4>();
		if (bIsEditableType)
			ParameterNames.push_back(UniformOverrideNames[i]);
	}
	// List of uniforms is not ordered, names are sorted to keep the same order of rows.
	std::sort(ParameterNames.begin(), ParameterNames.end());

	// All textures except LUT.
	std::vector<std::pair<std::string, FETexture*>> VolumeTextures;
	const std::vector<std::pair<std::string, FETexture*>> TextureOverrides = VolumeMaterial->GetAllTextureOverridePair();
	for (size_t i = 0; i < TextureOverrides.size(); i++)
	{
		if (TextureOverrides[i].first != "TransferFunctionTexture")
			VolumeTextures.push_back(TextureOverrides[i]);
	}

	float LabelWidth = ImGui::CalcTextSize("Stop color").x;
	for (size_t i = 0; i < ParameterNames.size(); i++)
		LabelWidth = std::max(LabelWidth, ImGui::CalcTextSize(ParameterNames[i].c_str()).x);

	for (size_t i = 0; i < VolumeTextures.size(); i++)
		LabelWidth = std::max(LabelWidth, ImGui::CalcTextSize(VolumeTextures[i].first.c_str()).x);

	LabelWidth = std::min(LabelWidth + 14.0f, floorf(ImGui::GetContentRegionAvail().x / 2.0f));

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));

	// Shader selector, populated from the volume system's shader list.
	RenderRowLabel("Shader", LabelWidth);
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::BeginCombo("##VolumeShader", VolumeMaterial->GetShader()->GetName().c_str(), ImGuiWindowFlags_None))
	{
		std::vector<FEShader*> VolumetricShaders = VOLUME_SYSTEM.GetVolumetricShaders();
		for (size_t i = 0; i < VolumetricShaders.size(); i++)
		{
			if (VolumetricShaders[i] == nullptr)
				continue;

			bool bSelected = (VolumetricShaders[i] == VolumeMaterial->GetShader());
			ImGui::PushID(UNIQUE_ID.ToString(VolumetricShaders[i]->GetID()).c_str());
			if (ImGui::Selectable(VolumetricShaders[i]->GetName().c_str(), bSelected))
				VolumeMaterial->SetShader(VolumetricShaders[i]);

			if (bSelected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	if (!ParameterNames.empty() && RenderGroupTitle("Shader parameters (" + std::to_string(ParameterNames.size()) + ")###VolumeParameters"))
	{
		for (size_t i = 0; i < ParameterNames.size(); i++)
		{
			const std::string& UniformName = ParameterNames[i];
			FEShaderUniformValue* UniformValue = VolumeMaterial->GetUniformOverride(UniformName);
			const std::string WidgetID = "##VolumeUniform_" + UniformName;

			RenderRowLabel(UniformName.c_str(), LabelWidth);
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (UniformValue->IsType<float>())
			{
				float Data = UniformValue->GetValue<float>();
				if (ImGui::DragFloat(WidgetID.c_str(), &Data, 0.01f))
					VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
			}
			else if (UniformValue->IsType<int>())
			{
				int Data = UniformValue->GetValue<int>();
				if (ImGui::DragInt(WidgetID.c_str(), &Data, 1.0f, 1, 4096))
					VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
			}
			else if (UniformValue->IsType<bool>())
			{
				bool Data = UniformValue->GetValue<bool>();
				if (ImGui::Checkbox(WidgetID.c_str(), &Data))
					VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
			}
			else if (UniformValue->IsType<glm::vec2>())
			{
				glm::vec2 Data = UniformValue->GetValue<glm::vec2>();
				if (ImGui::DragFloat2(WidgetID.c_str(), &Data.x, 0.01f))
					VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
			}
			else if (UniformValue->IsType<glm::vec3>())
			{
				glm::vec3 Data = UniformValue->GetValue<glm::vec3>();
				if (ImGui::DragFloat3(WidgetID.c_str(), &Data.x, 0.01f))
					VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
			}
			else if (UniformValue->IsType<glm::vec4>())
			{
				glm::vec4 Data = UniformValue->GetValue<glm::vec4>();
				if (ImGui::DragFloat4(WidgetID.c_str(), &Data.x, 0.01f))
					VolumeMaterial->UpdateUniformOverrideData(UniformName, Data);
			}
		}
	}

	if (!VolumeTextures.empty() && RenderGroupTitle("Volume textures (" + std::to_string(VolumeTextures.size()) + ")###VolumeTextures"))
	{
		for (size_t i = 0; i < VolumeTextures.size(); i++)
		{
			FETexture* CurrentTexture = VolumeTextures[i].second;
			std::string CurrentTextureCaption = CurrentTexture != nullptr ? CurrentTexture->GetName() : "None";

			RenderRowLabel(VolumeTextures[i].first.c_str(), LabelWidth);
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo(("##VolumeTexture" + VolumeTextures[i].first).c_str(), CurrentTextureCaption.c_str(), ImGuiWindowFlags_None))
			{
				std::vector<FEUUID> TextureIDList = RESOURCE_MANAGER.GetTextureIDList();
				for (size_t j = 0; j < TextureIDList.size(); j++)
				{
					FETexture* Texture = RESOURCE_MANAGER.GetTexture(TextureIDList[j]);
					if (Texture == nullptr || Texture->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
						continue;

					bool bSelected = Texture == CurrentTexture;
					ImGui::PushID(UNIQUE_ID.ToString(Texture->GetID()).c_str());
					if (ImGui::Selectable(Texture->GetName().c_str(), bSelected))
						VolumeMaterial->SetTextureOverride(VolumeTextures[i].first, Texture->GetID());

					if (bSelected)
						ImGui::SetItemDefaultFocus();
					ImGui::PopID();
				}
				ImGui::EndCombo();
			}
		}
	}

	// Transfer function editor, only for shaders that sample the LUT (TransferFunctionTexture).
	if (VOLUME_SYSTEM.DoesVolumeComponentHaveTransferFunction(VolumeComponent) && RenderGroupTitle("Transfer function###VolumeTransferFunction"))
	{
		// Show volume non-normalized value range.
		FETexture* VolumeTexture = VolumeMaterial->GetTextureOverride("VolumeTexture");
		const float DataValueLow = VolumeTexture != nullptr ? VolumeTexture->GetMinValue().x : 0.0f;
		const float DataValueHigh = VolumeTexture != nullptr ? VolumeTexture->GetMaxValue().x : 1.0f;
		TransferFunctionWidget.Render(VolumeEntity, DataValueLow, DataValueHigh, LabelWidth);
	}

	ImGui::PopStyleVar();
}

// Card of a resource that is assigned to a component: preview, name, lines of information and actions.
void FEEditorInspectorWindow::RenderResourceCard(const std::string ID, FETexture* PreviewTexture, const std::string Name, const std::vector<std::string>& InformationLines, DragAndDropTarget* DropTarget, const bool bHasResource, bool& bChangeRequested, bool& bShowInFolderRequested, const std::string ActionName)
{
	ImGui::PushID(ID.c_str());

	const float Padding = 10.0f;
	const float PreviewSize = 96.0f;
	const ImVec2 CardMin = ImGui::GetCursorScreenPos();
	const ImVec2 CardMax = ImVec2(CardMin.x + ImGui::GetContentRegionAvail().x, CardMin.y + PreviewSize + Padding * 2.0f);
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(CardMin, CardMax, FEEditorStyle::Card, 4.0f);
	DrawList->AddRect(CardMin, CardMax, FEEditorStyle::CardBorder, 4.0f);

	ImGui::SetCursorScreenPos(ImVec2(CardMin.x + Padding, CardMin.y + Padding));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	if (ImGui::ImageButton("PreviewButton", PreviewTexture->GetTextureID(), ImVec2(PreviewSize, PreviewSize), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)))
		bChangeRequested = true;
	ImGui::PopStyleVar(2);
	if (DropTarget != nullptr)
		DropTarget->StickToItem();

	if (ImGui::IsItemHovered())
	{
		DrawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), FEEditorStyle::Accent, 0.0f, 2.0f);
		if (bHasResource && ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("##Inspector_context_menu");
	}

	const float TextX = CardMin.x + Padding + PreviewSize + 14.0f;
	const float NameHeight = ImGui::GetFontSize();
	const float SmallFontSize = 16.0f;
	DrawList->PushClipRect(ImVec2(TextX, CardMin.y), ImVec2(CardMax.x - Padding, CardMax.y), true);
	DrawList->AddText(ImVec2(TextX, CardMin.y + Padding - 2.0f), bHasResource ? FEEditorStyle::Text : FEEditorStyle::TextMuted, Name.c_str());

	ImGui::PushFont(nullptr, SmallFontSize);
	float LineY = CardMin.y + Padding + NameHeight + 8.0f;
	for (size_t i = 0; i < InformationLines.size(); i++)
	{
		DrawList->AddText(ImVec2(TextX, LineY), FEEditorStyle::TextMuted, InformationLines[i].c_str());
		LineY += SmallFontSize + 4.0f;
	}
	DrawList->PopClipRect();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
	ImGui::SetCursorScreenPos(ImVec2(TextX, CardMax.y - Padding - (SmallFontSize + 8.0f)));
	const std::string DefaultActionName = bHasResource ? "Change..." : "Select...";
	if (ImGui::Button(ActionName.empty() ? DefaultActionName.c_str() : ActionName.c_str()))
		bChangeRequested = true;

	if (bHasResource)
	{
		ImGui::SameLine(0.0f, 8.0f);
		if (ImGui::Button("Show in folder"))
			bShowInFolderRequested = true;
	}
	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImGui::SetCursorScreenPos(ImVec2(CardMin.x, CardMax.y));
	ImGui::Dummy(ImVec2(0.0f, 0.0f));

	bContextMenuOpened = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##Inspector_context_menu"))
	{
		bContextMenuOpened = true;

		if (ImGui::MenuItem("Show in folder"))
			bShowInFolderRequested = true;

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	ImGui::PopID();
}

void FEEditorInspectorWindow::DisplayGameModelProperties(FEEntity* GameModelEntity)
{
	FEGameModelComponent& GameModelComponent = GameModelEntity->GetComponent<FEGameModelComponent>();
	FEGameModel* GameModel = GameModelComponent.GetGameModel();

	std::vector<std::string> InformationLines;
	// Mesh is rendered as a list of triangles, with three indices or vertices for each one.
	if (GameModel->GetMesh() != nullptr)
		InformationLines.push_back("Mesh (" + GetNumberWithSeparators(GameModel->GetMesh()->GetVertexCount() / 3) + " triangles): " + GameModel->GetMesh()->GetName());

	if (GameModel->GetMaterial() != nullptr)
		InformationLines.push_back("Material: " + GameModel->GetMaterial()->GetName());

	bool bChangeGameModel = false;
	bool bShowInFolder = false;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));
	RenderResourceCard("GameModelCard", PREVIEW_MANAGER.GetGameModelPreview(GameModel->GetID()), GameModel->GetName(), InformationLines, EntityChangeGameModelTarget, true, bChangeGameModel, bShowInFolder);

	RenderRowLabel("Wireframe", ImGui::CalcTextSize("Wireframe").x + 14.0f);
	bool bWireframe = GameModelComponent.IsWireframeMode();
	if (ImGui::Checkbox("##WireframeMode", &bWireframe))
		GameModelComponent.SetWireframeMode(bWireframe);

	ImGui::PopStyleVar();

	if (bChangeGameModel)
	{
		EntityToModify = GameModelEntity;
		SELECT_FEOBJECT_POPUP.Show(FE_GAMEMODEL, ChangeGameModelOfEntityCallback, GameModel);
	}

	if (bShowInFolder)
		CONTENT_BROWSER_WINDOW.OpenItemParentFolder(GameModel);
}

void FEEditorInspectorWindow::DisplayPointCloudProperties(FEEntity* PointCloudEntity)
{
	FEPointCloudComponent& PointCloudComponent = PointCloudEntity->GetComponent<FEPointCloudComponent>();
	FEPointCloud* PointCloud = PointCloudComponent.GetPointCloud();

	std::string Name = "No point cloud";
	FETexture* PreviewTexture = RESOURCE_MANAGER.NoTexture;
	std::vector<std::string> InformationLines;
	if (PointCloud != nullptr)
	{
		Name = PointCloud->GetName();
		PreviewTexture = PREVIEW_MANAGER.GetPreview(PointCloud->GetID());
		InformationLines.push_back("Points: " + GetNumberWithSeparators(PointCloud->GetPointCount()));
	}

	bool bChangePointCloud = false;
	bool bShowInFolder = false;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));
	RenderResourceCard("PointCloudCard", PreviewTexture, Name, InformationLines, EntityChangePointCloudTarget, PointCloud != nullptr, bChangePointCloud, bShowInFolder);
	ImGui::PopStyleVar();

	if (bChangePointCloud)
	{
		EntityToModify = PointCloudEntity;
		SELECT_FEOBJECT_POPUP.Show(FE_POINT_CLOUD, ChangePointCloudOfEntityCallback, PointCloud);
	}

	if (bShowInFolder)
		CONTENT_BROWSER_WINDOW.OpenItemParentFolder(PointCloud);
}

void FEEditorInspectorWindow::DisplayPrefabInstanceProperties(FEEntity* PrefabInstanceEntity)
{
	FEPrefab* Prefab = PrefabInstanceEntity->GetComponent<FEPrefabInstanceComponent>().GetPrefab();
	if (Prefab == nullptr)
	{
		ImGui::TextUnformatted("No prefab");
		return;
	}

	std::vector<std::string> InformationLines;
	if (Prefab->GetScene() != nullptr)
	{
		const size_t EntityCount = Prefab->GetScene()->GetEntityIDList().size();
		InformationLines.push_back("Prefab, " + GetNumberWithSeparators(EntityCount) + (EntityCount == 1 ? " entity" : " entities"));
	}
	InformationLines.push_back(UNIQUE_ID.ToString(Prefab->GetID()));

	bool bEditPrefab = false;
	bool bShowInFolder = false;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));
	RenderResourceCard("PrefabCard", PREVIEW_MANAGER.GetPrefabPreview(Prefab->GetID()), Prefab->GetName(), InformationLines, nullptr, true, bEditPrefab, bShowInFolder, "Edit prefab");
	ImGui::PopStyleVar();

	if (bEditPrefab)
		PREFAB_EDITOR_MANAGER.PrepareEditWinow(Prefab);

	if (bShowInFolder)
		CONTENT_BROWSER_WINDOW.OpenItemParentFolder(Prefab);
}

// Text field and a list of tags that are already used in the scene.
static void RenderTagField(FEScene* Scene, FEEntity* Entity)
{
	FETagComponent& TagComponent = Entity->GetComponent<FETagComponent>();
	std::string CurrentTag = TagComponent.GetTag();

	const float ListButtonWidth = ImGui::GetFrameHeight();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ListButtonWidth);
	if (ImGui::InputTextWithHint(("##Tag Edit" + UNIQUE_ID.ToString(Entity->GetID())).c_str(), "No tag", &CurrentTag))
		TagComponent.SetTag(CurrentTag);

	const ImVec2 FieldMin = ImGui::GetItemRectMin();
	const ImVec2 FieldMax = ImGui::GetItemRectMax();
	ImGui::SameLine(0.0f, 0.0f);
	if (ImGui::ArrowButton("##SceneTags", ImGuiDir_Down))
		ImGui::OpenPopup("SceneTags");
	ShowToolTip("Tags used in this scene");

	const float ListWidth = FieldMax.x - FieldMin.x + ListButtonWidth;
	ImGui::SetNextWindowPos(ImVec2(FieldMin.x, FieldMax.y + 2.0f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(ListWidth, 0.0f), ImVec2(ListWidth, 300.0f));
	if (ImGui::BeginPopup("SceneTags"))
	{
		std::set<std::string> SceneTags;
		const std::vector<FEUUID> EntityIDList = Scene->GetEntityIDListWithComponent<FETagComponent>();
		for (size_t i = 0; i < EntityIDList.size(); i++)
		{
			FEEntity* CurrentEntity = Scene->GetEntity(EntityIDList[i]);
			if (CurrentEntity == nullptr)
				continue;

			const std::string Tag = CurrentEntity->GetComponent<FETagComponent>().GetTag();
			if (!Tag.empty())
				SceneTags.insert(Tag);
		}

		ImGui::TextDisabled(SceneTags.empty() ? "No tags in this scene" : "Tags used in this scene");
		for (const std::string& Tag : SceneTags)
		{
			if (ImGui::Selectable(Tag.c_str(), Tag == CurrentTag))
				TagComponent.SetTag(Tag);
		}

		ImGui::EndPopup();
	}
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

	RenderRowLabel("ID");
	RenderIDText(UNIQUE_ID.ToString(EntitySelected->GetID()));

	RenderRowLabel("Name");
	ImGui::TextUnformatted(EntitySelected->GetName().c_str());

	if (EntitySelected->HasComponent<FETagComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FETagComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (RenderComponentHeader("Tag", typeid(FETagComponent)))
			RenderTagField(CurrentScene, EntitySelected);
	}

	if (EntitySelected->HasComponent<FETransformComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FETransformComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (RenderComponentHeader("Transform", typeid(FETransformComponent)))
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

		if (RenderComponentHeader("Game Model", typeid(FEGameModelComponent)))
			DisplayGameModelProperties(EntitySelected);
	}

	if (EntitySelected->HasComponent<FEPointCloudComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEPointCloudComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (RenderComponentHeader("Point cloud", typeid(FEPointCloudComponent)))
			DisplayPointCloudProperties(EntitySelected);
	}

	if (EntitySelected->HasComponent<FEVolumeComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEVolumeComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (RenderComponentHeader("Volumetric", typeid(FEVolumeComponent)))
			DisplayVolumeProperties(EntitySelected);
	}

	if (EntitySelected->HasComponent<FEPrefabInstanceComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEPrefabInstanceComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (RenderComponentHeader("Prefab Instance", typeid(FEPrefabInstanceComponent)))
			DisplayPrefabInstanceProperties(EntitySelected);
	}

	if (EntitySelected->HasComponent<FEInstancedComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FEInstancedComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (RenderComponentHeader("Instanced", typeid(FEInstancedComponent)))
			DisplayInstancedProperties(EntitySelected);
	}

	if (EntitySelected->HasComponent<FETerrainComponent>())
	{
		if (RenderComponentDeleteButton(EntitySelected, COMPONENTS_TOOL.GetComponentInfo<FETerrainComponent>()))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (RenderComponentHeader("Terrain", typeid(FETerrainComponent)))
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

		if (RenderComponentHeader("Light", typeid(FELightComponent)))
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

		if (RenderComponentHeader("Camera", typeid(FECameraComponent)))
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

		if (RenderComponentHeader("Virtual UI", typeid(FEVirtualUIComponent)))
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

		if (RenderComponentHeader("Native Script", typeid(FENativeScriptComponent)))
		{
			DisplayNativeScriptProperties(SELECTED.GetSelected(CurrentScene));
		}
	}

	EDITOR_PLUGIN_MANAGER.RenderInspectorSections(EntitySelected);

	RenderAddComponentMenu(EntitySelected);

	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditorInspectorWindow::CreateNewTerrainLayerWithMaterialCallback(std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		if (TerrainToWorkWith == nullptr)
			return;

		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetID());
		if (SelectedMaterial == nullptr)
			return;

		TERRAIN_SYSTEM.ActivateVacantLayerSlot(TerrainToWorkWith, SelectedMaterial);
	}

	TerrainToWorkWith = nullptr;
}

static size_t TemporaryLayerIndex = -1;
void FEEditorInspectorWindow::ChangeMaterialInTerrainLayerCallback(std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		if (TemporaryLayerIndex == -1)
			return;

		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetID());
		if (SelectedMaterial == nullptr)
			return;

		FETerrainComponent& TerrainComponent = TerrainToWorkWith->GetComponent<FETerrainComponent>();
		TerrainComponent.GetLayerInSlot(TemporaryLayerIndex)->SetMaterial(SelectedMaterial);
	}

	TerrainToWorkWith = nullptr;
	TemporaryLayerIndex = -1;
}

struct TerrainTool
{
	FE_TERRAIN_BRUSH_MODE BrushMode;
	const char* Name;
	const char* Hint;
};

// The first one shows the settings, the others are brushes.
static const TerrainTool TerrainTools[] = {
	{ FE_TERRAIN_BRUSH_NONE, "Settings", "" },
	{ FE_TERRAIN_BRUSH_SCULPT_DRAW, "Raise", "Left mouse raises the terrain, with Shift it lowers." },
	{ FE_TERRAIN_BRUSH_SCULPT_LEVEL, "Level", "Left mouse levels the terrain." },
	{ FE_TERRAIN_BRUSH_SCULPT_SMOOTH, "Smooth", "Left mouse smooths the terrain." },
	{ FE_TERRAIN_BRUSH_LAYER_DRAW, "Paint", "Left mouse paints the selected layer, with Shift it removes it." }
};
static const int TerrainToolCount = 5;
static const int TerrainPaintToolIndex = 4;

static int GetTerrainToolIndex(const FE_TERRAIN_BRUSH_MODE BrushMode)
{
	if (BrushMode == FE_TERRAIN_BRUSH_SCULPT_DRAW_INVERSED)
		return 1;

	for (int i = 0; i < TerrainToolCount; i++)
	{
		if (TerrainTools[i].BrushMode == BrushMode)
			return i;
	}

	return 0;
}

// Center should have integer coordinates.
static void RenderTerrainToolIcon(ImDrawList* DrawList, const FE_TERRAIN_BRUSH_MODE BrushMode, const ImVec2 Center, const ImU32 Color)
{
	const float HalfWidth = 10.0f;
	const float Base = Center.y + 8.0f;
	const ImU32 FaintColor = (Color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(110) << IM_COL32_A_SHIFT);

	auto RenderMound = [&](const float RadiusX, const float RadiusY, const float CutHeight, const ImU32 MoundColor) {
		const int Steps = 16;
		for (int i = 0; i <= Steps; i++)
		{
			const float Angle = IM_PI * static_cast<float>(i) / static_cast<float>(Steps);
			DrawList->PathLineTo(ImVec2(Center.x + cosf(Angle) * RadiusX, Base - std::min(sinf(Angle) * RadiusY, CutHeight)));
		}
		DrawList->PathFillConvex(MoundColor);
	};

	if (BrushMode == FE_TERRAIN_BRUSH_NONE)
	{
		const float LineOffsets[] = { -6.0f, 0.0f, 6.0f };
		const float KnobOffsets[] = { 4.0f, -5.0f, 1.0f };
		for (int i = 0; i < 3; i++)
		{
			DrawList->AddRectFilled(ImVec2(Center.x - HalfWidth, Center.y + LineOffsets[i] - 1.0f), ImVec2(Center.x + HalfWidth, Center.y + LineOffsets[i] + 1.0f), FaintColor);
			DrawList->AddCircleFilled(ImVec2(Center.x + KnobOffsets[i], Center.y + LineOffsets[i]), 3.0f, Color);
		}
	}
	else if (BrushMode == FE_TERRAIN_BRUSH_SCULPT_DRAW)
	{
		RenderMound(HalfWidth, 9.0f, 9.0f, Color);
		DrawList->AddRectFilled(ImVec2(Center.x - 1.0f, Center.y - 7.0f), ImVec2(Center.x + 1.0f, Center.y - 3.0f), Color);
		DrawList->AddTriangleFilled(ImVec2(Center.x, Center.y - 12.0f), ImVec2(Center.x + 4.5f, Center.y - 6.5f), ImVec2(Center.x - 4.5f, Center.y - 6.5f), Color);
	}
	else if (BrushMode == FE_TERRAIN_BRUSH_SCULPT_LEVEL)
	{
		RenderMound(HalfWidth, 17.0f, 7.0f, Color);
		const float ArrowY = Center.y - 6.0f;
		DrawList->AddRectFilled(ImVec2(Center.x - 6.0f, ArrowY - 1.0f), ImVec2(Center.x + 6.0f, ArrowY + 1.0f), Color);
		DrawList->AddTriangleFilled(ImVec2(Center.x - HalfWidth, ArrowY), ImVec2(Center.x - 5.0f, ArrowY - 4.0f), ImVec2(Center.x - 5.0f, ArrowY + 4.0f), Color);
		DrawList->AddTriangleFilled(ImVec2(Center.x + HalfWidth, ArrowY), ImVec2(Center.x + 5.0f, ArrowY + 4.0f), ImVec2(Center.x + 5.0f, ArrowY - 4.0f), Color);
	}
	else if (BrushMode == FE_TERRAIN_BRUSH_SCULPT_SMOOTH)
	{
		RenderMound(6.0f, 17.0f, 17.0f, FaintColor);
		RenderMound(HalfWidth, 8.0f, 8.0f, Color);
	}
	else if (BrushMode == FE_TERRAIN_BRUSH_LAYER_DRAW)
	{
		const ImVec2 HandleEnd = ImVec2(Center.x + 9.0f, Center.y - 9.0f);
		const ImVec2 HandleStart = ImVec2(Center.x - 1.0f, Center.y + 1.0f);
		DrawList->AddQuadFilled(ImVec2(HandleEnd.x - 1.5f, HandleEnd.y - 1.5f), ImVec2(HandleEnd.x + 1.5f, HandleEnd.y + 1.5f), ImVec2(HandleStart.x + 2.0f, HandleStart.y + 2.0f), ImVec2(HandleStart.x - 2.0f, HandleStart.y - 2.0f), Color);
		DrawList->AddQuadFilled(ImVec2(Center.x - 4.0f, Center.y - 1.0f), ImVec2(Center.x + 1.0f, Center.y + 4.0f), ImVec2(Center.x - 3.0f, Center.y + 8.0f), ImVec2(Center.x - 8.0f, Center.y + 3.0f), Color);
		DrawList->AddTriangleFilled(ImVec2(Center.x - 8.0f, Center.y + 3.0f), ImVec2(Center.x - 3.0f, Center.y + 8.0f), ImVec2(Center.x - 10.0f, Center.y + 10.0f), Color);
	}
}

static bool RenderTerrainToolTile(const TerrainTool& Tool, const bool bSelected, const ImVec2 Size)
{
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + Size.x, Min.y + Size.y);
	const bool bPressed = ImGui::InvisibleButton(Tool.Name, Size);
	const bool bHovered = ImGui::IsItemHovered();

	ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
	if (bSelected)
		Fill = bHovered ? FEEditorStyle::AccentHovered : FEEditorStyle::Accent;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, Fill, 4.0f);
	if (!bSelected)
		DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f);

	const ImU32 ContentColor = bSelected ? FEEditorStyle::TextOnAccent : FEEditorStyle::Text;
	const float NameFontSize = 16.0f;
	RenderTerrainToolIcon(DrawList, Tool.BrushMode, ImVec2(floorf(Min.x + Size.x / 2.0f), floorf(Min.y + 24.0f)), ContentColor);
	ImGui::PushFont(nullptr, NameFontSize);
	const float NameWidth = ImGui::CalcTextSize(Tool.Name).x;
	DrawList->AddText(ImVec2(floorf(Min.x + (Size.x - NameWidth) / 2.0f), Max.y - NameFontSize - 8.0f), ContentColor, Tool.Name);
	ImGui::PopFont();
	return bPressed;
}

static void RenderHintText(const std::string& Text)
{
	ImGui::PushFont(nullptr, 16.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
	ImGui::PushTextWrapPos(0.0f);
	ImGui::TextUnformatted(Text.c_str());
	ImGui::PopTextWrapPos();
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

enum class TerrainLayerAction
{
	None,
	Rename,
	ChangeMaterial,
	Fill,
	Clear,
	ImportMask,
	ExportMask,
	Delete
};

static TerrainLayerAction RenderTerrainLayerMenuItems(const bool bAllActions)
{
	TerrainLayerAction Action = TerrainLayerAction::None;
	if (bAllActions)
	{
		if (ImGui::MenuItem("Rename"))
			Action = TerrainLayerAction::Rename;

		if (ImGui::MenuItem("Change material..."))
			Action = TerrainLayerAction::ChangeMaterial;

		ImGui::Separator();
		if (ImGui::MenuItem("Fill"))
			Action = TerrainLayerAction::Fill;

		if (ImGui::MenuItem("Clear"))
			Action = TerrainLayerAction::Clear;

		ImGui::Separator();
	}

	if (ImGui::MenuItem("Import mask..."))
		Action = TerrainLayerAction::ImportMask;

	if (ImGui::MenuItem("Export mask..."))
		Action = TerrainLayerAction::ExportMask;

	ImGui::Separator();
	ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Danger);
	if (ImGui::MenuItem("Delete"))
		Action = TerrainLayerAction::Delete;
	ImGui::PopStyleColor();

	return Action;
}

// LayerIndex -1 adds a new layer.
void FEEditorInspectorWindow::SelectMaterialForTerrainLayer(FEEntity* TerrainEntity, const int LayerIndex)
{
	std::vector<FEObject*> SuitableMaterials;
	const std::vector<FEUUID> MaterialIDs = RESOURCE_MANAGER.GetMaterialIDList();
	for (size_t i = 0; i < MaterialIDs.size(); i++)
	{
		if (RESOURCE_MANAGER.GetMaterial(MaterialIDs[i])->IsCompactPacking())
			SuitableMaterials.push_back(RESOURCE_MANAGER.GetMaterial(MaterialIDs[i]));
	}

	if (SuitableMaterials.empty())
	{
		MessagePopUp::GetInstance().Show("No suitable material", "There are no materials with compact packing.");
		return;
	}

	TerrainToWorkWith = TerrainEntity;
	if (LayerIndex == -1)
	{
		SELECT_FEOBJECT_POPUP.Show(FE_MATERIAL, CreateNewTerrainLayerWithMaterialCallback, nullptr, SuitableMaterials);
	}
	else
	{
		TemporaryLayerIndex = static_cast<size_t>(LayerIndex);
		SELECT_FEOBJECT_POPUP.Show(FE_MATERIAL, ChangeMaterialInTerrainLayerCallback, TerrainEntity->GetComponent<FETerrainComponent>().GetLayerInSlot(LayerIndex)->GetMaterial(), SuitableMaterials);
	}
}

void FEEditorInspectorWindow::RenderTerrainLayers(FEEntity* TerrainEntity)
{
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	const int Columns = 4;
	const float Spacing = 8.0f;
	const float SmallFontSize = 16.0f;
	const float SwatchSize = floorf((ImGui::GetContentRegionAvail().x - Spacing * static_cast<float>(Columns - 1)) / static_cast<float>(Columns));
	const float NameHeight = SmallFontSize + 8.0f;
	const int LayerCount = TerrainComponent.LayersUsed();
	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	int SelectedLayer = -1;
	if (TerrainComponent.GetLayerInSlot(TERRAIN_SYSTEM.GetBrushLayerIndex()) != nullptr)
		SelectedLayer = static_cast<int>(TERRAIN_SYSTEM.GetBrushLayerIndex());

	TerrainLayerAction Action = TerrainLayerAction::None;
	bool bOpenLayerMenu = false;

	// One more tile adds a layer while there is a free slot.
	const int TileCount = LayerCount < FE_TERRAIN_MAX_LAYERS ? LayerCount + 1 : LayerCount;
	const ImVec2 GridStart = ImGui::GetCursorScreenPos();
	for (int i = 0; i < TileCount; i++)
	{
		const ImVec2 Min = ImVec2(GridStart.x + static_cast<float>(i % Columns) * (SwatchSize + Spacing), GridStart.y + static_cast<float>(i / Columns) * (SwatchSize + NameHeight + Spacing));
		const ImVec2 Max = ImVec2(Min.x + SwatchSize, Min.y + SwatchSize);

		ImGui::SetCursorScreenPos(Min);
		ImGui::PushID(i);
		const bool bPressed = ImGui::InvisibleButton("##TerrainLayer", ImVec2(SwatchSize, SwatchSize + NameHeight));
		const bool bHovered = ImGui::IsItemHovered();

		if (i == LayerCount)
		{
			ImGui::PopID();
			if (bPressed)
				SelectMaterialForTerrainLayer(TerrainEntity, -1);

			DrawList->AddRectFilled(Min, Max, bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button, 4.0f);
			DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f);
			const ImVec2 Center = ImVec2(floorf(Min.x + SwatchSize / 2.0f), floorf(Min.y + SwatchSize / 2.0f));
			DrawList->AddRectFilled(ImVec2(Center.x - 10.0f, Center.y - 1.0f), ImVec2(Center.x + 10.0f, Center.y + 1.0f), FEEditorStyle::Text);
			DrawList->AddRectFilled(ImVec2(Center.x - 1.0f, Center.y - 10.0f), ImVec2(Center.x + 1.0f, Center.y + 10.0f), FEEditorStyle::Text);

			ImGui::PushFont(nullptr, SmallFontSize);
			DrawList->AddText(ImVec2(floorf(Min.x + (SwatchSize - ImGui::CalcTextSize("Add layer").x) / 2.0f), Max.y + 4.0f), FEEditorStyle::TextMuted, "Add layer");
			ImGui::PopFont();
			continue;
		}

		if (i < static_cast<int>(TerrainChangeLayerMaterialTargets.size()))
			TerrainChangeLayerMaterialTargets[i]->StickToItem();
		ImGui::PopID();

		FETerrainLayer* Layer = TerrainComponent.GetLayerInSlot(i);
		ShowToolTip(Layer->GetName().c_str());
		if (bPressed || (bHovered && ImGui::IsMouseClicked(1)))
		{
			TERRAIN_SYSTEM.SetBrushLayerIndex(static_cast<size_t>(i));
			SelectedLayer = i;
			bOpenLayerMenu = !bPressed;
		}

		if (Layer->GetMaterial() != nullptr)
			DrawList->AddImageRounded(PREVIEW_MANAGER.GetMaterialPreview(Layer->GetMaterial()->GetID())->GetTextureID(), Min, Max, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), IM_COL32_WHITE, 4.0f);

		const bool bSelected = SelectedLayer == i;
		if (bSelected)
		{
			DrawList->AddRect(Min, Max, FEEditorStyle::Accent, 4.0f, 3.0f);
		}
		else if (bHovered)
		{
			DrawList->AddRect(Min, Max, FEEditorStyle::TextMuted, 4.0f, 2.0f);
		}

		ImGui::PushFont(nullptr, SmallFontSize);
		std::string ShownName = Layer->GetName();
		if (ImGui::CalcTextSize(ShownName.c_str()).x > SwatchSize)
		{
			while (ShownName.size() > 1 && ImGui::CalcTextSize((ShownName + "...").c_str()).x > SwatchSize)
				ShownName.pop_back();

			ShownName += "...";
		}

		DrawList->AddText(ImVec2(floorf(Min.x + (SwatchSize - ImGui::CalcTextSize(ShownName.c_str()).x) / 2.0f), Max.y + 4.0f), bSelected ? FEEditorStyle::Text : FEEditorStyle::TextMuted, ShownName.c_str());
		ImGui::PopFont();
	}

	const int RowCount = (TileCount + Columns - 1) / Columns;
	ImGui::SetCursorScreenPos(ImVec2(GridStart.x, GridStart.y + static_cast<float>(RowCount) * (SwatchSize + NameHeight + Spacing)));
	ImGui::Dummy(ImVec2(0.0f, 0.0f));

	if (SelectedLayer == -1)
		return;

	if (TerrainLayerRenameIndex != SelectedLayer)
		TerrainLayerRenameIndex = -1;

	// Card with the selected layer and its actions.
	FETerrainLayer* Layer = TerrainComponent.GetLayerInSlot(SelectedLayer);
	const float Padding = 10.0f;
	const float ButtonHeight = SmallFontSize + 8.0f;
	const float CardHeight = Padding * 2.0f + ImGui::GetFrameHeight() + SmallFontSize + ButtonHeight + 18.0f;
	const ImVec2 CardMin = ImGui::GetCursorScreenPos();
	const ImVec2 CardMax = ImVec2(CardMin.x + ImGui::GetContentRegionAvail().x, CardMin.y + CardHeight);
	DrawList->AddRectFilled(CardMin, CardMax, FEEditorStyle::Card, 4.0f);
	DrawList->AddRect(CardMin, CardMax, FEEditorStyle::CardBorder, 4.0f);

	if (TerrainLayerRenameIndex == SelectedLayer)
	{
		ImGui::SetCursorScreenPos(ImVec2(CardMin.x + Padding, CardMin.y + Padding));
		ImGui::SetNextItemWidth(CardMax.x - CardMin.x - Padding * 2.0f);
		if (!bLastFrameTerrainLayerRenameEditWasVisible)
		{
			ImGui::SetKeyboardFocusHere();
			bLastFrameTerrainLayerRenameEditWasVisible = true;
		}

		const bool bEnterPressed = ImGui::InputText("##TerrainLayerName", &TerrainLayerRenameBuffer, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
		if (bEnterPressed || ImGui::IsItemDeactivated())
		{
			if (!ImGui::IsKeyPressed(ImGuiKey_Escape) && !TerrainLayerRenameBuffer.empty() && TerrainLayerRenameBuffer != Layer->GetName())
			{
				Layer->SetName(TerrainLayerRenameBuffer);
				PROJECT_MANAGER.GetCurrent()->SetModified(true);
			}

			TerrainLayerRenameIndex = -1;
		}
	}
	else
	{
		DrawList->PushClipRect(CardMin, ImVec2(CardMax.x - Padding, CardMax.y), true);
		DrawList->AddText(ImVec2(CardMin.x + Padding, CardMin.y + Padding + ImGui::GetStyle().FramePadding.y), FEEditorStyle::Text, Layer->GetName().c_str());
		DrawList->PopClipRect();
	}

	ImGui::PushFont(nullptr, SmallFontSize);
	const std::string MaterialText = "Material: " + (Layer->GetMaterial() != nullptr ? Layer->GetMaterial()->GetName() : std::string("none"));
	DrawList->PushClipRect(CardMin, ImVec2(CardMax.x - Padding, CardMax.y), true);
	DrawList->AddText(ImVec2(CardMin.x + Padding, CardMin.y + Padding + ImGui::GetFrameHeight() + 4.0f), FEEditorStyle::TextMuted, MaterialText.c_str());
	DrawList->PopClipRect();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
	ImGui::SetCursorScreenPos(ImVec2(CardMin.x + Padding, CardMax.y - Padding - ButtonHeight));
	if (ImGui::Button("Rename"))
		Action = TerrainLayerAction::Rename;

	ImGui::SameLine(0.0f, 6.0f);
	if (ImGui::Button("Material..."))
		Action = TerrainLayerAction::ChangeMaterial;

	ImGui::SameLine(0.0f, 6.0f);
	if (ImGui::Button("Fill"))
		Action = TerrainLayerAction::Fill;

	ImGui::SameLine(0.0f, 6.0f);
	if (ImGui::Button("Clear"))
		Action = TerrainLayerAction::Clear;

	ImGui::SameLine(0.0f, 6.0f);
	if (ImGui::Button("More"))
		ImGui::OpenPopup("##TerrainLayerMoreMenu");
	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImGui::SetCursorScreenPos(ImVec2(CardMin.x, CardMax.y));
	ImGui::Dummy(ImVec2(0.0f, 0.0f));

	if (bOpenLayerMenu)
		ImGui::OpenPopup("##TerrainLayerMenu");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));
	if (ImGui::BeginPopup("##TerrainLayerMenu"))
	{
		const TerrainLayerAction MenuAction = RenderTerrainLayerMenuItems(true);
		if (MenuAction != TerrainLayerAction::None)
			Action = MenuAction;

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("##TerrainLayerMoreMenu"))
	{
		const TerrainLayerAction MenuAction = RenderTerrainLayerMenuItems(false);
		if (MenuAction != TerrainLayerAction::None)
			Action = MenuAction;

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	if (Action == TerrainLayerAction::Rename)
	{
		TerrainLayerRenameIndex = SelectedLayer;
		TerrainLayerRenameBuffer = Layer->GetName();
		bLastFrameTerrainLayerRenameEditWasVisible = false;
	}
	else if (Action == TerrainLayerAction::ChangeMaterial)
	{
		SelectMaterialForTerrainLayer(TerrainEntity, SelectedLayer);
	}
	else if (Action == TerrainLayerAction::Fill)
	{
		TERRAIN_SYSTEM.FillTerrainLayerMask(TerrainEntity, SelectedLayer);
	}
	else if (Action == TerrainLayerAction::Clear)
	{
		TERRAIN_SYSTEM.ClearTerrainLayerMask(TerrainEntity, SelectedLayer);
	}
	else if (Action == TerrainLayerAction::ImportMask)
	{
		FILE_SYSTEM.OpenFileDialog(TEXTURE_LOAD_FILTER, [TerrainEntity, SelectedLayer](const std::string& FilePath) {
			if (FilePath.empty())
				return;

			TERRAIN_SYSTEM.LoadTerrainLayerMask(TerrainEntity, FilePath, SelectedLayer);
			PROJECT_MANAGER.GetCurrent()->SetModified(true);
		});
	}
	else if (Action == TerrainLayerAction::ExportMask)
	{
		// The dialog already adds ".png" when the user does not type an extension.
		FILE_SYSTEM.SaveFileDialog("", TEXTURE_LOAD_FILTER, [TerrainEntity, SelectedLayer](std::string FilePath, int ChosenFilterIndex) {
			if (FILE_SYSTEM.GetFileExtension(FilePath) != ".png")
				FilePath += ".png";

			TERRAIN_SYSTEM.SaveTerrainLayerMask(TerrainEntity, FilePath, SelectedLayer);
			return true;
		});
	}
	else if (Action == TerrainLayerAction::Delete)
	{
		TERRAIN_SYSTEM.DeleteTerrainLayerMask(TerrainEntity, SelectedLayer);
	}
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
		TerrainChangeMaterialIndices.resize(TerrainComponent.LayersUsed());
		for (size_t i = 0; i < size_t(TerrainComponent.LayersUsed()); i++)
		{
			TerrainChangeMaterialIndices[i] = int(i);
			TerrainChangeLayerMaterialTargets[i] = DRAG_AND_DROP_MANAGER.AddTarget(FE_MATERIAL, TerrainChangeMaterialTargetCallback, (void**)&TerrainChangeMaterialIndices[i], "Drop to assing material to " + TerrainComponent.GetLayerInSlot(i)->GetName());
		}
	}

	const bool bHasLayers = TerrainComponent.GetLayerInSlot(0) != nullptr;
	if (TERRAIN_SYSTEM.GetBrushMode() == FE_TERRAIN_BRUSH_LAYER_DRAW && TerrainComponent.GetLayerInSlot(TERRAIN_SYSTEM.GetBrushLayerIndex()) == nullptr)
	{
		if (bHasLayers)
		{
			TERRAIN_SYSTEM.SetBrushLayerIndex(0);
		}
		else
		{
			TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_NONE);
			TerrainWithLayersShown = TerrainEntity;
		}
	}

	// Layers of a terrain without layers are shown with the brush off, the brush is turned on with the first layer.
	if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
	{
		TerrainWithLayersShown = nullptr;
	}
	else if (TerrainWithLayersShown == TerrainEntity && bHasLayers)
	{
		TERRAIN_SYSTEM.SetBrushLayerIndex(0);
		TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, FE_TERRAIN_BRUSH_LAYER_DRAW);
		TerrainWithLayersShown = nullptr;
	}

	int CurrentTool = GetTerrainToolIndex(TERRAIN_SYSTEM.GetBrushMode());
	if (TerrainWithLayersShown == TerrainEntity)
		CurrentTool = TerrainPaintToolIndex;

	const float LabelWidth = ImGui::CalcTextSize("Chunks per side").x + 14.0f;
	const float FieldSpacing = 6.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));

	const ImVec2 TileSize = ImVec2(floorf((ImGui::GetContentRegionAvail().x - FieldSpacing * static_cast<float>(TerrainToolCount - 1)) / static_cast<float>(TerrainToolCount)), 68.0f);
	for (int i = 0; i < TerrainToolCount; i++)
	{
		if (RenderTerrainToolTile(TerrainTools[i], CurrentTool == i, TileSize))
		{
			// Click on the active brush turns it off.
			FE_TERRAIN_BRUSH_MODE NewBrushMode = CurrentTool == i ? FE_TERRAIN_BRUSH_NONE : TerrainTools[i].BrushMode;
			TerrainWithLayersShown = nullptr;
			if (NewBrushMode == FE_TERRAIN_BRUSH_LAYER_DRAW)
			{
				if (!bHasLayers)
				{
					NewBrushMode = FE_TERRAIN_BRUSH_NONE;
					TerrainWithLayersShown = TerrainEntity;
				}
				else if (TerrainComponent.GetLayerInSlot(TERRAIN_SYSTEM.GetBrushLayerIndex()) == nullptr)
				{
					TERRAIN_SYSTEM.SetBrushLayerIndex(0);
				}
			}

			TERRAIN_SYSTEM.SetBrushMode(TerrainEntity, NewBrushMode);
		}

		if (i < TerrainToolCount - 1)
			ImGui::SameLine(0.0f, FieldSpacing);
	}

	if (CurrentTool == 0)
	{
		const FEAABB RealAABB = TERRAIN_SYSTEM.GetAABB(TerrainEntity);
		const glm::vec3 Size = RealAABB.GetMax() - RealAABB.GetMin();
		char SizeText[64] = {};
		snprintf(SizeText, sizeof(SizeText), "%.0f x %.0f m", fabsf(Size.x), fabsf(Size.z));
		RenderRowLabel("Size", LabelWidth);
		ImGui::TextUnformatted(SizeText);

		float HeightScale = TerrainComponent.GetHeightScale();
		if (RenderFloatRow("Height range", "##HeightRange", HeightScale, 1.0f, 0.001f, 100000.0f, LabelWidth, "%.1f m"))
			TerrainComponent.SetHeightScale(HeightScale);

		float LODLevel = TerrainComponent.GetLODLevel();
		if (RenderFloatRow("Detail level", "##LODLevel", LODLevel, 2.0f, 2.0f, 128.0f, LabelWidth, "%.0f"))
			TerrainComponent.SetLODLevel(LODLevel);
		ShowToolTip("Bigger detail level more details terrain will have and less performance you will get.");

		float ChunkPerSide = TerrainComponent.GetChunkPerSide();
		if (RenderFloatRow("Chunks per side", "##ChunkPerSide", ChunkPerSide, 2.0f, 1.0f, 16.0f, LabelWidth, "%.0f"))
			TerrainComponent.SetChunkPerSide(ChunkPerSide);

		float DisplacementScale = TerrainComponent.GetDisplacementScale();
		if (RenderFloatRow("Displacement", "##DisplacementScale", DisplacementScale, 0.02f, -10.0f, 10.0f, LabelWidth))
			TerrainComponent.SetDisplacementScale(DisplacementScale);

		RenderRowLabel("Height map", LabelWidth);
		const float HalfWidth = floorf((ImGui::GetContentRegionAvail().x - FieldSpacing) / 2.0f);
		if (ImGui::Button("Import...", ImVec2(HalfWidth, 0.0f)))
		{
			FILE_SYSTEM.OpenFileDialog(TEXTURE_LOAD_FILTER, [TerrainEntity](const std::string& FilePath) {
				if (FilePath.empty())
					return;

				TERRAIN_SYSTEM.LoadHeightMap(FilePath.c_str(), TerrainEntity);
				FETexture* LoadedTexture = TerrainEntity->GetComponent<FETerrainComponent>().GetHeightMap();
				if (LoadedTexture == RESOURCE_MANAGER.NoTexture)
				{
					LOG.Add(std::string("can't load height map: ") + FilePath, "FE_LOG_LOADING", FE_LOG_ERROR);
				}
				else
				{
					LoadedTexture->SetDirtyFlag(true);
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
				}
			});
		}

		ImGui::SameLine(0.0f, FieldSpacing);
		if (ImGui::Button("Export...", ImVec2(HalfWidth, 0.0f)))
		{
			// The dialog already adds ".png" when the user does not type an extension.
			FILE_SYSTEM.SaveFileDialog("", TEXTURE_LOAD_FILTER, [HeightMap = TerrainComponent.GetHeightMap()](std::string FilePath, int ChosenFilterIndex) {
				if (FILE_SYSTEM.GetFileExtension(FilePath) != ".png")
					FilePath += ".png";

				return RESOURCE_MANAGER.ExportFETextureToPNG(HeightMap, FilePath);
			});
		}

		if (TerrainComponent.GetHeightMap() != nullptr)
		{
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + LabelWidth, ImGui::GetCursorPosY() - 2.0f));
			RenderHintText(std::to_string(TerrainComponent.GetHeightMap()->GetWidth()) + " x " + std::to_string(TerrainComponent.GetHeightMap()->GetHeight()) + " pixels");
		}

		glm::vec2 TileMult = TerrainComponent.GetTileMult();
		RenderRowLabel("Tiling", LabelWidth);
		ImGui::SetNextItemWidth(HalfWidth);
		bool bTilingChanged = ImGui::DragFloat("##TilingX", &TileMult.x, 0.1f, 1.0f, 100.0f, "X %.1f");
		ImGui::SameLine(0.0f, FieldSpacing);
		ImGui::SetNextItemWidth(HalfWidth);
		bTilingChanged |= ImGui::DragFloat("##TilingY", &TileMult.y, 0.1f, 1.0f, 100.0f, "Y %.1f");
		if (bTilingChanged)
			TerrainComponent.SetTileMult(TileMult);

		if (RenderGroupTitle("Debug", "", false))
		{
			bool bWireframe = TerrainComponent.IsWireframeMode();
			RenderRowLabel("Wireframe", LabelWidth);
			if (ImGui::Checkbox("##Wireframe", &bWireframe))
				TerrainComponent.SetWireframeMode(bWireframe);

			FEShaderUniformValue CurrentValue;
			if (RESOURCE_MANAGER.GetShader(FEEngineResourceIDs::PBRShader)->GetUniformData("debugFlag", CurrentValue))
			{
				int DebugFlag = CurrentValue.GetValue<int>();
				RenderRowLabel("Shader view", LabelWidth);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::SliderInt("##DebugFlag", &DebugFlag, 0, 10))
					RESOURCE_MANAGER.GetShader(FEEngineResourceIDs::PBRShader)->UpdateUniformData("debugFlag", DebugFlag);
				ShowToolTip("Debug view of the PBR shader, it is the same for all objects.");
			}
		}
	}
	else
	{
		const bool bPaintWithoutLayers = CurrentTool == TerrainPaintToolIndex && !bHasLayers;
		RenderHintText(bPaintWithoutLayers ? "Add a layer to paint it on the terrain." : TerrainTools[CurrentTool].Hint);

		float BrushSize = TERRAIN_SYSTEM.GetBrushSize();
		if (RenderFloatRow("Size", "##BrushSize", BrushSize, 0.1f, 0.01f, 100.0f, LabelWidth, "%.2f"))
			TERRAIN_SYSTEM.SetBrushSize(BrushSize);

		float BrushIntensity = TERRAIN_SYSTEM.GetBrushIntensity();
		if (RenderFloatRow("Strength", "##BrushIntensity", BrushIntensity, 0.0001f, 0.0001f, 10.0f, LabelWidth, "%.4f"))
			TERRAIN_SYSTEM.SetBrushIntensity(BrushIntensity);

		if (CurrentTool == TerrainPaintToolIndex)
		{
			if (RenderGroupTitle("Layers", std::to_string(TerrainComponent.LayersUsed()) + " of " + std::to_string(FE_TERRAIN_MAX_LAYERS)))
				RenderTerrainLayers(TerrainEntity);
		}
	}

	ImGui::PopStyleVar();
}

enum class InstancedIcon
{
	None,
	Scatter,
	Cursor,
	Respawn,
	Add,
	ToGround,
	Delete
};

// Center should have integer coordinates.
static void RenderInstancedIcon(ImDrawList* DrawList, const InstancedIcon Icon, const ImVec2 Center, const ImU32 Color)
{
	if (Icon == InstancedIcon::Scatter)
	{
		const ImVec2 Offsets[] = { ImVec2(-7.0f, -6.0f), ImVec2(2.0f, -8.0f), ImVec2(8.0f, -1.0f), ImVec2(-2.0f, 0.0f), ImVec2(-8.0f, 6.0f), ImVec2(4.0f, 7.0f) };
		const float Sizes[] = { 2.5f, 2.0f, 2.5f, 3.0f, 2.0f, 2.5f };
		for (int i = 0; i < 6; i++)
			DrawList->AddCircleFilled(ImVec2(Center.x + Offsets[i].x, Center.y + Offsets[i].y), Sizes[i], Color);
	}
	else if (Icon == InstancedIcon::Cursor)
	{
		const ImVec2 Tip = ImVec2(Center.x - 6.0f, Center.y - 9.0f);
		DrawList->AddTriangleFilled(Tip, ImVec2(Tip.x + 12.0f, Tip.y + 11.0f), ImVec2(Tip.x, Tip.y + 16.0f), Color);
		DrawList->AddQuadFilled(ImVec2(Tip.x + 3.5f, Tip.y + 11.0f), ImVec2(Tip.x + 6.5f, Tip.y + 9.5f), ImVec2(Tip.x + 10.5f, Tip.y + 17.5f), ImVec2(Tip.x + 7.5f, Tip.y + 19.0f), Color);
	}
	else if (Icon == InstancedIcon::Respawn)
	{
		DrawList->PathArcTo(Center, 7.0f, 0.2f * IM_PI, 1.6f * IM_PI, 24);
		DrawList->PathStroke(Color, ImDrawFlags_None, 2.0f);
		DrawList->AddTriangleFilled(ImVec2(Center.x + 7.5f, Center.y - 7.0f), ImVec2(Center.x + 1.0f, Center.y - 3.5f), ImVec2(Center.x + 1.0f, Center.y - 10.5f), Color);
	}
	else if (Icon == InstancedIcon::Add)
	{
		DrawList->AddRectFilled(ImVec2(Center.x - 7.0f, Center.y - 1.0f), ImVec2(Center.x + 7.0f, Center.y + 1.0f), Color);
		DrawList->AddRectFilled(ImVec2(Center.x - 1.0f, Center.y - 7.0f), ImVec2(Center.x + 1.0f, Center.y + 7.0f), Color);
	}
	else if (Icon == InstancedIcon::ToGround)
	{
		DrawList->AddRectFilled(ImVec2(Center.x - 1.0f, Center.y - 9.0f), ImVec2(Center.x + 1.0f, Center.y), Color);
		DrawList->AddTriangleFilled(ImVec2(Center.x, Center.y + 5.0f), ImVec2(Center.x - 5.0f, Center.y - 1.0f), ImVec2(Center.x + 5.0f, Center.y - 1.0f), Color);
		DrawList->AddRectFilled(ImVec2(Center.x - 8.0f, Center.y + 7.0f), ImVec2(Center.x + 8.0f, Center.y + 9.0f), Color);
	}
	else if (Icon == InstancedIcon::Delete)
	{
		DrawList->AddLine(ImVec2(Center.x - 5.5f, Center.y - 5.5f), ImVec2(Center.x + 5.5f, Center.y + 5.5f), Color, 2.0f);
		DrawList->AddLine(ImVec2(Center.x - 5.5f, Center.y + 5.5f), ImVec2(Center.x + 5.5f, Center.y - 5.5f), Color, 2.0f);
	}
}

enum class InstancedButtonKind
{
	Normal,
	Primary,
	Danger
};

static bool RenderInstancedButton(const char* Name, const InstancedIcon Icon, const InstancedButtonKind Kind, const float Width)
{
	const float IconAreaSize = Icon == InstancedIcon::None ? 0.0f : 22.0f;
	const float IconToNameSpacing = Icon == InstancedIcon::None ? 0.0f : 8.0f;
	const float ContentWidth = IconAreaSize + IconToNameSpacing + ImGui::CalcTextSize(Name).x;
	const ImVec2 Size = ImVec2(Width, ImGui::GetFrameHeight() + 6.0f);
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + Size.x, Min.y + Size.y);
	const bool bPressed = ImGui::InvisibleButton(Name, Size);
	const bool bHovered = ImGui::IsItemHovered();

	ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
	ImU32 ContentColor = FEEditorStyle::Text;
	if (Kind == InstancedButtonKind::Primary)
	{
		Fill = bHovered ? FEEditorStyle::PrimaryButtonHovered : FEEditorStyle::PrimaryButton;
	}
	else if (Kind == InstancedButtonKind::Danger)
	{
		ContentColor = FEEditorStyle::Danger;
	}

	// GetColorU32 applies alpha of the style, so the button fades when it is disabled.
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, ImGui::GetColorU32(Fill), 4.0f);
	if (Kind != InstancedButtonKind::Primary)
		DrawList->AddRect(Min, Max, ImGui::GetColorU32(FEEditorStyle::ButtonBorder), 4.0f);

	const float ContentX = floorf(Min.x + (Size.x - ContentWidth) / 2.0f);
	RenderInstancedIcon(DrawList, Icon, ImVec2(floorf(ContentX + IconAreaSize / 2.0f), floorf(Min.y + Size.y / 2.0f)), ImGui::GetColorU32(ContentColor));
	DrawList->AddText(ImVec2(ContentX + IconAreaSize + IconToNameSpacing, Min.y + (Size.y - ImGui::GetFontSize()) / 2.0f), ImGui::GetColorU32(ContentColor), Name);
	return bPressed;
}

// Tile with an icon and a name, and a note under them.
static bool RenderInstancedModeTile(const char* Name, const std::string& Note, const InstancedIcon Icon, const bool bSelected, const ImVec2 Size)
{
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + Size.x, Min.y + Size.y);
	const bool bPressed = ImGui::InvisibleButton(Name, Size);
	const bool bHovered = ImGui::IsItemHovered();

	ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
	if (bSelected)
		Fill = bHovered ? FEEditorStyle::AccentHovered : FEEditorStyle::Accent;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, Fill, 4.0f);
	if (!bSelected)
		DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f);

	const ImU32 ContentColor = bSelected ? FEEditorStyle::TextOnAccent : FEEditorStyle::Text;
	const float IconAreaSize = 22.0f;
	const float IconToNameSpacing = 8.0f;
	const float NameCenterY = floorf(Min.y + 22.0f);
	const float ContentX = floorf(Min.x + (Size.x - IconAreaSize - IconToNameSpacing - ImGui::CalcTextSize(Name).x) / 2.0f);
	RenderInstancedIcon(DrawList, Icon, ImVec2(floorf(ContentX + IconAreaSize / 2.0f), NameCenterY), ContentColor);
	DrawList->AddText(ImVec2(ContentX + IconAreaSize + IconToNameSpacing, NameCenterY - ImGui::GetFontSize() / 2.0f), ContentColor, Name);

	const float NoteFontSize = 16.0f;
	ImGui::PushFont(nullptr, NoteFontSize);
	DrawList->AddText(ImVec2(floorf(Min.x + (Size.x - ImGui::CalcTextSize(Note.c_str()).x) / 2.0f), Max.y - NoteFontSize - 9.0f), bSelected ? FEEditorStyle::TextOnAccent : FEEditorStyle::TextMuted, Note.c_str());
	ImGui::PopFont();
	return bPressed;
}

void FEEditorInspectorWindow::DisplayInstancedProperties(FEEntity* InstancedEntity)
{
	FEScene* CurrentScene = EDITOR.GetFocusedScene();
	if (CurrentScene == nullptr)
		return;

	FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(CurrentScene->GetID());
	FEInstancedComponent& InstancedComponent = InstancedEntity->GetComponent<FEInstancedComponent>();
	const bool bSelectMode = INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(InstancedEntity);

	const float LabelWidth = ImGui::CalcTextSize("Layer strength").x + 14.0f;
	const float FieldSpacing = 6.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));

	const size_t ChangeCount = InstancedComponent.GetSpawnModificationCount();
	const ImVec2 TileSize = ImVec2(floorf((ImGui::GetContentRegionAvail().x - FieldSpacing) / 2.0f), 68.0f);
	if (RenderInstancedModeTile("Scatter", GetNumberWithSeparators(InstancedComponent.GetInstanceCount()) + " instances", InstancedIcon::Scatter, !bSelectMode, TileSize) && bSelectMode)
	{
		INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(InstancedEntity, false);
		SELECTED.Clear(CurrentScene);
		SELECTED.SetSelected(InstancedEntity);
	}

	ImGui::SameLine(0.0f, FieldSpacing);
	if (RenderInstancedModeTile("Edit by hand", ChangeCount == 0 ? "no changes" : GetNumberWithSeparators(ChangeCount) + " changes", InstancedIcon::Cursor, bSelectMode, TileSize) && !bSelectMode)
		INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(InstancedEntity, true);

	if (!bSelectMode)
	{
		FEEntity* Terrain = InstancedComponent.GetSnappedToTerrain();
		RenderRowLabel("Terrain", LabelWidth);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Terrain", Terrain == nullptr ? "none" : Terrain->GetName().c_str()))
		{
			if (ImGui::Selectable("none", Terrain == nullptr) && Terrain != nullptr)
				TERRAIN_SYSTEM.UnSnapInstancedEntity(Terrain, InstancedEntity);

			const std::vector<FEUUID> TerrainIDList = CurrentScene->GetEntityIDListWithComponent<FETerrainComponent>();
			for (size_t i = 0; i < TerrainIDList.size(); i++)
			{
				ImGui::PushID(UNIQUE_ID.ToString(TerrainIDList[i]).c_str());
				if (ImGui::Selectable(CurrentScene->GetEntity(TerrainIDList[i])->GetName().c_str(), Terrain != nullptr && Terrain->GetID() == TerrainIDList[i]))
					TERRAIN_SYSTEM.SnapInstancedEntity(CurrentScene->GetEntity(TerrainIDList[i]), InstancedEntity);
				ImGui::PopID();
			}

			ImGui::EndCombo();
		}
		ShowToolTip("Instances are placed on the surface of this terrain.");

		Terrain = InstancedComponent.GetSnappedToTerrain();
		if (Terrain != nullptr)
		{
			FETerrainComponent& TerrainComponent = Terrain->GetComponent<FETerrainComponent>();
			const int CurrentLayer = InstancedComponent.GetTerrainLayer();
			FETerrainLayer* Layer = CurrentLayer == -1 ? nullptr : TerrainComponent.GetLayerInSlot(CurrentLayer);

			RenderRowLabel("Only on layer", LabelWidth);
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##TerrainLayer", Layer == nullptr ? "any" : Layer->GetName().c_str()))
			{
				if (ImGui::Selectable("any", CurrentLayer == -1))
					TERRAIN_SYSTEM.UnConnectInstancedEntityFromLayer(InstancedEntity);

				for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
				{
					FETerrainLayer* LayerInSlot = TerrainComponent.GetLayerInSlot(i);
					if (LayerInSlot == nullptr)
						break;

					ImGui::PushID(static_cast<int>(i));
					if (ImGui::Selectable(LayerInSlot->GetName().c_str(), CurrentLayer == static_cast<int>(i)))
						TERRAIN_SYSTEM.ConnectInstancedEntityToLayer(Terrain, InstancedEntity, static_cast<int>(i));
					ImGui::PopID();
				}

				ImGui::EndCombo();
			}
			ShowToolTip("Instances are placed only where this layer is painted.");

			if (InstancedComponent.GetTerrainLayer() != -1)
			{
				float MinLayerIntensity = InstancedComponent.GetMinimalLayerIntensityToSpawn();
				if (RenderFloatRow("Layer strength", "##MinLayerIntensity", MinLayerIntensity, 0.01f, 0.0001f, 1.0f, LabelWidth, "at least %.2f"))
					InstancedComponent.SetMinimalLayerIntensityToSpawn(MinLayerIntensity);
				ShowToolTip("Minimal layer intensity to spawn.");
			}
		}

		if (RenderGroupTitle("Spawn###InstancedSpawn"))
		{
			RenderRowLabel("Count", LabelWidth);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragInt("##ObjectCount", &InstancedComponent.SpawnInfo.ObjectCount, 1.0f, 1, 10000000);
			if (InstancedComponent.SpawnInfo.ObjectCount <= 0)
				InstancedComponent.SpawnInfo.ObjectCount = 1;

			// Spawn takes a remainder of division by the whole part of the radius.
			RenderFloatRow("Radius", "##Radius", InstancedComponent.SpawnInfo.Radius, 1.0f, 1.0f, 100000.0f, LabelWidth, "%.1f m");
			if (InstancedComponent.SpawnInfo.Radius < 1.0f)
				InstancedComponent.SpawnInfo.Radius = 1.0f;

			RenderRowLabel("Scale", LabelWidth);
			const float HalfWidth = floorf((ImGui::GetContentRegionAvail().x - FieldSpacing) / 2.0f);
			float MinScale = InstancedComponent.SpawnInfo.GetMinScale();
			ImGui::SetNextItemWidth(HalfWidth);
			if (ImGui::DragFloat("##MinScale", &MinScale, 0.01f, 0.0f, 0.0f, "from %.2f"))
				InstancedComponent.SpawnInfo.SetMinScale(MinScale);

			ImGui::SameLine(0.0f, FieldSpacing);
			float MaxScale = InstancedComponent.SpawnInfo.GetMaxScale();
			ImGui::SetNextItemWidth(HalfWidth);
			if (ImGui::DragFloat("##MaxScale", &MaxScale, 0.01f, 0.0f, 0.0f, "to %.2f"))
				InstancedComponent.SpawnInfo.SetMaxScale(MaxScale);

			// Deviation is a part of the full turn, it is shown in degrees.
			RenderRowLabel("Rotation", LabelWidth);
			const ImU32 AxisColors[] = { FEEditorStyle::AxisX, FEEditorStyle::AxisY, FEEditorStyle::AxisZ };
			const char* AxisFormats[] = { "X %.0f\xC2\xB0", "Y %.0f\xC2\xB0", "Z %.0f\xC2\xB0" };
			const float ThirdWidth = floorf((ImGui::GetContentRegionAvail().x - FieldSpacing * 2.0f) / 3.0f);
			for (int i = 0; i < 3; i++)
			{
				float Degrees = InstancedComponent.SpawnInfo.RotationDeviation[i] * 360.0f;
				ImGui::PushID(i);
				ImGui::SetNextItemWidth(ThirdWidth);
				if (ImGui::DragFloat("##RotationDeviation", &Degrees, 1.0f, 3.6f, 360.0f, AxisFormats[i]))
					InstancedComponent.SpawnInfo.RotationDeviation[i] = std::clamp(Degrees / 360.0f, 0.01f, 1.0f);
				ShowToolTip("Each instance is turned around this axis by a random angle up to this one.");

				const ImVec2 FieldMin = ImGui::GetItemRectMin();
				ImGui::GetWindowDrawList()->AddRectFilled(FieldMin, ImVec2(FieldMin.x + 4.0f, ImGui::GetItemRectMax().y), AxisColors[i], 4.0f, ImDrawFlags_RoundCornersLeft);
				ImGui::PopID();

				if (i < 2)
					ImGui::SameLine(0.0f, FieldSpacing);
			}

			RenderRowLabel("Seed", LabelWidth);
			const float RandomButtonWidth = ImGui::CalcTextSize("Random").x + 24.0f;
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - RandomButtonWidth - FieldSpacing);
			ImGui::DragInt("##Seed", &InstancedComponent.SpawnInfo.Seed);
			ImGui::SameLine(0.0f, FieldSpacing);
			if (ImGui::Button("Random", ImVec2(RandomButtonWidth, 0.0f)))
				InstancedComponent.SpawnInfo.Seed = rand();
		}

		if (RenderInstancedButton("Respawn", InstancedIcon::Respawn, InstancedButtonKind::Primary, ImGui::GetContentRegionAvail().x))
		{
			INSTANCED_RENDERING_SYSTEM.ClearInstance(InstancedEntity);
			INSTANCED_RENDERING_SYSTEM.PopulateInstance(InstancedEntity, InstancedComponent.SpawnInfo);
		}

		if (ChangeCount != 0)
		{
			ImGui::PushFont(nullptr, 16.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Warning);
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextUnformatted(("Respawn drops " + GetNumberWithSeparators(ChangeCount) + (ChangeCount == 1 ? " change" : " changes") + " made by hand.").c_str());
			ImGui::PopTextWrapPos();
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}
	}
	else
	{
		const int InstanceIndex = CurrentSelectionData->InstancedSubObjectIndexSelected;
		if (InstanceIndex != -1)
		{
			if (RenderGroupTitle("Instance " + GetNumberWithSeparators(static_cast<size_t>(InstanceIndex)) + "###InstancedInstance", "of " + GetNumberWithSeparators(InstancedComponent.GetInstanceCount())))
			{
				FETransformComponent TemporaryTransform = FETransformComponent(InstancedComponent.GetTransformedInstancedMatrix(InstanceIndex));
				TemporaryTransform.SetSceneIndependent(true);
				bool bChanged = false;

				glm::vec3 Position = TemporaryTransform.GetPosition();
				if (RenderTransformRow("Position", Position, 0.1f, 0.0f, 0.0f) != -1)
				{
					TemporaryTransform.SetPosition(Position);
					bChanged = true;
				}

				glm::vec3 Rotation = TemporaryTransform.GetRotation();
				if (RenderTransformRow("Rotation", Rotation, 0.1f, -360.0f, 360.0f) != -1)
				{
					TemporaryTransform.SetRotation(Rotation);
					bChanged = true;
				}

				// Scale of an instance is uniform.
				glm::vec3 Scale = TemporaryTransform.GetScale();
				const int ChangedAxis = RenderTransformRow("Scale", Scale, Scale.x * 0.01f, 0.001f, 1000.0f);
				if (ChangedAxis != -1)
				{
					TemporaryTransform.SetScale(glm::vec3(Scale[ChangedAxis]));
					bChanged = true;
				}

				if (bChanged)
				{
					INSTANCED_RENDERING_SYSTEM.ModifyIndividualInstance(InstancedEntity, InstanceIndex, TemporaryTransform.GetWorldMatrix());
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
				}
			}

			const float ThirdWidth = floorf((ImGui::GetContentRegionAvail().x - FieldSpacing * 2.0f) / 3.0f);
			ImGui::BeginDisabled(InstancedComponent.GetSnappedToTerrain() == nullptr);
			if (RenderInstancedButton("To terrain", InstancedIcon::ToGround, InstancedButtonKind::Normal, ThirdWidth))
				INSTANCED_RENDERING_SYSTEM.TryToSnapIndividualInstance(InstancedEntity, InstanceIndex);
			ImGui::EndDisabled();
			ShowToolTip("Selected instance will attempt to snap to the terrain.");

			ImGui::SameLine(0.0f, FieldSpacing);
			if (RenderInstancedButton("Delete", InstancedIcon::Delete, InstancedButtonKind::Danger, ThirdWidth))
			{
				INSTANCED_RENDERING_SYSTEM.DeleteIndividualInstance(InstancedEntity, InstanceIndex);
				CurrentSelectionData->InstancedSubObjectIndexSelected = -1;
				PROJECT_MANAGER.GetCurrent()->SetModified(true);
			}

			ImGui::SameLine(0.0f, FieldSpacing);
			if (RenderInstancedButton("Done", InstancedIcon::None, InstancedButtonKind::Normal, ThirdWidth))
				CurrentSelectionData->InstancedSubObjectIndexSelected = -1;
			ShowToolTip("Back to all instances.");
		}
		else
		{
			RenderHintText("Click an instance in the scene to move, turn, scale or delete it.");
		}

		if (RenderInstancedButton("Add instance", InstancedIcon::Add, InstancedButtonKind::Normal, ImGui::GetContentRegionAvail().x))
		{
			FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(CurrentScene);
			if (MainCameraEntity != nullptr)
			{
				glm::mat4 NewInstanceMatrix = glm::identity<glm::mat4>();
				FETransformComponent& CameraTransformComponent = MainCameraEntity->GetComponent<FETransformComponent>();
				FECameraComponent& CameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
				NewInstanceMatrix = glm::translate(NewInstanceMatrix, CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + CameraComponent.GetForward() * 10.0f);
				INSTANCED_RENDERING_SYSTEM.AddIndividualInstance(InstancedEntity, NewInstanceMatrix);

				PROJECT_MANAGER.GetCurrent()->SetModified(true);
			}
		}
		ShowToolTip("Adds one instance in front of the camera.");
	}

	ImGui::PopStyleVar();
}

void FEEditorInspectorWindow::AddVirtualUIComponent(FEEntity* Entity)
{
	Entity->AddComponent<FEVirtualUIComponent>();
	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();
	VirtualUIComponent.SetWindowToListen(APPLICATION.GetMainWindow());
}

struct VirtualUIEventKind
{
	const char* Name;
	bool (FEVirtualUIComponent::*IsPassedThrough)() const;
	bool (FEVirtualUIComponent::*SetPassThrough)(bool);
};

// Three mouse kinds, two keyboard kinds and one for files.
static const VirtualUIEventKind VirtualUIEventKinds[] = {
	{ "Buttons", &FEVirtualUIComponent::IsMouseButtonPassThroughActive, &FEVirtualUIComponent::SetMouseButtonPassThrough },
	{ "Move", &FEVirtualUIComponent::IsMouseMovePassThroughActive, &FEVirtualUIComponent::SetMouseMovePassThrough },
	{ "Scroll", &FEVirtualUIComponent::IsScrollPassThroughActive, &FEVirtualUIComponent::SetScrollPassThrough },
	{ "Keys", &FEVirtualUIComponent::IsKeyPassThroughActive, &FEVirtualUIComponent::SetKeyPassThrough },
	{ "Text", &FEVirtualUIComponent::IsCharPassThroughActive, &FEVirtualUIComponent::SetCharPassThrough },
	{ "Dropped files", &FEVirtualUIComponent::IsDropPassThroughActive, &FEVirtualUIComponent::SetDropPassThrough }
};
static const int VirtualUIEventKindCount = 6;

void FEEditorInspectorWindow::DisplayVirtualUIProperties(FEEntity* VirtualUIEntity) const
{
	FEVirtualUIComponent& VirtualUIComponent = VirtualUIEntity->GetComponent<FEVirtualUIComponent>();

	// TO-DO: Give user ability to change window.
	FEWindow* CurrentWindow = VirtualUIComponent.GetWindowToListen();

	// TO-DO: Give user ability to change canvas mesh.
	FEMesh* CurrentCanvasMesh = VirtualUIComponent.GetCanvasMesh();

	int ActiveEventKindCount = 0;
	for (int i = 0; i < VirtualUIEventKindCount; i++)
	{
		if ((VirtualUIComponent.*VirtualUIEventKinds[i].IsPassedThrough)())
			ActiveEventKindCount++;
	}

	const std::string EventNote = std::to_string(ActiveEventKindCount) + " of " + std::to_string(VirtualUIEventKindCount);
	const bool bInputActive = VirtualUIComponent.IsInputActive();
	const float LabelWidth = ImGui::CalcTextSize("Resolution").x + 14.0f;
	const float FieldSpacing = 6.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));

	// Card with the picture that the interface draws now.
	const float Padding = 10.0f;
	const float PreviewSize = 96.0f;
	const float SmallFontSize = 16.0f;
	const ImVec2 CardMin = ImGui::GetCursorScreenPos();
	const ImVec2 CardMax = ImVec2(CardMin.x + ImGui::GetContentRegionAvail().x, CardMin.y + PreviewSize + Padding * 2.0f);
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(CardMin, CardMax, FEEditorStyle::Card, 4.0f);
	DrawList->AddRect(CardMin, CardMax, FEEditorStyle::CardBorder, 4.0f);

	const ImVec2 PreviewMin = ImVec2(CardMin.x + Padding, CardMin.y + Padding);
	const ImVec2 PreviewMax = ImVec2(PreviewMin.x + PreviewSize, PreviewMin.y + PreviewSize);
	DrawList->AddRectFilled(PreviewMin, PreviewMax, FEEditorStyle::Background);
	if (VirtualUIComponent.GetCurrentFrameTexture() != nullptr)
		DrawList->AddImage(VirtualUIComponent.GetCurrentFrameTexture()->GetTextureID(), PreviewMin, PreviewMax, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
	DrawList->AddRect(PreviewMin, PreviewMax, FEEditorStyle::CardBorder);

	glm::vec2 Resolution = VirtualUIComponent.GetCanvasResolution();
	char ResolutionText[64] = {};
	snprintf(ResolutionText, sizeof(ResolutionText), "%.0f x %.0f pixels", Resolution.x, Resolution.y);

	std::string WindowText = "No window to listen to";
	if (CurrentWindow != nullptr)
		WindowText = CurrentWindow == APPLICATION.GetMainWindow() ? "Listens to the main window" : "Listens to window " + CurrentWindow->GetTitle();

	const float TextX = PreviewMax.x + 14.0f;
	DrawList->PushClipRect(ImVec2(TextX, CardMin.y), ImVec2(CardMax.x - Padding, CardMax.y), true);
	DrawList->AddText(ImVec2(TextX, CardMin.y + Padding - 2.0f), FEEditorStyle::Text, ResolutionText);
	ImGui::PushFont(nullptr, SmallFontSize);
	float LineY = CardMin.y + Padding + ImGui::GetFontSize() + 12.0f;
	DrawList->AddText(ImVec2(TextX, LineY), FEEditorStyle::TextMuted, ("Canvas: " + (CurrentCanvasMesh != nullptr ? CurrentCanvasMesh->GetName() : std::string("none"))).c_str());
	LineY += SmallFontSize + 4.0f;
	DrawList->AddText(ImVec2(TextX, LineY), FEEditorStyle::TextMuted, WindowText.c_str());
	LineY += SmallFontSize + 4.0f;
	DrawList->AddText(ImVec2(TextX, LineY), bInputActive ? FEEditorStyle::Success : FEEditorStyle::TextMuted, bInputActive ? ("Input on, " + EventNote + " kinds of events").c_str() : "Input off");
	ImGui::PopFont();
	DrawList->PopClipRect();

	ImGui::SetCursorScreenPos(ImVec2(CardMin.x, CardMax.y));
	ImGui::Dummy(ImVec2(0.0f, 0.0f));

	RenderRowLabel("Resolution", LabelWidth);
	const float HalfWidth = floorf((ImGui::GetContentRegionAvail().x - FieldSpacing) / 2.0f);
	ImGui::SetNextItemWidth(HalfWidth);
	bool bResolutionChanged = ImGui::DragFloat("##ResolutionWidth", &Resolution.x, 1.0f, 1.0f, 4096.0f, "W %.0f");
	ImGui::SameLine(0.0f, FieldSpacing);
	ImGui::SetNextItemWidth(HalfWidth);
	bResolutionChanged |= ImGui::DragFloat("##ResolutionHeight", &Resolution.y, 1.0f, 1.0f, 4096.0f, "H %.0f");
	if (bResolutionChanged)
		VirtualUIComponent.SetCanvasResolution(Resolution);

	bool bNewInputActive = bInputActive;
	RenderRowLabel("Input", LabelWidth);
	if (ImGui::Checkbox("##InputActive", &bNewInputActive))
		VirtualUIComponent.SetInputActive(bNewInputActive);

	// Events are taken from the window, and only while the input is on.
	ImGui::BeginDisabled(!bInputActive || CurrentWindow == nullptr);
	if (RenderGroupTitle("Events from the window", EventNote))
	{
		const char* RowNames[] = { "Mouse", "Keyboard", "Files" };
		const int RowStarts[] = { 0, 3, 5, VirtualUIEventKindCount };
		for (int Row = 0; Row < 3; Row++)
		{
			RenderRowLabel(RowNames[Row], LabelWidth);
			for (int i = RowStarts[Row]; i < RowStarts[Row + 1]; i++)
			{
				const bool bPassedThrough = (VirtualUIComponent.*VirtualUIEventKinds[i].IsPassedThrough)();
				if (RenderSwitchButton(VirtualUIEventKinds[i].Name, bPassedThrough, ImDrawFlags_RoundCornersAll))
					(VirtualUIComponent.*VirtualUIEventKinds[i].SetPassThrough)(!bPassedThrough);

				if (i < RowStarts[Row + 1] - 1)
					ImGui::SameLine(0.0f, FieldSpacing);
			}
		}
	}
	ImGui::EndDisabled();

	if (CurrentWindow == nullptr)
	{
		RenderHintText("There is no window to take events from.");
	}
	else if (!bInputActive)
	{
		RenderHintText("Events are passed to the interface only while Input is on.");
	}

	ImGui::PopStyleVar();
}

void FEEditorInspectorWindow::AddGameModelComponent(FEEntity* Entity)
{
	INSPECTOR_WINDOW.EntityToWorkWith = Entity;
	SELECT_FEOBJECT_POPUP.Show(FE_GAMEMODEL, AddNewGameModelComponentCallback);
}

void FEEditorInspectorWindow::AddPointCloudComponent(FEEntity* Entity)
{
	Entity->AddComponent<FEPointCloudComponent>();
}

void FEEditorInspectorWindow::AddNewGameModelComponentCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (!SelectionsResult.empty() && SelectionsResult[0]->GetType() == FE_GAMEMODEL)
	{
		FEGameModel* SelectedGameModel = RESOURCE_MANAGER.GetGameModel(SelectionsResult[0]->GetID());
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
	TERRAIN_SYSTEM.SetHeightMap(RESOURCE_MANAGER.CreateBlankHeightMapTexture(1024, 1024), Entity);
}

void FEEditorInspectorWindow::AddInstancedComponent(FEEntity* Entity)
{
	Entity->AddComponent<FEInstancedComponent>();
}

void FEEditorInspectorWindow::AddNativeScriptComponent(FEEntity* Entity)
{
	Entity->AddComponent<FENativeScriptComponent>();
}

void FEEditorInspectorWindow::AddVolumeComponent(FEEntity* Entity)
{
	Entity->AddComponent<FEVolumeComponent>();
}

template<typename T>
constexpr bool bScriptVariableHasEditor = std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, std::string> ||
										  std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4> || std::is_same_v<T, FEPrefab*>;

// Field takes the rest of the row, WidthToLeave is kept free on the right. Returns true if value was changed.
template<typename T>
bool RenderScriptVariableEditor(const std::string ID, T& Value, const float WidthToLeave = 0.0f)
{
	const float Width = ImGui::GetContentRegionAvail().x - WidthToLeave;
	if constexpr (std::is_same_v<T, float>)
	{
		ImGui::SetNextItemWidth(Width);
		return ImGui::DragFloat(ID.c_str(), &Value, std::max(0.001f, std::abs(Value) * 0.01f));
	}
	else if constexpr (std::is_same_v<T, int>)
	{
		ImGui::SetNextItemWidth(Width);
		return ImGui::DragInt(ID.c_str(), &Value);
	}
	else if constexpr (std::is_same_v<T, bool>)
	{
		return ImGui::Checkbox(ID.c_str(), &Value);
	}
	else if constexpr (std::is_same_v<T, std::string>)
	{
		ImGui::SetNextItemWidth(Width);
		return ImGui::InputText(ID.c_str(), &Value);
	}
	else if constexpr (std::is_same_v<T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>)
	{
		const ImU32 AxisColors[] = { FEEditorStyle::AxisX, FEEditorStyle::AxisY, FEEditorStyle::AxisZ, FEEditorStyle::TextMuted };
		const int VectorCardinality = std::is_same_v<T, glm::vec2> ? 2 : std::is_same_v<T, glm::vec3> ? 3 : 4;
		const float FieldSpacing = 6.0f;
		const float FieldWidth = floorf((Width - FieldSpacing * (VectorCardinality - 1)) / VectorCardinality);

		bool bChanged = false;
		ImGui::PushID(ID.c_str());
		for (int i = 0; i < VectorCardinality; i++)
		{
			ImGui::PushID(i);
			ImGui::SetNextItemWidth(FieldWidth);
			if (ImGui::DragFloat("##Value", &Value[i], 0.1f))
				bChanged = true;

			const ImVec2 FieldMin = ImGui::GetItemRectMin();
			const ImVec2 FieldMax = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(FieldMin, ImVec2(FieldMin.x + 4.0f, FieldMax.y), AxisColors[i], 4.0f, ImDrawFlags_RoundCornersLeft);
			ImGui::PopID();

			if (i < VectorCardinality - 1)
				ImGui::SameLine(0.0f, FieldSpacing);
		}
		ImGui::PopID();

		return bChanged;
	}
	else if constexpr (std::is_same_v<T, FEPrefab*>)
	{
		bool bChanged = false;
		ImGui::SetNextItemWidth(Width);
		if (ImGui::BeginCombo(ID.c_str(), Value != nullptr ? Value->GetName().c_str() : "None"))
		{
			if (ImGui::Selectable("None", Value == nullptr))
			{
				Value = nullptr;
				bChanged = true;
			}

			std::vector<FEUUID> PrefabIDList = RESOURCE_MANAGER.GetPrefabIDList();
			for (size_t i = 0; i < PrefabIDList.size(); i++)
			{
				FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabIDList[i]);
				bool bIsSelected = CurrentPrefab == Value;
				ImGui::PushID(UNIQUE_ID.ToString(PrefabIDList[i]).c_str());
				if (ImGui::Selectable(CurrentPrefab->GetName().c_str(), bIsSelected))
				{
					Value = CurrentPrefab;
					bChanged = true;
				}
				ImGui::PopID();

				if (bIsSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		return bChanged;
	}
	else
	{
		return false;
	}
}

// FE_TO_DO: Make it more general with more templated magic.
template<typename T>
void HandleScriptVariable(FENativeScriptComponent& Component, const std::string VariableName, const float LabelWidth)
{
	if constexpr (bScriptVariableHasEditor<T>)
	{
		T Value;
		if (!Component.GetVariableValue(VariableName, Value))
			return;

		RenderRowLabel(VariableName.c_str(), LabelWidth);
		if (RenderScriptVariableEditor("##" + VariableName, Value))
			Component.SetVariableValue(VariableName, Value);
	}
}

// FE_TO_DO: Make it more general with more templated magic.
template<typename T>
void HandleScriptArrayVariable(FENativeScriptComponent& Component, const std::string VariableName, const float LabelWidth)
{
	if constexpr (bScriptVariableHasEditor<T>)
	{
		std::vector<T> Value;
		if (!Component.GetVariableValue(VariableName, Value))
			return;

		const float ButtonSize = ImGui::GetFrameHeight();
		const float ButtonSpacing = 6.0f;

		RenderRowLabel(VariableName.c_str(), LabelWidth);
		if (ImGui::Button(("+##" + VariableName).c_str(), ImVec2(ButtonSize, ButtonSize)))
		{
			Value.push_back(T());
			Component.SetVariableValue(VariableName, Value);
		}
		ShowToolTip("Add element");
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::TextDisabled(Value.size() == 1 ? "%d element" : "%d elements", static_cast<int>(Value.size()));

		const float ElementIndent = 16.0f;
		const float ElementLabelWidth = ElementIndent + ImGui::CalcTextSize("[00]").x + 10.0f;
		for (size_t i = 0; i < Value.size(); i++)
		{
			std::string ElementName = "##" + VariableName + "[" + std::to_string(i) + "]";
			RenderRowLabel(("[" + std::to_string(i) + "]").c_str(), ElementLabelWidth, ElementIndent);
			if constexpr (std::is_same_v<T, bool>)
			{
				// std::vector<bool> is a special case.
				bool bTemporaryValue = Value[i];
				if (RenderScriptVariableEditor(ElementName, bTemporaryValue, ButtonSize + ButtonSpacing))
				{
					Value[i] = bTemporaryValue;
					Component.SetVariableValue(VariableName, Value);
				}
			}
			else
			{
				if (RenderScriptVariableEditor(ElementName, Value[i], ButtonSize + ButtonSpacing))
					Component.SetVariableValue(VariableName, Value);
			}

			ImGui::SameLine(0.0f, ButtonSpacing);
			if (ImGui::Button(("x" + ElementName).c_str(), ImVec2(ButtonSize, ButtonSize)))
			{
				Value.erase(Value.begin() + i);
				Component.SetVariableValue(VariableName, Value);
				break;
			}
			ShowToolTip("Remove element");
		}
	}
}

static std::string GetScriptModuleName(const FEUUID& ModuleID)
{
	auto ModuleIterator = NATIVE_SCRIPT_SYSTEM.ActiveModules.find(ModuleID);
	if (ModuleIterator == NATIVE_SCRIPT_SYSTEM.ActiveModules.end() || ModuleIterator->second == nullptr || ModuleIterator->second->GetName().empty())
		return UNIQUE_ID.ToString(ModuleID);

	return ModuleIterator->second->GetName();
}

// Combo with a filter that lists scripts of all active modules. Returns true if script of the entity was changed.
static bool RenderScriptSelection(FEEntity* Entity)
{
	static std::string Filter;

	FENativeScriptComponent& Component = Entity->GetComponent<FENativeScriptComponent>();
	const bool bInitialized = Component.IsInitialized();
	const std::string CurrentScriptName = bInitialized ? Component.GetScriptData()->Name : "";
	const FEUUID CurrentModuleID = bInitialized ? Component.GetModuleID() : FEUUID();

	FEUUID ChosenModuleID;
	std::string ChosenScriptName;

	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::PushStyleColor(ImGuiCol_Text, bInitialized ? FEEditorStyle::Text : FEEditorStyle::TextMuted);
	const bool bListOpen = ImGui::BeginCombo("##ScriptSelection", bInitialized ? CurrentScriptName.c_str() : "Select script...", ImGuiComboFlags_HeightLarge);
	ImGui::PopStyleColor();
	if (bListOpen)
	{
		if (ImGui::IsWindowAppearing())
		{
			Filter = "";
			ImGui::SetKeyboardFocusHere();
		}

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##ScriptFilter", "Filter scripts...", &Filter);

		const std::string LowerCaseFilter = ToLowerCase(Filter);
		bool bAnyScriptShown = false;
		std::vector<FEUUID> ModuleList = NATIVE_SCRIPT_SYSTEM.GetActiveModuleIDList();
		for (size_t i = 0; i < ModuleList.size(); i++)
		{
			std::vector<std::string> ScriptList = NATIVE_SCRIPT_SYSTEM.GetActiveModuleScriptNameList(ModuleList[i]);
			std::sort(ScriptList.begin(), ScriptList.end());

			bool bModuleNameShown = false;
			for (size_t j = 0; j < ScriptList.size(); j++)
			{
				if (!LowerCaseFilter.empty() && ToLowerCase(ScriptList[j]).find(LowerCaseFilter) == std::string::npos)
					continue;

				if (!bModuleNameShown)
				{
					ImGui::PushFont(nullptr, 16.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Accent);
					ImGui::TextUnformatted(GetScriptModuleName(ModuleList[i]).c_str());
					ImGui::PopStyleColor();
					ImGui::PopFont();
					bModuleNameShown = true;
				}

				bAnyScriptShown = true;
				const bool bCurrent = ScriptList[j] == CurrentScriptName && ModuleList[i] == CurrentModuleID;
				if (ImGui::Selectable((ScriptList[j] + "##" + UNIQUE_ID.ToString(ModuleList[i])).c_str(), bCurrent))
				{
					ChosenModuleID = ModuleList[i];
					ChosenScriptName = ScriptList[j];
				}
			}
		}

		if (!bAnyScriptShown)
			ImGui::TextDisabled("No script with this name");

		ImGui::EndCombo();
	}

	if (ChosenScriptName.empty() || (ChosenScriptName == CurrentScriptName && ChosenModuleID == CurrentModuleID))
		return false;

	// Component can be initialized only once, so to change the script it is replaced.
	if (bInitialized)
	{
		Entity->RemoveComponent<FENativeScriptComponent>();
		Entity->AddComponent<FENativeScriptComponent>();
	}

	NATIVE_SCRIPT_SYSTEM.InitializeScriptComponent(Entity, ChosenModuleID, ChosenScriptName);
	return true;
}

void FEEditorInspectorWindow::DisplayNativeScriptProperties(FEEntity* NativeScriptEntity) const
{
	FENativeScriptComponent& NativeScriptComponent = NativeScriptEntity->GetComponent<FENativeScriptComponent>();

	if (!NativeScriptComponent.IsInitialized() && NativeScriptComponent.IsFailedToLoad())
	{
		ImGui::Text("Script failed to load.");
		FENativeScriptFailedToLoadData* FailedToLoadData = NativeScriptComponent.GetFailedToLoadData();
		ImGui::Text(("Module ID: " + UNIQUE_ID.ToString(FailedToLoadData->GetModuleID())).c_str());
		// And show raw data in multiline text box.
		Json::Value Data = FailedToLoadData->GetRawData();
		std::string RawData = Data.toStyledString();

		static char RawDataBuffer[10240] = {};

		strncpy_s(RawDataBuffer, RawData.c_str(), sizeof(RawDataBuffer) - 1);

		ImGui::InputTextMultiline("##FailedToLoadData", RawDataBuffer, sizeof(RawDataBuffer), ImVec2(500, 500), ImGuiInputTextFlags_ReadOnly);
		return;
	}

	std::vector<std::string> VariableNames;
	if (NativeScriptComponent.IsInitialized())
	{
		// Registry is not ordered, names are sorted to keep the same order of rows.
		for (const auto& Variable : NativeScriptComponent.GetScriptData()->VariablesRegistry)
			VariableNames.push_back(Variable.first);

		std::sort(VariableNames.begin(), VariableNames.end());
	}

	float LabelWidth = ImGui::CalcTextSize("Module ID").x;
	for (size_t i = 0; i < VariableNames.size(); i++)
		LabelWidth = std::max(LabelWidth, ImGui::CalcTextSize(VariableNames[i].c_str()).x);

	LabelWidth = std::min(LabelWidth + 14.0f, floorf(ImGui::GetContentRegionAvail().x / 2.0f));

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 6.0f));
	RenderRowLabel("Script", LabelWidth);
	// Component is replaced when the script is changed, so it is not used after that.
	const bool bScriptChanged = RenderScriptSelection(NativeScriptEntity);
	if (!bScriptChanged && NativeScriptComponent.IsInitialized())
	{
		FEUUID ModuleID = NativeScriptComponent.GetModuleID();
		const FEScriptData* ScriptData = NativeScriptComponent.GetScriptData();

		RenderRowLabel("Module", LabelWidth);
		ImGui::TextUnformatted(GetScriptModuleName(ModuleID).c_str());

		RenderRowLabel("Module ID", LabelWidth);
		RenderIDText(UNIQUE_ID.ToString(ModuleID));

		RenderRowLabel("Runs in", LabelWidth);
		ImGui::TextUnformatted(ScriptData->bRunInEditor ? "editor and game mode" : "game mode only");

		if (!VariableNames.empty())
			ImGui::Separator();

		for (size_t i = 0; i < VariableNames.size(); i++)
		{
			std::string VariableName = VariableNames[i];
			std::string VariableType = ScriptData->VariablesRegistry.at(VariableName).Type;

			// TO-DO: Make it more general with more templated magic.
			if (VariableType.find("vector<") != std::string::npos || VariableType.find("std::vector<") != std::string::npos)
			{
				if (VariableType.find("float>") != std::string::npos) HandleScriptArrayVariable<float>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("int>") != std::string::npos) HandleScriptArrayVariable<int>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("bool>") != std::string::npos) HandleScriptArrayVariable<bool>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("std::string>") != std::string::npos) HandleScriptArrayVariable<std::string>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("glm::vec2>") != std::string::npos) HandleScriptArrayVariable<glm::vec2>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("glm::vec3>") != std::string::npos) HandleScriptArrayVariable<glm::vec3>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("glm::vec4>") != std::string::npos) HandleScriptArrayVariable<glm::vec4>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEShader*>") != std::string::npos) HandleScriptArrayVariable<FEShader*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEMesh*>") != std::string::npos) HandleScriptArrayVariable<FEMesh*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FETexture*>") != std::string::npos) HandleScriptArrayVariable<FETexture*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEMaterial*>") != std::string::npos) HandleScriptArrayVariable<FEMaterial*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEGameModel*>") != std::string::npos) HandleScriptArrayVariable<FEGameModel*>(NativeScriptComponent, VariableName, LabelWidth);
				// TO-DO: Think how to make it work with enitity. Right now it is not consistent between editor and game mode scenes.
				//else if (VariableType.find("FEEntity*>") != std::string::npos) HandleScriptArrayVariable<FEEntity*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEFramebuffer*>") != std::string::npos) HandleScriptArrayVariable<FEFramebuffer*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEPostProcess*>") != std::string::npos) HandleScriptArrayVariable<FEPostProcess*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEPrefab*>") != std::string::npos) HandleScriptArrayVariable<FEPrefab*>(NativeScriptComponent, VariableName, LabelWidth);
				// TO-DO: Check if it is working.
				else if (VariableType.find("FEScene*>") != std::string::npos) HandleScriptArrayVariable<FEScene*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FEAssetPackage*>") != std::string::npos) HandleScriptArrayVariable<FEAssetPackage*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType.find("FENativeScriptModule*>") != std::string::npos) HandleScriptArrayVariable<FENativeScriptModule*>(NativeScriptComponent, VariableName, LabelWidth);
			}
			else
			{
				if (VariableType == "float") HandleScriptVariable<float>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "int") HandleScriptVariable<int>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "bool") HandleScriptVariable<bool>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "std::string") HandleScriptVariable<std::string>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "glm::vec2") HandleScriptVariable<glm::vec2>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "glm::vec3") HandleScriptVariable<glm::vec3>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "glm::vec4") HandleScriptVariable<glm::vec4>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEShader*") HandleScriptVariable<FEShader*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEMesh*") HandleScriptVariable<FEMesh*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FETexture*") HandleScriptVariable<FETexture*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEMaterial*") HandleScriptVariable<FEMaterial*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEGameModel*") HandleScriptVariable<FEGameModel*>(NativeScriptComponent, VariableName, LabelWidth);
				// TO-DO: Think how to make it work with enitity. Right now it is not consistent between editor and game mode scenes.
				//else if (VariableType == "FEEntity*") HandleScriptVariable<FEEntity*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEFramebuffer*") HandleScriptVariable<FEFramebuffer*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEPostProcess*") HandleScriptVariable<FEPostProcess*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEPrefab*") HandleScriptVariable<FEPrefab*>(NativeScriptComponent, VariableName, LabelWidth);
				// TO-DO: Check if it is working.
				else if (VariableType == "FEScene*") HandleScriptVariable<FEScene*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FEAssetPackage*") HandleScriptVariable<FEAssetPackage*>(NativeScriptComponent, VariableName, LabelWidth);
				else if (VariableType == "FENativeScriptModule*") HandleScriptVariable<FENativeScriptModule*>(NativeScriptComponent, VariableName, LabelWidth);
			}
		}
	}
	ImGui::PopStyleVar();
}