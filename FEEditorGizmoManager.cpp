#include "FEEditorGizmoManager.h"
using namespace FocalEngine;
#include "FEEditor.h"

GizmoManager::GizmoManager() {}
GizmoManager::~GizmoManager() {}

void GizmoManager::InitializeResources()
{
	SELECTED.SetOnUpdateFunction(OnSelectedObjectUpdate);
	FEMesh* TransformationGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "45191B6F172E3B531978692E.model").c_str(), "transformationGizmoMesh");
	RESOURCE_MANAGER.SetTag(TransformationGizmoMesh, EDITOR_RESOURCE_TAG);

	// TransformationXGizmo
	FEMaterial* CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	TransformationXGizmoGM = RESOURCE_MANAGER.CreateGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationXGizmoGM");
	RESOURCE_MANAGER.SetTag(TransformationXGizmoGM, EDITOR_RESOURCE_TAG);

	// TransformationYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	TransformationYGizmoGM = RESOURCE_MANAGER.CreateGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationYGizmoGM");
	RESOURCE_MANAGER.SetTag(TransformationYGizmoGM, EDITOR_RESOURCE_TAG);

	// TransformationZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	TransformationZGizmoGM = RESOURCE_MANAGER.CreateGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationZGizmoGM");
	RESOURCE_MANAGER.SetTag(TransformationZGizmoGM, EDITOR_RESOURCE_TAG);

	// Plane gizmos
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	TransformationXYGizmoGM = RESOURCE_MANAGER.CreateGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXYGizmoGM");
	RESOURCE_MANAGER.SetTag(TransformationXYGizmoGM, EDITOR_RESOURCE_TAG);

	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationYZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	TransformationYZGizmoGM = RESOURCE_MANAGER.CreateGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationYZGizmoGM");
	RESOURCE_MANAGER.SetTag(TransformationYZGizmoGM, EDITOR_RESOURCE_TAG);

	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	TransformationXZGizmoGM = RESOURCE_MANAGER.CreateGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXZGizmoGM");
	RESOURCE_MANAGER.SetTag(TransformationXZGizmoGM, EDITOR_RESOURCE_TAG);

	// Scale gizmos.
	FEMesh* ScaleGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "637C784B2E5E5C6548190E1B.model").c_str(), "scaleGizmoMesh");
	RESOURCE_MANAGER.SetTag(ScaleGizmoMesh, EDITOR_RESOURCE_TAG);

	// ScaleXGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	ScaleXGizmoGM = RESOURCE_MANAGER.CreateGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleXGizmoGM");
	RESOURCE_MANAGER.SetTag(ScaleXGizmoGM, EDITOR_RESOURCE_TAG);

	// ScaleYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	ScaleYGizmoGM = RESOURCE_MANAGER.CreateGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleYGizmoGM");
	RESOURCE_MANAGER.SetTag(ScaleYGizmoGM, EDITOR_RESOURCE_TAG);

	// ScaleZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	ScaleZGizmoGM = RESOURCE_MANAGER.CreateGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleZGizmoGM");
	RESOURCE_MANAGER.SetTag(ScaleZGizmoGM, EDITOR_RESOURCE_TAG);

	// RotateAroundAxis gizmos
	FEMesh* RotateGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "19622421516E5B317E1B5360.model").c_str(), "rotateGizmoMesh");
	RESOURCE_MANAGER.SetTag(RotateGizmoMesh, EDITOR_RESOURCE_TAG);

	// RotateXGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	RotateXGizmoGM = RESOURCE_MANAGER.CreateGameModel(RotateGizmoMesh, CurrentMaterial, "RotateXGizmoGM");
	RESOURCE_MANAGER.SetTag(RotateXGizmoGM, EDITOR_RESOURCE_TAG);

	// RotateYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	RotateYGizmoGM = RESOURCE_MANAGER.CreateGameModel(RotateGizmoMesh, CurrentMaterial, "RotateYGizmoGM");
	RESOURCE_MANAGER.SetTag(RotateYGizmoGM, EDITOR_RESOURCE_TAG);

	// RotateZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.SetTag(CurrentMaterial, EDITOR_RESOURCE_TAG);

	RotateZGizmoGM = RESOURCE_MANAGER.CreateGameModel(RotateGizmoMesh, CurrentMaterial, "RotateZGizmoGM");
	RESOURCE_MANAGER.SetTag(RotateZGizmoGM, EDITOR_RESOURCE_TAG);

	TransformationGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "456A31026A1C3152181A6064.texture").c_str(), "transformationGizmoIcon");
	RESOURCE_MANAGER.SetTag(TransformationGizmoIcon, EDITOR_RESOURCE_TAG);
	ScaleGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "3F2118296C1E4533506A472E.texture").c_str(), "scaleGizmoIcon");
	RESOURCE_MANAGER.SetTag(ScaleGizmoIcon, EDITOR_RESOURCE_TAG);
	RotateGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "7F6057403249580D73311B54.texture").c_str(), "rotateGizmoIcon");
	RESOURCE_MANAGER.SetTag(RotateGizmoIcon, EDITOR_RESOURCE_TAG);
}

void GizmoManager::HideAllGizmo(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	GizmoSceneData->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	GizmoSceneData->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	GizmoSceneData->bTransformationXGizmoActive = false;
	GizmoSceneData->bTransformationYGizmoActive = false;
	GizmoSceneData->bTransformationZGizmoActive = false;

	GizmoSceneData->bTransformationXYGizmoActive = false;
	GizmoSceneData->bTransformationYZGizmoActive = false;
	GizmoSceneData->bTransformationXZGizmoActive = false;

	GizmoSceneData->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	GizmoSceneData->bScaleXGizmoActive = false;
	GizmoSceneData->bScaleYGizmoActive = false;
	GizmoSceneData->bScaleZGizmoActive = false;

	GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	GizmoSceneData->bRotateXGizmoActive = false;
	GizmoSceneData->bRotateYGizmoActive = false;
	GizmoSceneData->bRotateZGizmoActive = false;
}

void GizmoManager::UpdateGizmoState(int NewState, FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	if (SELECTED.GetSelected(Scene) != nullptr && SELECTED.GetSelected(Scene)->HasComponent<FETerrainComponent>())
	{
		if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
			return;
	}

	if (NewState < 0 || NewState > 2)
		NewState = 0;

	GizmoSceneData->GizmosState = NewState;
	HideAllGizmo(Scene);

	if (SELECTED.GetSelected(Scene) == nullptr)
		return;

	switch (NewState)
	{
		case TRANSFORM_GIZMOS:
		{
			GizmoSceneData->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);

			GizmoSceneData->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);

			break;
		}
		case SCALE_GIZMOS:
		{
			GizmoSceneData->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			
			break;
		}
		case ROTATE_GIZMOS:
		{
			GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		
			break;
		}
		default:
			break;
	}
}

void GizmoManager::DeactivateAllGizmo(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	GizmoSceneData->bTransformationXGizmoActive = false;
	GizmoSceneData->bTransformationYGizmoActive = false;
	GizmoSceneData->bTransformationZGizmoActive = false;

	GizmoSceneData->bTransformationXYGizmoActive = false;
	GizmoSceneData->bTransformationYZGizmoActive = false;
	GizmoSceneData->bTransformationXZGizmoActive = false;

	GizmoSceneData->bScaleXGizmoActive = false;
	GizmoSceneData->bScaleYGizmoActive = false;
	GizmoSceneData->bScaleZGizmoActive = false;

	GizmoSceneData->bRotateXGizmoActive = false;
	GizmoSceneData->bRotateYGizmoActive = false;
	GizmoSceneData->bRotateZGizmoActive = false;
}

void GizmoManager::Update()
{
	auto PerSceneIterator = PerSceneData.begin();
	while (PerSceneIterator != PerSceneData.end())
	{
		if (SCENE_MANAGER.GetScene(PerSceneIterator->first) == nullptr)
		{
			PerSceneIterator = PerSceneData.erase(PerSceneIterator);
			continue;
		}

		FEGizmoSceneData* GizmoSceneData = PerSceneIterator->second;
		if (GizmoSceneData == nullptr)
		{
			PerSceneIterator++;
			continue;
		}
		
		FEScene* Scene = SCENE_MANAGER.GetScene(GizmoSceneData->SceneID);
		if (SELECTED.GetSelected(Scene) == nullptr || CAMERA_SYSTEM.GetMainCameraEntity(Scene) == nullptr)
		{
			HideAllGizmo(Scene);
			PerSceneIterator++;
			continue;
		}

		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FETransformComponent>();
		FETransformComponent& ObjTransform = GetTransformComponentOfSelectedObject(Scene);
		const glm::vec3 ObjectSpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glm::vec3 ToObject = ObjectSpaceOriginInWorldSpace - CameraTransformComponent.GetPosition(FE_WORLD_SPACE);
		ToObject = glm::normalize(ToObject);

		GizmoSceneData->ParentGizmoEntity->GetComponent<FETransformComponent>().SetPosition((CameraTransformComponent.GetPosition(FE_WORLD_SPACE) + ToObject * 0.15f));
		if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS)
		{
			// X Gizmos
			GizmoSceneData->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
			if (GizmoSceneData->bTransformationXZGizmoActive || GizmoSceneData->bTransformationXYGizmoActive || GizmoSceneData->bTransformationXGizmoActive)
				GizmoSceneData->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

			// Y Gizmos
			GizmoSceneData->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
			if (GizmoSceneData->bTransformationYZGizmoActive || GizmoSceneData->bTransformationXYGizmoActive || GizmoSceneData->bTransformationYGizmoActive)
				GizmoSceneData->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

			// Z Gizmos
			GizmoSceneData->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
			if (GizmoSceneData->bTransformationYZGizmoActive || GizmoSceneData->bTransformationXZGizmoActive || GizmoSceneData->bTransformationZGizmoActive)
				GizmoSceneData->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

			// XY Gizmos 
			GizmoSceneData->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
			if (GizmoSceneData->bTransformationXYGizmoActive)
				GizmoSceneData->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

			// YZ Gizmos
			GizmoSceneData->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
			if (GizmoSceneData->bTransformationYZGizmoActive)
				GizmoSceneData->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

			// XZ Gizmos
			GizmoSceneData->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
			if (GizmoSceneData->bTransformationXZGizmoActive)
				GizmoSceneData->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
		}
		else if (GizmoSceneData->GizmosState == SCALE_GIZMOS)
		{
			// X Gizmos
			GizmoSceneData->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
			if (GizmoSceneData->bScaleXGizmoActive)
				GizmoSceneData->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

			// Y Gizmos
			GizmoSceneData->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
			if (GizmoSceneData->bScaleYGizmoActive)
				GizmoSceneData->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

			// Z Gizmos
			GizmoSceneData->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
			if (GizmoSceneData->bScaleZGizmoActive)
				GizmoSceneData->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
		}
		else if (GizmoSceneData->GizmosState == ROTATE_GIZMOS)
		{
			if (SELECTED.SELECTED.GetSelected(Scene) != nullptr)
			{
				GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
				GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
				GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			}

			// X Gizmos
			GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
			if (GizmoSceneData->bRotateXGizmoActive)
			{
				GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
				GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
				GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
			}

			// Y Gizmos
			GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
			if (GizmoSceneData->bRotateYGizmoActive)
			{
				GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
				GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
				GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
			}

			// Z Gizmos
			GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
			if (GizmoSceneData->bRotateZGizmoActive)
			{
				GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().GetGameModel()->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
				GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
				GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
			}
		}

		PerSceneIterator++;
	}
}

bool GizmoManager::WasSelected(int Index, FEScene* Scene)
{
	if (Scene == nullptr)
		return false;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return false;

	DeactivateAllGizmo(Scene);

	FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData == nullptr)
		return false;

	FEEntity* SelectedEntity = CurrentSelectionData->SceneEntitiesUnderMouse[Index];
	FETransformComponent& CurrentTransform = SelectedEntity->GetComponent<FETransformComponent>();

	if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->TransformationXGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bTransformationXGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->TransformationYGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bTransformationYGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->TransformationZGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bTransformationZGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->TransformationXYGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bTransformationXYGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->TransformationYZGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bTransformationYZGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->TransformationXZGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bTransformationXZGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == SCALE_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->ScaleXGizmoEntity->GetObjectID())
	{
		if (SelectedEntity != nullptr && CurrentTransform.IsUniformScalingSet())
		{
			GizmoSceneData->bScaleXGizmoActive = true;
			GizmoSceneData->bScaleYGizmoActive = true;
			GizmoSceneData->bScaleZGizmoActive = true;
		}

		GizmoSceneData->bScaleXGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == SCALE_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->ScaleYGizmoEntity->GetObjectID())
	{
		if (SelectedEntity != nullptr && CurrentTransform.IsUniformScalingSet())
		{
			GizmoSceneData->bScaleXGizmoActive = true;
			GizmoSceneData->bScaleYGizmoActive = true;
			GizmoSceneData->bScaleZGizmoActive = true;
		}

		GizmoSceneData->bScaleYGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == SCALE_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->ScaleZGizmoEntity->GetObjectID())
	{
		if (SelectedEntity != nullptr && CurrentTransform.IsUniformScalingSet())
		{
			GizmoSceneData->bScaleXGizmoActive = true;
			GizmoSceneData->bScaleYGizmoActive = true;
			GizmoSceneData->bScaleZGizmoActive = true;
		}

		GizmoSceneData->bScaleZGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == ROTATE_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->RotateXGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bRotateXGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == ROTATE_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->RotateYGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bRotateYGizmoActive = true;
	}
	else if (GizmoSceneData->GizmosState == ROTATE_GIZMOS && SelectedEntity->GetObjectID() == GizmoSceneData->RotateZGizmoEntity->GetObjectID())
	{
		GizmoSceneData->bRotateZGizmoActive = true;
	}
	else
	{
		return false;
	}

	return true;
}

glm::vec3 GizmoManager::GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal, FEScene* Scene)
{
	if (Scene == nullptr)
		return glm::vec3();

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return glm::vec3();

	FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FECameraComponent>();
	FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(Scene);
	glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
	glm::ivec2 ViewportSize = glm::ivec2(CurrentViewport->GetWidth(), CurrentViewport->GetHeight());

	FETransformComponent& ObjTransform = GetTransformComponentOfSelectedObject(Scene);
	const glm::vec3 EntitySpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	const glm::vec3 LastFrameMouseRay = GEOMETRY.CreateMouseRayToWorld(LastMouseX, LastMouseY,
																	   CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
																	   ViewportPosition, ViewportSize);
	const glm::vec3 MouseRay = GEOMETRY.CreateMouseRayToWorld(MouseX, MouseY,
															  CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
															  ViewportPosition, ViewportSize);

	const glm::vec3 CameraPosition = CameraTransformComponent.GetPosition(FE_WORLD_SPACE);

	const float SignedDistanceToOrigin = glm::dot(PlaneNormal, EntitySpaceOriginInWorldSpace);

	const float Nominator = SignedDistanceToOrigin - glm::dot(CameraPosition, PlaneNormal);
	const float LastDenominator = glm::dot(LastFrameMouseRay, PlaneNormal);
	const float Denominator = glm::dot(MouseRay, PlaneNormal);

	if (Denominator == 0 || LastDenominator == 0)
		return glm::vec3(0.0f);

	const float LastIntersectionT = Nominator / LastDenominator;
	const float IntersectionT = Nominator / Denominator;

	const glm::vec3 LastPointOnPlane = CameraPosition + LastIntersectionT * LastFrameMouseRay;
	const glm::vec3 PointOnPlane = CameraPosition + IntersectionT * MouseRay;

	return PointOnPlane - LastPointOnPlane;
}

glm::vec3 GizmoManager::GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal, glm::vec3& LastMousePointOnPlane, FEScene* Scene)
{
	if (Scene == nullptr)
		return glm::vec3();

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return glm::vec3();

	FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FECameraComponent>();
	FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(Scene);
	glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
	glm::ivec2 ViewportSize = glm::ivec2(CurrentViewport->GetWidth(), CurrentViewport->GetHeight());

	FETransformComponent& ObjTransform = GetTransformComponentOfSelectedObject(Scene);
	const glm::vec3 EntitySpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	const glm::vec3 LastFrameMouseRay = GEOMETRY.CreateMouseRayToWorld(LastMouseX, LastMouseY,
																	   CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
																	   ViewportPosition, ViewportSize);
	const glm::vec3 MouseRay = GEOMETRY.CreateMouseRayToWorld(MouseX, MouseY,
															  CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
															  ViewportPosition, ViewportSize);

	const glm::vec3 CameraPosition = CameraTransformComponent.GetPosition(FE_WORLD_SPACE);

	const float SignedDistanceToOrigin = glm::dot(PlaneNormal, EntitySpaceOriginInWorldSpace);

	const float Nominator = SignedDistanceToOrigin - glm::dot(CameraPosition, PlaneNormal);
	const float LastDenominator = glm::dot(LastFrameMouseRay, PlaneNormal);
	const float Denominator = glm::dot(MouseRay, PlaneNormal);

	if (Denominator == 0 || LastDenominator == 0)
		return glm::vec3(0.0f);

	const float LastIntersectionT = Nominator / LastDenominator;
	const float IntersectionT = Nominator / Denominator;

	const glm::vec3 LastPointOnPlane = CameraPosition + LastIntersectionT * LastFrameMouseRay;
	const glm::vec3 PointOnPlane = CameraPosition + IntersectionT * MouseRay;

	LastMousePointOnPlane = LastPointOnPlane;
	return PointOnPlane;
}

void GizmoManager::MouseMoveTransformationGizmos(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	if (CAMERA_SYSTEM.GetMainCameraEntity(Scene) == nullptr)
		return;

	FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FECameraComponent>();
	FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(Scene);
	glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
	glm::ivec2 ViewportSize = glm::ivec2(CurrentViewport->GetWidth(), CurrentViewport->GetHeight());

	FETransformComponent& ObjTransform = GetTransformComponentOfSelectedObject(Scene);

	const glm::vec3 LastFrameMouseRay = GEOMETRY.CreateMouseRayToWorld(LastMouseX, LastMouseY,
																	   CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
																	   ViewportPosition, ViewportSize);
	const glm::vec3 MouseRay = GEOMETRY.CreateMouseRayToWorld(MouseX, MouseY,
															  CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
															  ViewportPosition, ViewportSize);

	float MouseRayParametricIntersection = 0.0f;
	float GizmoRayParametricIntersection = 0.0f;

	float LastFrameMouseRayParametricIntersection = 0.0f;
	float LastFrameGizmoRayParametricIntersection = 0.0f;

	bool bAppliedSomeChanges = false;

	if (GizmoSceneData->bTransformationXGizmoActive)
	{
		GEOMETRY.RaysIntersection(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), LastFrameMouseRay,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		GEOMETRY.RaysIntersection(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		if (abs(Difference) > FLT_EPSILON)
		{
			bAppliedSomeChanges = true;
			ObjTransform.SetPosition(ObjTransform.GetPosition(FE_WORLD_SPACE) + glm::vec3(Difference, 0.0f, 0.0f), FE_WORLD_SPACE);
		}
	}

	if (GizmoSceneData->bTransformationYGizmoActive)
	{
		GEOMETRY.RaysIntersection(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), LastFrameMouseRay,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		GEOMETRY.RaysIntersection(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		if (abs(Difference) > FLT_EPSILON)
		{
			bAppliedSomeChanges = true;
			ObjTransform.SetPosition(ObjTransform.GetPosition(FE_WORLD_SPACE) + glm::vec3(0.0f, Difference, 0.0f), FE_WORLD_SPACE);
		}
	}

	if (GizmoSceneData->bTransformationZGizmoActive)
	{
		GEOMETRY.RaysIntersection(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), LastFrameMouseRay,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		GEOMETRY.RaysIntersection(CameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		if (abs(Difference) > FLT_EPSILON)
		{
			bAppliedSomeChanges = true;
			ObjTransform.SetPosition(ObjTransform.GetPosition(FE_WORLD_SPACE) + glm::vec3(0.0f, 0.0f, Difference), FE_WORLD_SPACE);
		}
	}

	if (GizmoSceneData->bTransformationXYGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 0.0f, 1.0f), Scene);
		if (!GEOMETRY.IsEpsilonEqual(Difference, glm::vec3(0.0f)))
		{
			bAppliedSomeChanges = true;
			ObjTransform.SetPosition(ObjTransform.GetPosition(FE_WORLD_SPACE) + Difference, FE_WORLD_SPACE);
		}
	}

	if (GizmoSceneData->bTransformationYZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(1.0f, 0.0f, 0.0f), Scene);
		if (!GEOMETRY.IsEpsilonEqual(Difference, glm::vec3(0.0f)))
		{
			bAppliedSomeChanges = true;
			ObjTransform.SetPosition(ObjTransform.GetPosition(FE_WORLD_SPACE) + Difference, FE_WORLD_SPACE);
		}
	}

	if (GizmoSceneData->bTransformationXZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 1.0f, 0.0f), Scene);
		if (!GEOMETRY.IsEpsilonEqual(Difference, glm::vec3(0.0f)))
		{
			bAppliedSomeChanges = true;
			ObjTransform.SetPosition(ObjTransform.GetPosition(FE_WORLD_SPACE) + Difference, FE_WORLD_SPACE);
		}
	}

	if (bAppliedSomeChanges)
		ApplyChangesToSelectedObject(ObjTransform, Scene);
}

void GizmoManager::MouseMoveScaleGizmos(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCameraEntity(Scene)->GetComponent<FECameraComponent>();
	FETransformComponent& ObjTransform = GetTransformComponentOfSelectedObject(Scene);

	if (GizmoSceneData->bScaleXGizmoActive && GizmoSceneData->bScaleYGizmoActive && GizmoSceneData->bScaleZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(-CameraComponent.GetForward(), Scene);
		float Magnitude = Difference.x + Difference.y + Difference.z;
		
		glm::vec3 EntityScale = ObjTransform.GetScale();
		// Calculate the average current scale
		float AverageScale = (EntityScale.x + EntityScale.y + EntityScale.z) / 3.0f;

		// Adjust the magnitude based on the current scale
		// to prevent the object from scaling too fast or too slow
		float ScaleFactor = glm::clamp(1.0f / AverageScale, 0.001f, 1000.0f);
		Magnitude /= ScaleFactor;
		EntityScale += Magnitude;
		ObjTransform.SetScale(EntityScale);
	}
	else if (GizmoSceneData->bScaleXGizmoActive || GizmoSceneData->bScaleYGizmoActive || GizmoSceneData->bScaleZGizmoActive)
	{
		if (GizmoSceneData->bScaleXGizmoActive)
		{
			const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 0.0f, 1.0f), Scene);
			glm::vec3 EntityScale = ObjTransform.GetScale();
			EntityScale.x += Difference.x;
			ObjTransform.SetScale(EntityScale);
		}

		if (GizmoSceneData->bScaleYGizmoActive)
		{
			const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 0.0f, 1.0f), Scene);
			glm::vec3 EntityScale = ObjTransform.GetScale();
			EntityScale.y += Difference.y;
			ObjTransform.SetScale(EntityScale);
		}

		if (GizmoSceneData->bScaleZGizmoActive)
		{
			const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 1.0f, 0.0f), Scene);
			glm::vec3 EntityScale = ObjTransform.GetScale();
			EntityScale.z += Difference.z;
			ObjTransform.SetScale(EntityScale);
		}
	}

	ApplyChangesToSelectedObject(ObjTransform, Scene);
}

void GizmoManager::MouseMoveRotateGizmos(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	if (!GizmoSceneData->bRotateXGizmoActive &&
		!GizmoSceneData->bRotateYGizmoActive &&
		!GizmoSceneData->bRotateZGizmoActive)
		return;

	FETransformComponent& ObjTransform = GetTransformComponentOfSelectedObject(Scene);

	const float DifferenceX = static_cast<float>(MouseX - LastMouseX);
	const float DifferenceY = static_cast<float>(MouseY - LastMouseY);

	const float Difference = (DifferenceX + DifferenceY) / 2.0f;

	glm::vec3 AxisOfRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	if (GizmoSceneData->bRotateXGizmoActive)
		AxisOfRotation = glm::vec3(1.0f, 0.0f, 0.0f);

	if (GizmoSceneData->bRotateYGizmoActive)
		AxisOfRotation = glm::vec3(0.0f, 1.0f, 0.0f);

	if (GizmoSceneData->bRotateZGizmoActive)
		AxisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);

	ObjTransform.RotateAroundAxis(AxisOfRotation, Difference, FE_WORLD_SPACE);
	ApplyChangesToSelectedObject(ObjTransform, Scene);
}

void GizmoManager::MouseMove(const double LastMouseX, const double LastMouseY, const double MouseX, const double MouseY, FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	this->LastMouseX = LastMouseX;
	this->LastMouseY = LastMouseY;
	this->MouseX = MouseX;
	this->MouseY = MouseY;

	if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS)
	{
		MouseMoveTransformationGizmos(Scene);
	}
	else if (GizmoSceneData->GizmosState == SCALE_GIZMOS)
	{
		MouseMoveScaleGizmos(Scene);
	}
	else if (GizmoSceneData->GizmosState == ROTATE_GIZMOS)
	{
		MouseMoveRotateGizmos(Scene);
	}
}

void GizmoManager::OnSelectedObjectUpdate(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(Scene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	if (SELECTED.GetSelected(Scene) == nullptr)
	{
		GIZMO_MANAGER.HideAllGizmo(Scene);
	}
	else
	{
		if (SELECTED.GetSelected(Scene) != nullptr && SELECTED.GetSelected(Scene)->HasComponent<FETerrainComponent>())
		{
			if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
			{
				GIZMO_MANAGER.HideAllGizmo(Scene);
				return;
			}
		}

		if (GizmoSceneData->GizmosState == TRANSFORM_GIZMOS)
		{
			GizmoSceneData->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		}
		else if (GizmoSceneData->GizmosState == SCALE_GIZMOS)
		{
			GizmoSceneData->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		}
		else if (GizmoSceneData->GizmosState == ROTATE_GIZMOS)
		{
			GizmoSceneData->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GizmoSceneData->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		}
	}
}

FETransformComponent& GizmoManager::GetTransformComponentOfSelectedObject(FEScene* Scene)
{
	if (Scene == nullptr)
		return DummyTransformComponent;

	if (SELECTED.GetSelected(Scene) == nullptr)
		return DummyTransformComponent;

	FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1 && SELECTED.GetSelected(Scene)->HasComponent<FEInstancedComponent>())
	{
		FEInstancedComponent& InstancedComponent = SELECTED.GetSelected(Scene)->GetComponent<FEInstancedComponent>();

		glm::dvec3 Position, Scale;
		glm::dquat Rotation;
		GEOMETRY.DecomposeMatrixToTranslationRotationScale(InstancedComponent.GetTransformedInstancedMatrix(CurrentSelectionData->InstancedSubObjectIndexSelected), Position, Rotation, Scale);

		DummyTransformComponent.SetPosition(Position);
		DummyTransformComponent.SetQuaternion(Rotation);
		DummyTransformComponent.SetScale(Scale);

		DummyTransformComponent.SetSceneIndependent(true);
		return DummyTransformComponent;
	}
	else
	{
		return SELECTED.GetSelected(Scene)->GetComponent<FETransformComponent>();
	}
}

void GizmoManager::ApplyChangesToSelectedObject(FETransformComponent& Changes, FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	if (SELECTED.GetSelected(Scene) == nullptr)
		return;

	FESelectionData* CurrentSelectionData = SELECTED.GetSceneData(Scene->GetObjectID());
	if (CurrentSelectionData->InstancedSubObjectIndexSelected != -1)
	{
		INSTANCED_RENDERING_SYSTEM.ModifyIndividualInstance(SELECTED.GetSelected(Scene), CurrentSelectionData->InstancedSubObjectIndexSelected, Changes.GetWorldMatrix());
	}
}

void GizmoManager::ClearAllSceneData()
{
	auto PerSceneIterator = PerSceneData.begin();
	while (PerSceneIterator != PerSceneData.end())
	{
		delete PerSceneIterator->second;
		PerSceneIterator++;
	}

	PerSceneData.clear();
}

void GizmoManager::ClearSceneData(const std::string& SceneID)
{
	auto FoundSceneData = PerSceneData.find(SceneID);
	if (FoundSceneData != PerSceneData.end())
	{
		delete FoundSceneData->second;
		PerSceneData.erase(FoundSceneData);
	}
}

void GizmoManager::AddSceneData(const std::string& SceneID)
{
	FEScene* CurrentScene = SCENE_MANAGER.GetScene(SceneID);
	if (CurrentScene == nullptr)
		return;

	PerSceneData[SceneID] = new FEGizmoSceneData();
	PerSceneData[SceneID]->SceneID = SceneID;

	PerSceneData[SceneID]->ParentGizmoEntity = CurrentScene->CreateEntity("ParentGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->ParentGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->ParentGizmoGraphNode = CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->ParentGizmoEntity->GetObjectID());

	// TransformationXGizmo
	PerSceneData[SceneID]->TransformationXGizmoEntity = CurrentScene->CreateEntity("TransformationXGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->TransformationXGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->TransformationXGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationXGizmoGM")[0]);
	PerSceneData[SceneID]->TransformationXGizmoEntity->SetName("TransformationXGizmoEntity");
	PerSceneData[SceneID]->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	PerSceneData[SceneID]->TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->TransformationXGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// TransformationYGizmo
	PerSceneData[SceneID]->TransformationYGizmoEntity = CurrentScene->CreateEntity("TransformationYGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->TransformationYGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->TransformationYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationYGizmoGM")[0]);
	PerSceneData[SceneID]->TransformationYGizmoEntity->SetName("TransformationYGizmoEntity");
	PerSceneData[SceneID]->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	PerSceneData[SceneID]->TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->TransformationYGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// TransformationZGizmo
	PerSceneData[SceneID]->TransformationZGizmoEntity = CurrentScene->CreateEntity("TransformationZGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->TransformationZGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->TransformationZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationZGizmoGM")[0]);
	PerSceneData[SceneID]->TransformationZGizmoEntity->SetName("TransformationZGizmoEntity");
	PerSceneData[SceneID]->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	PerSceneData[SceneID]->TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->TransformationZGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// Plane gizmos
	PerSceneData[SceneID]->TransformationXYGizmoEntity = CurrentScene->CreateEntity("TransformationXYGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->TransformationXYGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->TransformationXYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationXYGizmoGM")[0]);
	PerSceneData[SceneID]->TransformationXYGizmoEntity->SetName("TransformationXYGizmoEntity");
	PerSceneData[SceneID]->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	PerSceneData[SceneID]->TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	PerSceneData[SceneID]->TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));

	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->TransformationXYGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	PerSceneData[SceneID]->TransformationYZGizmoEntity = CurrentScene->CreateEntity("TransformationYZGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->TransformationYZGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->TransformationYZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationYZGizmoGM")[0]);
	PerSceneData[SceneID]->TransformationYZGizmoEntity->SetName("TransformationYZGizmoEntity");
	PerSceneData[SceneID]->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	PerSceneData[SceneID]->TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	PerSceneData[SceneID]->TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->TransformationYZGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	PerSceneData[SceneID]->TransformationXZGizmoEntity = CurrentScene->CreateEntity("TransformationXZGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->TransformationXZGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->TransformationXZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationXZGizmoGM")[0]);
	PerSceneData[SceneID]->TransformationXZGizmoEntity->SetName("TransformationXZGizmoEntity");
	PerSceneData[SceneID]->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	PerSceneData[SceneID]->TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	PerSceneData[SceneID]->TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->TransformationXZGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// ScaleXGizmo
	PerSceneData[SceneID]->ScaleXGizmoEntity = CurrentScene->CreateEntity("ScaleXGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->ScaleXGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->ScaleXGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("ScaleXGizmoGM")[0]);
	PerSceneData[SceneID]->ScaleXGizmoEntity->SetName("ScaleXGizmoEntity");
	PerSceneData[SceneID]->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	PerSceneData[SceneID]->ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->ScaleXGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// ScaleYGizmo
	PerSceneData[SceneID]->ScaleYGizmoEntity = CurrentScene->CreateEntity("ScaleYGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->ScaleYGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->ScaleYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("ScaleYGizmoGM")[0]);
	PerSceneData[SceneID]->ScaleYGizmoEntity->SetName("ScaleYGizmoEntity");
	PerSceneData[SceneID]->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	PerSceneData[SceneID]->ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->ScaleYGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// ScaleZGizmo
	PerSceneData[SceneID]->ScaleZGizmoEntity = CurrentScene->CreateEntity("ScaleZGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->ScaleZGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->ScaleZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("ScaleZGizmoGM")[0]);
	PerSceneData[SceneID]->ScaleZGizmoEntity->SetName("ScaleZGizmoEntity");
	PerSceneData[SceneID]->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	PerSceneData[SceneID]->ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->ScaleZGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// RotateXGizmo
	PerSceneData[SceneID]->RotateXGizmoEntity = CurrentScene->CreateEntity("RotateXGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->RotateXGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->RotateXGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("RotateXGizmoGM")[0]);
	PerSceneData[SceneID]->RotateXGizmoEntity->SetName("RotateXGizmoEntity");
	PerSceneData[SceneID]->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->RotateXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	PerSceneData[SceneID]->RotateXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateXStandardRotation);
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->RotateXGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// RotateYGizmo
	PerSceneData[SceneID]->RotateYGizmoEntity = CurrentScene->CreateEntity("RotateYGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->RotateYGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->RotateYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("RotateYGizmoGM")[0]);
	PerSceneData[SceneID]->RotateYGizmoEntity->SetName("RotateYGizmoEntity");
	PerSceneData[SceneID]->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->RotateYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	PerSceneData[SceneID]->RotateYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateYStandardRotation);
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->RotateYGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);

	// RotateZGizmo
	PerSceneData[SceneID]->RotateZGizmoEntity = CurrentScene->CreateEntity("RotateZGizmoEntity");
	RESOURCE_MANAGER.SetTag(PerSceneData[SceneID]->RotateZGizmoEntity, EDITOR_RESOURCE_TAG);
	PerSceneData[SceneID]->RotateZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("RotateZGizmoGM")[0]);
	PerSceneData[SceneID]->RotateZGizmoEntity->SetName("RotateZGizmoEntity");
	PerSceneData[SceneID]->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	PerSceneData[SceneID]->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	PerSceneData[SceneID]->RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	PerSceneData[SceneID]->RotateZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	PerSceneData[SceneID]->RotateZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateZStandardRotation);
	CurrentScene->SceneGraph.MoveNode(CurrentScene->SceneGraph.GetNodeByEntityID(PerSceneData[SceneID]->RotateZGizmoEntity->GetObjectID())->GetObjectID(), PerSceneData[SceneID]->ParentGizmoGraphNode->GetObjectID(), false);
}

FEGizmoSceneData* GizmoManager::GetSceneData(const std::string& SceneID)
{
	auto FoundScene = PerSceneData.find(SceneID);
	if (FoundScene != PerSceneData.end())
	{
		return FoundScene->second;
	}

	return nullptr;
}