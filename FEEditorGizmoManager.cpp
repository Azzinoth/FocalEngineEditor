#include "FEEditorGizmoManager.h"
using namespace FocalEngine;
//using namespace GizmoManager;

GizmoManager* GizmoManager::Instance = nullptr;
GizmoManager::GizmoManager() {}
GizmoManager::~GizmoManager() {}

void GizmoManager::InitializeResources()
{
	SELECTED.SetOnUpdateFunc(OnSelectedObjectUpdate);
	FEMesh* TransformationGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "45191B6F172E3B531978692E.model").c_str(), "transformationGizmoMesh");
	RESOURCE_MANAGER.MakeMeshStandard(TransformationGizmoMesh);

	ParentGizmoEntity = SCENE.AddEmptyEntity("ParentGizmoEntity");
	FENaiveSceneEntity* ParentGizmoGraphEntity = SCENE.SceneGraph.GetEntityByOldEntityID(ParentGizmoEntity->GetObjectID());

	// TransformationXGizmo
	FEMaterial* CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationXGizmoEntity = SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationXGizmoGM"), "transformationXGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationXGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationXGizmoEntity->SetCastShadows(false);
	TransformationXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationXGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// TransformationYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationYGizmoEntity = SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationYGizmoGM"), "transformationYGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationYGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationYGizmoEntity->SetCastShadows(false);
	TransformationYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	TransformationYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// TransformationZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationZGizmoEntity = SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationZGizmoGM"), "transformationZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationZGizmoEntity->SetCastShadows(false);
	TransformationZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	TransformationZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// Plane gizmos
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationXYGizmoEntity = SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXYGizmoGM"), "transformationXYGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationXYGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationXYGizmoEntity->SetCastShadows(false);
	TransformationXYGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	TransformationXYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	TransformationXYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationXYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationYZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationYZGizmoEntity = SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationYZGizmoGM"), "transformationYZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationYZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationYZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationYZGizmoEntity->SetCastShadows(false);
	TransformationYZGizmoEntity->Transform.SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	TransformationYZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	TransformationYZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationYZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationYZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationXZGizmoEntity = SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXZGizmoGM"), "transformationXZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationXZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationXZGizmoEntity->SetCastShadows(false);
	TransformationXZGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	TransformationXZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	TransformationXZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationXZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationXZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// Scale gizmos.
	FEMesh* ScaleGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "637C784B2E5E5C6548190E1B.model").c_str(), "scaleGizmoMesh");
	RESOURCE_MANAGER.MakeMeshStandard(ScaleGizmoMesh);

	// ScaleXGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	ScaleXGizmoEntity = SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "scaleXGizmoGM"), "scaleXGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(ScaleXGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	ScaleXGizmoEntity->SetCastShadows(false);
	ScaleXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	ScaleXGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(ScaleXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// ScaleYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	ScaleYGizmoEntity = SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "scaleYGizmoGM"), "scaleYGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(ScaleYGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	ScaleYGizmoEntity->SetCastShadows(false);
	ScaleYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	ScaleYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(ScaleYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// ScaleZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	ScaleZGizmoEntity = SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "scaleZGizmoGM"), "scaleZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(ScaleZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	ScaleZGizmoEntity->SetCastShadows(false);
	ScaleZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	ScaleZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(ScaleZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// Rotate gizmos
	FEMesh* RotateGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "19622421516E5B317E1B5360.model").c_str(), "rotateGizmoMesh");
	RESOURCE_MANAGER.MakeMeshStandard(RotateGizmoMesh);

	// RotateXGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	RotateXGizmoEntity = SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "rotateXGizmoGM"), "rotateXGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(RotateXGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	RotateXGizmoEntity->SetCastShadows(false);
	RotateXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->Transform.SetRotation(RotateXStandardRotation);
	RotateXGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(RotateXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// RotateYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	RotateYGizmoEntity = SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "rotateYGizmoGM"), "rotateYGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(RotateYGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	RotateYGizmoEntity->SetCastShadows(false);
	RotateYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->Transform.SetRotation(RotateYStandardRotation);
	RotateYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(RotateYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// RotateZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	RotateZGizmoEntity = SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "rotateZGizmoGM"), "rotateZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(RotateZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	RotateZGizmoEntity->SetCastShadows(false);
	RotateZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->Transform.SetRotation(RotateZStandardRotation);
	RotateZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(RotateZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	TransformationGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "456A31026A1C3152181A6064.texture").c_str(), "transformationGizmoIcon");
	RESOURCE_MANAGER.MakeTextureStandard(TransformationGizmoIcon);
	ScaleGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "3F2118296C1E4533506A472E.texture").c_str(), "scaleGizmoIcon");
	RESOURCE_MANAGER.MakeTextureStandard(ScaleGizmoIcon);
	RotateGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "7F6057403249580D73311B54.texture").c_str(), "rotateGizmoIcon");
	RESOURCE_MANAGER.MakeTextureStandard(RotateGizmoIcon);
}

void GizmoManager::HideAllGizmo()
{
	TransformationXGizmoEntity->SetVisibility(false);
	TransformationYGizmoEntity->SetVisibility(false);
	TransformationZGizmoEntity->SetVisibility(false);

	TransformationXYGizmoEntity->SetVisibility(false);
	TransformationYZGizmoEntity->SetVisibility(false);
	TransformationXZGizmoEntity->SetVisibility(false);

	bTransformationXGizmoActive = false;
	bTransformationYGizmoActive = false;
	bTransformationZGizmoActive = false;

	bTransformationXYGizmoActive = false;
	bTransformationYZGizmoActive = false;
	bTransformationXZGizmoActive = false;

	ScaleXGizmoEntity->SetVisibility(false);
	ScaleYGizmoEntity->SetVisibility(false);
	ScaleZGizmoEntity->SetVisibility(false);

	bScaleXGizmoActive = false;
	bScaleYGizmoActive = false;
	bScaleZGizmoActive = false;

	RotateXGizmoEntity->SetVisibility(false);
	RotateYGizmoEntity->SetVisibility(false);
	RotateZGizmoEntity->SetVisibility(false);

	bRotateXGizmoActive = false;
	bRotateYGizmoActive = false;
	bRotateZGizmoActive = false;
}

void GizmoManager::UpdateGizmoState(int NewState)
{
	if (SELECTED.GetTerrain() != nullptr)
	{
		if (SELECTED.GetTerrain()->GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
			return;
	}

	if (NewState < 0 || NewState > 2)
		NewState = 0;

	GizmosState = NewState;
	HideAllGizmo();

	if (SELECTED.GetSelected() == nullptr || SELECTED.GetSelected()->GetType() == FE_CAMERA)
		return;

	switch (NewState)
	{
		case TRANSFORM_GIZMOS:
		{
			TransformationXGizmoEntity->SetVisibility(true);
			TransformationYGizmoEntity->SetVisibility(true);
			TransformationZGizmoEntity->SetVisibility(true);

			TransformationXYGizmoEntity->SetVisibility(true);
			TransformationYZGizmoEntity->SetVisibility(true);
			TransformationXZGizmoEntity->SetVisibility(true);

			break;
		}
		case SCALE_GIZMOS:
		{
			ScaleXGizmoEntity->SetVisibility(true);
			ScaleYGizmoEntity->SetVisibility(true);
			ScaleZGizmoEntity->SetVisibility(true);
			
			break;
		}
		case ROTATE_GIZMOS:
		{
			RotateXGizmoEntity->SetVisibility(true);
			RotateYGizmoEntity->SetVisibility(true);
			RotateZGizmoEntity->SetVisibility(true);
		
			break;
		}
		default:
			break;
	}
}

void GizmoManager::DeactivateAllGizmo()
{
	bTransformationXGizmoActive = false;
	bTransformationYGizmoActive = false;
	bTransformationZGizmoActive = false;

	bTransformationXYGizmoActive = false;
	bTransformationYZGizmoActive = false;
	bTransformationXZGizmoActive = false;

	bScaleXGizmoActive = false;
	bScaleYGizmoActive = false;
	bScaleZGizmoActive = false;

	bRotateXGizmoActive = false;
	bRotateYGizmoActive = false;
	bRotateZGizmoActive = false;
}

void GizmoManager::Render()
{
	if (SELECTED.GetSelected() == nullptr || SELECTED.GetSelected()->GetType() == FE_CAMERA)
	{
		HideAllGizmo();
		return;
	}

	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();
	const glm::vec3 ObjectSpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glm::vec3 ToObject = ObjectSpaceOriginInWorldSpace - ENGINE.GetCamera()->GetPosition();
	ToObject = glm::normalize(ToObject);

	// This will center gizmos in AABB center, but it will produce bug while moving object under some circumstances.
	/*if (SELECTED.getEntity() != nullptr && SELECTED.getEntity()->getType() == FE_ENTITY_INSTANCED && SELECTED.instancedSubObjectIndexSelected == -1)
	{
		FEAABB AABB = SELECTED.getEntity()->getAABB();
		glm::vec3 center = AABB.getMin() + ((AABB.getMax() - AABB.getMin()) / 2.0f);
		toObject = glm::normalize(center - ENGINE.getCamera()->getPosition());
	}*/

	ParentGizmoEntity->Transform.SetPosition((ENGINE.GetCamera()->GetPosition() + ToObject * 0.15f));
	if (GIZMO_MANAGER.GizmosState == TRANSFORM_GIZMOS)
	{
		// X Gizmos
		GIZMO_MANAGER.TransformationXGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bTransformationXZGizmoActive || GIZMO_MANAGER.bTransformationXYGizmoActive || GIZMO_MANAGER.bTransformationXGizmoActive)
			GIZMO_MANAGER.TransformationXGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Y Gizmos
		GIZMO_MANAGER.TransformationYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive || GIZMO_MANAGER.bTransformationXYGizmoActive || GIZMO_MANAGER.bTransformationYGizmoActive)
			GIZMO_MANAGER.TransformationYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Z Gizmos
		GIZMO_MANAGER.TransformationZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive || GIZMO_MANAGER.bTransformationXZGizmoActive || GIZMO_MANAGER.bTransformationZGizmoActive)
			GIZMO_MANAGER.TransformationZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// XY Gizmos
		GIZMO_MANAGER.TransformationXYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationXYGizmoActive)
			GIZMO_MANAGER.TransformationXYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// YZ Gizmos
		GIZMO_MANAGER.TransformationYZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive)
			GIZMO_MANAGER.TransformationYZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// XZ Gizmos
		GIZMO_MANAGER.TransformationXZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationXZGizmoActive)
			GIZMO_MANAGER.TransformationXZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
	}
	else if (GIZMO_MANAGER.GizmosState == SCALE_GIZMOS)
	{
		// X Gizmos
		GIZMO_MANAGER.ScaleXGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bScaleXGizmoActive)
			GIZMO_MANAGER.ScaleXGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Y Gizmos
		GIZMO_MANAGER.ScaleYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bScaleYGizmoActive)
			GIZMO_MANAGER.ScaleYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Z Gizmos
		GIZMO_MANAGER.ScaleZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bScaleZGizmoActive)
			GIZMO_MANAGER.ScaleZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
	}
	else if (GIZMO_MANAGER.GizmosState == ROTATE_GIZMOS)
	{
		if (SELECTED.SELECTED.GetSelected() != nullptr)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateYGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateZGizmoEntity->SetVisibility(true);
		}

		// X Gizmos
		GIZMO_MANAGER.RotateXGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bRotateXGizmoActive)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateYGizmoEntity->SetVisibility(false);
			GIZMO_MANAGER.RotateZGizmoEntity->SetVisibility(false);
		}

		// Y Gizmos
		GIZMO_MANAGER.RotateYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bRotateYGizmoActive)
		{
			GIZMO_MANAGER.RotateYGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateXGizmoEntity->SetVisibility(false);
			GIZMO_MANAGER.RotateZGizmoEntity->SetVisibility(false);
		}

		// Z Gizmos
		GIZMO_MANAGER.RotateZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bRotateZGizmoActive)
		{
			GIZMO_MANAGER.RotateZGizmoEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateXGizmoEntity->SetVisibility(false);
			GIZMO_MANAGER.RotateYGizmoEntity->SetVisibility(false);
		}
	}
}

bool GizmoManager::WasSelected(int Index)
{
	DeactivateAllGizmo();

	if (SELECTED.ObjectsUnderMouse[Index]->GetType() == FE_CAMERA)
		return true;

	FEEntity* SelectedEntity = nullptr;
	if (SELECTED.ObjectsUnderMouse[Index]->GetType() == FE_ENTITY)
		SelectedEntity = SCENE.GetEntity(SELECTED.ObjectsUnderMouse[Index]->GetObjectID());

	const int EntityNameHash = SelectedEntity == nullptr ? -1 : SelectedEntity->GetNameHash();
	if (GizmosState == TRANSFORM_GIZMOS && EntityNameHash == TransformationXGizmoEntity->GetNameHash())
	{
		bTransformationXGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && EntityNameHash == TransformationYGizmoEntity->GetNameHash())
	{
		bTransformationYGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && EntityNameHash == TransformationZGizmoEntity->GetNameHash())
	{
		bTransformationZGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && EntityNameHash == TransformationXYGizmoEntity->GetNameHash())
	{
		bTransformationXYGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && EntityNameHash == TransformationYZGizmoEntity->GetNameHash())
	{
		bTransformationYZGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && EntityNameHash == TransformationXZGizmoEntity->GetNameHash())
	{
		bTransformationXZGizmoActive = true;
	}
	else if (GizmosState == SCALE_GIZMOS && EntityNameHash == ScaleXGizmoEntity->GetNameHash())
	{
		if (SelectedEntity != nullptr && SelectedEntity->Transform.IsUniformScalingSet())
		{
			bScaleXGizmoActive = true;
			bScaleYGizmoActive = true;
			bScaleZGizmoActive = true;
		}

		bScaleXGizmoActive = true;
	}
	else if (GizmosState == SCALE_GIZMOS && EntityNameHash == ScaleYGizmoEntity->GetNameHash())
	{
		if (SelectedEntity != nullptr && SelectedEntity->Transform.IsUniformScalingSet())
		{
			bScaleXGizmoActive = true;
			bScaleYGizmoActive = true;
			bScaleZGizmoActive = true;
		}

		bScaleYGizmoActive = true;
	}
	else if (GizmosState == SCALE_GIZMOS && EntityNameHash == ScaleZGizmoEntity->GetNameHash())
	{
		if (SelectedEntity != nullptr && SelectedEntity->Transform.IsUniformScalingSet())
		{
			bScaleXGizmoActive = true;
			bScaleYGizmoActive = true;
			bScaleZGizmoActive = true;
		}

		bScaleZGizmoActive = true;
	}
	else if (GizmosState == ROTATE_GIZMOS && EntityNameHash == RotateXGizmoEntity->GetNameHash())
	{
		bRotateXGizmoActive = true;
	}
	else if (GizmosState == ROTATE_GIZMOS && EntityNameHash == RotateYGizmoEntity->GetNameHash())
	{
		bRotateYGizmoActive = true;
	}
	else if (GizmosState == ROTATE_GIZMOS && EntityNameHash == RotateZGizmoEntity->GetNameHash())
	{
		bRotateZGizmoActive = true;
	}
	else
	{
		return false;
	}

	return true;
}

glm::vec3 GizmoManager::GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal)
{
	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();
	const glm::vec3 EntitySpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);

	const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
	const glm::vec3 CameraPosition = ENGINE.GetCamera()->GetPosition();

	const float SignedDistanceToOrigin = glm::dot(PlaneNormal, EntitySpaceOriginInWorldSpace);

	const float Nominator = SignedDistanceToOrigin - glm::dot(CameraPosition, PlaneNormal);
	const float LastDenominator = glm::dot(LastMouseRayVector, PlaneNormal);
	const float Denominator = glm::dot(MouseRayVector, PlaneNormal);

	if (Denominator == 0 || LastDenominator == 0)
		return glm::vec3(0.0f);

	const float LastIntersectionT = Nominator / LastDenominator;
	const float IntersectionT = Nominator / Denominator;

	const glm::vec3 LastPointOnPlane = CameraPosition + LastIntersectionT * LastMouseRayVector;
	const glm::vec3 PointOnPlane = CameraPosition + IntersectionT * MouseRayVector;

	return PointOnPlane - LastPointOnPlane;
}

glm::vec3 GizmoManager::GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal, glm::vec3& LastMousePointOnPlane)
{
	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();
	const glm::vec3 EntitySpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);

	const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
	const glm::vec3 CameraPosition = ENGINE.GetCamera()->GetPosition();

	const float SignedDistanceToOrigin = glm::dot(PlaneNormal, EntitySpaceOriginInWorldSpace);

	const float Nominator = SignedDistanceToOrigin - glm::dot(CameraPosition, PlaneNormal);
	const float LastDenominator = glm::dot(LastMouseRayVector, PlaneNormal);
	const float Denominator = glm::dot(MouseRayVector, PlaneNormal);

	if (Denominator == 0 || LastDenominator == 0)
		return glm::vec3(0.0f);

	const float LastIntersectionT = Nominator / LastDenominator;
	const float IntersectionT = Nominator / Denominator;

	const glm::vec3 LastPointOnPlane = CameraPosition + LastIntersectionT * LastMouseRayVector;
	const glm::vec3 PointOnPlane = CameraPosition + IntersectionT * MouseRayVector;

	LastMousePointOnPlane = LastPointOnPlane;
	return PointOnPlane;
}

void GizmoManager::MoveSelectedEntityAlongAxis(const glm::vec3& AxisOfMovement, FETransformComponent& ObjectTransform)
{
	glm::vec3 NewPosition = ObjectTransform.GetPosition();
	glm::vec3 LocalAlternativeAxis = AxisOfMovement;

	FENaiveSceneEntity* SceneGraphParentEntity = SCENE.SceneGraph.GetEntityByOldEntityID(SELECTED.GetSelected()->GetObjectID());
	if (SceneGraphParentEntity != nullptr)
	{
		if (SceneGraphParentEntity->GetParent() != SCENE.SceneGraph.GetRoot())
		{
			FEEntity* ParentEntity = reinterpret_cast<FEEntity*>(SceneGraphParentEntity->GetParent()->GetOldStyleEntity());
			LocalAlternativeAxis = glm::inverse(ParentEntity->Transform.WorldSpaceMatrix) * glm::vec4(AxisOfMovement, 0.0f);
		}

		NewPosition += LocalAlternativeAxis;
		ObjectTransform.SetPosition(NewPosition);
	}
}

#include <glm/gtx/matrix_decompose.hpp>
void GizmoManager::RotateSelectedEntity(const glm::vec3& AxisOfRotation, FETransformComponent& ObjectTransform, const float& RotationAmount)
{
	glm::vec3 LocalAlternativeAxis = AxisOfRotation;

	FENaiveSceneEntity* SceneGraphParentEntity = SCENE.SceneGraph.GetEntityByOldEntityID(SELECTED.GetSelected()->GetObjectID());
	if (SceneGraphParentEntity != nullptr)
	{
		if (SceneGraphParentEntity->GetParent() != SCENE.SceneGraph.GetRoot())
		{
			FEEntity* ParentEntity = reinterpret_cast<FEEntity*>(SceneGraphParentEntity->GetParent()->GetOldStyleEntity());
			//LocalAlternativeAxis = glm::inverse(ParentEntity->Transform.GetQuaternion()) * glm::vec3(AxisOfRotation);

			glm::dvec3 DoubleScale;
			glm::dquat DoubleRotation;
			glm::dvec3 DoubleTranslation;
			glm::dvec3 DoubleSkew;
			glm::dvec4 DoublePerspective;
			glm::dmat4 DoubleNewChildLocalMatrix = ParentEntity->Transform.WorldSpaceMatrix;
			bool Success = glm::decompose(DoubleNewChildLocalMatrix, DoubleScale, DoubleRotation, DoubleTranslation, DoubleSkew, DoublePerspective);
			if (Success)
			{
				LocalAlternativeAxis = glm::inverse(glm::quat(DoubleRotation)) * glm::vec3(AxisOfRotation);
			}

			//LocalAlternativeAxis = glm::quat(glm::inverse(ParentEntity->Transform.WorldSpaceMatrix)) * glm::vec4(AxisOfRotation, 1.0f);
		}

		glm::quat RotationQuaternion = glm::quat(cos(RotationAmount * ANGLE_TORADIANS_COF / 2),
												 LocalAlternativeAxis.x * sin(RotationAmount * ANGLE_TORADIANS_COF / 2),
												 LocalAlternativeAxis.y * sin(RotationAmount * ANGLE_TORADIANS_COF / 2),
												 LocalAlternativeAxis.z * sin(RotationAmount * ANGLE_TORADIANS_COF / 2));

		ObjectTransform.RotateByQuaternion(RotationQuaternion);
	}
}

void GizmoManager::MouseMoveTransformationGizmos()
{
	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();

	float MouseRayParametricIntersection = 0.0f;
	float GizmoRayParametricIntersection = 0.0f;

	float LastFrameMouseRayParametricIntersection = 0.0f;
	float LastFrameGizmoRayParametricIntersection = 0.0f;

	if (GIZMO_MANAGER.bTransformationXGizmoActive)
	{
		const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), LastMouseRayVector,
								  glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), MouseRayVector,
								  glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		MoveSelectedEntityAlongAxis(glm::vec3(Difference, 0.0f, 0.0f), ObjTransform);
	}

	if (GIZMO_MANAGER.bTransformationYGizmoActive)
	{
		const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), LastMouseRayVector,
								  glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), MouseRayVector,
								  glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		MoveSelectedEntityAlongAxis(glm::vec3(0.0f, Difference, 0.0f), ObjTransform);
	}

	if (GIZMO_MANAGER.bTransformationZGizmoActive)
	{
		const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), LastMouseRayVector,
								  glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), MouseRayVector,
								  glm::vec3(ObjTransform.GetTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		MoveSelectedEntityAlongAxis(glm::vec3(0.0f, 0.0f, Difference), ObjTransform);
	}

	if (GIZMO_MANAGER.bTransformationXYGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 0.0f, 1.0f));
		MoveSelectedEntityAlongAxis(Difference, ObjTransform);
	}

	if (GIZMO_MANAGER.bTransformationYZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(1.0f, 0.0f, 0.0f));
		MoveSelectedEntityAlongAxis(Difference, ObjTransform);
	}

	if (GIZMO_MANAGER.bTransformationXZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 1.0f, 0.0f));
		MoveSelectedEntityAlongAxis(Difference, ObjTransform);
	}

	ApplyChangesToSelectedObject(ObjTransform);
}

void GizmoManager::MouseMoveScaleGizmos()
{
	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();

	if (GIZMO_MANAGER.bScaleXGizmoActive && GIZMO_MANAGER.bScaleYGizmoActive && GIZMO_MANAGER.bScaleZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(-ENGINE.GetCamera()->GetForward());
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
	else if (GIZMO_MANAGER.bScaleXGizmoActive || GIZMO_MANAGER.bScaleYGizmoActive || GIZMO_MANAGER.bScaleZGizmoActive)
	{
		if (GIZMO_MANAGER.bScaleXGizmoActive)
		{
			const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 EntityScale = ObjTransform.GetScale();
			EntityScale.x += Difference.x;
			ObjTransform.SetScale(EntityScale);
		}

		if (GIZMO_MANAGER.bScaleYGizmoActive)
		{
			const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 EntityScale = ObjTransform.GetScale();
			EntityScale.y += Difference.y;
			ObjTransform.SetScale(EntityScale);
		}

		if (GIZMO_MANAGER.bScaleZGizmoActive)
		{
			const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 EntityScale = ObjTransform.GetScale();
			EntityScale.z += Difference.z;
			ObjTransform.SetScale(EntityScale);
		}
	}

	ApplyChangesToSelectedObject(ObjTransform);
}

void GizmoManager::MouseMoveRotateGizmos()
{
	if (!GIZMO_MANAGER.bRotateXGizmoActive &&
		!GIZMO_MANAGER.bRotateYGizmoActive &&
		!GIZMO_MANAGER.bRotateZGizmoActive)
		return;

	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();

	const float DifferenceX = static_cast<float>(MouseX - LastMouseX);
	const float DifferenceY = static_cast<float>(MouseY - LastMouseY);

	const float Difference = (DifferenceX + DifferenceY) / 2.0f;

	glm::vec3 AxisOfRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	if (GIZMO_MANAGER.bRotateXGizmoActive)
		AxisOfRotation = glm::vec3(1.0f, 0.0f, 0.0f);

	if (GIZMO_MANAGER.bRotateYGizmoActive)
		AxisOfRotation = glm::vec3(0.0f, 1.0f, 0.0f);

	if (GIZMO_MANAGER.bRotateZGizmoActive)
		AxisOfRotation = glm::vec3(0.0f, 0.0f, 1.0f);

	RotateSelectedEntity(AxisOfRotation, ObjTransform, Difference);
	ApplyChangesToSelectedObject(ObjTransform);
}

void GizmoManager::MouseMove(const double LastMouseX, const double LastMouseY, const double MouseX, const double MouseY)
{
	this->LastMouseX = LastMouseX;
	this->LastMouseY = LastMouseY;
	this->MouseX = MouseX;
	this->MouseY = MouseY;

	if (GizmosState == TRANSFORM_GIZMOS)
	{
		MouseMoveTransformationGizmos();
	}
	else if (GizmosState == SCALE_GIZMOS)
	{
		MouseMoveScaleGizmos();
	}
	else if (GizmosState == ROTATE_GIZMOS)
	{
		MouseMoveRotateGizmos();
	}
}

void GizmoManager::OnSelectedObjectUpdate()
{
	if (SELECTED.GetSelected() == nullptr)
	{
		GIZMO_MANAGER.HideAllGizmo();
	}
	else
	{
		if (SELECTED.GetTerrain() != nullptr)
		{
			if (SELECTED.GetTerrain()->GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
			{
				GIZMO_MANAGER.HideAllGizmo();
				return;
			}
		}

		if (GIZMO_MANAGER.GizmosState == TRANSFORM_GIZMOS)
		{
			GIZMO_MANAGER.TransformationXGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationYGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationZGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationXYGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationYZGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationXZGizmoEntity->SetVisibility(true);
		}
		else if (GIZMO_MANAGER.GizmosState == SCALE_GIZMOS)
		{
			GIZMO_MANAGER.ScaleXGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.ScaleYGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.ScaleZGizmoEntity->SetVisibility(true);
		}
		else if (GIZMO_MANAGER.GizmosState == ROTATE_GIZMOS)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateYGizmoEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateZGizmoEntity->SetVisibility(true);
		}
	}
}

FETransformComponent GizmoManager::GetTransformComponentOfSelectedObject()
{
	if (SELECTED.GetSelected() == nullptr)
		FETransformComponent();

	if (SELECTED.GetSelected()->GetType() == FE_ENTITY)
	{
		return SELECTED.GetEntity()->Transform;
	}
	else if (SELECTED.GetSelected()->GetType() == FE_ENTITY_INSTANCED)
	{
		if (SELECTED.InstancedSubObjectIndexSelected != -1)
		{
			return reinterpret_cast<FEEntityInstanced*>(SELECTED.GetEntity())->GetTransformedInstancedMatrix(SELECTED.InstancedSubObjectIndexSelected);
		}
		else
		{
			return SELECTED.GetEntity()->Transform;
		}
	}
	else if (SELECTED.GetSelected()->GetType() == FE_TERRAIN)
	{
		return SELECTED.GetTerrain()->Transform;
	}
	else if (SELECTED.GetSelected()->GetType() == FE_DIRECTIONAL_LIGHT || SELECTED.GetSelected()->GetType() == FE_SPOT_LIGHT || SELECTED.GetSelected()->GetType() == FE_POINT_LIGHT)
	{
		return SELECTED.GetLight()->Transform;
	}

	return FETransformComponent();
}

void GizmoManager::ApplyChangesToSelectedObject(FETransformComponent Changes)
{
	if (SELECTED.GetSelected() == nullptr)
		FETransformComponent();

	if (SELECTED.GetSelected()->GetType() == FE_ENTITY)
	{
		SELECTED.GetEntity()->Transform = Changes;
	}
	else if (SELECTED.GetSelected()->GetType() == FE_ENTITY_INSTANCED)
	{
		if (SELECTED.InstancedSubObjectIndexSelected != -1)
		{
			reinterpret_cast<FEEntityInstanced*>(SELECTED.GetEntity())->ModifyInstance(SELECTED.InstancedSubObjectIndexSelected, Changes.GetTransformMatrix());
		}
		else
		{
			SELECTED.GetEntity()->Transform = Changes;
		}
	}
	else if (SELECTED.GetSelected()->GetType() == FE_TERRAIN)
	{
		SELECTED.GetTerrain()->Transform = Changes;
	}
	else if (SELECTED.GetSelected()->GetType() == FE_DIRECTIONAL_LIGHT || SELECTED.GetSelected()->GetType() == FE_SPOT_LIGHT || SELECTED.GetSelected()->GetType() == FE_POINT_LIGHT)
	{
		SELECTED.GetLight()->Transform = Changes;
	}
}

void GizmoManager::ReInitializeEntities()
{
	ParentGizmoEntity = SCENE.AddEmptyEntity("ParentGizmoEntity");
	ParentGizmoGraphEntity = SCENE.SceneGraph.GetEntityByOldEntityID(ParentGizmoEntity->GetObjectID());

	// TransformationXGizmo
	TransformationXGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXGizmoGM")[0], "transformationXGizmoEntity");
	TransformationXGizmoEntity->SetCastShadows(false);
	TransformationXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationXGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// TransformationYGizmo
	TransformationYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationYGizmoGM")[0], "transformationYGizmoEntity");
	TransformationYGizmoEntity->SetCastShadows(false);
	TransformationYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	TransformationYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// TransformationZGizmo
	TransformationZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationZGizmoGM")[0], "transformationZGizmoEntity");
	TransformationZGizmoEntity->SetCastShadows(false);
	TransformationZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	TransformationZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// Plane gizmos
	TransformationXYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXYGizmoGM")[0], "transformationXYGizmoEntity");
	TransformationXYGizmoEntity->SetCastShadows(false);
	TransformationXYGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	TransformationXYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	TransformationXYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationXYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	TransformationYZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationYZGizmoGM")[0], "transformationYZGizmoEntity");
	TransformationYZGizmoEntity->SetCastShadows(false);
	TransformationYZGizmoEntity->Transform.SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	TransformationYZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	TransformationYZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationYZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationYZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	TransformationXZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXZGizmoGM")[0], "transformationXZGizmoEntity");
	TransformationXZGizmoEntity->SetCastShadows(false);
	TransformationXZGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	TransformationXZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	TransformationXZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationXZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(TransformationXZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// ScaleXGizmo
	ScaleXGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("scaleXGizmoGM")[0], "scaleXGizmoEntity");
	ScaleXGizmoEntity->SetCastShadows(false);
	ScaleXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	ScaleXGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(ScaleXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// ScaleYGizmo
	ScaleYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("scaleYGizmoGM")[0], "scaleYGizmoEntity");
	ScaleYGizmoEntity->SetCastShadows(false);
	ScaleYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	ScaleYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(ScaleYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// ScaleZGizmo
	ScaleZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("scaleZGizmoGM")[0], "scaleZGizmoEntity");
	ScaleZGizmoEntity->SetCastShadows(false);
	ScaleZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	ScaleZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(ScaleZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// RotateXGizmo
	RotateXGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("rotateXGizmoGM")[0], "rotateXGizmoEntity");
	RotateXGizmoEntity->SetCastShadows(false);
	RotateXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->Transform.SetRotation(RotateXStandardRotation);
	RotateXGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(RotateXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// RotateYGizmo
	RotateYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("rotateYGizmoGM")[0], "rotateYGizmoEntity");
	RotateYGizmoEntity->SetCastShadows(false);
	RotateYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->Transform.SetRotation(RotateYStandardRotation);
	RotateYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(RotateYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());

	// RotateZGizmo
	RotateZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("rotateZGizmoGM")[0], "rotateZGizmoEntity");
	RotateZGizmoEntity->SetCastShadows(false);
	RotateZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->Transform.SetRotation(RotateZStandardRotation);
	RotateZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveEntity(SCENE.SceneGraph.GetEntityByOldEntityID(RotateZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphEntity->GetObjectID());
}