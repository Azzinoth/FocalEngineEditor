#pragma once

#include "../FEngine.h"
#include "FEEditorSelectedObject.h"
using namespace FocalEngine;

const int TRANSFORM_GIZMOS = 0;
const int SCALE_GIZMOS = 1;
const int ROTATE_GIZMOS = 2;

class FEGizmoSceneData
{
	friend class FEEditor;
	friend class FEEditorSelectedObject;
	friend class GizmoManager;
	friend class FEEditorInspectorWindow;
	friend class GyzmosSettingsWindow;

	std::string SceneID = "";
	
	int GizmosState = TRANSFORM_GIZMOS;

	// Parent Gizmo Entity
	FEEntity* ParentGizmoEntity = nullptr;
	FENaiveSceneGraphNode* ParentGizmoGraphNode = nullptr;

	FEEntity* TransformationXGizmoEntity = nullptr;
	FEEntity* TransformationYGizmoEntity = nullptr;
	FEEntity* TransformationZGizmoEntity = nullptr;

	bool bTransformationXGizmoActive = false;
	bool bTransformationYGizmoActive = false;
	bool bTransformationZGizmoActive = false;

	FEEntity* TransformationXYGizmoEntity = nullptr;
	FEEntity* TransformationYZGizmoEntity = nullptr;
	FEEntity* TransformationXZGizmoEntity = nullptr;

	bool bTransformationXYGizmoActive = false;
	bool bTransformationYZGizmoActive = false;
	bool bTransformationXZGizmoActive = false;

	// Scale part.
	FEEntity* ScaleXGizmoEntity = nullptr;
	FEEntity* ScaleYGizmoEntity = nullptr;
	FEEntity* ScaleZGizmoEntity = nullptr;

	bool bScaleXGizmoActive = false;
	bool bScaleYGizmoActive = false;
	bool bScaleZGizmoActive = false;

	// Rotate part.
	FEEntity* RotateXGizmoEntity = nullptr;
	FEEntity* RotateYGizmoEntity = nullptr;
	FEEntity* RotateZGizmoEntity = nullptr;

	bool bRotateXGizmoActive = false;
	bool bRotateYGizmoActive = false;
	bool bRotateZGizmoActive = false;

	glm::vec3 RotateXStandardRotation = glm::vec3(0.0f, 0.0f, -90.0f);
	glm::vec3 RotateYStandardRotation = glm::vec3(0.0f);
	glm::vec3 RotateZStandardRotation = glm::vec3(90.0f, 0.0f, 90.0f);

	FETexture* TransformationGizmoIcon = nullptr;
	FETexture* ScaleGizmoIcon = nullptr;
	FETexture* RotateGizmoIcon = nullptr;
};

class GizmoManager
{
	friend class FEEditor;
	friend class FEProjectManager;
	friend class GyzmosSettingsWindow;
public:
	SINGLETON_PUBLIC_PART(GizmoManager)

	float GizmosScale = 0.00175f;
	int GizmosState = TRANSFORM_GIZMOS;

	glm::vec3 RotateXStandardRotation = glm::vec3(0.0f, 0.0f, -90.0f);
	glm::vec3 RotateYStandardRotation = glm::vec3(0.0f);
	glm::vec3 RotateZStandardRotation = glm::vec3(90.0f, 0.0f, 90.0f);

	FETexture* TransformationGizmoIcon = nullptr;
	FETexture* ScaleGizmoIcon = nullptr;
	FETexture* RotateGizmoIcon = nullptr;

	void InitializeResources();

	void DeactivateAllGizmo(FEScene* Scene);
	void HideAllGizmo(FEScene* Scene);
	void UpdateGizmoState(int NewState, FEScene* Scene);
	void Update();
	bool WasSelected(int Index, FEScene* Scene);

	void MouseMove(double LastMouseX, double LastMouseY, double MouseX, double MouseY, FEScene* Scene);
private:
	SINGLETON_PRIVATE_PART(GizmoManager)

	double LastMouseX = 0, LastMouseY = 0, MouseX = 0, MouseY = 0;

	FEGameModel* TransformationXGizmoGM = nullptr;
	FEGameModel* TransformationYGizmoGM = nullptr;
	FEGameModel* TransformationZGizmoGM = nullptr;

	FEGameModel* TransformationXYGizmoGM = nullptr;
	FEGameModel* TransformationYZGizmoGM = nullptr;
	FEGameModel* TransformationXZGizmoGM = nullptr;

	FEGameModel* ScaleXGizmoGM = nullptr;
	FEGameModel* ScaleYGizmoGM = nullptr;
	FEGameModel* ScaleZGizmoGM = nullptr;

	FEGameModel* RotateXGizmoGM = nullptr;
	FEGameModel* RotateYGizmoGM = nullptr;
	FEGameModel* RotateZGizmoGM = nullptr;

	glm::vec3 GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal, FEScene* Scene);
	glm::vec3 GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal, glm::vec3& LastMousePointOnPlane, FEScene* Scene);

	void MouseMoveTransformationGizmos(FEScene* Scene);
	void MouseMoveScaleGizmos(FEScene* Scene);
	void MouseMoveRotateGizmos(FEScene* Scene);

	static void OnSelectedObjectUpdate(FEScene* Scene);

	FETransformComponent GetTransformComponentOfSelectedObject(FEScene* Scene);
	void ApplyChangesToSelectedObject(FETransformComponent Changes, FEScene* Scene);

	std::unordered_map<std::string, FEGizmoSceneData*> PerSceneData;
	void ClearAllSceneData();
	void ClearSceneData(const std::string& SceneID);
	void AddSceneData(const std::string& SceneID);
	FEGizmoSceneData* GetSceneData(const std::string& SceneID);
};

#define GIZMO_MANAGER GizmoManager::getInstance()