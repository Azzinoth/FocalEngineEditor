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
	friend class EditMaterialPopup;
	friend class DeleteMaterialPopup;
	friend class SelectFEObjectPopUp;
	friend class PrefabEditorWindow;

private:
	SINGLETON_PUBLIC_PART(FEEditorPreviewManager)
	SINGLETON_PRIVATE_PART(FEEditorPreviewManager)

	void InitializeResources();
	void ReInitializeEntities();
	void UpdateAll();

	FEFramebuffer* PreviewFB;
	FEEntity* PreviewEntity;
	FEPrefab* PreviewPrefab;
	FEGameModel* PreviewGameModel;
	FEMaterial* MeshPreviewMaterial;

	std::unordered_map<std::string, FETexture*> MeshPreviewTextures;
	std::unordered_map<std::string, FETexture*> MaterialPreviewTextures;
	std::unordered_map<std::string, FETexture*> GameModelPreviewTextures;
	std::unordered_map<std::string, FETexture*> PrefabPreviewTextures;

	static glm::vec4 OriginalClearColor;
	static FETransformComponent OriginalMeshTransform;
	static glm::vec3 OriginalCameraPosition;
	static float OriginalAspectRation;
	static float OriginalCameraPitch;
	static float OriginalCameraRoll;
	static float OriginalCameraYaw;
	static float OriginalExposure;

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