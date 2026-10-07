#pragma once

#include "FEEditorSceneWindow.h"

class FEProject;

class FEEditorSceneWindowManager
{
	friend class FEEditor;
	friend class FEEditorSceneWindow;
public:
	SINGLETON_PUBLIC_PART(FEEditorSceneWindowManager)

	void CreateSceneWindow(const FEUUID& SceneID, FEProject* CurrentProject = nullptr);
	void RegisterSceneWindow(FEEditorSceneWindow* SceneWindow);
	void DeleteSceneAndCleanup(const FEUUID& SceneID);

	FEEditorSceneWindow* GetSceneWindow(const FEUUID& SceneID);
	std::vector<FEUUID> GetOpenedScenesIDs() const;
	const std::vector<FEEditorSceneWindow*>& GetAllSceneWindows() const;

	bool SetFocusedScene(FEScene* NewSceneInFocus);
	bool SetFocusedScene(const FEUUID& NewSceneInFocusID);
	FEScene* GetFocusedScene() const;
	FEEditorSceneWindow* GetFocusedSceneWindow() const;

	void Clear();

	static void OnViewportResize(FEUUID ViewportID);
	void Update();

	void MouseButtonCallback(int Button, int Action, int Mods);
	void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
	void MouseMoveCallback(double Xpos, double Ypos, double LastXpos, double LastYpos);

	void RemoveUserClosedWindows();
private:
	SINGLETON_PRIVATE_PART(FEEditorSceneWindowManager)

	std::vector<FEEditorSceneWindow*> SceneWindows;
	std::unordered_map<FEUUID, bool> SeenScenesID;
	FEUUID FocusedSceneID;

	void BeforeChangeOfFocusedScene(FEScene* NewSceneInFocus);
};

#define EDITOR_SCENE_WINDOW_MANAGER FEEditorSceneWindowManager::GetInstance()
