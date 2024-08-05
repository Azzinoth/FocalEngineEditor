#include "FEEditorSelectedObject.h"
#include "FEProject.h"
#include "FEEditor.h"
using namespace FocalEngine;

FEEditorSelectedObject* FEEditorSelectedObject::Instance = nullptr;
FEEditorSelectedObject::FEEditorSelectedObject() {}
FEEditorSelectedObject::~FEEditorSelectedObject() {}

void FEEditorSelectedObject::InitializeResources()
{
	HALO_SELECTION_EFFECT.InitializeResources();
	
	PixelAccurateSelectionMaterial = RESOURCE_MANAGER.CreateMaterial("pixelAccurateSelectionMaterial");
	RESOURCE_MANAGER.MakeMaterialStandard(PixelAccurateSelectionMaterial);

	FEPixelAccurateSelection = RESOURCE_MANAGER.CreateShader("FEPixelAccurateSelection", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_VS.glsl").c_str(),
																						 RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 "4279660C7D3D27360358354E"/*"FEPixelAccurateSelection"*/);
	RESOURCE_MANAGER.MakeShaderStandard(FEPixelAccurateSelection);
	PixelAccurateSelectionMaterial->Shader = FEPixelAccurateSelection;

	FEPixelAccurateInstancedSelection = RESOURCE_MANAGER.CreateShader("FEPixelAccurateInstancedSelection", RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_INSTANCED_VS.glsl").c_str(),
																										   RESOURCE_MANAGER.LoadGLSL("Resources//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																									       nullptr,
																										   nullptr,
																										   nullptr,
																										   nullptr,
																										   "0E213D3542135C15471F0D6B"/*"FEPixelAccurateInstancedSelection"*/);

	RESOURCE_MANAGER.MakeShaderStandard(FEPixelAccurateInstancedSelection);

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

	return CurrentSelectionData->Container;
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
	}

	if (CurrentSelectionData->Container != nullptr && CurrentSelectionData->Container->HasComponent<FETerrainComponent>() && CurrentSelectionData->Container != SelectedObject)
		TERRAIN_SYSTEM.SetBrushMode(CurrentSelectionData->Container, FE_TERRAIN_BRUSH_NONE);

	CurrentSelectionData->Container = SelectedObject;
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

	if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1 && CurrentSelectionData->Container->HasComponent<FEInstancedComponent>())
	{
		INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(CurrentSelectionData->Container, false);
	}

	CurrentSelectionData->InstancedSubObjectIndexSelected = -1;
	CurrentSelectionData->Container = nullptr;

	if (!SCENE_MANAGER.GetActiveScenes().empty())
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
			FEAABB Box;
			if (CurrentEntity->HasComponent<FEGameModelComponent>() && !CurrentEntity->HasComponent<FEInstancedComponent>())
			{
				Box = CurrentEntity->GetComponent<FEGameModelComponent>().GameModel->GetMesh()->GetAABB().Transform(CurrentEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
			}
			else if (CurrentEntity->HasComponent<FEGameModelComponent>() && CurrentEntity->HasComponent<FEInstancedComponent>())
			{
				Box = INSTANCED_RENDERING_SYSTEM.GetAABB(CurrentEntity);
			}
			else if (CurrentEntity->HasComponent<FETerrainComponent>())
			{
				Box = TERRAIN_SYSTEM.GetAABB(CurrentEntity);
			}

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
		FEEntity* PotentiallySelectedEntity = CurrentSelectionData->SceneEntitiesUnderMouse[i];
		if (PotentiallySelectedEntity->HasComponent<FEGameModelComponent>())
		{
			FEGameModelComponent& GameModelComponent = PotentiallySelectedEntity->GetComponent<FEGameModelComponent>();
			if (!GameModelComponent.IsVisible())
				continue;

			// FIX ME! Prefab not supported
			FEMaterial* RegularMaterial = GameModelComponent.GameModel->Material;
			GameModelComponent.GameModel->Material = PixelAccurateSelectionMaterial;
			PixelAccurateSelectionMaterial->SetBaseColor(glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
			PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

			if (!PotentiallySelectedEntity->HasComponent<FEInstancedComponent>())
			{
				RENDERER.RenderGameModelComponent(PotentiallySelectedEntity, CurrentCamera);
			}
			else if (PotentiallySelectedEntity->HasComponent<FEInstancedComponent>())
			{
				// Update instanced object only if it is not in individual select mode
				if (!INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(PotentiallySelectedEntity))
				{
					PixelAccurateSelectionMaterial->Shader = FEPixelAccurateInstancedSelection;
					FEMaterial* RegularBillboardMaterials = GameModelComponent.GameModel->GetBillboardMaterial();
					GameModelComponent.GameModel->SetBillboardMaterial(PixelAccurateSelectionMaterial);

					FEInstancedComponent& InstancedComponent = PotentiallySelectedEntity->GetComponent<FEInstancedComponent>();
					RENDERER.RenderGameModelComponentWithInstanced(PotentiallySelectedEntity, CurrentCamera);

					PixelAccurateSelectionMaterial->Shader = FEPixelAccurateSelection;
					GameModelComponent.GameModel->SetBillboardMaterial(RegularBillboardMaterials);
				}
			}

			GameModelComponent.GameModel->Material = RegularMaterial;
			PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr);
			PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr, 1);
		}
		else if (PotentiallySelectedEntity->HasComponent<FETerrainComponent>())
		{
			FETerrainComponent& TerrainComponent = PotentiallySelectedEntity->GetComponent<FETerrainComponent>();
			if (!TerrainComponent.IsVisible())
				continue;

			TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
			TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
			RENDERER.RenderTerrainComponent(PotentiallySelectedEntity, CurrentCamera);
			TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f));
			TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
		}
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
			DummyEntity->GetComponent<FETagComponent>().SetTag(EDITOR_SCENE_TAG);
			if (!DummyEntity->HasComponent<FEGameModelComponent>())
				DummyEntity->AddComponent<FEGameModelComponent>();
			
			FEGameModelComponent& DummyGameModelComponent = DummyEntity->GetComponent<FEGameModelComponent>();
			FEGameModelComponent& OriginalGameModelComponent = InstancedSubObjectIterator->first->GetComponent<FEGameModelComponent>();

			DummyGameModelComponent.GameModel = OriginalGameModelComponent.GameModel;
			DummyGameModelComponent.SetVisibility(true);

			FEInstancedComponent& InstancedComponent = InstancedSubObjectIterator->first->GetComponent<FEInstancedComponent>();
			FETransformComponent& DummyTransformComponent = DummyEntity->GetComponent<FETransformComponent>();

			glm::dvec3 Position, Scale;
			glm::dquat Rotation;
			GEOMETRY.DecomposeMatrixToTranslationRotationScale(InstancedComponent.GetTransformedInstancedMatrix(InstancedSubObjectIterator->second[j]), Position, Rotation, Scale);

			DummyTransformComponent.SetPosition(Position);
			DummyTransformComponent.SetQuaternion(Rotation);
			DummyTransformComponent.SetScale(Scale);

			FEMaterial* RegularMaterial = OriginalGameModelComponent.GameModel->Material;
			DummyGameModelComponent.GameModel->Material = PixelAccurateSelectionMaterial;
			PixelAccurateSelectionMaterial->SetBaseColor(glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
			PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

			RENDERER.RenderGameModelComponent(DummyEntity, CurrentCamera);

			OriginalGameModelComponent.GameModel->Material = RegularMaterial;
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
		HALO_SELECTION_EFFECT.HaloMaterial->SetBaseColor(glm::vec3(1.0f, 0.25f, 0.0f));

		FECameraComponent& CameraComponent = CurrentCamera->GetComponent<FECameraComponent>();
		RENDERER.SetViewport(0, 0, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

		FEEntity* SelectedEntity = CurrentSelectionData->Container;

		if (SelectedEntity == nullptr)
		{
			HaloSelectionData->HaloObjectsFB->UnBind();
			HaloSelectionData->PostProcess->bActive = true;

			SceneIterator++;
			continue;
		}

		FEEntity* ParentPrefabInstance = PREFAB_INSTANCE_SYSTEM.GetParentPrefabInstanceIfAny(SelectedEntity);
		if (ParentPrefabInstance != nullptr && PREFAB_INSTANCE_SYSTEM.IsPrefabInstanceUnmodified(ParentPrefabInstance))
		{
			FENaiveSceneGraphNode* ParentPrefabInstanceNode = ParentPrefabInstance->GetParentScene()->SceneGraph.GetNodeByEntityID(ParentPrefabInstance->GetObjectID());
			std::vector<FENaiveSceneGraphNode*> PrefabElements = ParentPrefabInstanceNode->GetChildren();
			for (size_t i = 0; i < PrefabElements.size(); i++)
			{
				FEEntity* PrefabElement = PrefabElements[i]->GetEntity();
				if (PrefabElement->HasComponent<FEGameModelComponent>())
				{
					FEGameModelComponent& GameModelComponent = PrefabElement->GetComponent<FEGameModelComponent>();
					FEMaterial* RegularMaterial = GameModelComponent.GameModel->Material;

					GameModelComponent.GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;
					HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
					HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

					RENDERER.RenderGameModelComponent(PrefabElement, CurrentCamera);

					GameModelComponent.GameModel->Material = RegularMaterial;
				}
			}
		}
		else
		{
			if (SelectedEntity->HasComponent<FEGameModelComponent>())
			{
				FEGameModelComponent& GameModelComponent = SelectedEntity->GetComponent<FEGameModelComponent>();
				FEMaterial* RegularMaterial = GameModelComponent.GameModel->Material;

				GameModelComponent.GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

				if (!SelectedEntity->HasComponent<FEInstancedComponent>())
				{
					RENDERER.RenderGameModelComponent(SelectedEntity, CurrentCamera);
				}
				else if (SelectedEntity->HasComponent<FEInstancedComponent>())
				{
					if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1)
					{
						if (!CurrentSelectionData->DummyEntity->HasComponent<FEGameModelComponent>())
							CurrentSelectionData->DummyEntity->AddComponent<FEGameModelComponent>();

						FEGameModelComponent& DummyGameModelComponent = CurrentSelectionData->DummyEntity->GetComponent<FEGameModelComponent>();
						DummyGameModelComponent.GameModel = GameModelComponent.GameModel;
						DummyGameModelComponent.SetVisibility(true);

						FEInstancedComponent& InstancedComponent = SelectedEntity->GetComponent<FEInstancedComponent>();
						FETransformComponent& DummyTransformComponent = CurrentSelectionData->DummyEntity->GetComponent<FETransformComponent>();

						glm::dvec3 Position, Scale;
						glm::dquat Rotation;
						GEOMETRY.DecomposeMatrixToTranslationRotationScale(InstancedComponent.GetTransformedInstancedMatrix(CurrentSelectionData->InstancedSubObjectIndexSelected), Position, Rotation, Scale);

						DummyTransformComponent.SetPosition(Position);
						DummyTransformComponent.SetQuaternion(Rotation);
						DummyTransformComponent.SetScale(Scale);

						FEMaterial* RegularMaterial = GameModelComponent.GameModel->Material;
						DummyGameModelComponent.GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;

						HALO_SELECTION_EFFECT.HaloMaterial->SetBaseColor(glm::vec3(0.61f, 0.86f, 1.0f));
						HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
						HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

						RENDERER.RenderGameModelComponent(CurrentSelectionData->DummyEntity, CurrentCamera);

						GameModelComponent.GameModel->Material = RegularMaterial;
						DummyGameModelComponent.SetVisibility(false);
					}
					else
					{
						HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawInstancedObjectShader;
						FEMaterial* RegularBillboardMaterial = GameModelComponent.GameModel->GetBillboardMaterial();
						GameModelComponent.GameModel->SetBillboardMaterial(HALO_SELECTION_EFFECT.HaloMaterial);

						RENDERER.RenderGameModelComponentWithInstanced(SelectedEntity, CurrentCamera);

						HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawObjectShader;
						GameModelComponent.GameModel->SetBillboardMaterial(RegularBillboardMaterial);
					}
				}

				GameModelComponent.GameModel->Material = RegularMaterial;
			}
			else if (SelectedEntity->HasComponent<FETerrainComponent>())
			{
				FETerrainComponent& TerrainComponent = SelectedEntity->GetComponent<FETerrainComponent>();
				TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
				TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f, 0.25f, 0.0f));
				const float RegularLODLevel = TerrainComponent.GetLODLevel();
				TerrainComponent.SetLODLevel(0.0f);

				RENDERER.RenderTerrainComponent(SelectedEntity, CurrentCamera);

				TerrainComponent.SetLODLevel(RegularLODLevel);
				TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f));
				TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
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

	if (CurrentSelectionData->Container != nullptr)
	{
		if (CurrentSelectionData->Container->HasComponent<FEInstancedComponent>())
		{
			if (CurrentSelectionData->SceneEntitiesUnderMouse[Index]->GetObjectID() != CurrentSelectionData->Container->GetObjectID())
			{
				INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(CurrentSelectionData->Container, false);
				CurrentSelectionData->InstancedSubObjectIndexSelected = -1;
			}
		}
	}

	CurrentSelectionData->Container = CurrentSelectionData->SceneEntitiesUnderMouse[Index];
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

	PerSceneData[SceneID] = new FESelectionData();
	PerSceneData[SceneID]->SceneID = SceneID;

	FEEntity* MainCamera = CAMERA_SYSTEM.GetMainCameraEntity(CurrentScene);
	FETransformComponent& CameraTransformComponent = MainCamera->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = MainCamera->GetComponent<FECameraComponent>();

	PerSceneData[SceneID]->PixelAccurateSelectionFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());
	delete PerSceneData[SceneID]->PixelAccurateSelectionFB->GetColorAttachment();
	PerSceneData[SceneID]->PixelAccurateSelectionFB->SetColorAttachment(RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight()));

	if (CurrentScene->GetEntityByName("Editor_Selection_Dummy_Entity").empty())
	{
		PerSceneData[SceneID]->DummyEntity = CurrentScene->CreateEntity("Editor_Selection_Dummy_Entity");
		PerSceneData[SceneID]->DummyEntity->GetComponent<FETagComponent>().SetTag(EDITOR_SCENE_TAG);
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