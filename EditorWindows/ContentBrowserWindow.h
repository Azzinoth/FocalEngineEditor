#pragma once

#include "SceneGraphWindow.h"

	const std::vector<FEFileSystem::FileDialogFilter> OBJ_LOAD_FILTER =
	{
		{ "Wavefront OBJ files (*.obj)", { "obj" } }
	};

	const std::vector<FEFileSystem::FileDialogFilter> GLTF_FILTER =
	{
		{ "GL Transmission Format (*.gltf)", { "gltf" } }
	};

	const std::vector<FEFileSystem::FileDialogFilter> TEXTURE_LOAD_FILTER =
	{
		{ "Image files (*.png; *.jpg; *.bmp)", { "png", "jpg", "bmp" } }
	};

	const std::vector<FEFileSystem::FileDialogFilter> PLY_FILTER =
	{
		{ "Stanford PLY (*.ply)", { "ply" } }
	};

	const std::vector<FEFileSystem::FileDialogFilter> NATIVE_SCRIPT_MODULE_FILTER =
	{
		{ "Native script module (*.nativescriptmodule)", { "nativescriptmodule" } }
	};

	const std::vector<FEFileSystem::FileDialogFilter> ALL_IMPORT_LOAD_FILTER =
	{
		{ "All files (*.png; *.jpg; *.bmp; *.obj; *.gltf)", { "png", "jpg", "bmp", "obj", "gltf" } },
		{ "Image files (*.png; *.jpg; *.bmp)", { "png", "jpg", "bmp" } },
		{ "Wavefront OBJ files (*.obj)", { "obj" } },
		{ "GL Transmission Format (*.gltf)", { "gltf" } },
		{ "Stanford PLY (*.ply)", { "ply" } }
	};

    class FEEditorContentBrowserWindow
    {
        friend class FEEditor;
        friend class FEEditorInspectorWindow;

        SINGLETON_PRIVATE_PART(FEEditorContentBrowserWindow)

        // Visibility and state
        bool bVisible = true;
        bool bContextMenuOpened = false;

        // Callbacks and rendering
        static void MouseButtonCallback(int Button, int Action, int Mods);
        void Render();
        void RenderFilterMenu();
        void Clear();

        // Resource initialization and icons
        void InitializeResources();
        FETexture* FolderIcon = nullptr;
        FETexture* ShaderIcon = nullptr;
		FETexture* ShaderSmallIcon = nullptr;
        FETexture* VFSBackIcon = nullptr;
        FETexture* TextureIcon = nullptr;
        FETexture* Texture3DIcon = nullptr;
        FETexture* MeshIcon = nullptr;
        FETexture* MaterialIcon = nullptr;
        FETexture* GameModelIcon = nullptr;
        FETexture* PointCloudIcon = nullptr;
        FETexture* PrefabIcon = nullptr;
		FETexture* SceneIcon = nullptr;
		FETexture* AssetPackageIcon = nullptr;
        FETexture* AssetPackageSmallIcon = nullptr;
		FETexture* NativeScriptModuleIcon = nullptr;
		FETexture* NativeScriptModuleSmallIcon = nullptr;
        FETexture* AllIcon = nullptr;

        // Item selection and manipulation
        FETexture* GetItemPreview(FEObject* Item, ImVec2& UV0, ImVec2& UV1);
        FETexture* GetItemTypeIcon(FEObject* Item);
        void RenderItemCard(FEObject* Item, int Index, ImVec2 Min, int& DirectoryIndex);
        void OpenItem(FEObject* Item);
        void OpenItemParentFolder(FEObject* Object);
        void RenderEmptySpaceMenu();
        void RenderItemMenu(FEObject* Item);

        // Item properties and renaming
        const float ItemCardWidth = 140.0f;
        const float ItemCardHeight = 207.0f;
        const float ItemCardGap = 12.0f;
        int GetItemColumnCount(float Width) const;
        void StartRename(int Index);
    public:
        int ItemUnderMouse = -1;
        int RenameIndex = -1;
		std::string RenameBuffer = "";
        bool bLastFrameRenameEditWasVisible = false;
        static FEObject* ItemInFocus;

        // Resource filtering
        std::vector<FEUUID> AllResourcesIDs;
        std::vector<FEUUID> FilteredResourcesIDs;
		std::string NameFilter = "";
        std::vector<std::string> ObjectTypeFilters;
		std::vector<std::string> ObjectTagNegativeFilters;
        bool AnyFilterActive();
		bool ShouldPassVisibilityFilter(const FEUUID& ObjectID);
        void UpdateFilterForResources();
		std::string LastUsedNameFilter = "";
		std::vector<std::string> LastUsedObjectTypeFilters;
		std::vector<std::string> LastUsedObjectTagNegativeFilters;
		bool IsFilteredListOutdated();

        // Filter buttons

        // Drag and drop
        struct DirectoryDragAndDropCallbackInfo
        {
            std::string DirectoryPath;
        };
        std::vector <DirectoryDragAndDropCallbackInfo> DirectoryDragAndDropInfo;
        std::vector<DragAndDropTarget*> DirectoriesTargets;
        DragAndDropTarget* VFSBackButtonTarget = nullptr;
        DirectoryDragAndDropCallbackInfo VFSBackButtonInfo;

        static bool DirectoryDragAndDropCallback(FEObject* Object, void** Directory)
        {
            const DirectoryDragAndDropCallbackInfo* CallbackInfo = reinterpret_cast<DirectoryDragAndDropCallbackInfo*>(Directory);
            if (Object->GetType() == FE_NULL)
            {
                std::string OldPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
                if (OldPath.back() != '/')
                    OldPath += "/";
                VIRTUAL_FILE_SYSTEM.MoveDirectory(OldPath + Object->GetName(), CallbackInfo->DirectoryPath);
            }
            else
            {
                VIRTUAL_FILE_SYSTEM.MoveFile(Object, VIRTUAL_FILE_SYSTEM.GetCurrentPath(), CallbackInfo->DirectoryPath);
            }
            return true;
        }

        void UpdateDirectoryDragAndDropTargets();

    public:
        SINGLETON_PUBLIC_PART(FEEditorContentBrowserWindow)
    };

#define CONTENT_BROWSER_WINDOW FEEditorContentBrowserWindow::GetInstance()