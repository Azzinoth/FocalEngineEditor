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
	PreviewEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
	RESOURCE_MANAGER.SetTag(PreviewEntity, EDITOR_RESOURCE_TAG);

	LocalCameraEntity = PreviewScene->CreateEntity("EditorPreview CameraEntity");
	LocalCameraEntity->AddComponent<FECameraComponent>();
	FECameraComponent& CameraComponent = LocalCameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.SetRenderTargetSize(128, 128);
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
}

void FEEditorPreviewManager::UpdateAll()
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

	// if we are updating preview we should delete old texture.
	if (MeshPreviewTextures.find(MeshID) != MeshPreviewTextures.end())
		delete MeshPreviewTextures[MeshID];

	FETexture* CameraResult = RENDERER.GetCameraResult(LocalCameraEntity);
	if (CameraResult != nullptr)
		MeshPreviewTextures[MeshID] = RESOURCE_MANAGER.CreateCopyOfTexture(CameraResult);
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

	// if we somehow could not find preview, we will create it.
	if (MeshPreviewTextures.find(MeshID) == MeshPreviewTextures.end())
		CreateMeshPreview(MeshID);

	// if still we don't have it
	if (MeshPreviewTextures.find(MeshID) == MeshPreviewTextures.end())
		return RESOURCE_MANAGER.NoTexture;

	return MeshPreviewTextures[MeshID];
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

	// if we are updating preview we should delete old texture.
	if (MaterialPreviewTextures.find(MaterialID) != MaterialPreviewTextures.end())
		delete MaterialPreviewTextures[MaterialID];

	FETexture* CameraResult = RENDERER.GetCameraResult(LocalCameraEntity);
	if (CameraResult != nullptr)
	{
		MaterialPreviewTextures[MaterialID] = RESOURCE_MANAGER.CreateCopyOfTexture(CameraResult);
	}

	// Looking for all gameModels that uses this material to also update them.
	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelIDList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Material == PreviewMaterial && CurrentGameModel != PreviewGameModel)
			CreateGameModelPreview(CurrentGameModel->GetObjectID());
	}

	// Looking for all prefabs that uses this material to also update them.
	const std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabIDList();
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabList[i]);
		if (CurrentPrefab->IsUsingMaterial(MaterialID))
			CreatePrefabPreview(CurrentPrefab->GetObjectID());
	}
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

	// if we somehow could not find preview, we will create it.
	if (MaterialPreviewTextures.find(MaterialID) == MaterialPreviewTextures.end())
		CreateMaterialPreview(MaterialID);

	// if still we don't have it
	if (MaterialPreviewTextures.find(MaterialID) == MaterialPreviewTextures.end())
		return RESOURCE_MANAGER.NoTexture;

	return MaterialPreviewTextures[MaterialID];
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
	const glm::vec3 min = MeshAABB.GetMin();
	const glm::vec3 max = MeshAABB.GetMax();

	const float XSize = sqrt((max.x - min.x) * (max.x - min.x));
	const float YSize = sqrt((max.y - min.y) * (max.y - min.y));
	const float ZSize = sqrt((max.z - min.z) * (max.z - min.z));

	// Invert center point to get required translation vector for centering mesh at origin
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(max.x - XSize / 2.0f, max.y - YSize / 2.0f, max.z - ZSize / 2.0f));
	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	AfterPreviewActions();

	// if we are updating preview we should delete old texture.
	if (GameModelPreviewTextures.find(GameModelID) != GameModelPreviewTextures.end())
		delete GameModelPreviewTextures[GameModelID];

	FETexture* CameraResult = RENDERER.GetCameraResult(LocalCameraEntity);
	if (CameraResult != nullptr)
		GameModelPreviewTextures[GameModelID] = RESOURCE_MANAGER.CreateCopyOfTexture(CameraResult);
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
	const glm::vec3 min = MeshAABB.GetMin();
	const glm::vec3 max = MeshAABB.GetMax();

	const float XSize = sqrt((max.x - min.x) * (max.x - min.x));
	const float YSize = sqrt((max.y - min.y) * (max.y - min.y));
	const float ZSize = sqrt((max.z - min.z) * (max.z - min.z));

	// Invert center point to get required translation vector for centering mesh at origin
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(max.x - XSize / 2.0f, max.y - YSize / 2.0f, max.z - ZSize / 2.0f));
	LocalCameraEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));
	CAMERA_SYSTEM.IndividualUpdate(LocalCameraEntity, 0.0);

	RENDERER.Render(PreviewScene);

	AfterPreviewActions();

	FETexture* CameraResult = RENDERER.GetCameraResult(LocalCameraEntity);
	if (CameraResult != nullptr)
		*ResultingTexture = RESOURCE_MANAGER.CreateCopyOfTexture(CameraResult);
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
		RESOURCE_MANAGER.GetGameModel(GameModelID)->SetDirtyFlag(false);
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

	// if we somehow could not find preview, we will create it.
	if (GameModelPreviewTextures.find(GameModelID) == GameModelPreviewTextures.end())
		CreateGameModelPreview(GameModelID);

	// if still we don't have it
	if (GameModelPreviewTextures.find(GameModelID) == GameModelPreviewTextures.end())
		return RESOURCE_MANAGER.NoTexture;

	return GameModelPreviewTextures[GameModelID];
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
	CameraComponent.SetRenderTargetSize(128, 128);
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

	// If we are updating preview we should delete old texture.
	if (PrefabPreviewTextures.find(PrefabID) != PrefabPreviewTextures.end())
		delete PrefabPreviewTextures[PrefabID];

	FETexture* CameraResult = RENDERER.GetCameraResult(Camera);
	if (CameraResult != nullptr)
		PrefabPreviewTextures[PrefabID] = RESOURCE_MANAGER.CreateCopyOfTexture(CameraResult);

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

	// If we somehow could not find preview, we will create it.
	if (PrefabPreviewTextures.find(PrefabID) == PrefabPreviewTextures.end())
		CreatePrefabPreview(PrefabID);

	// If still we don't have it
	if (PrefabPreviewTextures.find(PrefabID) == PrefabPreviewTextures.end())
		return RESOURCE_MANAGER.NoTexture;

	return PrefabPreviewTextures[PrefabID];
}

void FEEditorPreviewManager::Clear()
{
	auto MeshIterator = MeshPreviewTextures.begin();
	while (MeshIterator != MeshPreviewTextures.end())
	{
		delete MeshIterator->second;
		MeshIterator++;
	}
	MeshPreviewTextures.clear();

	auto MaterialIterator = MaterialPreviewTextures.begin();
	while (MaterialIterator != MaterialPreviewTextures.end())
	{
		delete MaterialIterator->second;
		MaterialIterator++;
	}
	MaterialPreviewTextures.clear();

	auto GameModelIterator = GameModelPreviewTextures.begin();
	while (GameModelIterator != GameModelPreviewTextures.end())
	{
		delete GameModelIterator->second;
		GameModelIterator++;
	}
	GameModelPreviewTextures.clear();
}

FETexture* FEEditorPreviewManager::GetPreview(FEObject* Object)
{
	switch (Object->GetType())
	{
		case FE_TEXTURE:
			return reinterpret_cast<FETexture*>(Object);

		case FE_MESH:
			return GetMeshPreview(Object->GetObjectID());

		case FE_MATERIAL:
			return GetMaterialPreview(Object->GetObjectID());

		case FE_GAMEMODEL:
			return GetGameModelPreview(Object->GetObjectID());

		case FE_PREFAB:
			return GetPrefabPreview(Object->GetObjectID());
		
		default:
			return RESOURCE_MANAGER.NoTexture;
	}
}

FETexture* FEEditorPreviewManager::GetPreview(const std::string ObjectID)
{
	return GetPreview(OBJECT_MANAGER.GetFEObject(ObjectID));
}