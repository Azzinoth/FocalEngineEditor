#pragma once

#include "SelectPopups.h"

class EditGameModelPopup : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(EditGameModelPopup)

	FEGameModel* ObjectToWorkWith;
	FEGameModel* TemporaryModel = nullptr;
	FETexture* TemporaryPreview = nullptr;

	FEMaterial* UpdatedMaterial;
	static FEMaterial** MaterialToModify;
	FEMaterial* UpdatedBillboardMaterial;
	static FEMaterial** BillboardMaterialToModify;

	std::vector<FEMesh*> UpdatedLODMeshes;

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

	const float WindowPadding = 20.0f;
	const float ContentWidth = 1060.0f;
	const float ResultColumnWidth = 300.0f;
	const float MaterialCardHeight = 92.0f;
	const float LevelRowHeight = 84.0f;
	const float FooterHeight = 72.0f;
	const float MaxCullDistance = 5000.0f;

	// Number of levels that have a mesh, 1 when levels of detail are not used.
	size_t GetLevelCount();
	bool HasBillboardLevel();
	ImVec2 GetContentSize();
	ImVec2 GetWindowSize();

	void ApplyUpdatedResources();
	void CorrectLevelDistances();
	void SetUsingLevels(bool bUseLevels);
	void ApplyChanges();

	void ShowMeshSelection(size_t LevelIndex);
	void ShowMaterialSelection();
	void ShowBillboardMaterialSelection();

	void RenderResultColumn(ImVec2 Origin);
	void RenderMaterialCard(ImVec2 Min, float Width);
	void RenderLevelRow(size_t LevelIndex, ImVec2 Min, float Width);
	void RenderDistanceBar(ImVec2 Min, ImVec2 Size);

	static FEMesh** MeshToModify;
	static void ChangeMeshCallback(std::vector<FEObject*> SelectionsResult);
	static void ChangeMaterialCallback(std::vector<FEObject*> SelectionsResult);
	static void ChangeBillboardMaterialCallback(std::vector<FEObject*> SelectionsResult);
public:
	SINGLETON_PUBLIC_PART(EditGameModelPopup)

	void Show(FEGameModel* GameModel);
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
	FETexture* TemporaryContainer = nullptr;
	int TextureDestination = -1;

	static FEMaterial* ObjectToWorkWith;

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

	static bool DragAndDropNodeAreaTargetCallback(FEObject* Object, void** CallbackInfo);
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