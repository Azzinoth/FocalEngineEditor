#pragma once

#include "../../FEDearImguiWrapper/FEDearImguiWrapper.h"
#include "../../FEEditorDragAndDropManager.h"
#include <functional>

class FEEditorSceneWindow : public FEImGuiWindow
{
	friend class FEEditor;
private:
	static bool DragAndDropCallBack(FEObject* Object, void** UserData);
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

	 std::vector<FE_OBJECT_TYPE> AcceptedTypes;
	 std::vector<std::string> ToolTipTexts;
	 std::function<bool(FEObject* , void**)> CurrentDragAndDropCallback;
public:
	void Show();
	void Render() override;
	void OnRenderEnd() override;

	FEScene* GetScene() const;
};