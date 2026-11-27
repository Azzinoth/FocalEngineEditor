#include "VolumetricTextureViewWindow.h"
#include "SceneGraphWindow.h"
#include "TextureViewWindow.h"
#include "../FEEditor.h"

FEScene* VolumetricTextureViewWindow::CreateViewScene(FETexture* TextureToView)
{
	if (TextureToView == nullptr || TextureToView->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		return nullptr;

	return SCENE_MANAGER.CreateScene("Volumetric Texture View Scene", FEUUID(), FESceneFlag::Active | FESceneFlag::EditorMode | FESceneFlag::Renderable);
}

VolumetricTextureViewWindow::VolumetricTextureViewWindow(FETexture* TextureToView) : FEEditorSceneWindow(CreateViewScene(TextureToView))
{
	if (TextureToView == nullptr || TextureToView->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		return;

	TextureToViewID = TextureToView->GetID();
	bSelfContained = false;

	MaterialFor3DTextures = RESOURCE_MANAGER.CreateNewMaterial("MaterialFor3DTexturesView");
	MaterialFor3DTextures->SetMaterialType(FEMaterialType::Volumetric);
	MaterialFor3DTextures->SetBlendMode(FEMaterialBlendMode::Additive);
	MaterialFor3DTextures->SetShader(VOLUME_SYSTEM.GetVolumetricShaders()[0]);
	MaterialFor3DTextures->SetTextureOverride("VolumeTexture", TextureToView->GetID());
	RESOURCE_MANAGER.SetTag(MaterialFor3DTextures, EDITOR_RESOURCE_TAG);

	FEEntity* VolumeEntity = Scene->CreateEntity("Volumetric texture");
	VolumeEntity->AddComponent<FEVolumeComponent>();
	VolumeEntity->GetComponent<FEVolumeComponent>().SetMaterial(MaterialFor3DTextures);

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
		ViewCameraID = ViewCamera->GetID();

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
	CloseButton->SetDefaultColor(FEEditorStyle::ToVec4(FEEditorStyle::DangerButton));

	AcceptedTypes.clear();
	ToolTipTexts.clear();
}

VolumetricTextureViewWindow::~VolumetricTextureViewWindow()
{
	delete CloseButton;

	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject != nullptr)
		CurrentProject->SceneIDToEditorCameraID.erase(SceneID);

	DeleteSceneAndEditorData();

	delete MaterialFor3DTextures;
}

void VolumetricTextureViewWindow::Render()
{
	if (UNIQUE_ID.IsNull(TextureToViewID) || RESOURCE_MANAGER.GetTexture(TextureToViewID) == nullptr)
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
	const size_t VoxelCount = static_cast<size_t>(Width) * static_cast<size_t>(Height) * static_cast<size_t>(Depth);
	const size_t ResourceSizeBytes = VoxelCount * TextureViewWindow::EstimateBytesPerPixel(InternalFormat);
	const std::string ID = UNIQUE_ID.ToString(Texture->GetID());

	// The flipbook converter only supports single-channel float volumes.
	const bool bFormatSupported = (InternalFormat == GL_R16 || InternalFormat == GL_R32F);

	const float LabelWidth = 110.0f;
	const float SmallFontSize = 16.0f;
	const float PanelWidth  = 480.0f;
	const float PanelMargin = 8.0f;
	const float PanelX = GetWindow()->ContentRegionRect.GetWidth() - PanelWidth - PanelMargin;
	// A window that is not docked has a title bar above its content.
	ImGui::SetCursorPos(ImVec2(PanelX, std::max(PanelMargin, ImGui::GetCursorStartPos().y)));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.10f, 0.10f, 0.85f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
	ImGui::BeginChild("##VolumeTextureInfo", ImVec2(PanelWidth, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	TextureViewWindow::RenderPanelTitle("Volume texture");
	TextureViewWindow::RenderFactRow("Name", Texture->GetName(), LabelWidth);
	TextureViewWindow::RenderFactRow("ID", ID, LabelWidth, ID + "\nClick to copy", true);
	if (ImGui::IsItemClicked())
		ImGui::SetClipboardText(ID.c_str());

	TextureViewWindow::RenderFactRow("Size", GetNumberWithSeparators(Width) + " x " + GetNumberWithSeparators(Height) + " x " + GetNumberWithSeparators(Depth), LabelWidth);
	TextureViewWindow::RenderFactRow("Format", TextureViewWindow::GetFormatName(InternalFormat), LabelWidth, FETexture::TextureInternalFormatToString(InternalFormat));
	TextureViewWindow::RenderFactRow("Voxels", GetNumberWithSeparators(VoxelCount), LabelWidth);
	TextureViewWindow::RenderFactRow("In memory", TextureViewWindow::ReadableSize(ResourceSizeBytes), LabelWidth);

	ImGui::Dummy(ImVec2(0.0f, 6.0f));
	TextureViewWindow::RenderPanelTitle("Sampling");

	const char* FilterOptions[] = { "Nearest", "Linear" };
	RenderRowLabel("Filtering", LabelWidth);
	ImGui::SetNextItemWidth(-1.0f);
	int CurrentFilter = static_cast<int>(Texture->GetFilterType());
	if (ImGui::Combo("##Filtering", &CurrentFilter, FilterOptions, IM_ARRAYSIZE(FilterOptions)))
		Texture->SetFilterType(static_cast<FE_TEXTURE_MINMAG_FILTER_TYPE>(CurrentFilter));

	const char* WrapOptions[] = { "Repeat", "Mirrored repeat", "Clamp to edge", "Clamp to border" };
	RenderRowLabel("Wrap U", LabelWidth);
	ImGui::SetNextItemWidth(-1.0f);
	int CurrentWrapU = static_cast<int>(Texture->GetUWrapType());
	if (ImGui::Combo("##WrapU", &CurrentWrapU, WrapOptions, IM_ARRAYSIZE(WrapOptions)))
		Texture->SetUWrapType(static_cast<FE_TEXTURE_WRAP_TYPE>(CurrentWrapU));

	RenderRowLabel("Wrap V", LabelWidth);
	ImGui::SetNextItemWidth(-1.0f);
	int CurrentWrapV = static_cast<int>(Texture->GetVWrapType());
	if (ImGui::Combo("##WrapV", &CurrentWrapV, WrapOptions, IM_ARRAYSIZE(WrapOptions)))
		Texture->SetVWrapType(static_cast<FE_TEXTURE_WRAP_TYPE>(CurrentWrapV));

	RenderRowLabel("Wrap W", LabelWidth);
	ImGui::SetNextItemWidth(-1.0f);
	int CurrentWrapW = static_cast<int>(Texture->GetWWrapType());
	if (ImGui::Combo("##WrapW", &CurrentWrapW, WrapOptions, IM_ARRAYSIZE(WrapOptions)))
		Texture->SetWWrapType(static_cast<FE_TEXTURE_WRAP_TYPE>(CurrentWrapW));

	ImGui::Dummy(ImVec2(0.0f, 6.0f));
	TextureViewWindow::RenderPanelTitle("Convert");

	ImGui::BeginDisabled(!bFormatSupported);
	if (ImGui::Button("Convert to 2D flipbook", ImVec2(-1.0f, 0.0f)))
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

	ImGui::PushFont(nullptr, SmallFontSize);
	if (!bFormatSupported)
		ImGui::TextDisabled("Only R16 / R32F volumes can be converted.");

	if (!ConvertStatusMessage.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, bConvertSucceeded ? FEEditorStyle::Success : FEEditorStyle::Danger);
		ImGui::TextWrapped("%s", ConvertStatusMessage.c_str());
		ImGui::PopStyleColor();
	}
	ImGui::PopFont();

	ImGui::EndChild();
	ImGui::PopStyleVar();
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

	if (CurrentProject->SceneIDToEditorCameraID.find(Scene->GetID()) != CurrentProject->SceneIDToEditorCameraID.end())
	{
		FEEntity* CameraEntity = Scene->GetEntity(CurrentProject->SceneIDToEditorCameraID[Scene->GetID()]);
		if (CameraEntity != nullptr)
		{
			LOG.Add("VolumetricTextureViewWindow::InjectModelViewCamera: Editor camera already exists in scene " + Scene->GetName(), "FE_LOG_LOADING", FE_LOG_WARNING);
			return CameraEntity;
		}
		else
		{
			CurrentProject->SceneIDToEditorCameraID.erase(Scene->GetID());
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
	CurrentProject->SceneIDToEditorCameraID[Scene->GetID()] = CameraEntity->GetID();

	return CameraEntity;
}