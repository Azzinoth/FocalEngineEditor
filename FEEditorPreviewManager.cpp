#include "FEEditorPreviewManager.h"
using namespace FocalEngine;

FEEditorPreviewManager* FEEditorPreviewManager::Instance = nullptr;
FEEditorPreviewManager::FEEditorPreviewManager() {}
FEEditorPreviewManager::~FEEditorPreviewManager() {}

glm::vec4 FEEditorPreviewManager::OriginalClearColor = glm::vec4();
FETransformComponent FEEditorPreviewManager::OriginalTransform = FETransformComponent();
glm::vec3 FEEditorPreviewManager::OriginalCameraPosition = glm::vec3();
float FEEditorPreviewManager::OriginalAspectRation = 0.0f;
float FEEditorPreviewManager::OriginalCameraPitch = 0.0f;
float FEEditorPreviewManager::OriginalCameraRoll = 0.0f;
float FEEditorPreviewManager::OriginalCameraYaw = 0.0f;
float FEEditorPreviewManager::OriginalExposure = 0.0f;

void FEEditorPreviewManager::InitializeResources()
{
	PreviewFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, 128, 128);
	PreviewGameModel = new FEGameModel(nullptr, nullptr, "editorPreviewGameModel");
	PreviewPrefab = new FEPrefab(PreviewGameModel, "editorPreviewPrefab");
	PreviewEntity = SCENE.AddNewStyleEntity("editorPreviewEntity");
	PreviewEntity->AddComponent<FEGameModelComponent>(PreviewGameModel).SetVisibility(false);
	MeshPreviewMaterial = RESOURCE_MANAGER.CreateMaterial("meshPreviewMaterial");
	RESOURCE_MANAGER.MakeMaterialStandard(MeshPreviewMaterial);
	MeshPreviewMaterial->Shader = RESOURCE_MANAGER.CreateShader("FEMeshPreviewShader", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_MeshPreview_VS.glsl").c_str(),
																					   RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_MeshPreview_FS.glsl").c_str(),
																					   nullptr,
																					   nullptr,
																					   nullptr,
																					   nullptr,
																					   "607A53601357077F03770357"/*"FEMeshPreviewShader"*/);

	RESOURCE_MANAGER.MakeShaderStandard(MeshPreviewMaterial->Shader);
}

void FEEditorPreviewManager::ReInitializeEntities()
{
	PreviewEntity = SCENE.AddNewStyleEntity("editorPreviewEntity");
	PreviewEntity->AddComponent<FEGameModelComponent>(PreviewGameModel).SetVisibility(false);
}

void FEEditorPreviewManager::UpdateAll()
{
	Clear();

	const std::vector<std::string> MeshList = RESOURCE_MANAGER.GetMeshList();
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		CreateMeshPreview(MeshList[i]);
	}

	const std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialList();
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		CreateMaterialPreview(MaterialList[i]);
	}

	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		CreateGameModelPreview(GameModelList[i]);
	}
}

void FEEditorPreviewManager::BeforePreviewActions()
{
	PreviewFB->Bind();
	// We use these values even with the deferred renderer because the final image will not undergo gamma correction. Therefore, values exceeding 1.0f will not function correctly.
	OriginalClearColor = ENGINE.GetClearColor();
	ENGINE.SetClearColor(glm::vec4(0.55f, 0.73f, 0.87f, 1.0f));

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Saving currently used variables.
	OriginalCameraPosition = ENGINE.GetCamera()->GetPosition();
	OriginalAspectRation = ENGINE.GetCamera()->GetAspectRatio();
	OriginalCameraPitch = ENGINE.GetCamera()->GetPitch();
	OriginalCameraRoll = ENGINE.GetCamera()->GetRoll();
	OriginalCameraYaw = ENGINE.GetCamera()->GetYaw();
	OriginalExposure = ENGINE.GetCamera()->GetExposure();
	ENGINE.GetCamera()->SetExposure(1.0f);

	OriginalTransform = PreviewEntity->GetComponent<FETransformComponent>();

	// The transform impacts the AABB. Therefore, the necessary values must be set prior to any calculations.
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0, 0.0, 0.0));
	PreviewEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(1.0, 1.0, 1.0));
	PreviewEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(15.0, -15.0, 0.0));

	ENGINE.GetCamera()->SetAspectRatio(1.0f);
	FE_GL_ERROR(glViewport(0, 0, 128, 128));

	ENGINE.GetCamera()->SetPitch(0.0f);
	ENGINE.GetCamera()->SetRoll(0.0f);
	ENGINE.GetCamera()->SetYaw(0.0f);

	PreviewEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
}

void FEEditorPreviewManager::AfterPreviewActions()
{
	// We are reversing all of our previously applied transformations.
	PreviewEntity->GetComponent<FETransformComponent>() = OriginalTransform;

	ENGINE.GetCamera()->SetPosition(OriginalCameraPosition);
	ENGINE.GetCamera()->SetAspectRatio(OriginalAspectRation);
	ENGINE.GetCamera()->SetPitch(OriginalCameraPitch);
	ENGINE.GetCamera()->SetRoll(OriginalCameraRoll);
	ENGINE.GetCamera()->SetYaw(OriginalCameraYaw);

	PreviewFB->UnBind();

	ENGINE.SetClearColor(OriginalClearColor);

	PreviewEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
}

void FEEditorPreviewManager::CreateMeshPreview(const std::string MeshID)
{
	FEMesh* PreviewMesh = RESOURCE_MANAGER.GetMesh(MeshID);
	if (PreviewMesh == nullptr)
		return;

	PreviewGameModel->Mesh = PreviewMesh;
	PreviewGameModel->Material = MeshPreviewMaterial;

	BeforePreviewActions();

	//FEAABB MeshAABB = PreviewEntity->GetAABB();
	FEAABB MeshAABB = PreviewEntity->GetComponent<FEGameModelComponent>().GameModel->Mesh->GetAABB();
	MeshAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetTransformMatrix());
	const glm::vec3 Min = MeshAABB.GetMin();
	const glm::vec3 Max = MeshAABB.GetMax();

	const float XSize = sqrt((Max.x - Min.x) * (Max.x - Min.x));
	const float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	const float ZSize = sqrt((Max.z - Min.z) * (Max.z - Min.z));

	FEDirectionalLight* CurrentDirectionalLight = nullptr;
	const std::vector<std::string> LightList = SCENE.GetLightsList();
	for (size_t i = 0; i < LightList.size(); i++)
	{
		if (SCENE.GetLight(LightList[i])->GetType() == FE_DIRECTIONAL_LIGHT)
		{
			CurrentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.GetLight(LightList[i]));
			break;
		}
	}
	const glm::vec3 RegularLightRotation = CurrentDirectionalLight->Transform.GetRotation();
	CurrentDirectionalLight->Transform.SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	const float RegularLightIntensity = CurrentDirectionalLight->GetIntensity();
	CurrentDirectionalLight->SetIntensity(10.0f);

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(Max.x - XSize / 2.0f, Max.y - YSize / 2.0f, Max.z - ZSize / 2.0f));
	ENGINE.GetCamera()->SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));

	// Rendering mesh to texture.
	RENDERER.RenderGameModelComponent(PreviewEntity->GetComponent<FEGameModelComponent>(), PreviewEntity->GetComponent<FETransformComponent>(), ENGINE.GetCamera());

	CurrentDirectionalLight->Transform.SetRotation(RegularLightRotation);
	CurrentDirectionalLight->SetIntensity(RegularLightIntensity);

	AfterPreviewActions();

	// if we are updating preview we should delete old texture.
	if (MeshPreviewTextures.find(MeshID) != MeshPreviewTextures.end())
		delete MeshPreviewTextures[MeshID];

	MeshPreviewTextures[MeshID] = PreviewFB->GetColorAttachment();
	PreviewFB->SetColorAttachment(RESOURCE_MANAGER.CreateSameFormatTexture(PreviewFB->GetColorAttachment()));
}

FETexture* FEEditorPreviewManager::GetMeshPreview(const std::string MeshID)
{
	// if mesh's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.GetMesh(MeshID)->IsDirty())
	{
		CreateMeshPreview(MeshID);
		// if some game model uses this mesh we should also update it's preview
		const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelList();

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

	FEDirectionalLight* CurrentDirectionalLight = nullptr;
	const std::vector<std::string> LightList = SCENE.GetLightsList();
	for (size_t i = 0; i < LightList.size(); i++)
	{
		if (SCENE.GetLight(LightList[i])->GetType() == FE_DIRECTIONAL_LIGHT)
		{
			CurrentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.GetLight(LightList[i]));
			break;
		}
	}
	const glm::vec3 RegularLightRotation = CurrentDirectionalLight->Transform.GetRotation();
	CurrentDirectionalLight->Transform.SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	const float RegularLightIntensity = CurrentDirectionalLight->GetIntensity();
	CurrentDirectionalLight->SetIntensity(10.0f);

	PreviewGameModel->Mesh = RESOURCE_MANAGER.GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);
	PreviewGameModel->Material = PreviewMaterial;
	PreviewEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	BeforePreviewActions();

	ENGINE.GetCamera()->SetPosition(glm::vec3(0.0, 0.0, 70.0f));

	// Rendering material to texture
	RENDERER.RenderGameModelComponentForward(PreviewEntity->GetComponent<FEGameModelComponent>(), PreviewEntity->GetComponent<FETransformComponent>(), ENGINE.GetCamera(), true);

	CurrentDirectionalLight->Transform.SetRotation(RegularLightRotation);
	CurrentDirectionalLight->SetIntensity(RegularLightIntensity);

	AfterPreviewActions();

	// if we are updating preview we should delete old texture.
	if (MaterialPreviewTextures.find(MaterialID) != MaterialPreviewTextures.end())
		delete MaterialPreviewTextures[MaterialID];

	MaterialPreviewTextures[MaterialID] = PreviewFB->GetColorAttachment();
	PreviewFB->SetColorAttachment(RESOURCE_MANAGER.CreateSameFormatTexture(PreviewFB->GetColorAttachment()));

	// looking for all gameModels that uses this material to also update them
	const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelList();
	for (size_t i = 0; i < GameModelList.size(); i++)
	{
		const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);
		if (CurrentGameModel->Material == PreviewMaterial && CurrentGameModel != PreviewGameModel)
			CreateGameModelPreview(CurrentGameModel->GetObjectID());
	}

	// looking for all prefabs that uses this material to also update them
	const std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabList();
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabList[i]);
		if (CurrentPrefab->UsesMaterial(MaterialID))
			CreatePrefabPreview(CurrentPrefab->GetObjectID());
	}
}

FETexture* FEEditorPreviewManager::GetMaterialPreview(const std::string MaterialID)
{
	// if material's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.GetMaterial(MaterialID)->IsDirty())
	{
		CreateMaterialPreview(MaterialID);
		// if some game model uses this material we should also update it's preview
		const std::vector<std::string> GameModelList = RESOURCE_MANAGER.GetGameModelList();
		for (size_t i = 0; i < GameModelList.size(); i++)
		{
			const FEGameModel* CurrentGameModel = RESOURCE_MANAGER.GetGameModel(GameModelList[i]);

			if (CurrentGameModel->Material == RESOURCE_MANAGER.GetMaterial(MaterialID))
			{
				CreateGameModelPreview(CurrentGameModel->GetObjectID());

				// if some Prefab uses this game model we should also update it's preview
				std::vector<std::string> PrefabList = RESOURCE_MANAGER.GetPrefabList();
				for (size_t j = 0; j < PrefabList.size(); j++)
				{
					FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabList[j]);
					
					if (CurrentPrefab->UsesGameModel(CurrentGameModel->GetObjectID()))
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
	if (GameModel == nullptr)
		return;

	FEDirectionalLight* CurrentDirectionalLight = nullptr;
	const std::vector<std::string> LightList = SCENE.GetLightsList();
	for (size_t i = 0; i < LightList.size(); i++)
	{
		if (SCENE.GetLight(LightList[i])->GetType() == FE_DIRECTIONAL_LIGHT)
		{
			CurrentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.GetLight(LightList[i]));
			break;
		}
	}
	const glm::vec3 RegularLightRotation = CurrentDirectionalLight->Transform.GetRotation();
	CurrentDirectionalLight->Transform.SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	const float RegularLightIntensity = CurrentDirectionalLight->GetIntensity();
	CurrentDirectionalLight->SetIntensity(10.0f);

	const bool RegularFog = RENDERER.IsDistanceFogEnabled();
	RENDERER.SetDistanceFogEnabled(false);

	PreviewGameModel->Mesh = GameModel->Mesh;
	PreviewGameModel->Material = GameModel->Material;
	PreviewEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	BeforePreviewActions();

	//FEAABB MeshAABB = PreviewEntity->GetAABB();
	FEAABB MeshAABB = PreviewEntity->GetComponent<FEGameModelComponent>().GameModel->Mesh->GetAABB();
	MeshAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetTransformMatrix());
	const glm::vec3 min = MeshAABB.GetMin();
	const glm::vec3 max = MeshAABB.GetMax();

	const float XSize = sqrt((max.x - min.x) * (max.x - min.x));
	const float YSize = sqrt((max.y - min.y) * (max.y - min.y));
	const float ZSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(max.x - XSize / 2.0f, max.y - YSize / 2.0f, max.z - ZSize / 2.0f));
	ENGINE.GetCamera()->SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));

	// rendering game model to texture
	RENDERER.RenderGameModelComponentForward(PreviewEntity->GetComponent<FEGameModelComponent>(), PreviewEntity->GetComponent<FETransformComponent>(), ENGINE.GetCamera(), true);

	CurrentDirectionalLight->Transform.SetRotation(RegularLightRotation);
	CurrentDirectionalLight->SetIntensity(RegularLightIntensity);

	RENDERER.SetDistanceFogEnabled(RegularFog);

	AfterPreviewActions();

	// if we are updating preview we should delete old texture.
	if (GameModelPreviewTextures.find(GameModelID) != GameModelPreviewTextures.end())
		delete GameModelPreviewTextures[GameModelID];

	GameModelPreviewTextures[GameModelID] = PreviewFB->GetColorAttachment();
	PreviewFB->SetColorAttachment(RESOURCE_MANAGER.CreateSameFormatTexture(PreviewFB->GetColorAttachment()));
}

void FEEditorPreviewManager::CreateGameModelPreview(const FEGameModel* GameModel, FETexture** ResultingTexture)
{
	if (GameModel == nullptr)
		return;

	FEDirectionalLight* CurrentDirectionalLight = nullptr;
	const std::vector<std::string> LightList = SCENE.GetLightsList();
	for (size_t i = 0; i < LightList.size(); i++)
	{
		if (SCENE.GetLight(LightList[i])->GetType() == FE_DIRECTIONAL_LIGHT)
		{
			CurrentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.GetLight(LightList[i]));
			break;
		}
	}
	const glm::vec3 RegularLightRotation = CurrentDirectionalLight->Transform.GetRotation();
	CurrentDirectionalLight->Transform.SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	const float RegularLightIntensity = CurrentDirectionalLight->GetIntensity();
	CurrentDirectionalLight->SetIntensity(10.0f);

	PreviewGameModel->Mesh = GameModel->Mesh;
	PreviewGameModel->Material = GameModel->Material;
	PreviewEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	if (*ResultingTexture == nullptr)
		*ResultingTexture = RESOURCE_MANAGER.CreateSameFormatTexture(PreviewFB->GetColorAttachment());
	FETexture* TempTexture = PreviewFB->GetColorAttachment();
	PreviewFB->SetColorAttachment(*ResultingTexture);

	BeforePreviewActions();

	//FEAABB MeshAABB = PreviewEntity->GetAABB();
	FEAABB MeshAABB = PreviewEntity->GetComponent<FEGameModelComponent>().GameModel->Mesh->GetAABB();
	MeshAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetTransformMatrix());
	const glm::vec3 min = MeshAABB.GetMin();
	const glm::vec3 max = MeshAABB.GetMax();

	const float XSize = sqrt((max.x - min.x) * (max.x - min.x));
	const float YSize = sqrt((max.y - min.y) * (max.y - min.y));
	const float ZSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(max.x - XSize / 2.0f, max.y - YSize / 2.0f, max.z - ZSize / 2.0f));
	ENGINE.GetCamera()->SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));

	// rendering game model to texture
	RENDERER.RenderGameModelComponentForward(PreviewEntity->GetComponent<FEGameModelComponent>(), PreviewEntity->GetComponent<FETransformComponent>(), ENGINE.GetCamera(), true);

	CurrentDirectionalLight->Transform.SetRotation(RegularLightRotation);
	CurrentDirectionalLight->SetIntensity(RegularLightIntensity);

	AfterPreviewActions();

	PreviewFB->SetColorAttachment(TempTexture);
}

FETexture* FEEditorPreviewManager::GetGameModelPreview(const std::string GameModelID)
{
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
	const auto GameModelsList = RESOURCE_MANAGER.GetGameModelList();
	for (size_t i = 0; i < GameModelsList.size(); i++)
	{
		CreateGameModelPreview(GameModelsList[i]);
	}
}

void FEEditorPreviewManager::CreatePrefabPreview(const std::string PrefabID)
{
	FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(PrefabID);
	if (Prefab == nullptr)
		return;

	if (Prefab->ComponentsCount() < 1)
		return;

	FEGameModel* GameModel = Prefab->GetComponent(0)->GameModel;
	if (GameModel == nullptr)
		return;

	FEDirectionalLight* CurrentDirectionalLight = nullptr;
	const std::vector<std::string> LightList = SCENE.GetLightsList();
	for (size_t i = 0; i < LightList.size(); i++)
	{
		if (SCENE.GetLight(LightList[i])->GetType() == FE_DIRECTIONAL_LIGHT)
		{
			CurrentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.GetLight(LightList[i]));
			break;
		}
	}
	const glm::vec3 RegularLightRotation = CurrentDirectionalLight->Transform.GetRotation();
	CurrentDirectionalLight->Transform.SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	const float RegularLightIntensity = CurrentDirectionalLight->GetIntensity();
	CurrentDirectionalLight->SetIntensity(10.0f);

	const bool RegularFog = RENDERER.IsDistanceFogEnabled();
	RENDERER.SetDistanceFogEnabled(false);

	//Fix ME!
	//PreviewEntity->Prefab = Prefab;
	PreviewEntity->GetComponent<FEGameModelComponent>().GameModel = GameModel;
	PreviewEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	BeforePreviewActions();

	//PreviewEntity->SetDirtyFlag(true);
	//FEAABB MeshAABB = PreviewEntity->GetAABB();
	FEAABB MeshAABB = PreviewEntity->GetComponent<FEGameModelComponent>().GameModel->Mesh->GetAABB();
	MeshAABB.Transform(PreviewEntity->GetComponent<FETransformComponent>().GetTransformMatrix());
	const glm::vec3 min = MeshAABB.GetMin();
	const glm::vec3 max = MeshAABB.GetMax();

	const float XSize = sqrt((max.x - min.x) * (max.x - min.x));
	const float YSize = sqrt((max.y - min.y) * (max.y - min.y));
	const float ZSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	PreviewEntity->GetComponent<FETransformComponent>().SetPosition(-glm::vec3(max.x - XSize / 2.0f, max.y - YSize / 2.0f, max.z - ZSize / 2.0f));
	ENGINE.GetCamera()->SetPosition(glm::vec3(0.0, 0.0, std::max(std::max(XSize, YSize), ZSize) * 1.75f));

	// rendering game model to texture
	RENDERER.RenderGameModelComponentForward(PreviewEntity->GetComponent<FEGameModelComponent>(), PreviewEntity->GetComponent<FETransformComponent>(), ENGINE.GetCamera(), true);

	//PreviewEntity->Prefab = PreviewPrefab;
	PreviewEntity->GetComponent<FEGameModelComponent>().GameModel = PreviewGameModel;

	CurrentDirectionalLight->Transform.SetRotation(RegularLightRotation);
	CurrentDirectionalLight->SetIntensity(RegularLightIntensity);

	RENDERER.SetDistanceFogEnabled(RegularFog);

	AfterPreviewActions();

	// if we are updating preview we should delete old texture.
	if (PrefabPreviewTextures.find(PrefabID) != PrefabPreviewTextures.end())
		delete PrefabPreviewTextures[PrefabID];

	PrefabPreviewTextures[PrefabID] = PreviewFB->GetColorAttachment();
	PreviewFB->SetColorAttachment(RESOURCE_MANAGER.CreateSameFormatTexture(PreviewFB->GetColorAttachment()));
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
		
	// if Prefab's dirty flag is set we need to update preview
	if (CurrentPrefab->IsDirty())
	{
		CreatePrefabPreview(PrefabID);
		CurrentPrefab->SetDirtyFlag(false);
	}

	for (int i = 0; i < CurrentPrefab->ComponentsCount(); i++)
	{
		// if Prefab's material dirty flag is set we need to update preview
		if (CurrentPrefab->GetComponent(i)->GameModel->GetMaterial() != nullptr && CurrentPrefab->GetComponent(i)->GameModel->GetMaterial()->IsDirty())
		{
			CreateMaterialPreview(CurrentPrefab->GetComponent(i)->GameModel->GetMaterial()->GetObjectID());
			CurrentPrefab->GetComponent(i)->GameModel->GetMaterial()->SetDirtyFlag(false);
		}
	}

	// if we somehow could not find preview, we will create it.
	if (PrefabPreviewTextures.find(PrefabID) == PrefabPreviewTextures.end())
		CreatePrefabPreview(PrefabID);

	// if still we don't have it
	if (PrefabPreviewTextures.find(PrefabID) == PrefabPreviewTextures.end())
		return RESOURCE_MANAGER.NoTexture;

	return PrefabPreviewTextures[PrefabID];
}

void FEEditorPreviewManager::Clear()
{
	auto iterator = MeshPreviewTextures.begin();
	while (iterator != MeshPreviewTextures.end())
	{
		delete iterator->second;
		iterator++;
	}
	MeshPreviewTextures.clear();

	iterator = MaterialPreviewTextures.begin();
	while (iterator != MaterialPreviewTextures.end())
	{
		delete iterator->second;
		iterator++;
	}
	MaterialPreviewTextures.clear();

	iterator = GameModelPreviewTextures.begin();
	while (iterator != GameModelPreviewTextures.end())
	{
		delete iterator->second;
		iterator++;
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