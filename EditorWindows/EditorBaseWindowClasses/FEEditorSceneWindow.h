#pragma once

#include "../../FEDearImguiWrapper/FEDearImguiWrapper.h"
#include "../../FEEditorDragAndDropManager.h"
#include <functional>

class FEEditorSceneWindow : public FEImGuiWindow
{
	friend class FEEditor;

protected:
	FEEditorSceneWindow(FEScene* Scene, bool bMain = false);
	~FEEditorSceneWindow();
	
	 FEScene* Scene = nullptr;
	 DragAndDropTarget* SceneWindowTarget = nullptr;
	 
	 bool bJustAdded = true;
	 bool bMain = false;
	 bool bWindowHovered = false;

	 bool bSelfContained = true;
	 bool bWaitingForRemoval = false;
public:
	void Show();
	void Render() override;
	void OnRenderEnd() override;

	FEScene* GetScene() const;
};