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

	ParentGizmoEntity = SCENE.AddNewStyleEntity("ParentGizmoEntity");
	FENaiveSceneGraphNode* ParentGizmoGraphNode = SCENE.SceneGraph.GetNodeByEntityID(ParentGizmoEntity->GetObjectID());

	// TransformationXGizmo
	FEMaterial* CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationXGizmoEntity = SCENE.AddNewStyleEntity("TransformationXGizmoEntity");
	TransformationXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationXGizmoGM"), "TransformationXGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// TransformationYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	TransformationYGizmoEntity = SCENE.AddNewStyleEntity("TransformationYGizmoEntity");
	TransformationYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationYGizmoGM"), "TransformationYGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*TransformationYGizmoEntity = SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationYGizmoGM"), "transformationYGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationYGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationYGizmoEntity->SetCastShadows(false);
	TransformationYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	TransformationYGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// TransformationZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	TransformationZGizmoEntity = SCENE.AddNewStyleEntity("TransformationZGizmoEntity");
	TransformationZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationZGizmoGM"), "TransformationZGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*TransformationZGizmoEntity = SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationZGizmoGM"), "transformationZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(TransformationZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	TransformationZGizmoEntity->SetCastShadows(false);
	TransformationZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	TransformationZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	TransformationZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// Plane gizmos
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	TransformationXYGizmoEntity = SCENE.AddNewStyleEntity("TransformationXYGizmoEntity");
	TransformationXYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXYGizmoGM"), "TransformationXYGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	//TransformationXYGizmoEntity = SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXYGizmoGM"), "transformationXYGizmoEntity");
	//RESOURCE_MANAGER.MakePrefabStandard(TransformationXYGizmoEntity->Prefab);
	//RESOURCE_MANAGER.MakeGameModelStandard(TransformationXYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	//TransformationXYGizmoEntity->SetCastShadows(false);
	//TransformationXYGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	//TransformationXYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	//TransformationXYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	//TransformationXYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationYZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	TransformationYZGizmoEntity = SCENE.AddNewStyleEntity("TransformationYZGizmoEntity");
	TransformationYZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationYZGizmoGM"), "TransformationYZGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	//TransformationYZGizmoEntity = SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationYZGizmoGM"), "transformationYZGizmoEntity");
	//RESOURCE_MANAGER.MakePrefabStandard(TransformationYZGizmoEntity->Prefab);
	//RESOURCE_MANAGER.MakeGameModelStandard(TransformationYZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	//TransformationYZGizmoEntity->SetCastShadows(false);
	//TransformationYZGizmoEntity->Transform.SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	//TransformationYZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	//TransformationYZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	//TransformationYZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationYZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	TransformationXZGizmoEntity = SCENE.AddNewStyleEntity("TransformationXZGizmoEntity");
	TransformationXZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXZGizmoGM"), "TransformationXZGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	//TransformationXZGizmoEntity = SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXZGizmoGM"), "transformationXZGizmoEntity");
	//RESOURCE_MANAGER.MakePrefabStandard(TransformationXZGizmoEntity->Prefab);
	//RESOURCE_MANAGER.MakeGameModelStandard(TransformationXZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	//TransformationXZGizmoEntity->SetCastShadows(false);
	//TransformationXZGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	//TransformationXZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	//TransformationXZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	//TransformationXZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// Scale gizmos.
	FEMesh* ScaleGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "637C784B2E5E5C6548190E1B.model").c_str(), "scaleGizmoMesh");
	RESOURCE_MANAGER.MakeMeshStandard(ScaleGizmoMesh);

	// ScaleXGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	ScaleXGizmoEntity = SCENE.AddNewStyleEntity("ScaleXGizmoEntity");
	ScaleXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleXGizmoGM"), "ScaleXGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*ScaleXGizmoEntity = SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "scaleXGizmoGM"), "scaleXGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(ScaleXGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	ScaleXGizmoEntity->SetCastShadows(false);
	ScaleXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	ScaleXGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	ScaleYGizmoEntity = SCENE.AddNewStyleEntity("ScaleYGizmoEntity");
	ScaleYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleYGizmoGM"), "ScaleYGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*ScaleYGizmoEntity = SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "scaleYGizmoGM"), "scaleYGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(ScaleYGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	ScaleYGizmoEntity->SetCastShadows(false);
	ScaleYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	ScaleYGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("scaleZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	ScaleZGizmoEntity = SCENE.AddNewStyleEntity("ScaleZGizmoEntity");
	ScaleZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleZGizmoGM"), "ScaleZGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*ScaleZGizmoEntity = SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "scaleZGizmoGM"), "scaleZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(ScaleZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	ScaleZGizmoEntity->SetCastShadows(false);
	ScaleZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	ScaleZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// Rotate gizmos
	FEMesh* RotateGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "19622421516E5B317E1B5360.model").c_str(), "rotateGizmoMesh");
	RESOURCE_MANAGER.MakeMeshStandard(RotateGizmoMesh);

	// RotateXGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	RotateXGizmoEntity = SCENE.AddNewStyleEntity("RotateXGizmoEntity");
	RotateXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateXGizmoGM"), "RotateXGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateXStandardRotation);
	RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*RotateXGizmoEntity = SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "rotateXGizmoGM"), "rotateXGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(RotateXGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateXGizmoEntity->Prefab->GetComponent(0)->GameModel);
	RotateXGizmoEntity->SetCastShadows(false);
	RotateXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->Transform.SetRotation(RotateXStandardRotation);
	RotateXGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	RotateYGizmoEntity = SCENE.AddNewStyleEntity("RotateYGizmoEntity");
	RotateYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateYGizmoGM"), "RotateYGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateYStandardRotation);
	RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*RotateYGizmoEntity = SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "rotateYGizmoGM"), "rotateYGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(RotateYGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateYGizmoEntity->Prefab->GetComponent(0)->GameModel);
	RotateYGizmoEntity->SetCastShadows(false);
	RotateYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->Transform.SetRotation(RotateYStandardRotation);
	RotateYGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateZGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateZGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.1f, 0.9f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	RotateZGizmoEntity = SCENE.AddNewStyleEntity("RotateZGizmoEntity");
	RotateZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateZGizmoGM"), "RotateZGizmoEntity"));
	RESOURCE_MANAGER.MakePrefabStandard(RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel);
	RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateZStandardRotation);
	RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	/*RotateZGizmoEntity = SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "rotateZGizmoGM"), "rotateZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(RotateZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(RotateZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	RotateZGizmoEntity->SetCastShadows(false);
	RotateZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->Transform.SetRotation(RotateZStandardRotation);
	RotateZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	TransformationGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "456A31026A1C3152181A6064.texture").c_str(), "transformationGizmoIcon");
	RESOURCE_MANAGER.MakeTextureStandard(TransformationGizmoIcon);
	ScaleGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "3F2118296C1E4533506A472E.texture").c_str(), "scaleGizmoIcon");
	RESOURCE_MANAGER.MakeTextureStandard(ScaleGizmoIcon);
	RotateGizmoIcon = RESOURCE_MANAGER.LoadFETexture((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "7F6057403249580D73311B54.texture").c_str(), "rotateGizmoIcon");
	RESOURCE_MANAGER.MakeTextureStandard(RotateGizmoIcon);
}

void GizmoManager::HideAllGizmo()
{
	TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);

	TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);

	bTransformationXGizmoActive = false;
	bTransformationYGizmoActive = false;
	bTransformationZGizmoActive = false;

	bTransformationXYGizmoActive = false;
	bTransformationYZGizmoActive = false;
	bTransformationXZGizmoActive = false;

	ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);

	bScaleXGizmoActive = false;
	bScaleYGizmoActive = false;
	bScaleZGizmoActive = false;

	RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
	RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);

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
			TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);

			TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);

			break;
		}
		case SCALE_GIZMOS:
		{
			ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			
			break;
		}
		case ROTATE_GIZMOS:
		{
			RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
		
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

	ParentGizmoEntity->GetComponent<FETransformComponent>().SetPosition((ENGINE.GetCamera()->GetPosition() + ToObject * 0.15f));
	if (GIZMO_MANAGER.GizmosState == TRANSFORM_GIZMOS)
	{
		// X Gizmos
		GIZMO_MANAGER.TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bTransformationXZGizmoActive || GIZMO_MANAGER.bTransformationXYGizmoActive || GIZMO_MANAGER.bTransformationXGizmoActive)
			GIZMO_MANAGER.TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Y Gizmos
		GIZMO_MANAGER.TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive || GIZMO_MANAGER.bTransformationXYGizmoActive || GIZMO_MANAGER.bTransformationYGizmoActive)
			GIZMO_MANAGER.TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Z Gizmos
		GIZMO_MANAGER.TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive || GIZMO_MANAGER.bTransformationXZGizmoActive || GIZMO_MANAGER.bTransformationZGizmoActive)
			GIZMO_MANAGER.TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// XY Gizmos 
		GIZMO_MANAGER.TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationXYGizmoActive)
			GIZMO_MANAGER.TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// YZ Gizmos
		GIZMO_MANAGER.TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive)
			GIZMO_MANAGER.TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// XZ Gizmos
		GIZMO_MANAGER.TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationXZGizmoActive)
			GIZMO_MANAGER.TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
	}
	else if (GIZMO_MANAGER.GizmosState == SCALE_GIZMOS)
	{
		// X Gizmos
		GIZMO_MANAGER.ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bScaleXGizmoActive)
			GIZMO_MANAGER.ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Y Gizmos
		GIZMO_MANAGER.ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bScaleYGizmoActive)
			GIZMO_MANAGER.ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Z Gizmos
		GIZMO_MANAGER.ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bScaleZGizmoActive)
			GIZMO_MANAGER.ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
	}
	else if (GIZMO_MANAGER.GizmosState == ROTATE_GIZMOS)
	{
		if (SELECTED.SELECTED.GetSelected() != nullptr)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
		}

		// X Gizmos
		GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bRotateXGizmoActive)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
		}

		// Y Gizmos
		GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bRotateYGizmoActive)
		{
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
		}

		// Z Gizmos
		GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bRotateZGizmoActive)
		{
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->Prefab->GetComponent(0)->GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(false);
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

	FENaiveSceneGraphNode* SceneGraphParentEntity = SCENE.SceneGraph.GetNodeByEntityID(SELECTED.GetSelected()->GetObjectID());
	if (SceneGraphParentEntity != nullptr)
	{
		if (SceneGraphParentEntity->GetParent() != SCENE.SceneGraph.GetRoot())
		{
			FETransformComponent& ParentTransform = SceneGraphParentEntity->GetParent()->GetNewStyleEntity()->GetComponent<FETransformComponent>();
			//FEEntity* ParentEntity = reinterpret_cast<FEEntity*>(SceneGraphParentEntity->GetParent()->GetOldStyleEntity());
			LocalAlternativeAxis = glm::inverse(ParentTransform.WorldSpaceMatrix) * glm::vec4(AxisOfMovement, 0.0f);
		}

		NewPosition += LocalAlternativeAxis;
		ObjectTransform.SetPosition(NewPosition);
	}
}

#include <glm/gtx/matrix_decompose.hpp>
void GizmoManager::RotateSelectedEntity(const glm::vec3& AxisOfRotation, FETransformComponent& ObjectTransform, const float& RotationAmount)
{
	glm::vec3 LocalAlternativeAxis = AxisOfRotation;

	FENaiveSceneGraphNode* SceneGraphParentEntity = SCENE.SceneGraph.GetNodeByEntityID(SELECTED.GetSelected()->GetObjectID());
	if (SceneGraphParentEntity != nullptr)
	{
		if (SceneGraphParentEntity->GetParent() != SCENE.SceneGraph.GetRoot())
		{
			FETransformComponent& ParentTransform = SceneGraphParentEntity->GetParent()->GetNewStyleEntity()->GetComponent<FETransformComponent>();
			//FEEntity* ParentEntity = reinterpret_cast<FEEntity*>(SceneGraphParentEntity->GetParent()->GetOldStyleEntity());
			//LocalAlternativeAxis = glm::inverse(ParentEntity->Transform.GetQuaternion()) * glm::vec3(AxisOfRotation);

			glm::dvec3 DoubleScale;
			glm::dquat DoubleRotation;
			glm::dvec3 DoubleTranslation;
			glm::dvec3 DoubleSkew;
			glm::dvec4 DoublePerspective;
			glm::dmat4 DoubleNewChildLocalMatrix = ParentTransform.WorldSpaceMatrix;
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
			GIZMO_MANAGER.TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
		}
		else if (GIZMO_MANAGER.GizmosState == SCALE_GIZMOS)
		{
			GIZMO_MANAGER.ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
		}
		else if (GIZMO_MANAGER.GizmosState == ROTATE_GIZMOS)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetVisibility(true);
		}
	}
}

FETransformComponent GizmoManager::GetTransformComponentOfSelectedObject()
{
	if (SELECTED.GetSelected() == nullptr)
		return FETransformComponent();

	return SELECTED.GetSelected()->GetComponent<FETransformComponent>();

	/*if (SELECTED.GetSelected()->GetType() == FE_ENTITY)
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

	return FETransformComponent();*/
}

void GizmoManager::ApplyChangesToSelectedObject(FETransformComponent Changes)
{
	if (SELECTED.GetSelected() == nullptr)
		return;

	if (SELECTED.InstancedSubObjectIndexSelected != -1)
	{
		//reinterpret_cast<FEEntityInstanced*>(SELECTED.GetEntity())->ModifyInstance(SELECTED.InstancedSubObjectIndexSelected, Changes.GetTransformMatrix());
		assert(false);
	}

	SELECTED.GetSelected()->GetComponent<FETransformComponent>() = Changes;

	/*if (SELECTED.GetSelected()->GetType() == FE_ENTITY)
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
	}*/
}

void GizmoManager::ReInitializeEntities()
{
	ParentGizmoEntity = SCENE.AddNewStyleEntity("ParentGizmoEntity");
	ParentGizmoGraphNode = SCENE.SceneGraph.GetNodeByEntityID(ParentGizmoEntity->GetObjectID());

	// TransformationXGizmo
	TransformationXGizmoEntity = SCENE.AddNewStyleEntity("TransformationXGizmoEntity");
	TransformationXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXGizmoGM")[0], "TransformationXGizmoEntity"));
	TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// TransformationYGizmo
	TransformationYGizmoEntity = SCENE.AddNewStyleEntity("TransformationYGizmoEntity");
	TransformationYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationYGizmoGM")[0], "TransformationYGizmoEntity"));
	TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	//TransformationYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationYGizmoGM")[0], "transformationYGizmoEntity");
	//TransformationYGizmoEntity->SetCastShadows(false);
	//TransformationYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	//TransformationYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	//TransformationYGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// TransformationZGizmo
	TransformationZGizmoEntity = SCENE.AddNewStyleEntity("TransformationZGizmoEntity");
	TransformationZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationZGizmoGM")[0], "TransformationZGizmoEntity"));
	TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	//TransformationZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationZGizmoGM")[0], "transformationZGizmoEntity");
	//TransformationZGizmoEntity->SetCastShadows(false);
	//TransformationZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	//TransformationZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	//TransformationZGizmoEntity->SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// Plane gizmos
	TransformationXYGizmoEntity = SCENE.AddNewStyleEntity("TransformationXYGizmoEntity");
	TransformationXYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXYGizmoGM")[0], "TransformationXYGizmoEntity"));
	TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationXYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	/*TransformationXYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXYGizmoGM")[0], "transformationXYGizmoEntity");
	TransformationXYGizmoEntity->SetCastShadows(false);
	TransformationXYGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	TransformationXYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	TransformationXYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXYGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	TransformationYZGizmoEntity = SCENE.AddNewStyleEntity("TransformationYZGizmoEntity");
	TransformationYZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationYZGizmoGM")[0], "TransformationYZGizmoEntity"));
	TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationYZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	/*TransformationYZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationYZGizmoGM")[0], "transformationYZGizmoEntity");
	TransformationYZGizmoEntity->SetCastShadows(false);
	TransformationYZGizmoEntity->Transform.SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	TransformationYZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	TransformationYZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationYZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationYZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	TransformationXZGizmoEntity = SCENE.AddNewStyleEntity("TransformationXZGizmoEntity");
	TransformationXZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXZGizmoGM")[0], "TransformationXZGizmoEntity"));
	TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	TransformationXZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	/*TransformationXZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("TransformationXZGizmoGM")[0], "transformationXZGizmoEntity");
	TransformationXZGizmoEntity->SetCastShadows(false);
	TransformationXZGizmoEntity->Transform.SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	TransformationXZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	TransformationXZGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationXZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleXGizmo
	ScaleXGizmoEntity = SCENE.AddNewStyleEntity("ScaleXGizmoEntity");
	ScaleXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("ScaleXGizmoGM")[0], "ScaleXGizmoEntity"));
	ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	ScaleXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	/*ScaleXGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("scaleXGizmoGM")[0], "scaleXGizmoEntity");
	ScaleXGizmoEntity->SetCastShadows(false);
	ScaleXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->Transform.SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	ScaleXGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleYGizmo
	ScaleYGizmoEntity = SCENE.AddNewStyleEntity("ScaleYGizmoEntity");
	ScaleYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("ScaleYGizmoGM")[0], "ScaleYGizmoEntity"));
	ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	ScaleYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	/*ScaleYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("scaleYGizmoGM")[0], "scaleYGizmoEntity");
	ScaleYGizmoEntity->SetCastShadows(false);
	ScaleYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->Transform.SetRotation(glm::vec3(0.0f));
	ScaleYGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleZGizmo
	ScaleZGizmoEntity = SCENE.AddNewStyleEntity("ScaleZGizmoEntity");
	ScaleZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("ScaleZGizmoGM")[0], "ScaleZGizmoEntity"));
	ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	ScaleZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	/*ScaleZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("scaleZGizmoGM")[0], "scaleZGizmoEntity");
	ScaleZGizmoEntity->SetCastShadows(false);
	ScaleZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	ScaleZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateXGizmo
	RotateXGizmoEntity = SCENE.AddNewStyleEntity("RotateXGizmoEntity");
	RotateXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("RotateXGizmoGM")[0], "RotateXGizmoEntity"));
	RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	RotateXGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateXStandardRotation);
	/*RotateXGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("rotateXGizmoGM")[0], "rotateXGizmoEntity");
	RotateXGizmoEntity->SetCastShadows(false);
	RotateXGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->Transform.SetRotation(RotateXStandardRotation);
	RotateXGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateYGizmo
	RotateYGizmoEntity = SCENE.AddNewStyleEntity("RotateYGizmoEntity");
	RotateYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("RotateYGizmoGM")[0], "RotateYGizmoEntity"));
	RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	RotateYGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateYStandardRotation);
	/*RotateYGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("rotateYGizmoGM")[0], "rotateYGizmoEntity");
	RotateYGizmoEntity->SetCastShadows(false);
	RotateYGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->Transform.SetRotation(RotateYStandardRotation);
	RotateYGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateZGizmo
	RotateZGizmoEntity = SCENE.AddNewStyleEntity("RotateZGizmoEntity");
	RotateZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("RotateZGizmoGM")[0], "RotateZGizmoEntity"));
	RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetCastShadows(false);
	RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetReceivingShadows(false);
	RotateZGizmoEntity->GetComponent<FERenderableComponent>().OldStyleEntity->SetIsPostprocessApplied(false);
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateZStandardRotation);
	/*RotateZGizmoEntity = SCENE.AddEntity(RESOURCE_MANAGER.GetPrefabByName("rotateZGizmoGM")[0], "rotateZGizmoEntity");
	RotateZGizmoEntity->SetCastShadows(false);
	RotateZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->Transform.SetRotation(RotateZStandardRotation);
	RotateZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());
}