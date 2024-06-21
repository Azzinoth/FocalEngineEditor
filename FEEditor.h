﻿#pragma once

#include "EditorWindows/InspectorWindow.h"
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
};

#define EDITOR FEEditor::getInstance()