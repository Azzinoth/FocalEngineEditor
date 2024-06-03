#pragma once

#include "EditorWindows/ContentBrowserWindow.h"
#include <functional>

class DragAndDropTarget;
class FEEditor
{
public:
    SINGLETON_PUBLIC_PART(FEEditor)

    // Initialization and rendering
    void InitializeResources();
    void Render();

    // Mouse and input
    double GetLastMouseX() const;
    void SetLastMouseX(double NewValue);
    double GetLastMouseY() const;
    void SetLastMouseY(double NewValue);
    double GetMouseX() const;
    void SetMouseX(double NewValue);
    double GetMouseY() const;
    void SetMouseY(double NewValue);
    bool bLeftMousePressed = false;
    bool bShiftPressed = false;

    // Clipboard
    std::string GetObjectNameInClipboard();
    void SetObjectNameInClipboard(std::string NewValue);
private:
    SINGLETON_PRIVATE_PART(FEEditor)

    // Mouse and input
    double LastMouseX, LastMouseY;
    double MouseX, MouseY;
    bool bSceneWindowHovered;
    bool bIsCameraInputActive = false;

    // Clipboard
    std::string ObjectNameInClipboard;

    // Scene window
    static ImGuiWindow* SceneWindow;

    // Brush icons
    FETexture* SculptBrushIcon = nullptr;
    FETexture* LevelBrushIcon = nullptr;
    FETexture* SmoothBrushIcon = nullptr;
    FETexture* MouseCursorIcon = nullptr;
    FETexture* ArrowToGroundIcon = nullptr;
    FETexture* DrawBrushIcon = nullptr;

    // Drag and drop
    DragAndDropTarget* SceneWindowTarget = nullptr;

    // Callbacks
    static void OnCameraUpdate(FEBasicCamera* Camera);
    static void MouseButtonCallback(int Button, int Action, int Mods);
    static void MouseMoveCallback(double Xpos, double Ypos);
    static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
    static void RenderTargetResizeCallback(int NewW, int NewH);
    static void DropCallback(int Count, const char** Paths);
    static void CloseWindowCallBack();

    // Inspector
    void DisplayInspector();
    bool bInspectorVisible = true;
    void ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const;
    void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform) const;

    // Effects window
    bool bEffectsWindowVisible = true;
    void DisplayEffectsWindow() const;

    // Log window
    bool bLogWindowVisible = true;
    void DisplayLogWindow() const;

    // Light properties
    void DisplayLightProperties(FELight* Light) const;
    void DisplayLightsProperties() const;

    // Terrain settings
    ImGuiButton* ExportHeightMapButton = nullptr;
    ImGuiButton* ImportHeightMapButton = nullptr;
    ImGuiImageButton* SculptBrushButton = nullptr;
    ImGuiImageButton* LevelBrushButton = nullptr;
    ImGuiImageButton* SmoothBrushButton = nullptr;
    ImGuiImageButton* LayerBrushButton = nullptr;
    static bool EntityChangePrefabTargetCallBack(FEObject* Object, void** EntityPointer);
    static bool TerrainChangeMaterialTargetCallBack(FEObject* Object, void** LayerIndex);
    static FEEntity* EntityToModify;
    static void ChangePrefabOfEntityCallBack(std::vector<FEObject*> SelectionsResult);
    DragAndDropTarget* EntityChangePrefabTarget = nullptr;
    std::vector<int> TerrainChangeMaterialIndecies;
    std::vector<DragAndDropTarget*> TerrainChangeLayerMaterialTargets;
    int HoveredTerrainLayerItem = -1;
    void DisplayTerrainSettings(FETerrain* Terrain);
    int TerrainLayerRenameIndex = -1;
    char TerrainLayerRename[1024];
    bool bLastFrameTerrainLayerRenameEditWasVisiable = false;

    // Resource under mouse
    int TextureUnderMouse = -1;
    int MeshUnderMouse = -1;
    std::string ShaderIdUnderMouse;
    int MaterialUnderMouse = -1;
    int GameModelUnderMouse = -1;
    int EntityUnderMouse = -1;

    // Game mode
    bool bGameMode = false;
    bool IsInGameMode() const;
    void SetGameMode(bool GameMode);

    // Sub-windows
    void RenderAllSubWindows();

    // ImGui setup
    void SetUpImgui();
    void SetImguiStyle();
};

#define EDITOR FEEditor::getInstance()