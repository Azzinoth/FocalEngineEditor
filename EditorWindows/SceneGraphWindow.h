#pragma once

#include "ProjectWasModifiedPopUp.h"
#include "TreeView.h"
#include "FocalEngineBackend.h"

class FEEditorSceneGraphWindow
{
    friend class FEEditor;
    SINGLETON_PRIVATE_PART(FEEditorSceneGraphWindow)

    bool bVisible = true;

	FETexture* GameModelSceneGraphIcon = nullptr;
    FETexture* EntityIcon = nullptr;
    FETexture* InstancedEntityIcon = nullptr;
    FETexture* PrefabSceneGraphIcon = nullptr;
    FETexture* DirectionalLightIcon = nullptr;
    FETexture* SpotLightIcon = nullptr;
	FETexture* SkyDomeIcon = nullptr;
    FETexture* PointLightIcon = nullptr;
    FETexture* TerrainIcon = nullptr;
    FETexture* CameraIcon = nullptr;
	FETexture* LineIcon = nullptr;
	FETexture* VirtualUIIcon = nullptr;
	FETexture* PointCloudIcon = nullptr;
	FETexture* NativeScriptIcon = nullptr;
	FETexture* VolumeIcon = nullptr;

    FETexture* VisibilityOnIcon = nullptr;
    FETexture* VisibilityOffIcon = nullptr;

	bool bLastFrameWasInvisible = true;

    // Drag and drop
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

    // Rendering and initialization.
    void Render();
    void Clear();
    void InitializeResources();

    SceneGraphUI::TreeView* SceneGraphUI = nullptr;
	FESceneGraphBackend* SceneGraphBackend = nullptr;
	std::string LastFrameRootNodeID = "";
	std::string LastFrameSelectedEntityID = "";
    static void OnNodeClicked(SceneGraphUI::NodeHandle Node, ImGuiMouseButton_ MouseButton);
    static bool IsSelected(SceneGraphUI::NodeHandle Node);
    static std::string GetDisplayedName(SceneGraphUI::NodeHandle Node);
	static void OnNodeHovered(SceneGraphUI::NodeHandle Node);
	static void AfterNodeRender(SceneGraphUI::NodeHandle Node);
    static void ContextMenuRenderingFunction(SceneGraphUI::NodeHandle Node);
	static void RenameNodeFunction(SceneGraphUI::NodeHandle Node, std::string NewName);

    SceneGraphUI::NodeWidget CameraComponentIndicator;
    SceneGraphUI::NodeWidget LightComponentIndicator;
    SceneGraphUI::NodeWidget GameModelComponentIndicator;
    SceneGraphUI::NodeWidget TerrainComponentIndicator;
    SceneGraphUI::NodeWidget InstancedEntityComponentIndicator;
    SceneGraphUI::NodeWidget PrefabSceneGraphIndicator;
    SceneGraphUI::NodeWidget SkyDomeComponentIndicator;
    SceneGraphUI::NodeWidget LineComponentIndicator;
    SceneGraphUI::NodeWidget VirtualUIComponentIndicator;
    SceneGraphUI::NodeWidget PointCloudComponentIndicator;
    SceneGraphUI::NodeWidget NativeScriptComponentIndicator;
    SceneGraphUI::NodeWidget VolumeComponentIndicator;

    SceneGraphUI::NodeWidget VisibilityToggleWidget;
public:
    SINGLETON_PUBLIC_PART(FEEditorSceneGraphWindow)

    SceneGraphUI::TreeView* GetSceneGraphUI() const;
};

#define SCENE_GRAPH_WINDOW FEEditorSceneGraphWindow::GetInstance()