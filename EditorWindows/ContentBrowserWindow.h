#pragma once

#include "SceneGraphWindow.h"
//using namespace FocalEngine;

//namespace FocalEngine
//{
#ifdef FE_WIN_32
	const COMDLG_FILTERSPEC OBJ_LOAD_FILTER[] =
	{
		{ L"Wavefront OBJ files (*.obj)", L"*.obj" }
	};

	const COMDLG_FILTERSPEC TEXTURE_LOAD_FILTER[] =
	{
		{ L"Image files (*.png; *.jpg; *.bmp)", L"*.png;*.jpg;*.bmp" }
	};

	const COMDLG_FILTERSPEC ALL_IMPORT_LOAD_FILTER[] =
	{
		{ L"All files (*.png; *.jpg; *.bmp; *.obj)", L"*.png;*.jpg;*.bmp;*.obj" },
		{ L"Image files (*.png; *.jpg; *.bmp)", L"*.png;*.jpg;*.bmp" },
		{ L"Wavefront OBJ files (*.obj)", L"*.obj" }
	};
#endif

    class FEEditorContentBrowserWindow
    {
        friend class FEEditor;
        SINGLETON_PRIVATE_PART(FEEditorContentBrowserWindow)

        // Visibility and state
        bool bVisible = true;
        bool bShouldOpenContextMenu = false;

        // Callbacks and rendering
        static void MouseButtonCallback(int Button, int Action, int Mods);
        void Render();
        void RenderFilterMenu();
        void Clear();

        // Resource initialization and icons
        void InitializeResources();
        FETexture* FolderIcon = nullptr;
        FETexture* ShaderIcon = nullptr;
        FETexture* VFSBackIcon = nullptr;
        FETexture* TextureIcon = nullptr;
        FETexture* MeshIcon = nullptr;
        FETexture* MaterialIcon = nullptr;
        FETexture* GameModelIcon = nullptr;
        FETexture* PrefabIcon = nullptr;
        FETexture* AllIcon = nullptr;

        // Item selection and manipulation
        void ChooseTexturesItem(FETexture*& PreviewTexture, FETexture*& SmallAdditionTypeIcon, ImVec2& UV0, ImVec2& UV1, FEObject* Item);
        void OpenItemParentFolder(FEObject* Object);

        // Item properties and renaming
        float ItemIconSize = 128.0;
        int ItemUnderMouse = -1;
        int RenameIndex = -1;
        char RenameBuffer[1024];
        bool bLastFrameRenameEditWasVisiable = false;
        static FEObject* ItemInFocus;

        // Resource filtering
        std::vector<FEObject*> AllResources;
        std::vector<FEObject*> FilteredResources;
        char FilterForResources[512];
        std::string ObjTypeFilterForResources;
        void UpdateFilterForResources();

        // Filter buttons
        ImGuiImageButton* FilterAllTypesButton = nullptr;
        ImGuiImageButton* FilterTextureTypeButton = nullptr;
        ImGuiImageButton* FilterMeshTypeButton = nullptr;
        ImGuiImageButton* FilterMaterialTypeButton = nullptr;
        ImGuiImageButton* FilterGameModelTypeButton = nullptr;
        ImGuiImageButton* FilterPrefabTypeButton = nullptr;

        // Drag and drop
        struct DirectoryDragAndDropCallbackInfo
        {
            std::string DirectoryPath;
        };
        std::vector <DirectoryDragAndDropCallbackInfo> DirectoryDragAndDropInfo;
        std::vector<DragAndDropTarget*> DirectoriesTargets;
        DragAndDropTarget* VFSBackButtonTarget = nullptr;
        DirectoryDragAndDropCallbackInfo VFSBackButtoninfo;

        static bool DirectoryDragAndDropCallback(FEObject* Object, void** Directory)
        {
            const DirectoryDragAndDropCallbackInfo* info = reinterpret_cast<DirectoryDragAndDropCallbackInfo*>(Directory);
            if (Object->GetType() == FE_NULL)
            {
                std::string OldPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
                if (OldPath.back() != '/')
                    OldPath += "/";
                VIRTUAL_FILE_SYSTEM.MoveDirectory(OldPath + Object->GetName(), info->DirectoryPath);
            }
            else
            {
                VIRTUAL_FILE_SYSTEM.MoveFile(Object, VIRTUAL_FILE_SYSTEM.GetCurrentPath(), info->DirectoryPath);
            }
            return true;
        }

        void UpdateDirectoryDragAndDropTargets();

    public:
        SINGLETON_PUBLIC_PART(FEEditorContentBrowserWindow)
    };

#define CONTENT_BROWSER_WINDOW FEEditorContentBrowserWindow::getInstance()
//}