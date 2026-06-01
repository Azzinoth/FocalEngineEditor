#include "VolumetricTextureViewWindow.h"
#include "SceneGraphWindow.h"
#include "TextureViewWindow.h"
#include "../FEEditor.h"

FEScene* VolumetricTextureViewWindow::CreateViewScene(FETexture* TextureToView)
{
	if (TextureToView == nullptr || TextureToView->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		return nullptr;

	return SCENE_MANAGER.CreateScene("Volumetric Texture View Scene", "", FESceneFlag::Active | FESceneFlag::EditorMode | FESceneFlag::Renderable);
}

VolumetricTextureViewWindow::VolumetricTextureViewWindow(FETexture* TextureToView) : FEEditorSceneWindow(CreateViewScene(TextureToView))
{
	if (TextureToView == nullptr || TextureToView->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		return;

	TextureToViewID = TextureToView->GetObjectID();
	bSelfContained = false;

	FEEntity* VolumeEntity = Scene->CreateEntity("Volumetric texture");
	VolumeEntity->AddComponent<FEVolumeComponent>();
	VolumeEntity->GetComponent<FEVolumeComponent>().SetVolumetricTexture(TextureToView);

	FEEntity* SkyDomeEntity = Scene->CreateEntity("Volumetric view skydome");
	RESOURCE_MANAGER.SetTag(SkyDomeEntity, EDITOR_RESOURCE_TAG);
	SkyDomeEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(100.0f));
	SkyDomeEntity->AddComponent<FESkyDomeComponent>();

	FEEntity* LightEntity = Scene->CreateEntity("Volumetric view light");
	RESOURCE_MANAGER.SetTag(LightEntity, EDITOR_RESOURCE_TAG);
	LightEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	LightEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();
	LightComponent.SetIntensity(4.3f);
	LightComponent.SetCastShadows(false);

	FEEntity* ViewCamera = InjectModelViewCamera(Scene);
	if (ViewCamera != nullptr)
	{
		ViewCameraID = ViewCamera->GetObjectID();

		FEAABB VolumeAABB = Scene->GetEntityAABB(VolumeEntity);
		float LongestAxis = VolumeAABB.GetLongestAxisLength();

		if (ViewCamera->HasComponent<FENativeScriptComponent>())
		{
			FENativeScriptComponent& CameraScript = ViewCamera->GetComponent<FENativeScriptComponent>();
			CameraScript.SetVariableValue("TargetPosition", VolumeAABB.GetCenter());
			CameraScript.SetVariableValue("DistanceToModel", LongestAxis > 1.0f ? LongestAxis * 2.5f : 2.5f);
			CAMERA_SYSTEM.IndividualUpdate(ViewCamera, 0.0);
		}
	}

	CloseButton = new ImGuiButton("Close");
	CloseButton->SetSize(ImVec2(140, 24));
	CloseButton->SetDefaultColor(ImVec4(0.8f, 0.2f, 0.2f, 1.0f));

	AcceptedTypes.clear();
	ToolTipTexts.clear();
}

VolumetricTextureViewWindow::~VolumetricTextureViewWindow()
{
	delete CloseButton;

	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject != nullptr)
		CurrentProject->SceneIDToEditorCameraID.erase(SceneID);

	// Using ID instead of Scene pointer because it's possible that scene was already deleted.
	SCENE_MANAGER.DeleteScene(SceneID);
}

void VolumetricTextureViewWindow::Render()
{
	if (TextureToViewID.empty() || RESOURCE_MANAGER.GetTexture(TextureToViewID) == nullptr)
		return;

	FEEditorSceneWindow::Render();

	if (!IsVisible())
		return;

	RenderInfoOverlay();

	CloseButton->SetSize(ImVec2(GetWindow()->ContentRegionRect.GetWidth() / 10.0f, GetWindow()->Rect().GetHeight() / 20.0f));
	CloseButton->SetPosition(ImVec2(GetWindow()->ContentRegionRect.GetWidth() - CloseButton->GetSize().x - 5.0f,
									GetWindow()->Rect().GetHeight() - CloseButton->GetSize().y - 5.0f));
	CloseButton->Render();
	if (CloseButton->IsClicked())
	{
		bUserRequestedClose = true;
		Close();
	}

	FEEditorSceneWindow::OnRenderEnd();
}

void VolumetricTextureViewWindow::RenderInfoOverlay()
{
	FETexture* Texture = RESOURCE_MANAGER.GetTexture(TextureToViewID);
	if (Texture == nullptr)
		return;

	const int Width  = Texture->GetWidth();
	const int Height = Texture->GetHeight();
	const int Depth  = Texture->GetDepth();
	const GLint InternalFormat = Texture->GetInternalFormat();
	const std::string ResolvedFormat = FETexture::TextureInternalFormatToString(InternalFormat);
	const std::string FormatString = ResolvedFormat.empty() ? ("Unknown (" + std::to_string(InternalFormat) + ")") : ResolvedFormat;
	const size_t VoxelCount = static_cast<size_t>(Width) * static_cast<size_t>(Height) * static_cast<size_t>(Depth);
	const size_t ResourceSizeBytes = VoxelCount * TextureViewWindow::EstimateBytesPerPixel(InternalFormat);

	// The flipbook converter only supports single-channel float volumes.
	const bool bFormatSupported = (InternalFormat == GL_R16 || InternalFormat == GL_R32F);

	const float LineHeight = ImGui::GetTextLineHeightWithSpacing();
	const float PanelWidth  = 480.0f;
	const float PanelMargin = 8.0f;
	float PanelHeight = ImGui::GetStyle().WindowPadding.y * 2.0f
					  + LineHeight * 8.0f
					  + LineHeight * 1.5f
					  + ImGui::GetFrameHeightWithSpacing();
	if (!bFormatSupported)
		PanelHeight += LineHeight;
	if (!ConvertStatusMessage.empty())
		PanelHeight += LineHeight * 2.0f;

	const float PanelX = GetWindow()->ContentRegionRect.GetWidth() - PanelWidth - PanelMargin;
	ImGui::SetCursorPos(ImVec2(PanelX, PanelMargin));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.10f, 0.85f));
	ImGui::BeginChild("##VolumeTextureInfo", ImVec2(PanelWidth, PanelHeight), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::TextColored(ImVec4(0.95f, 0.90f, 0.0f, 1.0f), "Volume Texture");
	ImGui::Separator();
	ImGui::Spacing();

	auto DrawRow = [](const char* Label, const std::string& Value)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Label);
		ImGui::TableSetColumnIndex(1);
		ImGui::TextWrapped("%s", Value.c_str());
	};

	const ImGuiTableFlags TableFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoBordersInBody;
	if (ImGui::BeginTable("##VolumeInfoTable", 2, TableFlags))
	{
		ImGui::TableSetupColumn("##Label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);

		DrawRow("Name:", Texture->GetName());
		DrawRow("Object ID:", Texture->GetObjectID());
		DrawRow("Dimensions:", std::to_string(Width) + " x " + std::to_string(Height) + " x " + std::to_string(Depth));
		DrawRow("Format:", FormatString);
		DrawRow("Voxels:", std::to_string(VoxelCount));
		DrawRow("Resource size:", TextureViewWindow::ReadableSize(ResourceSizeBytes));

		ImGui::EndTable();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::TextColored(ImVec4(0.95f, 0.90f, 0.0f, 1.0f), "Convert");

	ImGui::BeginDisabled(!bFormatSupported);
	if (ImGui::Button("Convert to 2D Flipbook", ImVec2(-1.0f, 0.0f)))
	{
		int Columns = 0;
		int Rows = 0;
		FETexture* ResultTexture = RESOURCE_MANAGER.Convert3DTextureToFlipbook2D(Texture, Columns, Rows);
		if (ResultTexture != nullptr)
		{
			VIRTUAL_FILE_SYSTEM.CreateFile(ResultTexture, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
			if (PROJECT_MANAGER.GetCurrent() != nullptr)
			{
				PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(ResultTexture);
				PROJECT_MANAGER.GetCurrent()->SetModified(true);
			}

			bConvertSucceeded = true;
			ConvertStatusMessage = "Created 2D flipbook \"" + ResultTexture->GetName() + "\" (" + std::to_string(Columns) + " x " + std::to_string(Rows) + " grid).";
		}
		else
		{
			bConvertSucceeded = false;
			ConvertStatusMessage = "Conversion failed. See the log for details.";
		}
	}
	ImGui::EndDisabled();

	if (!bFormatSupported)
		ImGui::TextDisabled("Only R16 / R32F volumes can be converted.");

	if (!ConvertStatusMessage.empty())
	{
		const ImVec4 StatusColor = bConvertSucceeded ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f) : ImVec4(0.95f, 0.5f, 0.5f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, StatusColor);
		ImGui::TextWrapped("%s", ConvertStatusMessage.c_str());
		ImGui::PopStyleColor();
	}

	ImGui::EndChild();
	ImGui::PopStyleColor();
}

FEEntity* VolumetricTextureViewWindow::InjectModelViewCamera(FEScene* Scene)
{
	if (Scene == nullptr)
		return nullptr;

	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject == nullptr)
	{
		LOG.Add("VolumetricTextureViewWindow::InjectModelViewCamera: No project opened.", "FE_LOG_LOADING", FE_LOG_WARNING);
		return nullptr;
	}

	if (CurrentProject->SceneIDToEditorCameraID.find(Scene->GetObjectID()) != CurrentProject->SceneIDToEditorCameraID.end())
	{
		FEEntity* CameraEntity = Scene->GetEntity(CurrentProject->SceneIDToEditorCameraID[Scene->GetObjectID()]);
		if (CameraEntity != nullptr)
		{
			LOG.Add("VolumetricTextureViewWindow::InjectModelViewCamera: Editor camera already exists in scene " + Scene->GetName(), "FE_LOG_LOADING", FE_LOG_WARNING);
			return CameraEntity;
		}
		else
		{
			CurrentProject->SceneIDToEditorCameraID.erase(Scene->GetObjectID());
		}
	}

	FEEntity* CameraEntity = nullptr;
	std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Model view camera prefab");
	if (CameraPrefab.size() == 0)
	{
		LOG.Add("VolumetricTextureViewWindow::InjectModelViewCamera: Camera prefab not found! Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

		CameraEntity = Scene->CreateEntity("Volumetric texture view camera");
		CameraEntity->AddComponent<FECameraComponent>();
	}
	else
	{
		FEPrefab* CameraPrefabToUse = CameraPrefab[0];
		std::vector<FEEntity*> AddedEntities = SCENE_MANAGER.InstantiatePrefab(CameraPrefabToUse, Scene, true);
		if (AddedEntities.empty())
		{
			LOG.Add("VolumetricTextureViewWindow::InjectModelViewCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Volumetric texture view camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
		else
		{
			CameraEntity = AddedEntities[0];
		}

		if (CameraEntity == nullptr)
		{
			LOG.Add("VolumetricTextureViewWindow::InjectModelViewCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Volumetric texture view camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
	}

	RESOURCE_MANAGER.SetTag(CameraEntity, EDITOR_RESOURCE_TAG);

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.SetDistanceFogEnabled(false);
	CameraComponent.SetSSAOEnabled(false);

	CAMERA_SYSTEM.SetMainCamera(CameraEntity);

	// FEEditorSceneWindow::Render resolves the camera for EditorMode scenes through this map.
	CurrentProject->SceneIDToEditorCameraID[Scene->GetObjectID()] = CameraEntity->GetObjectID();

	return CameraEntity;
}