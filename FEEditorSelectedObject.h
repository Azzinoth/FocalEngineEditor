#pragma once

#include "FEEditorHaloSelectionEffect.h"
using namespace FocalEngine;

//#define EDITOR_SELECTION_DEBUG_MODE

class FESelectionData
{
	friend class FEEditor;
	friend class FEEditorSelectedObject;
	friend class GizmoManager;
	friend class FEEditorInspectorWindow;

	std::string SceneID = "";
public:
	FEFramebuffer* PixelAccurateSelectionFB = nullptr;

	std::vector<FEEntity*> SceneEntitiesUnderMouse;
	FEEntity* PotentiallySelectedEntity = nullptr;

	std::string SelectedEntityID = "";
	FEEntity* DummyEntity = nullptr;

	bool CheckForSelectionisNeeded = false;

	std::unordered_map<FEEntity*, std::vector<int>> InstancedSubObjectsInfo;
	int InstancedSubObjectIndexSelected = -1;

	unsigned char* ColorUnderMouse = new unsigned char[3];
	int ColorIndex = -1;

	~FESelectionData()
	{
		delete PixelAccurateSelectionFB;
		delete[] ColorUnderMouse;
	}
};

class FEEditorSelectedObject
{
	friend class FEEditor;
	friend class GizmoManager;
	friend class FEEditorInspectorWindow;
	friend class FEEditorSceneWindow;

	SINGLETON_PRIVATE_PART(FEEditorSelectedObject)
public:
	SINGLETON_PUBLIC_PART(FEEditorSelectedObject)

	void InitializeResources();
	void UpdateResources(FEScene* Scene);
	void SetOnUpdateFunction(std::function<void(FEScene*)> Func);

	FEEntity* GetSelected(FEScene* Scene);

	void SetSelected(FEEntity* SelectedObject);
	void Clear(FEScene* Scene);
	void ClearAll();

	void DetermineEntityUnderMouse(double MouseX, double MouseY, FEScene* Scene);
	int GetIndexOfObjectUnderMouse(double MouseX, double MouseY, FEScene* Scene);
	void SetSelectedByIndex(size_t Index, FEScene* Scene);

	void OnCameraUpdate() const;

	FESelectionData* GetSceneData(const std::string& SceneID);
private:
	FEShader* FEPixelAccurateInstancedSelection = nullptr;
	FEShader* FEPixelAccurateSelection = nullptr;
	FEMaterial* PixelAccurateSelectionMaterial = nullptr;

	std::function<void(FEScene*)> OnUpdateFunction = nullptr;

	int DebugGetLastColorIndex(FEScene* Scene);

	std::unordered_map<std::string, FESelectionData*> PerSceneData;
	void ClearAllSceneData();
	void ClearSceneData(const std::string& SceneID);
	void AddSceneData(const std::string& SceneID);

	void RenderEntitySelectionColorID(FEEntity* Entity, glm::vec3 ColorID, FEEntity* CameraEntity, FESelectionData* CurrentSelectionData);
	void RenderEntityHaloEffectInternal(FEEntity* Entity, glm::vec3 Color, FEEntity* CameraEntity, FESelectionData* CurrentSelectionData);
};

#define SELECTED FEEditorSelectedObject::GetInstance()