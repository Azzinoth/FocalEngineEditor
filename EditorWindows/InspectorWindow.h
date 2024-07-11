#pragma once

#include "ContentBrowserWindow.h"

class FEEditorInspectorWindow
{
    friend class FEEditor;
    SINGLETON_PRIVATE_PART(FEEditorInspectorWindow)

    // Visibility
    bool bVisible = true;
    static FEEntity* EntityToModify;

    // Light properties
    void DisplayLightProperties(FEEntity* LightEntity) const;

    // Terrain settings
    static FEEntity* TerrainToWorkWith;
    static void CreateNewTerrainLayerWithMaterialCallBack(std::vector<FEObject*> SelectionsResult);
    static void ChangeMaterialInTerrainLayerCallBack(std::vector<FEObject*> SelectionsResult);
    ImGuiButton* ExportHeightMapButton = nullptr;
    ImGuiButton* ImportHeightMapButton = nullptr;
    ImGuiImageButton* SculptBrushButton = nullptr;
    ImGuiImageButton* LevelBrushButton = nullptr;
    ImGuiImageButton* SmoothBrushButton = nullptr;
    ImGuiImageButton* LayerBrushButton = nullptr;
    static bool EntityChangePrefabTargetCallBack(FEObject* Object, void** EntityPointer);
    static bool TerrainChangeMaterialTargetCallBack(FEObject* Object, void** LayerIndex);
    static void ChangePrefabOfEntityCallBack(std::vector<FEObject*> SelectionsResult);
    DragAndDropTarget* EntityChangePrefabTarget = nullptr;
    std::vector<int> TerrainChangeMaterialIndecies;
    std::vector<DragAndDropTarget*> TerrainChangeLayerMaterialTargets;
    int HoveredTerrainLayerItem = -1;
    void DisplayTerrainSettings(FEEntity* TerrainEntity);
    int TerrainLayerRenameIndex = -1;
    char TerrainLayerRename[1024];
    bool bLastFrameTerrainLayerRenameEditWasVisiable = false;

    // Brush icons
    FETexture* SculptBrushIcon = nullptr;
    FETexture* LevelBrushIcon = nullptr;
    FETexture* SmoothBrushIcon = nullptr;
    FETexture* MouseCursorIcon = nullptr;
    FETexture* ArrowToGroundIcon = nullptr;
    FETexture* DrawBrushIcon = nullptr;

    // Context menu
    bool bShouldOpenContextMenu = false;

    bool bLeftMousePressed = false;
    bool bShiftPressed = false;

    void ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const;
    void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform) const;
   
    // Rendering and initialization
    void Render();
    void Clear();
    void InitializeResources();

public:
    SINGLETON_PUBLIC_PART(FEEditorInspectorWindow)
};

#define INSPECTOR_WINDOW FEEditorInspectorWindow::getInstance()