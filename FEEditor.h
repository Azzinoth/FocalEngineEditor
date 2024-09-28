#pragma once

#include "EditorWindows/InspectorWindow.h"
#include "EditorWindows/EditorBaseWindowClasses/FEEditorSceneWindow.h"
#include <functional>

class FEEditor
{
    friend class FEEditorSceneWindow;
    friend class FEProjectManager;
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

    void AddEditorScene(FEScene* Scene);
    void AddCustomEditorScene(FEEditorSceneWindow* SceneWindow);
    FEEditorSceneWindow* GetEditorSceneWindow(std::string SceneID);

    std::vector<std::string> GetEditorOpenedScenesIDs() const;

    bool SetFocusedScene(FEScene* NewSceneInFocus);
	bool SetFocusedScene(std::string NewSceneInFocusID);
    FEScene* GetFocusedScene() const;

    bool IsInGameMode() const;
    void SetGameMode(bool GameMode);
private:
    SINGLETON_PRIVATE_PART(FEEditor)

    // Mouse and input
    double LastMouseX, LastMouseY;
    double MouseX, MouseY;

    std::string FocusedEditorSceneID = "";
    ImGuiID DockspaceID = 0;

    std::vector<FEEditorSceneWindow*> EditorSceneWindows;

	void DeleteScene(std::string SceneID);

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
	bool SetGameModeInternal(bool GameMode);
	std::unordered_map<std::string, FEScene*> ParentIDToScenesInGameMode;
	bool DuplicateScenesForGameMode();

    // Sub-windows
    void RenderAllSubWindows();

    // ImGui setup
    void SetUpImgui();
    void SetImguiStyle();

    void OnProjectClose();

    void BeforeChangeOfFocusedScene(FEScene* NewSceneInFocus);
};

#define EDITOR FEEditor::GetInstance()