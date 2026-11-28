#pragma once

#include "ContentBrowserWindow.h"
#include "../FEEditorScriptingSystem.h"
#include "../FEProjectBuildSystem.h"
#include "VolumeTransferFunctionWidget.h"

class FEEditorInspectorWindow
{
    friend class FEEditorSceneGraphWindow;
    friend class FEEditor;
    SINGLETON_PRIVATE_PART(FEEditorInspectorWindow)

    // Visibility
    bool bVisible = true;
    bool bContextMenuOpened = false;
    static FEEntity* EntityToModify;

    void DisplayLightProperties(FEEntity* LightEntity) const;
    void DisplayCameraProperties(FEEntity* CameraEntity) const;
	void DisplayVirtualUIProperties(FEEntity* VirtualUIEntity) const;
	void DisplayNativeScriptProperties(FEEntity* NativeScriptEntity) const;
	void DisplayGameModelProperties(FEEntity* GameModelEntity);
	void DisplayPointCloudProperties(FEEntity* PointCloudEntity);
	void DisplayPrefabInstanceProperties(FEEntity* PrefabInstanceEntity);
	void DisplayVolumeProperties(FEEntity* VolumeEntity);
	void DisplayInstancedProperties(FEEntity* InstancedEntity);
	void RenderResourceCard(std::string ID, FETexture* PreviewTexture, std::string Name, const std::vector<std::string>& InformationLines, DragAndDropTarget* DropTarget, bool bHasResource, bool& bChangeRequested, bool& bShowInFolderRequested, std::string ActionName = "");

    // Terrain settings
    static FEEntity* TerrainToWorkWith;
    static void CreateNewTerrainLayerWithMaterialCallback(std::vector<FEObject*> SelectionsResult);
    static void ChangeMaterialInTerrainLayerCallback(std::vector<FEObject*> SelectionsResult);
    static bool EntityChangeGameModelTargetCallback(FEObject* Object, void** EntityPointer);
	static bool EntityChangePointCloudTargetCallback(FEObject* Object, void** EntityPointer);
    static bool TerrainChangeMaterialTargetCallback(FEObject* Object, void** LayerIndex);
    static void ChangeGameModelOfEntityCallback(std::vector<FEObject*> SelectionsResult);
    static void ChangePointCloudOfEntityCallback(const std::vector<FEObject*> SelectionsResult);
    DragAndDropTarget* EntityChangeGameModelTarget = nullptr;
	DragAndDropTarget* EntityChangePointCloudTarget = nullptr;
    std::vector<int> TerrainChangeMaterialIndices;
    std::vector<DragAndDropTarget*> TerrainChangeLayerMaterialTargets;
    void DisplayTerrainSettings(FEEntity* TerrainEntity);
    void RenderTerrainLayers(FEEntity* TerrainEntity);
    void SelectMaterialForTerrainLayer(FEEntity* TerrainEntity, int LayerIndex);
    FEEntity* TerrainWithLayersShown = nullptr;
    int TerrainLayerRenameIndex = -1;
	std::string TerrainLayerRenameBuffer = "";
    bool bLastFrameTerrainLayerRenameEditWasVisible = false;

    // Icons
    FETexture* LockIcon = nullptr;

    // Context menu
    bool bShouldOpenContextMenu = false;

    VolumeTransferFunctionWidget TransferFunctionWidget;

    bool bLeftMousePressedInSceneWindow = false;
    bool bShiftPressed = false;

    void ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const;
    void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform) const;
   
    // Rendering and initialization
    void Render();
    void Clear();
    void InitializeResources();

    void RenderAddComponentMenu(FEEntity* Entity);

    bool AddComponent(FEEntity* Entity, std::string ComponentName);
    std::unordered_map<std::type_index, std::function<void(FEEntity*)>> AddComponentHandlers;
	std::unordered_map<std::type_index, std::function<void(FEEntity*)>> RemoveComponentHandlers;

	bool RenderComponentDeleteButton(FEEntity* Entity, FEComponentTypeInfo* ComponentInfo) const;

	FEEntity* EntityToWorkWith = nullptr;
    static void AddLightComponent(FEEntity* Entity);
    static void AddCameraComponent(FEEntity* Entity);
	static void AddGameModelComponent(FEEntity* Entity);
	static void AddPointCloudComponent(FEEntity* Entity);
    static void AddNewGameModelComponentCallback(const std::vector<FEObject*> SelectionsResult);
	static void AddTerrainComponent(FEEntity* Entity);
	static void AddInstancedComponent(FEEntity* Entity);
    static void AddVirtualUIComponent(FEEntity* Entity);
	static void AddNativeScriptComponent(FEEntity* Entity);
	static void AddVolumeComponent(FEEntity* Entity);
public:
    SINGLETON_PUBLIC_PART(FEEditorInspectorWindow)
};

#define INSPECTOR_WINDOW FEEditorInspectorWindow::GetInstance()