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
	friend class DeletePointCloudPopup;
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
	void ReCreateAll();

	void Update();

	FEScene* PreviewScene = nullptr;

	FEEntity* PreviewEntity;
	FEGameModel* PreviewGameModel;
	FEMaterial* MeshPreviewMaterial;

	FEEntity* LocalSunEntity;
	FEEntity* LocalCameraEntity;

	// Saved scene settings
	bool bIsRegularFogEnabled = false;

	std::unordered_map<FEUUID, FETexture*> PreviewTextures;

	void StorePreview(const FEUUID& ObjectID, FETexture* CameraResult);
	void RemovePreview(const FEUUID& ObjectID);
	FETexture* GetCachedPreview(const FEUUID& ObjectID, const std::function<void()>& CreateFunction);

	static glm::vec4 OriginalClearColor;
	static FETransformComponent OriginalTransform;

	void BeforePreviewActions();
	void AfterPreviewActions();

	void CreateMeshPreview(const FEUUID& MeshID);
	FETexture* GetMeshPreview(const FEUUID& MeshID);

	void CreateMaterialPreview(const FEUUID& MaterialID);
	FETexture* GetMaterialPreview(const FEUUID& MaterialID);

	void CreateGameModelPreview(const FEUUID& GameModelID);
	void CreateGameModelPreview(const FEGameModel* GameModel, FETexture** ResultingTexture);
	FETexture* GetGameModelPreview(const FEUUID& GameModelID);
	void UpdateAllGameModelPreviews();

	void CreatePointCloudPreview(const FEUUID& PointCloudID);
	FETexture* GetPointCloudPreview(const FEUUID& PointCloudID);

	void CreatePrefabPreview(const FEUUID& PrefabID);
	void CreatePrefabPreview(FEPrefab* Prefab, FETexture** ResultingTexture);
	FETexture* GetPrefabPreview(const FEUUID& PrefabID);

	void CreateScenePreview(const FEUUID& SceneID);
	FETexture* GetScenePreview(const FEUUID& SceneID);

	FENewMaterial* MaterialFor3DTextures = nullptr;
	void CreateTexture3DPreview(const FEUUID& TextureID);
	FETexture* GetTexture3DPreview(const FEUUID& TextureID);

	FETexture* GetPreview(FEObject* Object);
	FETexture* GetPreview(const FEUUID& ObjectID);

	void CheckAndUpdateIfNeededGameModelPreview(const FEUUID& ObjectIDThatWasChanged);
	void CheckAndUpdateIfNeededPrefabPreview(const FEUUID& GameModelIDThatWasChanged);

	void Clear();
};

#define PREVIEW_MANAGER FEEditorPreviewManager::GetInstance()