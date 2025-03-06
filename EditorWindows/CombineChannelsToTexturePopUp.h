#pragma once

#include "GyzmosSettingsWindow.h"

class CombineChannelsToTexturePopUp : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(CombineChannelsToTexturePopUp)

	// ************** Drag&Drop **************
	struct NodeAreaTargetCallbackInfo
	{
		FETexture* Texture;
	};
	NodeAreaTargetCallbackInfo DragAndDropCallbackInfo;
	DragAndDropTarget* NodeAreaTarget;

	FEEditorTextureCreatingNode* TextureNode = nullptr;

	static bool DragAndDropnodeAreaTargetCallback(FEObject* Object, void** CallbackInfo);
	// ************** Drag&Drop END **************

	// ************** Node area **************
	static VisNodeSys::NodeArea* CurrentNodeArea;

	static FETexture* TextureForNewNode;
	static void NodeSystemMainContextMenu();
	static void TextureNodeCreationCallback(std::vector<FEObject*> SelectionsResult);

	static ImVec2 WindowPosition;
	static ImVec2 NodeGridRelativePosition;
	static ImVec2 MousePositionWhenContextMenuWasOpened;
	// ************** Node area END **************
public:
	SINGLETON_PUBLIC_PART(CombineChannelsToTexturePopUp)

	void Show();
	void Render() override;
};