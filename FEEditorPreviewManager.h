#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEEditorPreviewManager
{
	friend class FEEditor;
	friend class FEEditorInspectorWindow;
	friend class FEEditorContentBrowserWindow;
	friend class FEProjectManager;
	friend class DeleteTexturePopup;
	friend class RenameMeshPopUp;
	friend class DeleteMeshPopup;
	friend class SelectMeshPopUp;
	friend class SelectMaterialPopUp;
	friend class SelectGameModelPopUp;
	friend class EditGameModelPopup;
	friend class EditMaterialWindow;
	friend class DeleteMaterialPopup;
	friend class SelectFEObjectPopUp;
	friend class FEPrefabEditorManager;

private:
	SINGLETON_PUBLIC_PART(FEEditorPreviewManager)
	SINGLETON_PRIVATE_PART(FEEditorPreviewManager)

	void InitializeResources();
	void UpdateAll();

	FEScene* PreviewScene = nullptr;

	FEEntity* PreviewEntity;
	FEGameModel* PreviewGameModel;
	FEMaterial* MeshPreviewMaterial;

	FEEntity* LocalSunEntity;
	FEEntity* LocalCameraEntity;

	// Saved scene settings
	bool bIsRegularFogEnabled = false;

	std::unordered_map<std::string, FETexture*> MeshPreviewTextures;
	std::unordered_map<std::string, FETexture*> MaterialPreviewTextures;
	std::unordered_map<std::string, FETexture*> GameModelPreviewTextures;
	std::unordered_map<std::string, FETexture*> PrefabPreviewTextures;

	static glm::vec4 OriginalClearColor;
	static FETransformComponent OriginalTransform;

	void BeforePreviewActions();
	void AfterPreviewActions();

	void CreateMeshPreview(std::string MeshID);
	FETexture* GetMeshPreview(std::string MeshID);

	void CreateMaterialPreview(std::string MaterialID);
	FETexture* GetMaterialPreview(std::string MaterialID);

	void CreateGameModelPreview(std::string GameModelID);
	void CreateGameModelPreview(const FEGameModel* GameModel, FETexture** ResultingTexture);
	FETexture* GetGameModelPreview(std::string GameModelID);
	void UpdateAllGameModelPreviews();

	void CreatePrefabPreview(std::string PrefabID);
	void CreatePrefabPreview(FEPrefab* Prefab, FETexture** ResultingTexture);
	FETexture* GetPrefabPreview(std::string PrefabID);

	FETexture* GetPreview(FEObject* Object);
	FETexture* GetPreview(std::string ObjectID);

	void Clear();
};

#define PREVIEW_MANAGER FEEditorPreviewManager::getInstance()