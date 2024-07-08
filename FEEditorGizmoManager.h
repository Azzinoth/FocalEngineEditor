#pragma once

#include "../FEngine.h"
#include "FEEditorSelectedObject.h"
using namespace FocalEngine;

class FEEditor;
struct SelectedObject;

const int TRANSFORM_GIZMOS = 0;
const int SCALE_GIZMOS = 1;
const int ROTATE_GIZMOS = 2;

class GizmoManager
{
	friend FEEditor;
	friend SelectedObject;
public:
	SINGLETON_PUBLIC_PART(GizmoManager)

	float GizmosScale = 0.00175f;
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

	void InitializeResources();
	void ReInitializeEntities();

	void DeactivateAllGizmo();
	void HideAllGizmo();
	void UpdateGizmoState(int NewState);
	void Render();
	bool WasSelected(int Index);

	void MouseMove(double LastMouseX, double LastMouseY, double MouseX, double MouseY);
private:
	SINGLETON_PRIVATE_PART(GizmoManager)

	double LastMouseX = 0, LastMouseY = 0, MouseX = 0, MouseY = 0;

	glm::vec3 GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal);
	glm::vec3 GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal, glm::vec3& LastMousePointOnPlane);

	void MouseMoveTransformationGizmos();
	void MouseMoveScaleGizmos();
	void MouseMoveRotateGizmos();

	static void OnSelectedObjectUpdate();

	FETransformComponent GetTransformComponentOfSelectedObject();
	void MoveSelectedEntityAlongAxis(const glm::vec3& AxisOfMovement, FETransformComponent& ObjectTransform);
	void RotateSelectedEntity(const glm::vec3& AxisOfRotation, FETransformComponent& ObjectTransform, const float& RotationAmount);
	void ApplyChangesToSelectedObject(FETransformComponent Changes);
};

#define GIZMO_MANAGER GizmoManager::getInstance()