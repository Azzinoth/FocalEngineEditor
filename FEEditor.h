#pragma once

#include "EditorWindows/InspectorWindow.h"
#include "EditorWindows/EditorBaseWindowClasses/FEEditorSceneWindow.h"
#include <functional>

class FEEditor
{
    friend class FEEditorSceneWindow;
    friend class FEProjectManager;
    // FIX ME! It is temporary solution
    friend class FEPrefabEditorManager;
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

    // Clipboard
    std::string GetSceneEntityIDInClipboard();
    void SetSceneEntityIDInClipboard(std::string NewValue);

    FEScene* GetFocusedScene() const;
private:
    SINGLETON_PRIVATE_PART(FEEditor)

    // Mouse and input
    double LastMouseX, LastMouseY;
    double MouseX, MouseY;

    std::string FocusedEditorSceneID = "";
    ImGuiID DockspaceID = 0;

    void AddEditorScene(FEScene* Scene, bool bMain = false);
    void AddCustomEditorScene(FEEditorSceneWindow* SceneWindow);
    std::vector<FEEditorSceneWindow*> EditorSceneWindows;

    // Clipboard
    std::string SceneEntityIDInClipboard;

    // Callbacks
    static void AfterEngineUpdate();
    static void MouseButtonCallback(int Button, int Action, int Mods);
    static void MouseMoveCallback(double Xpos, double Ypos);
    static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
    static void OnViewportResize(std::string ViewportID);
    static void DropCallback(int Count, const char** Paths);
    static void CloseWindowCallBack();

    // Effects window
    bool bEffectsWindowVisible = true;
    void DisplayEffectsWindow() const;

    // Log window
    bool bLogWindowVisible = true;
    void DisplayLogWindow() const;

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

    void OnProjectClose();

    void SetFocusedScene(FEScene* NewSceneInFocus);
    void BeforeChangeOfFocusedScene(FEScene* NewSceneInFocus);
};

#define EDITOR FEEditor::getInstance()