#pragma once

#include "SelectPopups.h"

class EditGameModelPopup : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(EditGameModelPopup)

	enum EDIT_GAME_MODEL_LOD_MODE
	{
		NO_LOD_MODE = 0,
		HAS_LOD_MODE = 1,
	};

	FEGameModel* ObjToWorkWith;
	FEGameModel* TempModel = nullptr;
	FETexture* TempPreview = nullptr;

	FEMaterial* UpdatedMaterial;
	static FEMaterial** MaterialToModify;
	FEMaterial* UpdatedBillboardMaterial;
	static FEMaterial** BillboardMaterialToModify;

	std::vector<FEMesh*> UpdatedLODMeshs;

	ImGuiButton* CancelButton;
	ImGuiButton* ApplyButton;
	ImGuiButton* ChangeMaterialButton;
	ImGuiButton* ChangeBillboardMaterialButton;
	ImGuiButton* AddBillboard;

	std::vector<ImGuiButton*> ChangeLODMeshButton;
	ImGuiButton* DeleteLODMeshButton;

	int CurrentMode = NO_LOD_MODE;

	FERangeConfigurator* LODGroups;
	RECT LOD0RangeVisualization;

	std::vector<ImColor> LODColors;

	// ************** Drag&Drop **************
	struct MeshTargetCallbackInfo
	{
		int LODLevel;
		EditGameModelPopup* Window;
	};
	std::vector<MeshTargetCallbackInfo> LODMeshCallbackInfo;
	std::vector<DragAndDropTarget*> LODMeshTarget;

	struct MaterialTargetCallbackInfo
	{
		bool bBillboardMaterial;
		EditGameModelPopup* Window;
	};
	MaterialTargetCallbackInfo MaterialCallbackInfo;
	MaterialTargetCallbackInfo BillboardMaterialCallbackInfo;
	DragAndDropTarget* MaterialTarget;
	DragAndDropTarget* BillboardMaterialTarget;

	static bool DragAndDropLODMeshCallback(FEObject* Object, void** CallbackInfo);
	static bool DragAndDropMaterialCallback(FEObject* Object, void** CallbackInfo);
	// ************** Drag&Drop END **************

	bool IsLastSetupLOD(size_t LODIndex);

	const float NO_LOD_WINDOW_WIDTH = 460.0f;
	const float NO_LOD_WINDOW_HEIGHT = 520.0f;

	static FEMesh** MeshToModify;
	static void ChangeMeshCallBack(std::vector<FEObject*> SelectionsResult);
	static void ChangeMaterialCallBack(std::vector<FEObject*> SelectionsResult);
	static void ChangeBillboardMaterialCallBack(std::vector<FEObject*> SelectionsResult);
public:
	SINGLETON_PUBLIC_PART(EditGameModelPopup)

	void Show(FEGameModel* GameModel);
	void SwitchMode(int ToMode);
	void DisplayLODGroups();
	void Render() override;
	void Close();
};

class EditMaterialWindow : public FEImGuiWindow
{
	friend class FEEditor;

	SINGLETON_PRIVATE_PART(EditMaterialWindow)

	ImGuiButton* CancelButton;
	ImGuiImageButton* IconButton = nullptr;
	int TextureCount = 0;

	std::vector<std::string> Channels = { "r", "g", "b", "a" };
	int TextureFromListUnderMouse = -1;
	FETexture* TempContainer = nullptr;
	int TextureDestination = -1;

	static FEMaterial* ObjToWorkWith;

	// ************** Node area **************
	static VisNodeSys::NodeArea* MaterialNodeArea;

	static ImVec2 WindowPosition;
	static ImVec2 NodeGridRelativePosition;
	static ImVec2 MousePositionWhenContextMenuWasOpened;

	static FETexture* TextureForNewNode;
	static void NodeSystemMainContextMenu();
	static void TextureNodeCreationCallback(std::vector<FEObject*> SelectionsResult);
	static void TextureNodeCallback(VisNodeSys::Node* Node, VisNodeSys::NODE_EVENT EventWithNode);
	// ************** Node area END **************
	
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

	// ************** Scene **************
	FEScene* PreviewScene;
	FEGameModel* PreviewGameModel;
	FEEntity* PreviewEntity;
	FEEntity* PreviewCameraEntity;
	FEEntity* PreviewLightEntity;

	bool bWindowHovered = false;
	bool bCameraOutputHovered = false;
	static void MouseButtonCallback(const int Button, const int Action, int Mods);
	// ************** Scene END **************

	void Stop();

	FEEntity* InjectModelViewCamera(FEScene* Scene);
public:
	SINGLETON_PUBLIC_PART(EditMaterialWindow)

	void Show(FEMaterial* Material);
	void Render() override;
	void Close();
};

#define EDITOR_MATERIAL_WINDOW EditMaterialWindow::GetInstance()