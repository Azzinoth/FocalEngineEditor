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

	ParentGizmoEntity = SCENE.AddEntity("ParentGizmoEntity");
	FENaiveSceneGraphNode* ParentGizmoGraphNode = SCENE.SceneGraph.GetNodeByEntityID(ParentGizmoEntity->GetObjectID());

	// TransformationXGizmo
	FEMaterial* CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);
	TransformationXGizmoEntity = SCENE.AddEntity("TransformationXGizmoEntity");
	//TransformationXGizmoEntity->AddComponent<FEGameModelComponent>(SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationXGizmoGM"), "TransformationXGizmoEntity"));
	TransformationXGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationXGizmoGM"));
	TransformationXGizmoEntity->SetName("TransformationXGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// TransformationYGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("transformationYGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.1f, 0.9f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	TransformationYGizmoEntity = SCENE.AddEntity("TransformationYGizmoEntity");
	//TransformationYGizmoEntity->AddComponent<FERenderableComponent>(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationYGizmoGM"), "TransformationYGizmoEntity"));
	TransformationYGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationYGizmoGM"));
	TransformationYGizmoEntity->SetName("TransformationYGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	TransformationZGizmoEntity = SCENE.AddEntity("TransformationZGizmoEntity");
	//TransformationZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationZGizmoGM"), "TransformationZGizmoEntity"));
	TransformationZGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(TransformationGizmoMesh, CurrentMaterial, "TransformationZGizmoGM"));
	TransformationYGizmoEntity->SetName("TransformationZGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	TransformationXYGizmoEntity = SCENE.AddEntity("TransformationXYGizmoEntity");
	//TransformationXYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXYGizmoGM"), "TransformationXYGizmoEntity"));
	TransformationXYGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXYGizmoGM"));
	TransformationXYGizmoEntity->SetName("TransformationXYGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	TransformationYZGizmoEntity = SCENE.AddEntity("TransformationYZGizmoEntity");
	//TransformationYZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationYZGizmoGM"), "TransformationYZGizmoEntity"));
	TransformationYZGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationYZGizmoGM"));
	TransformationYZGizmoEntity->SetName("TransformationYZGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	TransformationXZGizmoEntity = SCENE.AddEntity("TransformationXZGizmoEntity");
	//TransformationXZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXZGizmoGM"), "TransformationXZGizmoEntity"));
	TransformationXZGizmoEntity->SetName("TransformationXZGizmoEntity");
	TransformationXZGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(RESOURCE_MANAGER.GetMesh("84251E6E0D0801363579317R"/*"cube"*/), CurrentMaterial, "TransformationXZGizmoGM"));
	RESOURCE_MANAGER.MakeGameModelStandard(TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	ScaleXGizmoEntity = SCENE.AddEntity("ScaleXGizmoEntity");
	//ScaleXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleXGizmoGM"), "ScaleXGizmoEntity"));
	ScaleXGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleXGizmoGM"));
	ScaleXGizmoEntity->SetName("ScaleXGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	ScaleYGizmoEntity = SCENE.AddEntity("ScaleYGizmoEntity");
	//ScaleYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleYGizmoGM"), "ScaleYGizmoEntity"));
	ScaleYGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleYGizmoGM"));
	ScaleYGizmoEntity->SetName("ScaleYGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	ScaleZGizmoEntity = SCENE.AddEntity("ScaleZGizmoEntity");
	//ScaleZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleZGizmoGM"), "ScaleZGizmoEntity"));
	ScaleZGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "ScaleZGizmoGM"));
	ScaleZGizmoEntity->SetName("ScaleZGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	/*ScaleZGizmoEntity = SCENE.AddEntity(new FEGameModel(ScaleGizmoMesh, CurrentMaterial, "scaleZGizmoGM"), "scaleZGizmoEntity");
	RESOURCE_MANAGER.MakePrefabStandard(ScaleZGizmoEntity->Prefab);
	RESOURCE_MANAGER.MakeGameModelStandard(ScaleZGizmoEntity->Prefab->GetComponent(0)->GameModel);
	ScaleZGizmoEntity->SetCastShadows(false);
	ScaleZGizmoEntity->Transform.SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->Transform.SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	ScaleZGizmoEntity->SetIsPostprocessApplied(false);*/
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateAroundAxis gizmos
	FEMesh* RotateGizmoMesh = RESOURCE_MANAGER.LoadFEMesh((RESOURCE_MANAGER.GetDefaultResourcesFolder() + "19622421516E5B317E1B5360.model").c_str(), "rotateGizmoMesh");
	RESOURCE_MANAGER.MakeMeshStandard(RotateGizmoMesh);

	// RotateXGizmo
	CurrentMaterial = RESOURCE_MANAGER.CreateMaterial("rotateXGizmoMaterial");
	CurrentMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);
	CurrentMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	CurrentMaterial->AddParameter(FEShaderParam(glm::vec3(0.9f, 0.1f, 0.1f), "baseColor"));
	RESOURCE_MANAGER.MakeMaterialStandard(CurrentMaterial);

	RotateXGizmoEntity = SCENE.AddEntity("RotateXGizmoEntity");
	//RotateXGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateXGizmoGM"), "RotateXGizmoEntity"));
	RotateXGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateXGizmoGM"));
	RotateXGizmoEntity->SetName("RotateXGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(RotateXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateXStandardRotation);
	RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	RotateYGizmoEntity = SCENE.AddEntity("RotateYGizmoEntity");
	//RotateYGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateYGizmoGM"), "RotateYGizmoEntity"));
	RotateYGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateYGizmoGM"));
	RotateYGizmoEntity->SetName("RotateYGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(RotateYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateYStandardRotation);
	RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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

	RotateZGizmoEntity = SCENE.AddEntity("RotateZGizmoEntity");
	//RotateZGizmoEntity->AddComponent<FERenderableComponent>(SCENE.AddEntity(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateZGizmoGM"), "RotateZGizmoEntity"));
	RotateZGizmoEntity->AddComponent<FEGameModelComponent>(new FEGameModel(RotateGizmoMesh, CurrentMaterial, "RotateZGizmoGM"));
	RotateZGizmoEntity->SetName("RotateZGizmoEntity");
	RESOURCE_MANAGER.MakeGameModelStandard(RotateZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel);
	RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateZStandardRotation);
	RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
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
	TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	bTransformationXGizmoActive = false;
	bTransformationYGizmoActive = false;
	bTransformationZGizmoActive = false;

	bTransformationXYGizmoActive = false;
	bTransformationYZGizmoActive = false;
	bTransformationXZGizmoActive = false;

	ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	bScaleXGizmoActive = false;
	bScaleYGizmoActive = false;
	bScaleZGizmoActive = false;

	RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
	RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);

	bRotateXGizmoActive = false;
	bRotateYGizmoActive = false;
	bRotateZGizmoActive = false;
}

void GizmoManager::UpdateGizmoState(int NewState)
{
	if (SELECTED.GetSelected() != nullptr && SELECTED.GetSelected()->HasComponent<FETerrainComponent>())
	{
		if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
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
			TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);

			TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);

			break;
		}
		case SCALE_GIZMOS:
		{
			ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			
			break;
		}
		case ROTATE_GIZMOS:
		{
			RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		
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
	// If we cleared the scene, we should not render gizmos.
	if (SCENE.GetEntityByName("TransformationXGizmoEntity").empty() || SCENE.GetEntityByName("TransformationXGizmoEntity")[0] == nullptr)
		return;

	if (SELECTED.GetSelected() == nullptr || SELECTED.GetSelected()->GetType() == FE_CAMERA)
	{
		HideAllGizmo();
		return;
	}

	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();
	const glm::vec3 ObjectSpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
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
		GIZMO_MANAGER.TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bTransformationXZGizmoActive || GIZMO_MANAGER.bTransformationXYGizmoActive || GIZMO_MANAGER.bTransformationXGizmoActive)
			GIZMO_MANAGER.TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Y Gizmos
		GIZMO_MANAGER.TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive || GIZMO_MANAGER.bTransformationXYGizmoActive || GIZMO_MANAGER.bTransformationYGizmoActive)
			GIZMO_MANAGER.TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Z Gizmos
		GIZMO_MANAGER.TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive || GIZMO_MANAGER.bTransformationXZGizmoActive || GIZMO_MANAGER.bTransformationZGizmoActive)
			GIZMO_MANAGER.TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// XY Gizmos 
		GIZMO_MANAGER.TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationXYGizmoActive)
			GIZMO_MANAGER.TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// YZ Gizmos
		GIZMO_MANAGER.TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationYZGizmoActive)
			GIZMO_MANAGER.TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// XZ Gizmos
		GIZMO_MANAGER.TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.0f, 1.0f, 1.0f));
		if (GIZMO_MANAGER.bTransformationXZGizmoActive)
			GIZMO_MANAGER.TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
	}
	else if (GIZMO_MANAGER.GizmosState == SCALE_GIZMOS)
	{
		// X Gizmos
		GIZMO_MANAGER.ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bScaleXGizmoActive)
			GIZMO_MANAGER.ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Y Gizmos
		GIZMO_MANAGER.ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bScaleYGizmoActive)
			GIZMO_MANAGER.ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));

		// Z Gizmos
		GIZMO_MANAGER.ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bScaleZGizmoActive)
			GIZMO_MANAGER.ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
	}
	else if (GIZMO_MANAGER.GizmosState == ROTATE_GIZMOS)
	{
		if (SELECTED.SELECTED.GetSelected() != nullptr)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		}

		// X Gizmos
		GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.9f, 0.1f, 0.1f));
		if (GIZMO_MANAGER.bRotateXGizmoActive)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
		}

		// Y Gizmos
		GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.9f, 0.1f));
		if (GIZMO_MANAGER.bRotateYGizmoActive)
		{
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
		}

		// Z Gizmos
		GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(0.1f, 0.1f, 0.9f));
		if (GIZMO_MANAGER.bRotateZGizmoActive)
		{
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FEGameModelComponent>().GameModel->Material->SetBaseColor(glm::vec3(1.5f, 1.5f, 0.2f));
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
		}
	}
}

bool GizmoManager::WasSelected(int Index)
{
	DeactivateAllGizmo();

	if (SELECTED.SceneEntitiesUnderMouse[Index]->GetType() == FE_CAMERA)
		return true;

	FEEntity* SelectedEntity = SELECTED.SceneEntitiesUnderMouse[Index];
	FETransformComponent& CurrentTransform = SelectedEntity->GetComponent<FETransformComponent>();

	if (GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == TransformationXGizmoEntity->GetObjectID())
	{
		bTransformationXGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == TransformationYGizmoEntity->GetObjectID())
	{
		bTransformationYGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == TransformationZGizmoEntity->GetObjectID())
	{
		bTransformationZGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == TransformationXYGizmoEntity->GetObjectID())
	{
		bTransformationXYGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == TransformationYZGizmoEntity->GetObjectID())
	{
		bTransformationYZGizmoActive = true;
	}
	else if (GizmosState == TRANSFORM_GIZMOS && SelectedEntity->GetObjectID() == TransformationXZGizmoEntity->GetObjectID())
	{
		bTransformationXZGizmoActive = true;
	}
	else if (GizmosState == SCALE_GIZMOS && SelectedEntity->GetObjectID() == ScaleXGizmoEntity->GetObjectID())
	{
		if (SelectedEntity != nullptr && CurrentTransform.IsUniformScalingSet())
		{
			bScaleXGizmoActive = true;
			bScaleYGizmoActive = true;
			bScaleZGizmoActive = true;
		}

		bScaleXGizmoActive = true;
	}
	else if (GizmosState == SCALE_GIZMOS && SelectedEntity->GetObjectID() == ScaleYGizmoEntity->GetObjectID())
	{
		if (SelectedEntity != nullptr && CurrentTransform.IsUniformScalingSet())
		{
			bScaleXGizmoActive = true;
			bScaleYGizmoActive = true;
			bScaleZGizmoActive = true;
		}

		bScaleYGizmoActive = true;
	}
	else if (GizmosState == SCALE_GIZMOS && SelectedEntity->GetObjectID() == ScaleZGizmoEntity->GetObjectID())
	{
		if (SelectedEntity != nullptr && CurrentTransform.IsUniformScalingSet())
		{
			bScaleXGizmoActive = true;
			bScaleYGizmoActive = true;
			bScaleZGizmoActive = true;
		}

		bScaleZGizmoActive = true;
	}
	else if (GizmosState == ROTATE_GIZMOS && SelectedEntity->GetObjectID() == RotateXGizmoEntity->GetObjectID())
	{
		bRotateXGizmoActive = true;
	}
	else if (GizmosState == ROTATE_GIZMOS && SelectedEntity->GetObjectID() == RotateYGizmoEntity->GetObjectID())
	{
		bRotateYGizmoActive = true;
	}
	else if (GizmosState == ROTATE_GIZMOS && SelectedEntity->GetObjectID() == RotateZGizmoEntity->GetObjectID())
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
	const glm::vec3 EntitySpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

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
	const glm::vec3 EntitySpaceOriginInWorldSpace = glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

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
			FETransformComponent& ParentTransform = SceneGraphParentEntity->GetParent()->GetEntity()->GetComponent<FETransformComponent>();
			LocalAlternativeAxis = glm::inverse(ParentTransform.GetWorldMatrix()) * glm::vec4(AxisOfMovement, 0.0f);
		}

		NewPosition += LocalAlternativeAxis;
		ObjectTransform.SetPosition(NewPosition);
	}
}

void GizmoManager::MouseMoveTransformationGizmos()
{
	FETransformComponent ObjTransform = GetTransformComponentOfSelectedObject();

	float MouseRayParametricIntersection = 0.0f;
	float GizmoRayParametricIntersection = 0.0f;

	float LastFrameMouseRayParametricIntersection = 0.0f;
	float LastFrameGizmoRayParametricIntersection = 0.0f;

	bool bAppliedSomeChanges = false;

	if (GIZMO_MANAGER.bTransformationXGizmoActive)
	{
		const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), LastMouseRayVector,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), MouseRayVector,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		if (abs(Difference) > FLT_EPSILON)
		{
			//float OldY = ObjTransform.GetPosition(false)[1];
			bAppliedSomeChanges = true;



			//ObjTransform.SetPosition(ObjTransform.GetPosition(false) + glm::vec3(Difference, 0.0f, 0.0f), false);

			ObjTransform.SetPosition(ObjTransform.GetPosition(false) + glm::vec3(Difference, 0.0f, 0.0f), false);

			//MoveAlongAxis(glm::vec3(Difference, 0.0f, 0.0f), ObjTransform);
		}
	}

	if (GIZMO_MANAGER.bTransformationYGizmoActive)
	{
		const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), LastMouseRayVector,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), MouseRayVector,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		if (abs(Difference) > FLT_EPSILON)
		{
			bAppliedSomeChanges = true;
			MoveSelectedEntityAlongAxis(glm::vec3(0.0f, Difference, 0.0f), ObjTransform);
		}
	}

	if (GIZMO_MANAGER.bTransformationZGizmoActive)
	{
		const glm::vec3 LastMouseRayVector = SELECTED.MouseRay(LastMouseX, LastMouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), LastMouseRayVector,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f),
								  LastFrameMouseRayParametricIntersection, LastFrameGizmoRayParametricIntersection);

		const glm::vec3 MouseRayVector = SELECTED.MouseRay(MouseX, MouseY);
		GEOMETRY.RaysIntersection(ENGINE.GetCamera()->GetPosition(), MouseRayVector,
								  glm::vec3(ObjTransform.GetWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f),
								  MouseRayParametricIntersection, GizmoRayParametricIntersection);

		const float Difference = GizmoRayParametricIntersection - LastFrameGizmoRayParametricIntersection;
		if (abs(Difference) > FLT_EPSILON)
		{
			bAppliedSomeChanges = true;
			MoveSelectedEntityAlongAxis(glm::vec3(0.0f, 0.0f, Difference), ObjTransform);
		}
	}

	if (GIZMO_MANAGER.bTransformationXYGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 0.0f, 1.0f));
		if (!GEOMETRY.IsEpsilonEqual(Difference, glm::vec3(0.0f)))
		{
			bAppliedSomeChanges = true;
			MoveSelectedEntityAlongAxis(Difference, ObjTransform);
		}
	}

	if (GIZMO_MANAGER.bTransformationYZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(1.0f, 0.0f, 0.0f));
		if (!GEOMETRY.IsEpsilonEqual(Difference, glm::vec3(0.0f)))
		{
			bAppliedSomeChanges = true;
			MoveSelectedEntityAlongAxis(Difference, ObjTransform);
		}
	}

	if (GIZMO_MANAGER.bTransformationXZGizmoActive)
	{
		const glm::vec3 Difference = GetMousePositionDifferenceOnPlane(glm::vec3(0.0f, 1.0f, 0.0f));
		if (!GEOMETRY.IsEpsilonEqual(Difference, glm::vec3(0.0f)))
		{
			bAppliedSomeChanges = true;
			MoveSelectedEntityAlongAxis(Difference, ObjTransform);
		}
	}

	if (bAppliedSomeChanges)
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

	ObjTransform.RotateAroundAxis(AxisOfRotation, Difference, false);
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
		if (SELECTED.GetSelected() != nullptr && SELECTED.GetSelected()->HasComponent<FETerrainComponent>())
		{
			if (TERRAIN_SYSTEM.GetBrushMode() != FE_TERRAIN_BRUSH_NONE)
			{
				GIZMO_MANAGER.HideAllGizmo();
				return;
			}
		}

		if (GIZMO_MANAGER.GizmosState == TRANSFORM_GIZMOS)
		{
			GIZMO_MANAGER.TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		}
		else if (GIZMO_MANAGER.GizmosState == SCALE_GIZMOS)
		{
			GIZMO_MANAGER.ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		}
		else if (GIZMO_MANAGER.GizmosState == ROTATE_GIZMOS)
		{
			GIZMO_MANAGER.RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
			GIZMO_MANAGER.RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		}
	}
}

FETransformComponent GizmoManager::GetTransformComponentOfSelectedObject()
{
	if (SELECTED.GetSelected() == nullptr)
		return FETransformComponent();

	if (SELECTED.InstancedSubObjectIndexSelected != -1 && SELECTED.GetSelected()->HasComponent<FEInstancedComponent>())
	{
		FEInstancedComponent& InstancedComponent = SELECTED.GetSelected()->GetComponent<FEInstancedComponent>();
		return InstancedComponent.GetTransformedInstancedMatrix(SELECTED.InstancedSubObjectIndexSelected);
	}
	else
	{
		return SELECTED.GetSelected()->GetComponent<FETransformComponent>();
	}

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
		INSTANCED_RENDERING_SYSTEM.ModifyIndividualInstance(SELECTED.GetSelected(), SELECTED.InstancedSubObjectIndexSelected, Changes.GetWorldMatrix());
	}
	else
	{
		SELECTED.GetSelected()->GetComponent<FETransformComponent>() = Changes;
	}

	/*if (SELECTED.GetSelected()->GetType() == FE_ENTITY)
	{
		SELECTED.GetEntity()->Transform = Changes;
	}
	else if (SELECTED.GetSelected()->GetType() == FE_ENTITY_INSTANCED)
	{
		if (SELECTED.InstancedSubObjectIndexSelected != -1)
		{
			reinterpret_cast<FEEntityInstanced*>(SELECTED.GetEntity())->ModifyInstance(SELECTED.InstancedSubObjectIndexSelected, Changes.GetWorldMatrix());
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
	ParentGizmoEntity = SCENE.AddEntity("ParentGizmoEntity");
	ParentGizmoGraphNode = SCENE.SceneGraph.GetNodeByEntityID(ParentGizmoEntity->GetObjectID());

	// TransformationXGizmo
	TransformationXGizmoEntity = SCENE.AddEntity("TransformationXGizmoEntity");
	TransformationXGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationXGizmoGM")[0]);
	TransformationXGizmoEntity->SetName("TransformationXGizmoEntity");
	TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// TransformationYGizmo
	TransformationYGizmoEntity = SCENE.AddEntity("TransformationYGizmoEntity");
	TransformationYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationYGizmoGM")[0]);
	TransformationYGizmoEntity->SetName("TransformationYGizmoEntity");
	TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// TransformationZGizmo
	TransformationZGizmoEntity = SCENE.AddEntity("TransformationZGizmoEntity");
	TransformationZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationZGizmoGM")[0]);
	TransformationZGizmoEntity->SetName("TransformationZGizmoEntity");
	TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	TransformationZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// Plane gizmos
	TransformationXYGizmoEntity = SCENE.AddEntity("TransformationXYGizmoEntity");
	TransformationXYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationXYGizmoGM")[0]);
	TransformationXYGizmoEntity->SetName("TransformationXYGizmoEntity");
	TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationXYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.005f, 0.0f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale, GizmosScale * 0.02f));
	TransformationXYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	TransformationYZGizmoEntity = SCENE.AddEntity("TransformationYZGizmoEntity");
	TransformationYZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationYZGizmoGM")[0]);
	TransformationYZGizmoEntity->SetName("TransformationYZGizmoEntity");
	TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationYZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.0f, 0.005f, 0.005f));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 0.02f, GizmosScale, GizmosScale));
	TransformationYZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationYZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	TransformationXZGizmoEntity = SCENE.AddEntity("TransformationXZGizmoEntity");
	TransformationXZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("TransformationXZGizmoGM")[0]);
	TransformationXZGizmoEntity->SetName("TransformationXZGizmoEntity");
	TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	TransformationXZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetPosition(glm::vec3(0.005f, 0.0f, 0.005f));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale, GizmosScale * 0.02f, GizmosScale));
	TransformationXZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(TransformationXZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleXGizmo
	ScaleXGizmoEntity = SCENE.AddEntity("ScaleXGizmoEntity");
	ScaleXGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("ScaleXGizmoGM")[0]);
	ScaleXGizmoEntity->SetName("ScaleXGizmoEntity");
	ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	ScaleXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleYGizmo
	ScaleYGizmoEntity = SCENE.AddEntity("ScaleYGizmoEntity");
	ScaleYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("ScaleYGizmoGM")[0]);
	ScaleYGizmoEntity->SetName("ScaleYGizmoEntity");
	ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	ScaleYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(0.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// ScaleZGizmo
	ScaleZGizmoEntity = SCENE.AddEntity("ScaleZGizmoEntity");
	ScaleZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("ScaleZGizmoGM")[0]);
	ScaleZGizmoEntity->SetName("ScaleZGizmoEntity");
	ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	ScaleZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale));
	ScaleZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(ScaleZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateXGizmo
	RotateXGizmoEntity = SCENE.AddEntity("RotateXGizmoEntity");
	RotateXGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("RotateXGizmoGM")[0]);
	RotateXGizmoEntity->SetName("RotateXGizmoEntity");
	RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	RotateXGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateXGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateXStandardRotation);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateXGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateYGizmo
	RotateYGizmoEntity = SCENE.AddEntity("RotateYGizmoEntity");
	RotateYGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("RotateYGizmoGM")[0]);
	RotateYGizmoEntity->SetName("RotateYGizmoEntity");
	RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	RotateYGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateYGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateYStandardRotation);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateYGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());

	// RotateZGizmo
	RotateZGizmoEntity = SCENE.AddEntity("RotateZGizmoEntity");
	RotateZGizmoEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModelByName("RotateZGizmoGM")[0]);
	RotateZGizmoEntity->SetName("RotateZGizmoEntity");
	RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetCastShadows(false);
	RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetReceivingShadows(false);
	RotateZGizmoEntity->GetComponent<FEGameModelComponent>().SetIsPostprocessApplied(false);
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(GizmosScale * 2.0f));
	RotateZGizmoEntity->GetComponent<FETransformComponent>().SetRotation(RotateZStandardRotation);
	// Temporary overly complicated solution, because of new and old entity system.
	SCENE.SceneGraph.MoveNode(SCENE.SceneGraph.GetNodeByEntityID(RotateZGizmoEntity->GetObjectID())->GetObjectID(), ParentGizmoGraphNode->GetObjectID());
}