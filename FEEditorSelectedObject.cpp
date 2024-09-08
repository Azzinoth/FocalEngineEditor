#include "FEEditorSelectedObject.h"
#include "FEProject.h"
#include "FEEditor.h"
using namespace FocalEngine;

FEEditorSelectedObject::FEEditorSelectedObject() {}
FEEditorSelectedObject::~FEEditorSelectedObject() {}

void FEEditorSelectedObject::InitializeResources()
{
	HALO_SELECTION_EFFECT.InitializeResources();
	
	PixelAccurateSelectionMaterial = RESOURCE_MANAGER.CreateMaterial("pixelAccurateSelectionMaterial");
	RESOURCE_MANAGER.SetTag(PixelAccurateSelectionMaterial, EDITOR_RESOURCE_TAG);

	FEPixelAccurateSelection = RESOURCE_MANAGER.CreateShader("FEPixelAccurateSelection", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_VS.glsl").c_str(),
																						 RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 "4279660C7D3D27360358354E"/*"FEPixelAccurateSelection"*/);
	RESOURCE_MANAGER.SetTag(FEPixelAccurateSelection, EDITOR_RESOURCE_TAG);
	PixelAccurateSelectionMaterial->Shader = FEPixelAccurateSelection;

	FEPixelAccurateInstancedSelection = RESOURCE_MANAGER.CreateShader("FEPixelAccurateInstancedSelection", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_INSTANCED_VS.glsl").c_str(),
																										   RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																									       nullptr,
																										   nullptr,
																										   nullptr,
																										   nullptr,
																										   "0E213D3542135C15471F0D6B"/*"FEPixelAccurateInstancedSelection"*/);

	RESOURCE_MANAGER.SetTag(FEPixelAccurateInstancedSelection, EDITOR_RESOURCE_TAG);

	const FEShaderParam ColorParam(glm::vec3(0.0f, 0.0f, 0.0f), "baseColor");
	PixelAccurateSelectionMaterial->AddParameter(ColorParam);
}

void FEEditorSelectedObject::UpdateResources(FEScene* Scene)
{
	HALO_SELECTION_EFFECT.UpdateResources(Scene);

	if (Scene == nullptr)
		return;

	FESelectionData* CurrentSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return;

	FEEntity* CurrentCamera = CAMERA_SYSTEM.GetMainCameraEntity(Scene);
	FECameraComponent& CameraComponent = CurrentCamera->GetComponent<FECameraComponent>();

	delete CurrentSelectionData->PixelAccurateSelectionFB;
	CurrentSelectionData->PixelAccurateSelectionFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());
	delete CurrentSelectionData->PixelAccurateSelectionFB->GetColorAttachment();
	CurrentSelectionData->PixelAccurateSelectionFB->SetColorAttachment(RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight()));
}

void FEEditorSelectedObject::SetOnUpdateFunction(std::function<void(FEScene*)> Func)
{
	OnUpdateFunction = Func;
}

FEEntity* FEEditorSelectedObject::GetSelected(FEScene* Scene)
{
	if (Scene == nullptr)
		return nullptr;

	FESelectionData* CurrentSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return nullptr;

	return Scene->GetEntity(CurrentSelectionData->SelectedEntityID);
}

void FEEditorSelectedObject::SetSelected(FEEntity* SelectedObject)
{
	if (SelectedObject == nullptr)
		return;

	FEScene* CurrentScene = SelectedObject->GetParentScene();
	if (CurrentScene == nullptr)
		return;

	FESelectionData* CurrentSelectionData = GetSceneData(CurrentScene->GetObjectID());
	if (CurrentSelectionData == nullptr)
	{
		AddSceneData(CurrentScene->GetObjectID());
		CurrentSelectionData = GetSceneData(CurrentScene->GetObjectID());

		// If the data is still null log error and return
		if (CurrentSelectionData == nullptr)
		{
			LOG.Add("FEEditorSelectedObject::SetSelected: Could not create selection data for scene: " + CurrentScene->GetObjectID(), "FE_LOG_ERROR", FE_LOG_ERROR);
			return;
		}
	}

	FEEntity* CurrentlySelectedEntity = CurrentScene->GetEntity(CurrentSelectionData->SelectedEntityID);

	if (CurrentlySelectedEntity != nullptr && CurrentlySelectedEntity->HasComponent<FETerrainComponent>() && CurrentlySelectedEntity != SelectedObject)
		TERRAIN_SYSTEM.SetBrushMode(CurrentlySelectedEntity, FE_TERRAIN_BRUSH_NONE);

	CurrentSelectionData->SelectedEntityID = SelectedObject->GetObjectID();
	if (OnUpdateFunction != nullptr)
		OnUpdateFunction(CurrentScene);
}

void FEEditorSelectedObject::Clear(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FESelectionData* CurrentSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return;

	FEEntity* CurrentlySelectedEntity = Scene->GetEntity(CurrentSelectionData->SelectedEntityID);
	if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1 && CurrentlySelectedEntity->HasComponent<FEInstancedComponent>())
	{
		INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(CurrentlySelectedEntity, false);
	}

	CurrentSelectionData->InstancedSubObjectIndexSelected = -1;
	CurrentSelectionData->SelectedEntityID = "";

	if (!SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active).empty())
		if (OnUpdateFunction != nullptr)
			OnUpdateFunction(Scene);
}

void FEEditorSelectedObject::DetermineEntityUnderMouse(const double MouseX, const double MouseY, FEScene* Scene)
{
	FESelectionData* CurrentSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return;

	CurrentSelectionData->SceneEntitiesUnderMouse.clear();
	CurrentSelectionData->InstancedSubObjectsInfo.clear();

	FEEntity* MainCamera = CAMERA_SYSTEM.GetMainCameraEntity(Scene);
	FETransformComponent& CameraTransformComponent = MainCamera->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = MainCamera->GetComponent<FECameraComponent>();

	FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(Scene);
	glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
	glm::ivec2 ViewportSize = glm::ivec2(CurrentViewport->GetWidth(), CurrentViewport->GetHeight());

	const glm::vec3 MouseRay = GEOMETRY.CreateMouseRayToWorld(MouseX, MouseY,
															  CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
															  ViewportPosition, ViewportSize);

	const std::vector<std::string> EntityList = Scene->GetEntityIDList();
	for (size_t i = 0; i < EntityList.size(); i++)
	{
		float Distance = 0;

		FEEntity* CurrentEntity = Scene->GetEntity(EntityList[i]);
		if (CurrentEntity != nullptr)
		{
			FEAABB Box = Scene->GetEntityAABB(CurrentEntity);
			if (Box.RayIntersect(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay, Distance))
			{
				if (CurrentEntity->HasComponent<FEInstancedComponent>())
				{
					FEInstancedComponent& InstancedComponent = CurrentEntity->GetComponent<FEInstancedComponent>();
					if (INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(CurrentEntity))
					{
						CurrentSelectionData->InstancedSubObjectsInfo[CurrentEntity] = std::vector<int>();
						for (size_t j = 0; j < InstancedComponent.IndividualInstancedAABB.size(); j++)
						{
							if (InstancedComponent.IndividualInstancedAABB[j].RayIntersect(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay, Distance))
							{
								CurrentSelectionData->InstancedSubObjectsInfo[CurrentEntity].push_back(static_cast<int>(j));
							}
						}
					}
				}

				CurrentSelectionData->SceneEntitiesUnderMouse.push_back(CurrentEntity);
			}
		}
	}
}

void FEEditorSelectedObject::RenderEntitySelectionColorID(FEEntity* Entity, glm::vec3 ColorID, FEEntity* CameraEntity, FESelectionData* CurrentSelectionData)
{
	if (Entity == nullptr || CameraEntity == nullptr)
		return;

	if (Entity->HasComponent<FEGameModelComponent>())
	{
		FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
		if (!GameModelComponent.IsVisible())
			return;

		FEMaterial* RegularMaterial = GameModelComponent.GetGameModel()->Material;
		GameModelComponent.GetGameModel()->Material = PixelAccurateSelectionMaterial;
		PixelAccurateSelectionMaterial->SetBaseColor(ColorID);
		PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
		PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
		PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			RENDERER.RenderGameModelComponent(Entity, CameraEntity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{
			// Update instanced object only if it is not in individual select mode
			if (!INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(Entity))
			{
				PixelAccurateSelectionMaterial->Shader = FEPixelAccurateInstancedSelection;
				FEMaterial* RegularBillboardMaterials = GameModelComponent.GetGameModel()->GetBillboardMaterial();
				GameModelComponent.GetGameModel()->SetBillboardMaterial(PixelAccurateSelectionMaterial);

				FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
				RENDERER.RenderGameModelComponentWithInstanced(Entity, CameraEntity);

				PixelAccurateSelectionMaterial->Shader = FEPixelAccurateSelection;
				GameModelComponent.GetGameModel()->SetBillboardMaterial(RegularBillboardMaterials);
			}
		}

		GameModelComponent.GetGameModel()->Material = RegularMaterial;
		PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr);
		PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr, 1);
	}
	else if (Entity->HasComponent<FETerrainComponent>())
	{
		FETerrainComponent& TerrainComponent = Entity->GetComponent<FETerrainComponent>();
		if (!TerrainComponent.IsVisible())
			return;

		TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
		TerrainComponent.Shader->UpdateParameterData("baseColor", ColorID);
		RENDERER.RenderTerrainComponent(Entity, CameraEntity);
		TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f));
		TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
	}
	else if (Entity->HasComponent<FEInstancedComponent>() && Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		PixelAccurateSelectionMaterial->Shader = FEPixelAccurateInstancedSelection;

		FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
		for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
		{
			FEEntity* EntityWithGameModel = INSTANCED_RENDERING_SYSTEM.GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
			if (EntityWithGameModel == nullptr)
				continue;

			FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
			if (!GameModelComponent.IsVisible())
				continue;

			FEMaterial* RegularMaterial = GameModelComponent.GetGameModel()->Material;
			GameModelComponent.GetGameModel()->Material = PixelAccurateSelectionMaterial;
			PixelAccurateSelectionMaterial->SetBaseColor(ColorID);
			PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);
			
			FEMaterial* RegularBillboardMaterials = GameModelComponent.GetGameModel()->GetBillboardMaterial();
			GameModelComponent.GetGameModel()->SetBillboardMaterial(PixelAccurateSelectionMaterial);

			RENDERER.RenderGameModelComponentWithInstanced(Entity, nullptr, false, false, i);

			GameModelComponent.GetGameModel()->SetBillboardMaterial(RegularBillboardMaterials);
			GameModelComponent.GetGameModel()->Material = RegularMaterial;
			PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr);
			PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr, 1);
		}

		PixelAccurateSelectionMaterial->Shader = FEPixelAccurateSelection;
	}
	else if (Entity->HasComponent<FEVirtualUIComponent>())
	{
		FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();
		if (!VirtualUIComponent.IsVisible() || VirtualUIComponent.IsInputActive())
			return;

		PixelAccurateSelectionMaterial->SetBaseColor(ColorID);
		PixelAccurateSelectionMaterial->ClearAllTexturesInfo();

		VIRTUAL_UI_SYSTEM.RenderVirtualUIComponent(Entity, PixelAccurateSelectionMaterial);

		PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr);
		PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr, 1);
	}

	PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
}

int FEEditorSelectedObject::GetIndexOfObjectUnderMouse(const double MouseX, const double MouseY, FEScene* Scene)
{
	FESelectionData* CurrentSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return -1;

#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (!CurrentSelectionData->CheckForSelectionisNeeded)
		return -1;
#endif

	CurrentSelectionData->CheckForSelectionisNeeded = false;

	int LocalMouseX = static_cast<int>(MouseX);
	int LocalMouseY = static_cast<int>(MouseY);

	FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(Scene);
	glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
	LocalMouseX -= ViewportPosition.x;
	LocalMouseY -= ViewportPosition.y;

	// If the mouse is out of the viewport, return -1
	if (LocalMouseY < 0)
		return -1;

	CurrentSelectionData->PixelAccurateSelectionFB->Bind();
	glm::ivec4 OriginalViewport = RENDERER.GetViewport();
	RENDERER.SetViewport(0, 0, CurrentSelectionData->PixelAccurateSelectionFB->GetWidth(), CurrentSelectionData->PixelAccurateSelectionFB->GetHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	FEEntity* CurrentCamera = CAMERA_SYSTEM.GetMainCameraEntity(Scene);

	for (size_t i = 0; i < CurrentSelectionData->SceneEntitiesUnderMouse.size(); i++)
	{
#ifdef EDITOR_SELECTION_DEBUG_MODE
		int r = (i + 1) * 50 & 255;
		int g = ((i + 1) * 50 >> 8) & 255;
		int b = ((i + 1) * 50 >> 16) & 255;
#else
		int r = (i + 1) & 255;
		int g = ((i + 1) >> 8) & 255;
		int b = ((i + 1) >> 16) & 255;
#endif

		RenderEntitySelectionColorID(CurrentSelectionData->SceneEntitiesUnderMouse[i],
									 glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f),
									 CurrentCamera, CurrentSelectionData);
	}

	int LastColorShiftIndex = static_cast<int>(CurrentSelectionData->SceneEntitiesUnderMouse.size() - 1);

	auto InstancedSubObjectIterator = CurrentSelectionData->InstancedSubObjectsInfo.begin();
	while (InstancedSubObjectIterator != CurrentSelectionData->InstancedSubObjectsInfo.end())
	{
		for (size_t j = 0; j < InstancedSubObjectIterator->second.size(); j++)
		{
			LastColorShiftIndex++;
			int r = (LastColorShiftIndex + 1) & 255;
			int g = ((LastColorShiftIndex + 1) >> 8) & 255;
			int b = ((LastColorShiftIndex + 1) >> 16) & 255;

			static FEEntity* DummyEntity = Scene->CreateEntity("DummyEntity");
			RESOURCE_MANAGER.SetTag(DummyEntity, EDITOR_RESOURCE_TAG);
			if (!DummyEntity->HasComponent<FEGameModelComponent>())
				DummyEntity->AddComponent<FEGameModelComponent>();
			
			FEGameModelComponent& DummyGameModelComponent = DummyEntity->GetComponent<FEGameModelComponent>();
			FEGameModelComponent& OriginalGameModelComponent = InstancedSubObjectIterator->first->GetComponent<FEGameModelComponent>();

			DummyGameModelComponent.SetGameModel(OriginalGameModelComponent.GetGameModel());
			DummyGameModelComponent.SetVisibility(true);

			FEInstancedComponent& InstancedComponent = InstancedSubObjectIterator->first->GetComponent<FEInstancedComponent>();
			FETransformComponent& DummyTransformComponent = DummyEntity->GetComponent<FETransformComponent>();

			glm::dvec3 Position, Scale;
			glm::dquat Rotation;
			GEOMETRY.DecomposeMatrixToTranslationRotationScale(InstancedComponent.GetTransformedInstancedMatrix(InstancedSubObjectIterator->second[j]), Position, Rotation, Scale);

			DummyTransformComponent.SetPosition(Position);
			DummyTransformComponent.SetQuaternion(Rotation);
			DummyTransformComponent.SetScale(Scale);

			RenderEntitySelectionColorID(DummyEntity, glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f), CurrentCamera, CurrentSelectionData);

			DummyGameModelComponent.SetVisibility(false);
		}
		InstancedSubObjectIterator++;
	}

	FETransformComponent& CameraTransformComponent = CurrentCamera->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = CurrentCamera->GetComponent<FECameraComponent>();

	FE_GL_ERROR(glReadPixels(static_cast<GLint>(LocalMouseX), GLint(CameraComponent.GetRenderTargetHeight() - LocalMouseY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, CurrentSelectionData->ColorUnderMouse));
	CurrentSelectionData->PixelAccurateSelectionFB->UnBind();
	RENDERER.SetViewport(OriginalViewport);

#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (!CurrentSelectionData->SceneEntitiesUnderMouse.empty())
	{
		CurrentSelectionData->ColorIndex = 0;
		CurrentSelectionData->ColorIndex |= static_cast<int>(CurrentSelectionData->ColorUnderMouse[2]);
		CurrentSelectionData->ColorIndex <<= 8;
		CurrentSelectionData->ColorIndex |= static_cast<int>(CurrentSelectionData->ColorUnderMouse[1]);
		CurrentSelectionData->ColorIndex <<= 8;
		CurrentSelectionData->ColorIndex |= static_cast<int>(CurrentSelectionData->ColorUnderMouse[0]);

		CurrentSelectionData->ColorIndex -= 1;

		if (CurrentSelectionData->ColorIndex != -1 && CurrentSelectionData->ColorIndex >= static_cast<int>(CurrentSelectionData->SceneEntitiesUnderMouse.size()))
		{
			CurrentSelectionData->ColorIndex -= static_cast<int>(CurrentSelectionData->SceneEntitiesUnderMouse.size());

			const FEEntity* SelectedSubObjectInInstance = nullptr;
			auto InstancedSubObjectIterator = CurrentSelectionData->InstancedSubObjectsInfo.begin();
			while (InstancedSubObjectIterator != CurrentSelectionData->InstancedSubObjectsInfo.end())
			{
				if (CurrentSelectionData->ColorIndex < static_cast<int>(InstancedSubObjectIterator->second.size()))
				{
					CurrentSelectionData->InstancedSubObjectIndexSelected = InstancedSubObjectIterator->second[CurrentSelectionData->ColorIndex];
					SelectedSubObjectInInstance = InstancedSubObjectIterator->first;
					break;
				}

				CurrentSelectionData->ColorIndex -= static_cast<int>(InstancedSubObjectIterator->second.size());
				InstancedSubObjectIterator++;
			}

			if (SelectedSubObjectInInstance != nullptr)
			{
				for (size_t i = 0; i < CurrentSelectionData->SceneEntitiesUnderMouse.size(); i++)
				{
					if (CurrentSelectionData->SceneEntitiesUnderMouse[i]->GetObjectID() == SelectedSubObjectInInstance->GetObjectID())
					{
						return static_cast<int>(i);
					}
				}
			}
		}
		else if (CurrentSelectionData->ColorIndex != -1)
		{
			return CurrentSelectionData->ColorIndex;
		}
	}

	SELECTED.Clear(Scene);
	return -1;
#else
	CurrentSelectionData->ColorIndex = 0;
	CurrentSelectionData->ColorIndex |= int(CurrentSelectionData->ColorUnderMouse[2]);
	CurrentSelectionData->ColorIndex <<= 8;
	CurrentSelectionData->ColorIndex |= int(CurrentSelectionData->ColorUnderMouse[1]);
	CurrentSelectionData->ColorIndex <<= 8;
	CurrentSelectionData->ColorIndex |= int(CurrentSelectionData->ColorUnderMouse[0]);

	CurrentSelectionData->ColorIndex /= 50;
	CurrentSelectionData->ColorIndex -= 1;

	return -1;
#endif
}

void FEEditorSelectedObject::RenderEntityHaloEffectInternal(FEEntity* Entity, glm::vec3 Color, FEEntity* CameraEntity, FESelectionData* CurrentSelectionData)
{
	if (Entity == nullptr)
		return;

	HALO_SELECTION_EFFECT.HaloMaterial->SetBaseColor(Color);

	if (Entity->HasComponent<FEGameModelComponent>())
	{
		FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
		FEMaterial* RegularMaterial = GameModelComponent.GetGameModel()->Material;

		GameModelComponent.GetGameModel()->Material = HALO_SELECTION_EFFECT.HaloMaterial;
		HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
		HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			RENDERER.RenderGameModelComponent(Entity, CameraEntity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{
			HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawInstancedObjectShader;
			
			FEMaterial* RegularBillboardMaterial = GameModelComponent.GetGameModel()->GetBillboardMaterial();
			GameModelComponent.GetGameModel()->SetBillboardMaterial(HALO_SELECTION_EFFECT.HaloMaterial);

			RENDERER.RenderGameModelComponentWithInstanced(Entity, CameraEntity);

			GameModelComponent.GetGameModel()->SetBillboardMaterial(RegularBillboardMaterial);
			
			HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawObjectShader;
		}

		GameModelComponent.GetGameModel()->Material = RegularMaterial;
	}
	else if (Entity->HasComponent<FETerrainComponent>())
	{
		FETerrainComponent& TerrainComponent = Entity->GetComponent<FETerrainComponent>();
		TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
		TerrainComponent.Shader->UpdateParameterData("baseColor", Color);
		const float RegularLODLevel = TerrainComponent.GetLODLevel();
		TerrainComponent.SetLODLevel(0.0f);

		RENDERER.RenderTerrainComponent(Entity, CameraEntity);

		TerrainComponent.SetLODLevel(RegularLODLevel);
		TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f));
		TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
	}
	else if (Entity->HasComponent<FEInstancedComponent>() && Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawInstancedObjectShader;

		FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
		for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
		{
			FEEntity* EntityWithGameModel = INSTANCED_RENDERING_SYSTEM.GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
			if (EntityWithGameModel == nullptr)
				continue;

			FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();

			FEMaterial* RegularMaterial = GameModelComponent.GetGameModel()->Material;

			GameModelComponent.GetGameModel()->Material = HALO_SELECTION_EFFECT.HaloMaterial;
			HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
			HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

			FEMaterial* RegularBillboardMaterial = GameModelComponent.GetGameModel()->GetBillboardMaterial();
			GameModelComponent.GetGameModel()->SetBillboardMaterial(HALO_SELECTION_EFFECT.HaloMaterial);

			RENDERER.RenderGameModelComponentWithInstanced(Entity, nullptr, false, false, i);

			GameModelComponent.GetGameModel()->Material = RegularMaterial;
			GameModelComponent.GetGameModel()->SetBillboardMaterial(RegularBillboardMaterial);
		}

		HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawObjectShader;
	}
	else if (Entity->HasComponent<FEVirtualUIComponent>())
	{
		FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();
		if (!VirtualUIComponent.IsVisible() || VirtualUIComponent.IsInputActive())
			return;

		VIRTUAL_UI_SYSTEM.RenderVirtualUIComponent(Entity, HALO_SELECTION_EFFECT.HaloMaterial);
	}

	HALO_SELECTION_EFFECT.HaloMaterial->ClearAllTexturesInfo();
}

void FEEditorSelectedObject::OnCameraUpdate() const
{
	auto SceneIterator = PerSceneData.begin();
	while (SceneIterator != PerSceneData.end())
	{
		FESelectionData* CurrentSelectionData = SceneIterator->second;

		FEScene* CurrentScene = SCENE_MANAGER.GetScene(SceneIterator->first);
		if (CurrentScene == nullptr)
		{
			SceneIterator++;
			continue;
		}

		FEHaloSelectionData* HaloSelectionData = HALO_SELECTION_EFFECT.GetSceneData(CurrentScene->GetObjectID());
		FEEntity* CurrentCamera = CAMERA_SYSTEM.GetMainCameraEntity(CurrentScene);
		if (CurrentCamera == nullptr || HaloSelectionData->PostProcess == nullptr)
		{
			SceneIterator++;
			continue;
		}

		HaloSelectionData->HaloObjectsFB->Bind();
		HALO_SELECTION_EFFECT.HaloMaterial->ClearAllTexturesInfo();

		FECameraComponent& CameraComponent = CurrentCamera->GetComponent<FECameraComponent>();
		RENDERER.SetViewport(0, 0, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

		FEEntity* SelectedEntity = CurrentScene->GetEntity(CurrentSelectionData->SelectedEntityID);
		if (SelectedEntity == nullptr)
		{
			HaloSelectionData->HaloObjectsFB->UnBind();
			HaloSelectionData->PostProcess->bActive = true;

			SceneIterator++;
			continue;
		}

		// If selected entity is instanced and in individual select mode.
		if (SelectedEntity->HasComponent<FEInstancedComponent>() && CurrentSelectionData->InstancedSubObjectIndexSelected != -1)
		{
			FEGameModelComponent& GameModelComponent = SelectedEntity->GetComponent<FEGameModelComponent>();
			
			if (!CurrentSelectionData->DummyEntity->HasComponent<FEGameModelComponent>())
				CurrentSelectionData->DummyEntity->AddComponent<FEGameModelComponent>();

			FEGameModelComponent& DummyGameModelComponent = CurrentSelectionData->DummyEntity->GetComponent<FEGameModelComponent>();
			DummyGameModelComponent.SetGameModel(GameModelComponent.GetGameModel());
			DummyGameModelComponent.SetVisibility(true);

			FEInstancedComponent& InstancedComponent = SelectedEntity->GetComponent<FEInstancedComponent>();
			FETransformComponent& DummyTransformComponent = CurrentSelectionData->DummyEntity->GetComponent<FETransformComponent>();

			glm::dvec3 Position, Scale;
			glm::dquat Rotation;
			GEOMETRY.DecomposeMatrixToTranslationRotationScale(InstancedComponent.GetTransformedInstancedMatrix(CurrentSelectionData->InstancedSubObjectIndexSelected), Position, Rotation, Scale);

			DummyTransformComponent.SetPosition(Position);
			DummyTransformComponent.SetQuaternion(Rotation);
			DummyTransformComponent.SetScale(Scale);

			SELECTED.RenderEntityHaloEffectInternal(CurrentSelectionData->DummyEntity, glm::vec3(0.61f, 0.86f, 1.0f), CurrentCamera, CurrentSelectionData);

			DummyGameModelComponent.SetVisibility(false);
		}
		else
		{
			SELECTED.RenderEntityHaloEffectInternal(SelectedEntity, glm::vec3(1.0f, 0.25f, 0.0f), CurrentCamera, CurrentSelectionData);
			FENaiveSceneGraphNode* SelectedEntityNode = CurrentScene->SceneGraph.GetNodeByEntityID(SelectedEntity->GetObjectID());
			std::vector<FENaiveSceneGraphNode*> AllChildren = SelectedEntityNode->GetRecursiveChildren();

			for (size_t i = 0; i < AllChildren.size(); i++)
			{
				FEEntity* CurrentEntity = AllChildren[i]->GetEntity();
				SELECTED.RenderEntityHaloEffectInternal(CurrentEntity, glm::vec3(0.61f, 0.86f, 1.0f), CurrentCamera, CurrentSelectionData);
			}
		}

		HaloSelectionData->HaloObjectsFB->UnBind();
		HaloSelectionData->PostProcess->bActive = true;

		SceneIterator++;
	}
}

int FEEditorSelectedObject::DebugGetLastColorIndex(FEScene* Scene)
{
	FESelectionData* CurrentSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return -1;

	return CurrentSelectionData->ColorIndex;
}

void FEEditorSelectedObject::SetSelectedByIndex(const size_t Index, FEScene* Scene)
{
	FESelectionData* CurrentSelectionData = GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return;

	if (Index < 0 || Index >= CurrentSelectionData->SceneEntitiesUnderMouse.size())
		return;

	FEEntity* CurrentlySelectedEntity = Scene->GetEntity(CurrentSelectionData->SelectedEntityID);
	if (CurrentlySelectedEntity != nullptr)
	{
		if (CurrentlySelectedEntity->HasComponent<FEInstancedComponent>())
		{
			if (CurrentSelectionData->SceneEntitiesUnderMouse[Index]->GetObjectID() != CurrentlySelectedEntity->GetObjectID())
			{
				INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(CurrentlySelectedEntity, false);
				CurrentSelectionData->InstancedSubObjectIndexSelected = -1;
			}
		}
	}

	CurrentSelectionData->SelectedEntityID = CurrentSelectionData->SceneEntitiesUnderMouse[Index]->GetObjectID();
	if (OnUpdateFunction != nullptr)
		OnUpdateFunction(Scene);
}

void FEEditorSelectedObject::ClearAllSceneData()
{
	auto PerSceneIterator = PerSceneData.begin();
	while (PerSceneIterator != PerSceneData.end())
	{
		delete PerSceneIterator->second;
		PerSceneIterator++;
	}

	PerSceneData.clear();
}

void FEEditorSelectedObject::ClearSceneData(const std::string& SceneID)
{
	auto FoundSceneData = PerSceneData.find(SceneID);
	if (FoundSceneData != PerSceneData.end())
	{
		delete FoundSceneData->second;
		PerSceneData.erase(FoundSceneData);
	}
}

void FEEditorSelectedObject::AddSceneData(const std::string& SceneID)
{
	FEScene* CurrentScene = SCENE_MANAGER.GetScene(SceneID);
	if (CurrentScene == nullptr)
		return;

	if (PerSceneData.find(SceneID) != PerSceneData.end())
		return;

	FEEntity* MainCamera = CAMERA_SYSTEM.GetMainCameraEntity(CurrentScene);
	if (MainCamera == nullptr)
		return;

	PerSceneData[SceneID] = new FESelectionData();
	PerSceneData[SceneID]->SceneID = SceneID;

	FETransformComponent& CameraTransformComponent = MainCamera->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = MainCamera->GetComponent<FECameraComponent>();

	PerSceneData[SceneID]->PixelAccurateSelectionFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());
	delete PerSceneData[SceneID]->PixelAccurateSelectionFB->GetColorAttachment();
	PerSceneData[SceneID]->PixelAccurateSelectionFB->SetColorAttachment(RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight()));

	if (CurrentScene->GetEntityByName("Editor_Selection_Dummy_Entity").empty())
	{
		PerSceneData[SceneID]->DummyEntity = CurrentScene->CreateEntity("Editor_Selection_Dummy_Entity");
		RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->DummyEntity, EDITOR_RESOURCE_TAG);
	}

	HALO_SELECTION_EFFECT.AddSceneData(SceneID);
}

FESelectionData* FEEditorSelectedObject::GetSceneData(const std::string& SceneID)
{
	auto FoundScene = PerSceneData.find(SceneID);
	if (FoundScene != PerSceneData.end())
	{
		return FoundScene->second;
	}

	return nullptr;
}

void FEEditorSelectedObject::ClearAll()
{
	SELECTED.ClearAllSceneData();
}