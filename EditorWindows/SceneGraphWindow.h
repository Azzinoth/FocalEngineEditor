#pragma once

#include "ProjectWasModifiedPopUp.h"

class FEEditorSceneGraphWindow
{
    friend class FEEditor;
    SINGLETON_PRIVATE_PART(FEEditorSceneGraphWindow)

    FEScene* CurrentScene = nullptr;

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
    char FilterForEntities[512];

    // Context menu
    bool bShouldOpenContextMenu = false;

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

        // FIX ME! Temporary solution, only supports one scene
        std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
        if (!ActiveScenes.empty())
        {
            FEScene* CurrentScene = SCENE_MANAGER.GetActiveScenes()[0];
            return CurrentScene->SceneGraph.MoveNode(SceneEntity->GetObjectID(), NodeTarget->GetObjectID());
        }

        return false;
    }

    void UpdateSceneNodeDragAndDropTargets();

    // Rendering and initialization
    void Render();
    void RenderNewSceneGraph();
    void RenderSubTree(FENaiveSceneGraphNode* SubTreeRoot);
    void Clear();
    void InitializeResources();

public:
    SINGLETON_PUBLIC_PART(FEEditorSceneGraphWindow)

    void SetScene(FEScene* Scene);
    FEScene* GetScene();
};

#define SCENE_GRAPH_WINDOW FEEditorSceneGraphWindow::getInstance()