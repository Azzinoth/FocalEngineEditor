#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEHaloSelectionData
{
	friend class FEEditor;
	friend class FEEditorSelectedObject;
	friend class FEEditorHaloSelectionEffect;

	std::string SceneID = "";
	FEFramebuffer* HaloObjectsFB = nullptr;
	FEPostProcess* PostProcess = nullptr;

	~FEHaloSelectionData()
	{
		delete HaloObjectsFB;
	}
};

class FEEditorHaloSelectionEffect
{
	friend class FEEditorSelectedObject;
	friend class FEEditor;
private:
	SINGLETON_PUBLIC_PART(FEEditorHaloSelectionEffect)
	SINGLETON_PRIVATE_PART(FEEditorHaloSelectionEffect)

	void InitializeResources();
	void UpdateResources(FEScene* Scene);

	FEMaterial* HaloMaterial = nullptr;
	FEShader* HaloDrawInstancedObjectShader = nullptr;
	FEShader* HaloDrawObjectShader = nullptr;
	FEShader* HaloFinalShader = nullptr;

	std::unordered_map<std::string, FEHaloSelectionData*> PerSceneData;
	void ClearAllSceneData();
	void ClearSceneData(const std::string& SceneID);
	void AddSceneData(const std::string& SceneID);
	FEHaloSelectionData* GetSceneData(const std::string& SceneID);
};

#define HALO_SELECTION_EFFECT FEEditorHaloSelectionEffect::getInstance()