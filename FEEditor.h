#pragma once

#include "EditorWindows/InspectorWindow.h"
#include "EditorWindows/SceneWindow/FEEditorSceneWindow.h"
#include "EditorWindows/SceneWindow/FEEditorSceneWindowManager.h"
#include <functional>

class FEEditor
{
    friend class FEEditorSceneWindow;
    friend class FEProjectManager;
    friend class FEPrefabEditorManager;
public:
    SINGLETON_PUBLIC_PART(FEEditor)

    std::string GetEditorVersion();        // "1.0.0"
    std::string GetEditorBuildInfo();      // "build 231+52 (dev, ed4c7ce-dirty)"
	std::string GetEditorFullVersion();    // "Focal Engine Editor 1.0.0 build 231+52 (dev, ed4c7ce-dirty)"
    std::string GetEditorBuildTimestamp(); // "20260207232613"
    int GetEditorBuildNumber();            // 231

    // Initialization and rendering
    void InitializeResources();
	bool HadImGuiIniFileAtStartup() const;
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
    FEUUID GetSceneEntityIDInClipboard();
    void SetSceneEntityIDInClipboard(const FEUUID& NewValue);

    void CreateEditorWindowForScene(const FEUUID& SceneID, FEProject* CurrentProject = nullptr);
    void RegisterEditorSceneWindow(FEEditorSceneWindow* SceneWindow);
    FEEditorSceneWindow* GetEditorSceneWindow(const FEUUID& SceneID);

    std::vector<FEUUID> GetEditorOpenedScenesIDs() const;

    bool SetFocusedScene(FEScene* NewSceneInFocus);
	bool SetFocusedScene(const FEUUID& NewSceneInFocusID);
    FEScene* GetFocusedScene() const;

    bool IsInGameMode() const;
    void SetGameMode(bool GameMode);

    void UpdateBeforeRender();

	void RenderTemporaryDebugWindow();
private:
    SINGLETON_PRIVATE_PART(FEEditor)

    // Mouse and input
    double LastMouseX, LastMouseY;
    double MouseX, MouseY;

    // Clipboard
    FEUUID SceneEntityIDInClipboard;

    // Callbacks
    static void AfterEngineUpdate();
    static void MouseButtonCallback(int Button, int Action, int Mods);
    static void MouseMoveCallback(double Xpos, double Ypos);
    static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
    static void OnViewportResize(FEUUID ViewportID);
    static void DropCallback(int Count, const char** Paths);
    static void CloseWindowCallBack();

    // Effects window
    bool bEditorCamerasWindowVisible = true;
    void DisplayEditorCamerasWindow() const;

    // Log window
    bool bLogWindowVisible = true;
    void DisplayLogWindow() const;

	// About window
    bool bShouldOpenAboutWindow = false;
    void ShowAboutDialog();
    void RenderAboutWindow();

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
	std::unordered_map<FEUUID, FEScene*> ParentIDToScenesInGameMode;
	bool DuplicateScenesForGameMode();

    // Sub-windows
    void RenderAllSubWindows();

    // ImGui setup
    bool bHadImGuiIniFileAtStartup = false;
	void SetUpDocking();
    void SetUpImGui();
    void SetImGuiStyle();

    void CloseProjectAndCleanup();
};

#define EDITOR FEEditor::GetInstance()