#include "FEEditorSelectedObject.h"
using namespace FocalEngine;

FEEditorSelectedObject* FEEditorSelectedObject::Instance = nullptr;
FEEditorSelectedObject::FEEditorSelectedObject() {}
FEEditorSelectedObject::~FEEditorSelectedObject() {}

void FEEditorSelectedObject::InitializeResources()
{
	HALO_SELECTION_EFFECT.InitializeResources();
	
	PixelAccurateSelectionFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());
	delete PixelAccurateSelectionFB->GetColorAttachment();
	PixelAccurateSelectionFB->SetColorAttachment(RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight()));

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

	// FIX ME! Temporary solution, only supports one scene
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	if (!ActiveScenes.empty())
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetActiveScenes()[0];
		DummyEntity = CurrentScene->CreateEntity("Editor_Selection_Dummy_Entity");
	}
}

void FEEditorSelectedObject::ReInitializeResources()
{
	HALO_SELECTION_EFFECT.ReInitializeResources();

	delete PixelAccurateSelectionFB;
	PixelAccurateSelectionFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());
	delete PixelAccurateSelectionFB->GetColorAttachment();
	PixelAccurateSelectionFB->SetColorAttachment(RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight()));

	// FIX ME! Temporary solution, only supports one scene
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	if (!ActiveScenes.empty())
	{
		FEScene* CurrentScene = SCENE_MANAGER.GetActiveScenes()[0];
		if (CurrentScene->GetEntityByName("Editor_Selection_Dummy_Entity").empty())
			DummyEntity = CurrentScene->CreateEntity("Editor_Selection_Dummy_Entity");
	}
}

void FEEditorSelectedObject::SetOnUpdateFunc(void(*Func)())
{
	OnUpdateFunc = Func;
}

FEEntity* FEEditorSelectedObject::GetSelected() const
{
	return Container;
}

bool FEEditorSelectedObject::GetDirtyFlag() const
{
	return bDirtyFlag;
}

void FEEditorSelectedObject::SetDirtyFlag(const bool NewValue)
{
	bDirtyFlag = NewValue;
}

void FEEditorSelectedObject::SetSelected(FEEntity* SelectedObject)
{
	if (SelectedObject == nullptr)
		return;

	if (Container != nullptr && Container->HasComponent<FETerrainComponent>() && Container != SelectedObject)
		TERRAIN_SYSTEM.SetBrushMode(Container, FE_TERRAIN_BRUSH_NONE);

	if (Container != SelectedObject)
		bDirtyFlag = true;

	Container = SelectedObject;
	if (OnUpdateFunc != nullptr)
		OnUpdateFunc();
}

void FEEditorSelectedObject::Clear()
{
	if (InstancedSubObjectIndexSelected != -1 && Container->HasComponent<FEInstancedComponent>())
	{
		INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(Container, false);
	}

	InstancedSubObjectIndexSelected = -1;
	Container = nullptr;
	bDirtyFlag = true;

	if (!SCENE_MANAGER.GetActiveScenes().empty())
		if (OnUpdateFunc != nullptr)
			OnUpdateFunc();
}

glm::dvec3 FEEditorSelectedObject::MouseRay(const double MouseX, const double MouseY) const
{
	glm::dvec2 NormalizedMouseCoords;
	NormalizedMouseCoords.x = (2.0f * MouseX) / ENGINE.GetRenderTargetWidth() - 1;
	NormalizedMouseCoords.y = 1.0f - (2.0f * (MouseY)) / ENGINE.GetRenderTargetHeight();

	const glm::dvec4 ClipCoords = glm::dvec4(NormalizedMouseCoords.x, NormalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 EyeCoords = glm::inverse(ENGINE.GetCamera()->GetProjectionMatrix()) * ClipCoords;
	EyeCoords.z = -1.0f;
	EyeCoords.w = 0.0f;
	glm::dvec3 WorldRay = glm::inverse(ENGINE.GetCamera()->GetViewMatrix()) * EyeCoords;
	WorldRay = glm::normalize(WorldRay);

	return WorldRay;
}

void FEEditorSelectedObject::DetermineEntityUnderMouse(const double MouseX, const double MouseY)
{
	SELECTED.SceneEntitiesUnderMouse.clear();
	SELECTED.InstancedSubObjectsInfo.clear();

	// FIX ME! Temporary solution, only supports one scene
	FEScene* CurrentScene = SCENE_MANAGER.GetActiveScenes()[0];

	const glm::vec3 MouseRayVector = MouseRay(MouseX, MouseY);
	const std::vector<std::string> EntityList = CurrentScene->GetEntityIDList();
	for (size_t i = 0; i < EntityList.size(); i++)
	{
		float Distance = 0;

		FEEntity* CurrentEntity = CurrentScene->GetEntity(EntityList[i]);
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

			if (Box.RayIntersect(ENGINE.GetCamera()->GetPosition(), MouseRayVector, Distance))
			{
				if (CurrentEntity->HasComponent<FEInstancedComponent>())
				{
					FEInstancedComponent& InstancedComponent = CurrentEntity->GetComponent<FEInstancedComponent>();
					if (INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(CurrentEntity))
					{
						InstancedSubObjectsInfo[CurrentEntity] = std::vector<int>();
						for (size_t j = 0; j < InstancedComponent.IndividualInstancedAABB.size(); j++)
						{
							if (InstancedComponent.IndividualInstancedAABB[j].RayIntersect(ENGINE.GetCamera()->GetPosition(), MouseRayVector, Distance))
							{
								InstancedSubObjectsInfo[CurrentEntity].push_back(static_cast<int>(j));
							}
						}
					}
				}

				SELECTED.SceneEntitiesUnderMouse.push_back(CurrentEntity);
			}
		}
	}
}

int FEEditorSelectedObject::GetIndexOfObjectUnderMouse(const double MouseX, const double MouseY)
{
#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (!CheckForSelectionisNeeded)
		return -1;
#endif

	CheckForSelectionisNeeded = false;

	PixelAccurateSelectionFB->Bind();
	glm::vec4 OriginalClearColor = ENGINE.GetClearColor();
	ENGINE.SetClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	for (size_t i = 0; i < SELECTED.SceneEntitiesUnderMouse.size(); i++)
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
		FEEntity* PotentiallySelectedEntity = SELECTED.SceneEntitiesUnderMouse[i];
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
				RENDERER.RenderGameModelComponent(PotentiallySelectedEntity, ENGINE.GetCamera(), false);
			}
			else if (PotentiallySelectedEntity->HasComponent<FEInstancedComponent>())
			{
				// Render instanced object only if it is not in individual select mode
				if (!INSTANCED_RENDERING_SYSTEM.IsIndividualSelectMode(PotentiallySelectedEntity))
				{
					PixelAccurateSelectionMaterial->Shader = FEPixelAccurateInstancedSelection;
					FEMaterial* RegularBillboardMaterials = GameModelComponent.GameModel->GetBillboardMaterial();
					GameModelComponent.GameModel->SetBillboardMaterial(PixelAccurateSelectionMaterial);

					FEInstancedComponent& InstancedComponent = PotentiallySelectedEntity->GetComponent<FEInstancedComponent>();
					RENDERER.RenderGameModelComponentWithInstanced(PotentiallySelectedEntity, ENGINE.GetCamera(), nullptr, false, false);

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
			RENDERER.RenderTerrainComponent(PotentiallySelectedEntity, ENGINE.GetCamera());
			TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f));
			TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
		}
	}

	int LastColorShiftIndex = static_cast<int>(SELECTED.SceneEntitiesUnderMouse.size() - 1);

	// FIX ME! Temporary solution, only supports one scene
	FEScene* CurrentScene = SCENE_MANAGER.GetActiveScenes()[0];

	auto it = InstancedSubObjectsInfo.begin();
	while (it != InstancedSubObjectsInfo.end())
	{
		for (size_t j = 0; j < it->second.size(); j++)
		{
			LastColorShiftIndex++;
			int r = (LastColorShiftIndex + 1) & 255;
			int g = ((LastColorShiftIndex + 1) >> 8) & 255;
			int b = ((LastColorShiftIndex + 1) >> 16) & 255;

			static FEEntity* DummyEntity = CurrentScene->CreateEntity("DummyEntity");
			if (!DummyEntity->HasComponent<FEGameModelComponent>())
				DummyEntity->AddComponent<FEGameModelComponent>();
			
			FEGameModelComponent& DummyGameModelComponent = DummyEntity->GetComponent<FEGameModelComponent>();
			FEGameModelComponent& OriginalGameModelComponent = it->first->GetComponent<FEGameModelComponent>();

			DummyGameModelComponent.GameModel = OriginalGameModelComponent.GameModel;
			DummyGameModelComponent.SetVisibility(true);

			FEInstancedComponent& InstancedComponent = it->first->GetComponent<FEInstancedComponent>();
			FETransformComponent& DummyTransformComponent = DummyEntity->GetComponent<FETransformComponent>();
			DummyTransformComponent = FETransformComponent(InstancedComponent.GetTransformedInstancedMatrix(it->second[j]));

			FEMaterial* RegularMaterial = OriginalGameModelComponent.GameModel->Material;
			DummyGameModelComponent.GameModel->Material = PixelAccurateSelectionMaterial;
			PixelAccurateSelectionMaterial->SetBaseColor(glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
			PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
			PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

			RENDERER.RenderGameModelComponent(DummyEntity/*DummyGameModelComponent, DummyTransformComponent*/, ENGINE.GetCamera(), false);

			OriginalGameModelComponent.GameModel->Material = RegularMaterial;
			DummyGameModelComponent.SetVisibility(false);
		}
		it++;
	}

	FE_GL_ERROR(glReadPixels(static_cast<GLint>(MouseX), GLint(ENGINE.GetRenderTargetHeight() - MouseY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, ColorUnderMouse));
	PixelAccurateSelectionFB->UnBind();

	ENGINE.SetClearColor(OriginalClearColor);

#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (!SELECTED.SceneEntitiesUnderMouse.empty())
	{
		ColorIndex = 0;
		ColorIndex |= static_cast<int>(ColorUnderMouse[2]);
		ColorIndex <<= 8;
		ColorIndex |= static_cast<int>(ColorUnderMouse[1]);
		ColorIndex <<= 8;
		ColorIndex |= static_cast<int>(ColorUnderMouse[0]);

		ColorIndex -= 1;

		if (ColorIndex != -1 && ColorIndex >= static_cast<int>(SELECTED.SceneEntitiesUnderMouse.size()))
		{
			ColorIndex -= static_cast<int>(SELECTED.SceneEntitiesUnderMouse.size());

			const FEEntity* SelectedSubObjectInInstance = nullptr;
			auto it = InstancedSubObjectsInfo.begin();
			while (it != InstancedSubObjectsInfo.end())
			{
				if (ColorIndex < static_cast<int>(it->second.size()))
				{
					InstancedSubObjectIndexSelected = it->second[ColorIndex];
					SelectedSubObjectInInstance = it->first;
					break;
				}

				ColorIndex -= static_cast<int>(it->second.size());
				it++;
			}

			if (SelectedSubObjectInInstance != nullptr)
			{
				for (size_t i = 0; i < SELECTED.SceneEntitiesUnderMouse.size(); i++)
				{
					if (SELECTED.SceneEntitiesUnderMouse[i]->GetObjectID() == SelectedSubObjectInInstance->GetObjectID())
					{
						return static_cast<int>(i);
					}
				}
			}
		}
		else if (ColorIndex != -1)
		{
			return ColorIndex;
		}
	}

	SELECTED.Clear();
	return -1;
#else
	ColorIndex = 0;
	ColorIndex |= int(ColorUnderMouse[2]);
	ColorIndex <<= 8;
	ColorIndex |= int(ColorUnderMouse[1]);
	ColorIndex <<= 8;
	ColorIndex |= int(ColorUnderMouse[0]);

	ColorIndex /= 50;
	ColorIndex -= 1;

	return -1;
#endif
}

void FEEditorSelectedObject::OnCameraUpdate() const
{
	HALO_SELECTION_EFFECT.HaloObjectsFb->Bind();
	HALO_SELECTION_EFFECT.HaloMaterial->ClearAllTexturesInfo();
	HALO_SELECTION_EFFECT.HaloMaterial->SetBaseColor(glm::vec3(1.0f, 0.25f, 0.0f));
	FE_GL_ERROR(glViewport(0, 0, RENDERER.SceneToTextureFB->GetWidth(), RENDERER.SceneToTextureFB->GetHeight()));
	glm::vec4 OriginalClearColor = ENGINE.GetClearColor();
	ENGINE.SetClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	//FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

	if (Container == nullptr /*|| !Container->HasComponent<FEGameModelComponent>()*/)
	{
		HALO_SELECTION_EFFECT.HaloObjectsFb->UnBind();
		ENGINE.SetClearColor(OriginalClearColor);
		HALO_SELECTION_EFFECT.PostProcess->bActive = true;

		return;
	}

	if (Container->HasComponent<FEGameModelComponent>())
	{

		FEGameModelComponent& GameModelComponent = Container->GetComponent<FEGameModelComponent>();
		FEMaterial* RegularMaterial = GameModelComponent.GameModel->Material;

		GameModelComponent.GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;
		HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
		HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

		if (!Container->HasComponent<FEInstancedComponent>())
		{
			RENDERER.RenderGameModelComponent(Container, ENGINE.GetCamera(), false);
		}
		else if (Container->HasComponent<FEInstancedComponent>())
		{
			if (InstancedSubObjectIndexSelected != -1)
			{
				if (!DummyEntity->HasComponent<FEGameModelComponent>())
					DummyEntity->AddComponent<FEGameModelComponent>();

				FEGameModelComponent& DummyGameModelComponent = DummyEntity->GetComponent<FEGameModelComponent>();
				DummyGameModelComponent.GameModel = GameModelComponent.GameModel;
				DummyGameModelComponent.SetVisibility(true);

				FEInstancedComponent& InstancedComponent = Container->GetComponent<FEInstancedComponent>();
				DummyEntity->GetComponent<FETransformComponent>() = FETransformComponent(InstancedComponent.GetTransformedInstancedMatrix(InstancedSubObjectIndexSelected));

				FEMaterial* RegularMaterial = GameModelComponent.GameModel->Material;
				DummyGameModelComponent.GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;

				HALO_SELECTION_EFFECT.HaloMaterial->SetBaseColor(glm::vec3(0.61f, 0.86f, 1.0f));
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

				RENDERER.RenderGameModelComponent(DummyEntity, ENGINE.GetCamera(), false);

				GameModelComponent.GameModel->Material = RegularMaterial;
				DummyGameModelComponent.SetVisibility(false);
			}
			else
			{
				HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawInstancedObjectShader;
				FEMaterial* RegularBillboardMaterial = GameModelComponent.GameModel->GetBillboardMaterial();
				GameModelComponent.GameModel->SetBillboardMaterial(HALO_SELECTION_EFFECT.HaloMaterial);

				RENDERER.RenderGameModelComponentWithInstanced(Container, ENGINE.GetCamera(), nullptr, false, false);

				HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawObjectShader;
				GameModelComponent.GameModel->SetBillboardMaterial(RegularBillboardMaterial);
			}
		}

		GameModelComponent.GameModel->Material = RegularMaterial;
	}
	else if (Container->HasComponent<FETerrainComponent>())
	{
		FETerrainComponent& TerrainComponent = Container->GetComponent<FETerrainComponent>();
		TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
		TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f, 0.25f, 0.0f));
		const float RegularLODLevel = TerrainComponent.GetLODLevel();
		TerrainComponent.SetLODLevel(0.0f);

		RENDERER.RenderTerrainComponent(Container, ENGINE.GetCamera());

		TerrainComponent.SetLODLevel(RegularLODLevel);
		TerrainComponent.Shader->UpdateParameterData("baseColor", glm::vec3(1.0f));
		TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
	}

	HALO_SELECTION_EFFECT.HaloObjectsFb->UnBind();
	ENGINE.SetClearColor(OriginalClearColor);
	HALO_SELECTION_EFFECT.PostProcess->bActive = true;
}

int FEEditorSelectedObject::DebugGetLastColorIndex() const
{
	return ColorIndex;
}

void FEEditorSelectedObject::SetSelectedByIndex(const size_t Index)
{
	if (Index < 0 || Index >= SceneEntitiesUnderMouse.size())
		return;

	if (Container != nullptr)
	{
		if (Container->HasComponent<FEInstancedComponent>())
		{
			if (SceneEntitiesUnderMouse[Index]->GetObjectID() != Container->GetObjectID())
			{
				INSTANCED_RENDERING_SYSTEM.SetIndividualSelectMode(Container, false);
				InstancedSubObjectIndexSelected = -1;
			}
		}
	}

	Container = SceneEntitiesUnderMouse[Index];
	if (OnUpdateFunc != nullptr)
		OnUpdateFunc();
}