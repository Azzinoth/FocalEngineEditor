#pragma once

#include "CombineChannelsToTexturePopUp.h"
#include "EditorBaseWindowClasses/FEEditorSceneWindow.h"

class FEPrefabSceneEditorWindow : public FEEditorSceneWindow
{
	friend class FEPrefabEditorManager;

	~FEPrefabSceneEditorWindow();

	ImGuiButton* CloseButton = nullptr;
	ImGuiButton* ApplyButton = nullptr;
public:
	FEPrefabSceneEditorWindow(FEScene* Scene);
	void Render() override;
};

class FEPrefabEditorManager
{
	friend class FEEditor;
	friend class FEPrefabSceneEditorWindow;

	SINGLETON_PRIVATE_PART(FEPrefabEditorManager)

	std::unordered_map<FEPrefab*, FEPrefabSceneEditorWindow*> PrefabWindows;
	bool bClearing = false;

	void ApplyModificationsToPrefabScene(FEPrefabSceneEditorWindow* Window);
public:
	SINGLETON_PUBLIC_PART(FEPrefabEditorManager)

	void PrepareEditWinow(FEPrefab* Prefab);
	void Clear();
};

#define PREFAB_EDITOR_MANAGER FEPrefabEditorManager::getInstance()