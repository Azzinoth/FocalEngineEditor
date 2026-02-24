#pragma once

#include "FESceneGraphUI.h"

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

	FESceneGraphUI* SceneGraphUI = nullptr;
	std::string LastFrameRootNodeID = "";
    static void OnNodeClicked(FENaiveSceneGraphNode* Node, ImGuiMouseButton_ MouseButton);
    static bool IsSelected(FENaiveSceneGraphNode* Node);
    static std::string GetDisplayedName(FENaiveSceneGraphNode* Node);
	static void OnNodeHovered(FENaiveSceneGraphNode* Node);
	static void AfterNodeRender(FENaiveSceneGraphNode* Node);
    static void ContextMenuRenderingFunction(FENaiveSceneGraphNode* Node);

    FESceneGraphNodeWidget CameraComponentIndicator;
	FESceneGraphNodeWidget LightComponentIndicator;
	FESceneGraphNodeWidget GameModelComponentIndicator;
	FESceneGraphNodeWidget TerrainComponentIndicator;
	FESceneGraphNodeWidget InstancedEntityComponentIndicator;
    FESceneGraphNodeWidget PrefabSceneGraphIndicator;
	FESceneGraphNodeWidget SkyDomeComponentIndicator;
	FESceneGraphNodeWidget LineComponentIndicator;
    FESceneGraphNodeWidget VirtualUIComponentIndicator;
	FESceneGraphNodeWidget PointCloudComponentIndicator;
	FESceneGraphNodeWidget NativeScriptComponentIndicator;

	FESceneGraphNodeWidget VisibilityToggleWidget;
public:
    SINGLETON_PUBLIC_PART(FEEditorSceneGraphWindow)
};

#define SCENE_GRAPH_WINDOW FEEditorSceneGraphWindow::GetInstance()