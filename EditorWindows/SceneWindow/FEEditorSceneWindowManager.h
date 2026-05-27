#pragma once

#include "FEEditorSceneWindow.h"

class FEProject;

class FEEditorSceneWindowManager
{
	friend class FEEditor;
	friend class FEEditorSceneWindow;
public:
	SINGLETON_PUBLIC_PART(FEEditorSceneWindowManager)

	void CreateSceneWindow(const std::string& SceneID, FEProject* CurrentProject = nullptr);
	void RegisterSceneWindow(FEEditorSceneWindow* SceneWindow);
	void DeleteSceneAndCleanup(std::string SceneID);

	FEEditorSceneWindow* GetSceneWindow(std::string SceneID);
	std::vector<std::string> GetOpenedScenesIDs() const;
	const std::vector<FEEditorSceneWindow*>& GetAllSceneWindows() const;

	bool SetFocusedScene(FEScene* NewSceneInFocus);
	bool SetFocusedScene(std::string NewSceneInFocusID);
	FEScene* GetFocusedScene() const;
	FEEditorSceneWindow* GetFocusedSceneWindow() const;

	void Clear();

	static void OnViewportResize(std::string ViewportID);
	void Update();

	void MouseButtonCallback(int Button, int Action, int Mods);
	void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
	void MouseMoveCallback(double Xpos, double Ypos, double LastXpos, double LastYpos);

	void RemoveUserClosedWindows();
private:
	SINGLETON_PRIVATE_PART(FEEditorSceneWindowManager)

	std::vector<FEEditorSceneWindow*> SceneWindows;
	std::unordered_map<std::string, bool> SeenScenesID;
	std::string FocusedSceneID;

	void BeforeChangeOfFocusedScene(FEScene* NewSceneInFocus);
};

#define EDITOR_SCENE_WINDOW_MANAGER FEEditorSceneWindowManager::GetInstance()
