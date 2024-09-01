#pragma once

#include "ProjectWasModifiedPopUp.h"

class FEEditorSceneGraphWindow
{
    friend class FEEditor;
    SINGLETON_PRIVATE_PART(FEEditorSceneGraphWindow)

    // Visibility
    bool bVisible = true;

    // Item color management
    void SetCorrectItemColor(FEObject* SceneObject) const;
    void PopCorrectItemColor(FEObject* SceneObject);
    ImVec4 TerrainItemColor = ImVec4(67.0f / 255.0f, 155.0f / 255.0f, 60.0f / 255.0f, 1.0f);
    ImVec4 EntityItemColor = ImVec4(141.0f / 255.0f, 141.0f / 255.0f, 233.0f / 255.0f, 1.0f);
    ImVec4 InstancedEntityItemColor = ImVec4(80.0f / 255.0f, 72.0f / 255.0f, 1.0f, 1.0f);
    ImVec4 CameraItemColor = ImVec4(0.0f, 215.0f / 255.0f, 201.0f / 255.0f, 1.0f);
    ImVec4 LightItemColor = ImVec4(243.0f / 255.0f, 230.0f / 255.0f, 31.0f / 255.0f, 1.0f);

    // Icon management
    void DrawCorrectIcon(const FEObject* SceneObject) const;
    FETexture* EntityIcon = nullptr;
    FETexture* InstancedEntityIcon = nullptr;
    FETexture* DirectionalLightIcon = nullptr;
    FETexture* SpotLightIcon = nullptr;
    FETexture* PointLightIcon = nullptr;
    FETexture* TerrainIcon = nullptr;
    FETexture* CameraIcon = nullptr;

    // Entity filtering
	static const size_t FilterInputBufferSize = 2048;
    char FilterForEntities[FilterInputBufferSize];
	bool bIsPlaceHolderTextUsed = true;
	bool bFilterInputWasFocused = false;
	std::string PlaceHolderTextString = "Filter entities...";
	void RenderFilterInput();
    static int FilterInputTextCallback(ImGuiInputTextCallbackData* data);

    // Context menu
    bool bShouldOpenContextMenu = false;
	bool bLastFrameWasInvisible = true;

    // Entity modification
    static FEEntity* EntityToModify;
    static void ChangePrefabOfEntityCallBack(std::vector<FEObject*> SelectionsResult);

    // Drag and drop
    int64_t ItemUnderMouse = 0;
    int SceneNodeDragAndDropTargetIndex = -1;
    std::unordered_map <int64_t, DragAndDropTarget*> SceneNodeDragAndDropTargets;
    DragAndDropTarget* GetSceneNodeDragAndDropTarget(FENaiveSceneGraphNode* NodeToFind);

    bool bSceneNodeTargetsDirty = true;
    static bool SceneNodeDragAndDropCallback(FEObject* Object, void** SceneNodeTarget)
    {
        if (Object == nullptr || SceneNodeTarget == nullptr ||  Object->GetType() != FE_SCENE_GRAPH_NODE)
			return false;
		
        FENaiveSceneGraphNode* NodeTarget = reinterpret_cast<FENaiveSceneGraphNode*>(SceneNodeTarget);
		FENaiveSceneGraphNode* SceneEntity = reinterpret_cast<FENaiveSceneGraphNode*>(Object);

        FEScene* CurrentScene = SceneEntity->GetEntity()->GetParentScene();
        return CurrentScene->SceneGraph.MoveNode(SceneEntity->GetObjectID(), NodeTarget->GetObjectID());
    }

    //void UpdateSceneNodeDragAndDropTargets();

    // Rendering and initialization.
    void Render();
    void RenderSceneGraph();
    ImRect RenderSubTree(FENaiveSceneGraphNode* SubTreeRoot);
    void Clear();
    void InitializeResources();

    bool bBackgroundColorSwitch = true;
	ImRect SceneGraphBackgroundRect;

	// Colors
    ImVec4 EvenNodeBackgroundColor = ImVec4(50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f);
    ImVec4 OddNodeBackgroundColor = ImVec4(90.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);
    ImVec4 VerticalTreeLineColor = ImVec4(0.5f, 140.0f / 255.0f, 170.0f / 255.0f, 1.0f);

    // Layout
    float VerticalTreeLineXOffset = -6.5f;  // Used to nicely line up with the arrow symbol.
    float VerticalTreeLineYOffset = -8.5f;  // Used to nicely line up with the arrow symbol.
    float HorizontalTreeLineLength = 32.0f;  // Used to make sure the horizontal line is "touching" the arrow symbol of children.
    float HorizontalTreeLineLengthParentOffset = -15.6f;  // Used to create small gap for nodes that have children.
    float BackgroundColorYShift = -2.0f;  // Used to shift background color of the scene graph tree.
    float BackgroundHeightModifier = -4.0f;  // Modifier of the scene graph background height.
	bool bUseNodeBackground = true;  // Do we want to render the background of the scene graph tree?
	bool bIndintationAwareNodeBackground = false;  // Used to make sure the background of the scene graph tree is indented properly.

    void RenderNodeBackground();

	std::string SceneGraphNodeHoveredID = "";
public:
    SINGLETON_PUBLIC_PART(FEEditorSceneGraphWindow)
};

#define SCENE_GRAPH_WINDOW FEEditorSceneGraphWindow::GetInstance()