#pragma once

#include "ProjectWasModifiedPopUp.h"

class FEEditorSceneGraphWindow
{
    friend class FEEditor;
    SINGLETON_PRIVATE_PART(FEEditorSceneGraphWindow)

    // Visibility
    bool bVisible = true;

    // Icon management
    void DrawCorrectIcon(FEEntity* SceneEntity) const;
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