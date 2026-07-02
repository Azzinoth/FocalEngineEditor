#include "FEEditorPreviewManager.h"
#include "FEEditor.h"
using namespace FocalEngine;

FEEditorPreviewManager::FEEditorPreviewManager() {}
FEEditorPreviewManager::~FEEditorPreviewManager() {}

glm::vec4 FEEditorPreviewManager::OriginalClearColor = glm::vec4();
FETransformComponent FEEditorPreviewManager::OriginalTransform = FETransformComponent();

void FEEditorPreviewManager::InitializeResources()
{
	PreviewScene = SCENE_MANAGER.CreateScene("EditorPreviewScene", "", FESceneFlag::Active);
	RESOURCE_MANAGER.SetTag(PreviewScene, EDITOR_RESOURCE_TAG);

	PreviewGameModel = new FEGameModel(nullptr, nullptr, "editorPreviewGameModel");
	MeshPreviewMaterial = RESOURCE_MANAGER.CreateMaterial("meshPreviewMaterial");
	RESOURCE_MANAGER.SetTag(MeshPreviewMaterial, EDITOR_RESOURCE_TAG);
	MeshPreviewMaterial->Shader = RESOURCE_MANAGER.CreateShader("FEMeshPreviewShader", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_MeshPreview_VS.glsl").c_str(),
																					   RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_MeshPreview_FS.glsl").c_str(),
																					   nullptr,
																					   nullptr,
																					   nullptr,
																					   nullptr,
																					   "607A53601357077F03770357"/*"FEMeshPreviewShader"*/);

	RESOURCE_MANAGER.SetTag(MeshPreviewMaterial->Shader, EDITOR_RESOURCE_TAG);

	PreviewEntity = PreviewScene->CreateEntity("EditorPreviewEntity");
	PreviewEntity->AddComponent<FEGameModelComponent>(PreviewGameModel);
	RESOURCE_MANAGER.SetTag(PreviewEntity, EDITOR_RESOURCE_TAG);

	LocalCameraEntity = PreviewScene->CreateEntity("EditorPreview CameraEntity");
	LocalCameraEntity->AddComponent<FECameraComponent>();
	FECameraComponent& CameraComponent = LocalCameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.TryToSetViewportSize(128, 128);
	CameraComponent.SetDistanceFogEnabled(false);
	CAMERA_SYSTEM.SetMainCamera(LocalCameraEntity);

	LocalSunEntity = PreviewScene->CreateEntity("EditorPreview LightEntity");
	LocalSunEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = LocalSunEntity->GetComponent<FELightComponent>();
	LocalSunEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(10.0f);
	LightComponent.SetCastShadows(false);

	FEEntity* SkyDome = PreviewScene->CreateEntity("SkyDome");
	SkyDome->GetComponent<FETransformComponent>().SetScale(glm::vec3(150.0f));
	SkyDome->AddComponent<FESkyDomeComponent>();

	PreviewScene->SetFlag(FESceneFlag::Active | FESceneFlag::Renderable, false);
	PreviewScene->SetFlag(FESceneFlag::EditorMode, true);

	MaterialFor3DTextures = RESOURCE_MANAGER.CreateNewMaterial("MaterialFor3DTexturesPreview");
	MaterialFor3DTextures->SetMaterialType(FEMaterialType::Volumetric);
	MaterialFor3DTextures->SetBlendMode(FEMaterialBlendMode::Additive);
	MaterialFor3DTextures->SetShader(VOLUME_SYSTEM.GetVolumetricShaders()[0]);
	RESOURCE_MANAGER.SetTag(MaterialFor3DTextures, EDITOR_RESOURCE_TAG);
}

void FEEditorPreviewManager::ReCreateAll()
{
	Clear();

	const std::vector<std::string> MeshList = RESOURCE_MANAGER.GetMeshIDList();
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		CreateMeshPreview(MeshList[i]);
	}

	const std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialIDList();
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		FEObject* CurrentMaterial = OBJECT_MANAGER.GetFEObject(MaterialList[i]);
		if (CurrentMaterial->GetTag() != EDITOR_RESOURCE_TAG)
			CreateMaterialPreview(MaterialList[i]);
	}

	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		FEObject* CurrentGameModel = OBJECT_MANAGER.GetFEObject(GameModelList[i]);
		if (CurrentGameModel->GetTag() != EDITOR_RESOURCE_TAG)
			CreateGameModelPreview(GameModelList[i]);
	}
}

void FEEditorPreviewManager::BeforePreviewActions()
{
	PreviewScene->SetFlag(FESceneFlag::Active | FESceneFlag::Renderable, true);

	// The transform impacts the AABB. Therefore, the necessary values must be set prior to any calculations.
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, 0.0));
	PreviewEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(1.0, 1.0, 1.0));
	PreviewEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(15.0, -15.0, 0.0));
}

void FEEditorPreviewManager::AfterPreviewActions()
{
	PreviewScene->SetFlag(FESceneFlag::Active | FESceneFlag::Renderable, false);

	PreviewGameModel->Mesh = nullptr;
	PreviewGameModel->Material = nullptr;
}

void FEEditorPreviewManager::StorePreview(const std::string& ObjectID, FETexture* CameraResult)
{
	// If we are updating an existing preview we should delete the old texture.
	if (PreviewTextures.find(ObjectID) != PreviewTextures.end())
		delete PreviewTextures[ObjectID];

	if (CameraResult != nullptr)
		PreviewTextures[ObjectID] = RESOURCE_MANAGER.CreateCopyOfTexture(CameraResult);
}

void FEEditorPreviewManager::RemovePreview(const std::string& ObjectID)
{
	const auto PreviewIterator = PreviewTextures.find(ObjectID);
	if (PreviewIterator == PreviewTextures.end())
		return;

	delete PreviewIterator->second;
	PreviewTextures.erase(PreviewIterator);
}

FETexture* FEEditorPreviewManager::GetCachedPreview(const std::string& ObjectID, const std::function<void()>& CreateFunction)
{
	// If we somehow could not find the preview, we will create it.
	if (PreviewTextures.find(ObjectID) == PreviewTextures.end())
		CreateFunction();

	// If we still don't have it.
	if (PreviewTextures.find(ObjectID) == PreviewTextures.end())
		return RESOURCE_MANAGER.NoTexture;

	return PreviewTextures[ObjectID];
}

void FEEditorPreviewManager::CreateMeshPreview(const std::string MeshID)
{
	FEMesh* PreviewMesh = RESOURCE_MANAGER.GetMesh(MeshID);
	if (PreviewMesh == nullptr)
		return;

	PreviewGameModel->Mesh = PreviewMesh;
	PreviewGameModel->Material = MeshPreviewMaterial;

	BeforePreviewActions();

	FEAABB MeshAABB = PreviewEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Mesh->GetAABB();
	MeshAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
	const glm::vec3 Min = MeshAABB.GetMin();
	const glm::vec3 Max = MeshAABB.GetMax();

	const float XSize = sqrt((Max.x - Min.x) * (Max.x - Min.x));
	const float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	const float ZSize = sqrt((Max.z - Min.z) * (Max.z - Min.z));

	// Invert center point to get required translation vector for centering mesh at origin
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(Max.x - XSize / 2.0f, Max.y - YSize / 2.0f, Max.z - ZSize / 2.0f));
	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	AfterPreviewActions();

	StorePreview(MeshID, RENDERER.GetCameraResult(LocalCameraEntity));

	CheckAndUpdateIfNeededGameModelPreview(MeshID);
}

FETexture* FEEditorPreviewManager::GetMeshPreview(const std::string MeshID)
{
	// if mesh's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.GetMesh(MeshID)->IsDirty())
	{
		CreateMeshPreview(MeshID);
		// if some game model uses this mesh we should also update its preview
		const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();

		for (size_t i = 0; i < GameModelList.size(); i++)
		{
			const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);

			if (CurrentGameModel->Mesh == RESOURCE_MANAGER.GetMesh(MeshID))
				CreateGameModelPreview(CurrentGameModel->GetObjectID());
		}

		RESOURCE_MANAGER.GetMesh(MeshID)->SetDirtyFlag(false);
	}

	return GetCachedPreview(MeshID, [&]() { CreateMeshPreview(MeshID); });
}

void FEEditorPreviewManager::CreateMaterialPreview(const std::string MaterialID)
{
	FEMaterial* PreviewMaterial = RESOURCE_MANAGER.GetMaterial(MaterialID);
	if (PreviewMaterial == nullptr)
		return;

	PreviewGameModel->Mesh = RESOURCE_MANAGER.GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);
	PreviewGameModel->Material = PreviewMaterial;
	PreviewEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	BeforePreviewActions();

	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, 50.0f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	AfterPreviewActions();

	StorePreview(MaterialID, RENDERER.GetCameraResult(LocalCameraEntity));

	// Looking for all gameModels that uses this material to also update them.
	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Material == PreviewMaterial && CurrentGameModel != PreviewGameModel)
			CreateGameModelPreview(CurrentGameModel->GetObjectID());
	}

	CheckAndUpdateIfNeededGameModelPreview(MaterialID);

	//// Looking for all prefabs that uses this material to also update them.
	//const std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabIDList();
	//for (size_t i = 0; i < PrefabList.size(); i++)
	//{
	//	FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabList[i]);
	//	if (CurrentPrefab->IsUsingMaterial(MaterialID))
	//		CreatePrefabPreview(CurrentPrefab->GetObjectID());
	//}
}

FETexture* FEEditorPreviewManager::GetMaterialPreview(const std::string MaterialID)
{
	// if material's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.GetMaterial(MaterialID)->IsDirty())
	{
		CreateMaterialPreview(MaterialID);
		// if some game model uses this material we should also update its preview
		const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
		for (size_t i = 0; i < GameModelList.size(); i++)
		{
			const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);

			if (CurrentGameModel->Material == RESOURCE_MANAGER.GetMaterial(MaterialID))
			{
				CreateGameModelPreview(CurrentGameModel->GetObjectID());

				// If some Prefab uses this game model we should also update its preview.
				std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabIDList();
				for (size_t j = 0; j < PrefabList.size(); j++)
				{
					FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabList[j]);
					
					if (CurrentPrefab->IsUsingGameModel(CurrentGameModel->GetObjectID()))
						CreatePrefabPreview(CurrentPrefab->GetObjectID());
				}
			}
		}

		RESOURCE_MANAGER.GetMaterial(MaterialID)->SetDirtyFlag(false);
	}	

	return GetCachedPreview(MaterialID, [&]() { CreateMaterialPreview(MaterialID); });
}

void FEEditorPreviewManager::CreateGameModelPreview(const std::string GameModelID)
{
	const FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(GameModelID);
	if (GameModel == nullptr || GameModel->Mesh == nullptr || GameModel->Material == nullptr)
		return;

	PreviewGameModel->Mesh = GameModel->Mesh;
	PreviewGameModel->Material = GameModel->Material;
	PreviewEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	BeforePreviewActions();

	FEAABB MeshAABB = PreviewEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Mesh->GetAABB();
	MeshAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
	const glm::vec3 Min = MeshAABB.GetMin();
	const glm::vec3 Max = MeshAABB.GetMax();

	const float XSize = sqrt((Max.x - Min.x) * (Max.x - Min.x));
	const float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	const float ZSize = sqrt((Max.z - Min.z) * (Max.z - Min.z));

	// Invert center point to get required translation vector for centering mesh at origin
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(Max.x - XSize / 2.0f, Max.y - YSize / 2.0f, Max.z - ZSize / 2.0f));
	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	AfterPreviewActions();

	StorePreview(GameModelID, RENDERER.GetCameraResult(LocalCameraEntity));

	CheckAndUpdateIfNeededPrefabPreview(GameModelID);
}

void FEEditorPreviewManager::CreateGameModelPreview(const FEGameModel* GameModel, FETexture** ResultingTexture)
{
	if (GameModel == nullptr)
		return;

	PreviewGameModel->Mesh = GameModel->Mesh;
	PreviewGameModel->Material = GameModel->Material;
	PreviewEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);

	BeforePreviewActions();

	FEAABB MeshAABB = PreviewEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Mesh->GetAABB();
	MeshAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
	const glm::vec3 Min = MeshAABB.GetMin();
	const glm::vec3 Max = MeshAABB.GetMax();

	const float XSize = sqrt((Max.x - Min.x) * (Max.x - Min.x));
	const float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	const float ZSize = sqrt((Max.z - Min.z) * (Max.z - Min.z));

	// Invert center point to get required translation vector for centering mesh at origin
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(Max.x - XSize / 2.0f, Max.y - YSize / 2.0f, Max.z - ZSize / 2.0f));
	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	AfterPreviewActions();

	FETexture* CameraResult = RENDERER.GetCameraResult(LocalCameraEntity);
	if (CameraResult != nullptr)
		*ResultingTexture = RESOURCE_MANAGER.CreateCopyOfTexture(CameraResult);

	CheckAndUpdateIfNeededPrefabPreview(GameModel->GetObjectID());
}

void FEEditorPreviewManager::CheckAndUpdateIfNeededGameModelPreview(const std::string ObjectIDThatWasChanged)
{
	FEObject* ObjectThatWasChanged = OBJECT_MANAGER.GetFEObject(ObjectIDThatWasChanged);
	if (ObjectThatWasChanged == nullptr)
		return;

	if (ObjectThatWasChanged->GetType() != FE_MESH)
	{
		const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
		for (size_t i = 0; i < GameModelList.size(); i++)
		{
			const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
			if (CurrentGameModel->Material == RESOURCE_MANAGER.GetMaterial(ObjectIDThatWasChanged))
				CreateGameModelPreview(CurrentGameModel->GetObjectID());
		}
		return;
	}
	else if (ObjectThatWasChanged->GetType() != FE_MATERIAL)
	{
		const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
		for (size_t i = 0; i < GameModelList.size(); i++)
		{
			const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
			if (CurrentGameModel->Material == RESOURCE_MANAGER.GetMaterial(ObjectIDThatWasChanged))
				CreateGameModelPreview(CurrentGameModel->GetObjectID());
		}
		return;
	}
}

void FEEditorPreviewManager::CheckAndUpdateIfNeededPrefabPreview(const std::string GameModelIDThatWasChanged)
{
	FEGameModel* GameModelThatWasChanged = RESOURCE_MANAGER.GetGameModel(GameModelIDThatWasChanged);
	if (GameModelThatWasChanged == nullptr)
		return;

	std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabIDList();
	for (size_t j = 0; j < PrefabList.size(); j++)
	{
		FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabList[j]);

		if (CurrentPrefab->IsUsingGameModel(GameModelThatWasChanged->GetObjectID()))
			CreatePrefabPreview(CurrentPrefab->GetObjectID());
	}
}

FETexture* FEEditorPreviewManager::GetGameModelPreview(const std::string GameModelID)
{
	if (RESOURCE_MANAGER.GetGameModel(GameModelID) == nullptr)
	{
		LOG.Add("FEEditorPreviewManager::GetGameModelPreview could not find game model with ID: " + GameModelID, "FE_LOG_RENDERING", FE_LOG_ERROR);
		return RESOURCE_MANAGER.NoTexture;
	}

	// if game model's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.GetGameModel(GameModelID)->IsDirty())
	{
		CreateGameModelPreview(GameModelID);

		FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelID);
		CheckAndUpdateIfNeededPrefabPreview(GameModelID);

		CurrentGameModel->SetDirtyFlag(false);
	}

	// if game model's material dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.GetGameModel(GameModelID)->GetMaterial() != nullptr && RESOURCE_MANAGER.GetGameModel(GameModelID)->GetMaterial()->IsDirty())
	{
		CreateMaterialPreview(RESOURCE_MANAGER.GetGameModel(GameModelID)->GetMaterial()->GetObjectID());
		// This material could use muiltiple GM so we should update all GMs.
		UpdateAllGameModelPreviews();
		RESOURCE_MANAGER.GetGameModel(GameModelID)->GetMaterial()->SetDirtyFlag(false);
		//createGameModelPreview(gameModelID);
	}

	return GetCachedPreview(GameModelID, [&]() { CreateGameModelPreview(GameModelID); });
}

void FEEditorPreviewManager::UpdateAllGameModelPreviews()
{
	// Getting list of all game models.
	const auto GameModelsList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelsList.size(); i++)
	{
		CreateGameModelPreview(GameModelsList[i]);
	}
}

void FEEditorPreviewManager::CreatePointCloudPreview(std::string PointCloudID)
{
	FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(PointCloudID);
	if (PointCloud == nullptr)
		return;

	PreviewEntity->AddComponent<FEPointCloudComponent>(PointCloud);
	BeforePreviewActions();

	FEAABB PointCloudAABB = PointCloud->GetAABB();
	PointCloudAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
	const glm::vec3 Min = PointCloudAABB.GetMin();
	const glm::vec3 Max = PointCloudAABB.GetMax();

	const float XSize = sqrt((Max.x - Min.x) * (Max.x - Min.x));
	const float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	const float ZSize = sqrt((Max.z - Min.z) * (Max.z - Min.z));

	// Invert center point to get required translation vector for centering mesh at origin
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(Max.x - XSize / 2.0f, Max.y - YSize / 2.0f, Max.z - ZSize / 2.0f));
	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	PreviewEntity->RemoveComponent<FEPointCloudComponent>();
	AfterPreviewActions();

	StorePreview(PointCloudID, RENDERER.GetCameraResult(LocalCameraEntity));
}

FETexture* FEEditorPreviewManager::GetPointCloudPreview(std::string PointCloudID)
{
	FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(PointCloudID);
	if (PointCloud == nullptr)
	{
		LOG.Add("FEEditorPreviewManager::GetPointCloudPreview could not find point cloud with ID: " + PointCloudID, "FE_LOG_RENDERING", FE_LOG_ERROR);
		return RESOURCE_MANAGER.NoTexture;
	}

	// If point cloud dirty flag is set we need to update preview
	if (PointCloud->IsDirty())
	{
		CreatePointCloudPreview(PointCloudID);
		PointCloud->SetDirtyFlag(false);
	}

	return GetCachedPreview(PointCloudID, [&]() { CreatePointCloudPreview(PointCloudID); });
}

void FEEditorPreviewManager::CreatePrefabPreview(const std::string PrefabID)
{
	FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(PrefabID);
	if (Prefab == nullptr || Prefab->GetScene() == nullptr)
		return;

	FEScene* CurrentPrefabScene = SCENE_MANAGER.DuplicateScene(Prefab->GetScene(), "Scene: " + Prefab->GetName(), nullptr, FESceneFlag::Active);

	// Because by default camera is looking at 0,0,0 we need to place "empty" entity at 0,0,0.
	// To ensure that scene AABB would include some entity at 0,0,0.
	CurrentPrefabScene->CreateEntity("Empty entity");

	FEAABB SceneAABB = CurrentPrefabScene->GetSceneAABB([](FEEntity* Entity) -> bool {
		if (Entity->GetTag() == EDITOR_RESOURCE_TAG)
			return false;

		if (Entity->HasComponent<FESkyDomeComponent>())
			return false;

		if (Entity->HasComponent<FECameraComponent>())
			return false;

		return true;
	});

	FEEntity* Camera = CurrentPrefabScene->CreateEntity("Prefab scene camera");
	RESOURCE_MANAGER.SetTag(Camera, EDITOR_RESOURCE_TAG);
	Camera->AddComponent<FECameraComponent>();
	Camera->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, SceneAABB.GetLongestAxisLength() * 2));
	FECameraComponent& CameraComponent = Camera->GetComponent<FECameraComponent>();
	CameraComponent.TryToSetViewportSize(128, 128);
	CameraComponent.SetDistanceFogEnabled(false);
	CameraComponent.SetSSAOEnabled(false);
	CAMERA_SYSTEM.SetMainCamera(Camera);
	// To make sure that next scene FEAABB calculation will include correct camera position.
	CAMERA_SYSTEM.IndividualUpdate(Camera, 0.0);

	FEEntity* SkyDomeEntity = CurrentPrefabScene->CreateEntity("Prefab scene skydome");
	RESOURCE_MANAGER.SetTag(SkyDomeEntity, EDITOR_RESOURCE_TAG);
	SkyDomeEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(100.0f));
	SkyDomeEntity->AddComponent<FESkyDomeComponent>();

	FEEntity* LightEntity = CurrentPrefabScene->CreateEntity("Prefab scene light");
	RESOURCE_MANAGER.SetTag(LightEntity, EDITOR_RESOURCE_TAG);
	LightEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();
	LightEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(4.3f);
	SceneAABB = CurrentPrefabScene->GetSceneAABB([](FEEntity* Entity) -> bool {
		if (Entity->GetTag() == EDITOR_RESOURCE_TAG && !Entity->HasComponent<FECameraComponent>())
			return false;

		if (Entity->HasComponent<FESkyDomeComponent>())
			return false;

		return true;
	});
	LightComponent.SetShadowCoverage(SceneAABB.GetLongestAxisLength() * 2);
	LightComponent.SetCastShadows(true);

	RENDERER.Render(CurrentPrefabScene);

	StorePreview(PrefabID, RENDERER.GetCameraResult(Camera));

	SCENE_MANAGER.DeleteScene(CurrentPrefabScene);
}

void CreatePrefabPreview(FEPrefab* Prefab, FETexture** ResultingTexture)
{

}

FETexture* FEEditorPreviewManager::GetPrefabPreview(const std::string PrefabID)
{
	FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabID);
	if (CurrentPrefab == nullptr)
	{
		LOG.Add("FEEditorPreviewManager::GetPrefabPreview could not find prefab with ID: " + PrefabID, "FE_LOG_RENDERING", FE_LOG_ERROR);
		return RESOURCE_MANAGER.NoTexture;
	}
		
	// If Prefab's dirty flag is set we need to update preview.
	if (CurrentPrefab->IsDirty())
	{
		CreatePrefabPreview(PrefabID);
		CurrentPrefab->SetDirtyFlag(false);
	}

	return GetCachedPreview(PrefabID, [&]() { CreatePrefabPreview(PrefabID); });
}

void FEEditorPreviewManager::Clear()
{
	auto PreviewIterator = PreviewTextures.begin();
	while (PreviewIterator != PreviewTextures.end())
	{
		delete PreviewIterator->second;
		PreviewIterator++;
	}
	PreviewTextures.clear();
}

void FEEditorPreviewManager::CreateScenePreview(std::string SceneID)
{
	FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
	if (Scene == nullptr)
	{
		LOG.Add("FEEditorPreviewManager::CreateScenePreview could not find scene with ID: " + SceneID, "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}
	bool bWasActive = Scene->HasFlag(FESceneFlag::Active);
	Scene->SetFlag(FESceneFlag::Active, true);

	FEAABB SceneAABB = Scene->GetSceneAABB([](FEEntity* Entity) -> bool {
		if (Entity->GetTag() == EDITOR_RESOURCE_TAG)
			return false;

		if (Entity->HasComponent<FESkyDomeComponent>())
			return false;

		if (Entity->HasComponent<FECameraComponent>())
			return false;

		return true;
	});

	FEEntity* TemporaryCamera = Scene->CreateEntity("Temporary scene camera");
	RESOURCE_MANAGER.SetTag(TemporaryCamera, EDITOR_RESOURCE_TAG);
	TemporaryCamera->AddComponent<FECameraComponent>();
	FETransformComponent& CameraTransform = TemporaryCamera->GetComponent<FETransformComponent>();
	CameraTransform.SetSceneIndependent(true);
	glm::vec3 CenterOfAABB = SceneAABB.GetCenter();
	CameraTransform.SetPosition(glm::vec3(0.0, 0.0, SceneAABB.GetLongestAxisLength() * 2));
	//LookAt(TemporaryCamera, CenterOfAABB);
	CAMERA_SYSTEM.PointCameraAt(TemporaryCamera, CenterOfAABB);
	FECameraComponent& CameraComponent = TemporaryCamera->GetComponent<FECameraComponent>();
	CameraComponent.TryToSetViewportSize(128, 128);
	CameraComponent.SetDistanceFogEnabled(false);
	CameraComponent.SetSSAOEnabled(false);
	FEEntity* PreviousMainCamera = CAMERA_SYSTEM.GetMainCamera(Scene);
	CAMERA_SYSTEM.SetMainCamera(TemporaryCamera);

	CAMERA_SYSTEM.IndividualUpdate(TemporaryCamera, 0.0);

	RENDERER.Render(Scene);

	StorePreview(SceneID, RENDERER.GetCameraResult(TemporaryCamera));

	CAMERA_SYSTEM.SetMainCamera(PreviousMainCamera);
	Scene->DeleteEntity(TemporaryCamera);

	Scene->SetFlag(FESceneFlag::Active, bWasActive);
}

FETexture* FEEditorPreviewManager::GetScenePreview(std::string SceneID)
{
	FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
	if (Scene == nullptr)
	{
		LOG.Add("FEEditorPreviewManager::GetScenePreview could not find scene with ID: " + SceneID, "FE_LOG_RENDERING", FE_LOG_ERROR);
		return RESOURCE_MANAGER.NoTexture;
	}

	// If scene's dirty flag is set we need to update preview.
	if (Scene->IsDirty())
	{
		CreateScenePreview(SceneID);
		Scene->SetDirtyFlag(false);
	}

	return GetCachedPreview(SceneID, [&]() { CreateScenePreview(SceneID); });
}

void FEEditorPreviewManager::CreateTexture3DPreview(std::string TextureID)
{
	FETexture* Texture = RESOURCE_MANAGER.GetTexture(TextureID);
	if (Texture == nullptr || Texture->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		return;

	const std::vector<FEShader*> VolumetricShaders = VOLUME_SYSTEM.GetVolumetricShaders();
	if (VolumetricShaders.empty())
		return;

	BeforePreviewActions();

	PreviewEntity->AddComponent<FEVolumeComponent>();
	FEVolumeComponent& VolumeComponent = PreviewEntity->GetComponent<FEVolumeComponent>();
	VolumeComponent.SetMaterial(MaterialFor3DTextures);
	VolumeComponent.GetMaterial()->SetTextureOverride("VolumeTexture", Texture->GetObjectID());

	FEAABB VolumeAABB = PreviewScene->GetEntityAABB(PreviewEntity);
	const glm::vec3 Min = VolumeAABB.GetMin();
	const glm::vec3 Max = VolumeAABB.GetMax();

	const float XSize = sqrt((Max.x - Min.x) * (Max.x - Min.x));
	const float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	const float ZSize = sqrt((Max.z - Min.z) * (Max.z - Min.z));

	// Invert center point to get required translation vector for centering the volume at origin.
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(Max.x - XSize / 2.0f, Max.y - YSize / 2.0f, Max.z - ZSize / 2.0f));
	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	PreviewEntity->RemoveComponent<FEVolumeComponent>();
	AfterPreviewActions();

	StorePreview(TextureID, RENDERER.GetCameraResult(LocalCameraEntity));
}

FETexture* FEEditorPreviewManager::GetTexture3DPreview(std::string TextureID)
{
	FETexture* Texture = RESOURCE_MANAGER.GetTexture(TextureID);
	if (Texture == nullptr || Texture->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		return RESOURCE_MANAGER.NoTexture;

	return GetCachedPreview(TextureID, [&]() { CreateTexture3DPreview(TextureID); });
}

FETexture* FEEditorPreviewManager::GetPreview(FEObject* Object)
{
	switch (Object->GetType())
	{
		case FE_TEXTURE:
		{
			FETexture* Texture = reinterpret_cast<FETexture*>(Object);
			if (Texture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
				return GetTexture3DPreview(Object->GetObjectID());

			return Texture;
		}

		case FE_MESH:
			return GetMeshPreview(Object->GetObjectID());

		case FE_MATERIAL:
			return GetMaterialPreview(Object->GetObjectID());

		case FE_GAMEMODEL:
			return GetGameModelPreview(Object->GetObjectID());

		case FE_POINT_CLOUD:
			return GetPointCloudPreview(Object->GetObjectID());

		case FE_PREFAB:
			return GetPrefabPreview(Object->GetObjectID());

		case FE_SCENE:
			return GetScenePreview(Object->GetObjectID());
		
		default:
			return RESOURCE_MANAGER.NoTexture;
	}
}

FETexture* FEEditorPreviewManager::GetPreview(const std::string ObjectID)
{
	return GetPreview(OBJECT_MANAGER.GetFEObject(ObjectID));
}

void FEEditorPreviewManager::Update()
{
	// Check if any material is dirty.
	// FIX ME! Dirty flag systems is not working properly. It should be fixed.
	const std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialIDList();
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		FEMaterial* CurrentMaterial = RESOURCE_MANAGER.GetMaterial(MaterialList[i]);
		if (CurrentMaterial->GetTag() == ENGINE_RESOURCE_TAG ||
			CurrentMaterial->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		if (CurrentMaterial->IsDirty())
		{
			CreateMaterialPreview(MaterialList[i]);
			CurrentMaterial->SetDirtyFlag(false);
		}
	}
}