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
	FEEditorSceneWindow(FEScene* Scene);
	~FEEditorSceneWindow();
	
	 FEScene* Scene = nullptr;
	 std::string SceneID = "";
	 DragAndDropTarget* SceneWindowTarget = nullptr;
	 
	 bool bJustAdded = true;
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