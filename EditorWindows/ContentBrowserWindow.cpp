#include "ContentBrowserWindow.h"
#include "TextureViewWindow.h"
#include "VolumetricTextureViewWindow.h"
#include "../FEEditor.h"
using namespace FocalEngine;

FEObject* FEEditorContentBrowserWindow::ItemInFocus = nullptr;

FEEditorContentBrowserWindow::FEEditorContentBrowserWindow()
{
	INPUT.AddMouseButtonCallback(&FEEditorContentBrowserWindow::MouseButtonCallback);
}

static FETexture* TemporaryTexture = nullptr;
static void AddTransparencyToTextureCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (TemporaryTexture == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_TEXTURE)
	{
		FETexture* OriginalTexture = TemporaryTexture;

		FETexture* NewTexture = RESOURCE_MANAGER.CreateTextureWithTransparency(OriginalTexture, reinterpret_cast<FETexture*>(SelectionsResult[0]));
		if (NewTexture == nullptr)
		{
			TemporaryTexture = nullptr;
			return;
		}

		unsigned char* NewRawData = NewTexture->GetRawData();
		const int MaxDimension = std::max(OriginalTexture->GetWidth(), OriginalTexture->GetHeight());
		const size_t MipCount = size_t(floor(log2(MaxDimension)) + 1);
		OriginalTexture->UpdateRawData(NewRawData, MipCount);
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
		PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(OriginalTexture);

		delete[] NewRawData;
		RESOURCE_MANAGER.DeleteFETexture(NewTexture);
	}

	TemporaryTexture = nullptr;
}

static void CreateNewPrefabCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (!SelectionsResult.empty() && SelectionsResult[0]->GetType() == FE_GAMEMODEL)
	{
		FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab();
		for (int i = 0; i < SelectionsResult.size(); i++)
		{
			if (SelectionsResult[i]->GetType() == FE_GAMEMODEL)
			{
				FEEntity* NewEntity = NewPrefab->GetScene()->CreateEntity(SelectionsResult[i]->GetName());
				NewEntity->AddComponent<FEGameModelComponent>(reinterpret_cast<FEGameModel*>(SelectionsResult[i]));
			}
		}

		PROJECT_MANAGER.GetCurrent()->SetModified(true);
		VIRTUAL_FILE_SYSTEM.CreateFile(NewPrefab, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	}

	SELECT_FEOBJECT_POPUP.SetOneObjectSelectonMode(true);
}

void FEEditorContentBrowserWindow::MouseButtonCallback(int Button, int Action, int Mods)
{
	ItemInFocus = nullptr;
}

void FEEditorContentBrowserWindow::OpenItemParentFolder(FEObject* Object)
{
	VIRTUAL_FILE_SYSTEM.SetCurrentPath(VIRTUAL_FILE_SYSTEM.LocateFile(Object));
	ImGui::SetWindowFocus("Content Browser");
	NameFilter = "";
	ItemInFocus = Object;

	const auto Content = VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	size_t TotalItemCount = Content.size();

	size_t ItemIndex = 0;
	for (size_t i = 0; i < Content.size(); i++)
	{
		FEObject* CurrentObject = OBJECT_MANAGER.GetFEObject(Content[i]);
		if (CurrentObject != nullptr && CurrentObject->GetObjectID() == Object->GetObjectID())
		{
			ItemIndex = i;
			break;
		}
	}

	ImGuiWindow* ContentBrowserWindow = ImGui::FindWindowByName("Content Browser");
	const int IconsPerWindowWidth = (int)(ContentBrowserWindow->Rect().GetWidth() / (ItemIconSize + 8 + 32));
	const size_t ItemRow = ItemIndex / IconsPerWindowWidth;

	if (ContentBrowserWindow != nullptr)
	{
		ContentBrowserWindow->Scroll.y = float(ItemRow * (ItemIconSize + 8 + 8 + 32));
	}
}

void FEEditorContentBrowserWindow::Render()
{
	if (!bVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None);

	RenderFilterMenu();

	bool bOpenContextMenu = false;
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		bOpenContextMenu = true;

	if (bOpenContextMenu)
		ImGui::OpenPopup("##Content_Browser_Context_Menu");

	bContextMenuOpened = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##Content_Browser_Context_Menu"))
	{
		bContextMenuOpened = true;

		if (ItemUnderMouse == -1)
		{
			if (ImGui::MenuItem("Import Asset..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, ALL_IMPORT_LOAD_FILTER, 5);
				if (!FilePath.empty())
				{
					if (EDITOR.GetFocusedScene() != nullptr)
					{
						const std::vector<FEObject*> LoadedObjects = EDITOR.GetFocusedScene()->ImportAsset(FilePath.c_str());
						for (size_t i = 0; i < LoadedObjects.size(); i++)
						{
							if (LoadedObjects[i] != nullptr)
							{
								VIRTUAL_FILE_SYSTEM.CreateFile(LoadedObjects[i], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
								PROJECT_MANAGER.GetCurrent()->SetModified(true);
								PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(LoadedObjects[i]);
							}
						}
					}
				}
			}

			if (ImGui::MenuItem("*DEPRECATED*Import Tree..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, OBJ_LOAD_FILTER, 1);
				if (!FilePath.empty())
				{
					const std::vector<FEObject*> LoadedObjects = RESOURCE_MANAGER.ImportOBJ(FilePath, true);
					for (size_t i = 0; i < LoadedObjects.size(); i++)
					{
						if (LoadedObjects[i] != nullptr)
						{
							VIRTUAL_FILE_SYSTEM.CreateFile(LoadedObjects[i], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
							PROJECT_MANAGER.GetCurrent()->SetModified(true);
							PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(LoadedObjects[i]);
						}
					}
				}
			}

			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("Add folder"))
				{
					const std::string NewDirectoryName = VIRTUAL_FILE_SYSTEM.CreateDirectory(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					UpdateDirectoryDragAndDropTargets();

					for (size_t i = 0; i < FilteredResourcesIDs.size(); i++)
					{
						FEObject* CurrentResource = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[i]);
						if (CurrentResource == nullptr)
							continue;

						if (CurrentResource->GetName() == NewDirectoryName)
						{
							RenameIndex = int(i);
							RenameBuffer = CurrentResource->GetName();
							bLastFrameRenameEditWasVisible = false;
							break;
						}
					}
				}

				if (ImGui::BeginMenu("Texture"))
				{
					if (ImGui::MenuItem("Combine channels..."))
						CombineChannelsToTexturePopUp::GetInstance().Show();

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Create new material"))
				{
					FEMaterial* NewMat = RESOURCE_MANAGER.CreateMaterial("");
					if (NewMat)
					{
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						NewMat->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

						NewMat->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);

						VIRTUAL_FILE_SYSTEM.CreateFile(NewMat, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					}
				}

				if (ImGui::MenuItem("Create new game model"))
				{
					FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel();
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					VIRTUAL_FILE_SYSTEM.CreateFile(NewGameModel, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				}

				if (ImGui::MenuItem("Create new prefab"))
				{
					SELECT_FEOBJECT_POPUP.SetOneObjectSelectonMode(false);
					SELECT_FEOBJECT_POPUP.Show(FE_GAMEMODEL, CreateNewPrefabCallback);
				}

				if (ImGui::MenuItem("Create new scene"))
				{
					FEScene* NewScene = SCENE_MANAGER.CreateScene("", "", FESceneFlag::EditorMode);
					//PROJECT_MANAGER.GetCurrent()->InjectEditorCamera(NewScene);
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					VIRTUAL_FILE_SYSTEM.CreateFile(NewScene, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				}

				/*if (ImGui::MenuItem("Create new asset package"))
				{
					FEAssetPackage* NewAssetPackage = RESOURCE_MANAGER.CreateAssetPackage();
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					VIRTUAL_FILE_SYSTEM.CreateFile(NewAssetPackage, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				}*/

				if (ImGui::MenuItem("Create new native script module"))
				{
					FENativeScriptModule* NewNativeScriptModule = RESOURCE_MANAGER.CreateNativeScriptModule();
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					VIRTUAL_FILE_SYSTEM.CreateFile(NewNativeScriptModule, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				}

				ImGui::EndMenu();
			}
		}
		else if (ItemUnderMouse >= 0 && ItemUnderMouse < (int)FilteredResourcesIDs.size() && OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[ItemUnderMouse]) != nullptr)
		{
			FEObject* ObjectUnderMouse = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[ItemUnderMouse]);

			std::string FullPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (FullPath.back() != '/')
				FullPath += '/';
			FullPath += ObjectUnderMouse->GetName();

			const bool ReadOnlyItem = VIRTUAL_FILE_SYSTEM.IsReadOnly(ObjectUnderMouse, FullPath);

			if (ReadOnlyItem)
				ImGui::MenuItem("Read Only");
			
			if (!ReadOnlyItem)
			{
				if (ImGui::MenuItem("Rename"))
				{
					RenameIndex = ItemUnderMouse;

					RenameBuffer = ObjectUnderMouse->GetName();
					bLastFrameRenameEditWasVisible = false;
				}
			}

			if (!ReadOnlyItem && ObjectUnderMouse->GetType() == FE_MATERIAL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					EDITOR_MATERIAL_WINDOW.Show(RESOURCE_MANAGER.GetMaterial(ObjectUnderMouse->GetObjectID()));
				}
			}

			if (!ReadOnlyItem && ObjectUnderMouse->GetType() == FE_GAMEMODEL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					EditGameModelPopup::GetInstance().Show(RESOURCE_MANAGER.GetGameModel(ObjectUnderMouse->GetObjectID()));
				}

				if (ImGui::MenuItem("Create Prefab out of this Game Model"))
				{
					FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(ObjectUnderMouse->GetObjectID());
					if (GameModel != nullptr)
					{
						FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab();
						FEEntity* NewEntity = NewPrefab->GetScene()->CreateEntity(GameModel->GetName());
						NewEntity->AddComponent<FEGameModelComponent>(GameModel);
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						VIRTUAL_FILE_SYSTEM.CreateFile(NewPrefab, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					}
				}
			}

			if (!ReadOnlyItem && ObjectUnderMouse->GetType() == FE_PREFAB)
			{
				if (ImGui::MenuItem("Edit"))
				{
					PREFAB_EDITOR_MANAGER.PrepareEditWinow(RESOURCE_MANAGER.GetPrefab(ObjectUnderMouse->GetObjectID()));
				}
			}

			if (ObjectUnderMouse->GetType() == FE_SHADER)
			{
				if (ImGui::MenuItem("Edit"))
				{
					ShaderEditorWindow::GetInstance().Show(RESOURCE_MANAGER.GetShader(ObjectUnderMouse->GetObjectID()));
				}
			}

			if (!ReadOnlyItem && ObjectUnderMouse->GetType() == FE_NATIVE_SCRIPT_MODULE)
			{
				FENativeScriptModule* NativeScriptModule = RESOURCE_MANAGER.GetNativeScriptModule(ObjectUnderMouse->GetObjectID());
				if (NativeScriptModule != nullptr)
				{
					bool bHaveVSProjectReady = false;
					if (NativeScriptModule->GetProject() != nullptr)
					{
						bHaveVSProjectReady = NativeScriptModule->GetProject()->IsVSProjectFolderValidAndIntact();
					}

					if (bHaveVSProjectReady)
					{
						if (ImGui::MenuItem("Open VS Project"))
						{
							if (!NativeScriptModule->GetProject()->RunVSProject())
								LOG.Add("FEEditorContentBrowserWindow::Render: Failed to run Visual Studio project!", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_ERROR);
						}
					}
					else
					{
						bool bHaveRecoverableVSProject = NativeScriptModule->GetProject()->HasRecoverableVSProjectData();
						if (bHaveRecoverableVSProject)
						{
							if (ImGui::MenuItem("Recover VS Project"))
							{
								std::string ProjectFolder = PROJECT_MANAGER.GetCurrent()->GetProjectFolder();
								NativeScriptModule->GetProject()->SetWorkingDirectory(ProjectFolder);
								if (!NativeScriptModule->GetProject()->RecreateVSProject())
								{
									LOG.Add("FEEditorContentBrowserWindow::Render: Failed to regenerate Visual Studio project!", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_ERROR);
								}
							}
						}
						else
						{
							if (ImGui::MenuItem("Create VS Project"))
							{
								std::string ProjectFolder = PROJECT_MANAGER.GetCurrent()->GetProjectFolder();
								NativeScriptModule->GetProject()->SetWorkingDirectory(ProjectFolder);
								if (!NativeScriptModule->GetProject()->CreateNewVSProject("FirstScript"))
								{
									LOG.Add("FEEditorContentBrowserWindow::Render: Failed to create Visual Studio project!", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_ERROR);
								}
							}
						}
					}
				}
				else
				{
					LOG.Add("FEEditorContentBrowserWindow::Render: Native script module is nullptr", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_WARNING);
				}
			}

			if (!ReadOnlyItem)
			{
				if (ImGui::MenuItem("Delete"))
				{
					if (ObjectUnderMouse->GetType() == FE_NULL)
					{
						DeleteDirectoryPopup::GetInstance().Show(ObjectUnderMouse->GetName());
					}
					else if (ObjectUnderMouse->GetType() == FE_MESH)
					{
						DeleteMeshPopup::GetInstance().Show(RESOURCE_MANAGER.GetMesh(ObjectUnderMouse->GetObjectID()));
					}
					else if (ObjectUnderMouse->GetType() == FE_POINT_CLOUD)
					{
						DeletePointCloudPopup::GetInstance().Show(RESOURCE_MANAGER.GetPointCloud(ObjectUnderMouse->GetObjectID()));
					}
					else if (ObjectUnderMouse->GetType() == FE_TEXTURE)
					{
						DeleteTexturePopup::GetInstance().Show(RESOURCE_MANAGER.GetTexture(ObjectUnderMouse->GetObjectID()));
					}
					else if (ObjectUnderMouse->GetType() == FE_MATERIAL)
					{
						DeleteMaterialPopup::GetInstance().Show(RESOURCE_MANAGER.GetMaterial(ObjectUnderMouse->GetObjectID()));
					}
					else if (ObjectUnderMouse->GetType() == FE_GAMEMODEL)
					{
						DeleteGameModelPopup::GetInstance().Show(RESOURCE_MANAGER.GetGameModel(ObjectUnderMouse->GetObjectID()));
					}
					else if (ObjectUnderMouse->GetType() == FE_PREFAB)
					{
						DeletePrefabPopup::GetInstance().Show(RESOURCE_MANAGER.GetPrefab(ObjectUnderMouse->GetObjectID()));
					}
					else if (ObjectUnderMouse->GetType() == FE_SCENE)
					{
						//DeleteScenePopup::GetInstance().Show(FilteredResources[ItemUnderMouse]->GetObjectID());
					}
					else if (ObjectUnderMouse->GetType() == FE_SHADER)
					{
						//
					}
					else if (ObjectUnderMouse->GetType() == FE_NATIVE_SCRIPT_MODULE)
					{
						//
					}
				}
			}

			if (ObjectUnderMouse->GetType() == FE_MESH)
			{
				if (ImGui::BeginMenu("Export"))
				{
					FEMesh* MeshToExport = RESOURCE_MANAGER.GetMesh(ObjectUnderMouse->GetObjectID());

					if (ImGui::MenuItem("as OBJ"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, OBJ_LOAD_FILTER, 1);

						if (!FilePath.empty())
						{
							if (FilePath.find(".obj") == std::string::npos)
								FilePath += ".obj";

							RESOURCE_MANAGER.ExportFEMeshToOBJ(MeshToExport, FilePath);
						}
					}

					if (ImGui::MenuItem("as PLY"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, PLY_FILTER, 1);

						if (!FilePath.empty())
						{
							if (FilePath.find(".ply") == std::string::npos)
								FilePath += ".ply";

							RESOURCE_MANAGER.ExportFEMeshToPLY(MeshToExport, FilePath);
						}
					}

					ImGui::EndMenu();
				}
			}

			if (ObjectUnderMouse->GetType() == FE_POINT_CLOUD)
			{
				FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(ObjectUnderMouse->GetObjectID());

				if (ImGui::BeginMenu("Export"))
				{
					if (ImGui::MenuItem("as PLY"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, PLY_FILTER, 1);

						if (!FilePath.empty())
						{
							if (FilePath.find(".ply") == std::string::npos)
								FilePath += ".ply";

							RESOURCE_MANAGER.ExportFEPointCloudToPLY(PointCloud, FilePath);
						}
					}

					if (ImGui::MenuItem("as LAS"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, LAS_FILTER, 1);
						if (!FilePath.empty())
						{
							if (FilePath.find(".las") == std::string::npos)
								FilePath += ".las";

							RESOURCE_MANAGER.ExportFEPointCloudToLAS(PointCloud, FilePath);
						}
					}

					if (ImGui::MenuItem("as LAZ"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, LAZ_FILTER, 1);
						if (!FilePath.empty())
						{
							if (FilePath.find(".laz") == std::string::npos)
								FilePath += ".laz";

							RESOURCE_MANAGER.ExportFEPointCloudToLAZ(PointCloud, FilePath);
						}
					}

					ImGui::EndMenu();
				}

				bool bUseAdvancedRendering = PointCloud->IsAdvancedRenderingEnabled();
				if (ImGui::MenuItem("Advanced rendering", NULL, &bUseAdvancedRendering))
				{
					PointCloud->SetAdvancedRenderingEnabled(bUseAdvancedRendering);
				}
			}

			if (ObjectUnderMouse->GetType() == FE_TEXTURE)
			{
				if (ImGui::BeginMenu("Convert"))
				{
					if (ImGui::MenuItem("Texture channels to individual textures"))
					{
						const std::vector<FETexture*> NewTextures = RESOURCE_MANAGER.ChannelsToFETextures(RESOURCE_MANAGER.GetTexture(ObjectUnderMouse->GetObjectID()));

						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[0]);
						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[1]);
						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[2]);
						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[3]);

						PROJECT_MANAGER.GetCurrent()->SetModified(true);

						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[0], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[1], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[2], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[3], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					}

					if (ImGui::MenuItem("Resize"))
					{
						FETexture* TextureToResize = RESOURCE_MANAGER.GetTexture(ObjectUnderMouse->GetObjectID());
						ResizeTexturePopup::GetInstance().Show(TextureToResize);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Add transparency"))
				{
					if (ImGui::MenuItem("Choose transparency mask"))
					{
						TemporaryTexture = reinterpret_cast<FETexture*>(ObjectUnderMouse);
						SELECT_FEOBJECT_POPUP.Show(FE_TEXTURE, AddTransparencyToTextureCallback);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Export"))
				{
					FETexture* TextureToExport = RESOURCE_MANAGER.GetTexture(ObjectUnderMouse->GetObjectID());

					if (ImGui::MenuItem("as PNG"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, TEXTURE_LOAD_FILTER, 1);

						if (!FilePath.empty())
						{
							FilePath += ".png";
							RESOURCE_MANAGER.ExportFETextureToPNG(TextureToExport, FilePath);
						}
					}

					ImGui::EndMenu();
				}
			}

			if (ObjectUnderMouse->GetType() == FE_SCENE)
			{
				std::string SceneID = ObjectUnderMouse->GetObjectID();
				bool bIsAlreadyOpened = EDITOR.GetEditorSceneWindow(SceneID) != nullptr;
				
				if (!bIsAlreadyOpened)
				{
					if (ImGui::MenuItem("Open"))
					{
						EDITOR.CreateEditorWindowForScene(SceneID);
					}
				}
				else
				{
					ImGui::BeginDisabled();
					ImGui::MenuItem("Already opened");
					ImGui::EndDisabled();
				}

				if (ImGui::MenuItem("Duplicate"))
				{
					FEScene* NewScene = SCENE_MANAGER.DuplicateScene(SceneID, "", [](FEEntity* EntityToCheck) {
						return !(EntityToCheck->GetTag() == EDITOR_RESOURCE_TAG);
					});

					if (NewScene != nullptr)
					{
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						VIRTUAL_FILE_SYSTEM.CreateFile(NewScene, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					}
				}

				FEScene* StartingScene = SCENE_MANAGER.GetStartingScene();
				bool bIsSceneStarting = false;
				if (StartingScene != nullptr)
					bIsSceneStarting = StartingScene->GetObjectID() == SceneID;
				
				if (ImGui::MenuItem("Starting scene", NULL, &bIsSceneStarting))
				{
					if (bIsSceneStarting)
						SCENE_MANAGER.SetStartingScene(SceneID);
				}
			}
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditorContentBrowserWindow::Clear()
{
	AllResourcesIDs.clear();
	FilteredResourcesIDs.clear();
	ItemUnderMouse = -1;
	RenameIndex = -1;
	NameFilter = "";
}

void FEEditorContentBrowserWindow::InitializeResources()
{
	FolderIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/folderIcon.png", "folderIcon");
	RESOURCE_MANAGER.SetTag(FolderIcon, EDITOR_RESOURCE_TAG);

	ShaderIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/shaderIcon.png", "shaderIcon");
	RESOURCE_MANAGER.SetTag(ShaderIcon, EDITOR_RESOURCE_TAG);

	ShaderSmallIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/Shader_ContentBrowser_32_32_Icon.png", "Shader_Small_ContentBrowser_Icon");
	RESOURCE_MANAGER.SetTag(ShaderSmallIcon, EDITOR_RESOURCE_TAG);

	VFSBackIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/VFSBackIcon.png", "VFSBackIcon");
	RESOURCE_MANAGER.SetTag(VFSBackIcon, EDITOR_RESOURCE_TAG);

	TextureIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/textureContentBrowserIcon.png", "textureContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(TextureIcon, EDITOR_RESOURCE_TAG);

	Texture3DIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/Texture3DContentBrowserIcon.png", "texture3DContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(Texture3DIcon, EDITOR_RESOURCE_TAG);

	MeshIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/meshContentBrowserIcon.png", "meshContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(MeshIcon, EDITOR_RESOURCE_TAG);

	MaterialIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/materialContentBrowserIcon.png", "materialContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(MaterialIcon, EDITOR_RESOURCE_TAG);

	GameModelIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/gameModelContentBrowserIcon.png", "gameModelContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(GameModelIcon, EDITOR_RESOURCE_TAG);

	PointCloudIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/PointCloudContentBrowserIcon.png", "pointCloudContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(PointCloudIcon, EDITOR_RESOURCE_TAG);

	PrefabIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/prefabContentBrowserIcon.png", "prefabContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(PrefabIcon, EDITOR_RESOURCE_TAG);

	SceneIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/Scene_ContentBrowser_Icon.png", "Scene_ContentBrowser_Icon");
	RESOURCE_MANAGER.SetTag(SceneIcon, EDITOR_RESOURCE_TAG);

	AssetPackageIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/AssetPackage_ContentBrowser_Icon.png", "AssetPackage_ContentBrowser_Icon");
	RESOURCE_MANAGER.SetTag(AssetPackageIcon, EDITOR_RESOURCE_TAG);

	AssetPackageSmallIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/AssetPackage_ContentBrowser_32_32_Icon.png", "AssetPackage_Small_ContentBrowser_Icon");
	RESOURCE_MANAGER.SetTag(AssetPackageSmallIcon, EDITOR_RESOURCE_TAG);

	NativeScriptModuleIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/NativeScriptModule_ContentBrowser_Icon.png", "NativeScriptModule_ContentBrowser_Icon");
	RESOURCE_MANAGER.SetTag(NativeScriptModuleIcon, EDITOR_RESOURCE_TAG);

	NativeScriptModuleSmallIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/NativeScriptModule_ContentBrowser_32_32_Icon.png", "NativeScriptModule_Small_ContentBrowser_Icon");
	RESOURCE_MANAGER.SetTag(NativeScriptModuleSmallIcon, EDITOR_RESOURCE_TAG);

	AllIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/allContentBrowserIcon.png", "allIcon");
	FilterAllTypesButton = new ImGuiImageButton(AllIcon);
	RESOURCE_MANAGER.SetTag(AllIcon, EDITOR_RESOURCE_TAG);
	FilterAllTypesButton->SetSize(ImVec2(32, 32));

	FilterShaderTypeButton = new ImGuiImageButton(ShaderSmallIcon);
	FilterShaderTypeButton->SetSize(ImVec2(32, 32));

	FilterTextureTypeButton = new ImGuiImageButton(TextureIcon);
	FilterTextureTypeButton->SetSize(ImVec2(32, 32));

	FilterMeshTypeButton = new ImGuiImageButton(MeshIcon);
	FilterMeshTypeButton->SetSize(ImVec2(32, 32));

	FilterMaterialTypeButton = new ImGuiImageButton(MaterialIcon);
	FilterMaterialTypeButton->SetSize(ImVec2(32, 32));

	FilterGameModelTypeButton = new ImGuiImageButton(GameModelIcon);
	FilterGameModelTypeButton->SetSize(ImVec2(32, 32));

	FilterPointCloudTypeButton = new ImGuiImageButton(PointCloudIcon);
	FilterPointCloudTypeButton->SetSize(ImVec2(32, 32));

	FilterPrefabTypeButton = new ImGuiImageButton(PrefabIcon);
	FilterPrefabTypeButton->SetSize(ImVec2(32, 32));

	FilterSceneTypeButton = new ImGuiImageButton(SceneIcon);
	FilterSceneTypeButton->SetSize(ImVec2(32, 32));

	FilterAssetPackageTypeButton = new ImGuiImageButton(AssetPackageSmallIcon);
	FilterAssetPackageTypeButton->SetSize(ImVec2(32, 32));

	FilterNativeScriptModuleTypeButton = new ImGuiImageButton(NativeScriptModuleSmallIcon);
	FilterNativeScriptModuleTypeButton->SetSize(ImVec2(32, 32));
}

void FEEditorContentBrowserWindow::ChooseTexturesItem(FETexture*& PreviewTexture, FETexture*& SmallAdditionTypeIcon, ImVec2& UV0, ImVec2& UV1, FEObject* Item)
{
	if (Item->GetType() == FE_NULL)
	{
		UV0 = ImVec2(0.0f, 0.0f);
		UV1 = ImVec2(1.0f, 1.0f);

		PreviewTexture = FolderIcon;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
	}
	else if (Item->GetType() == FE_SHADER)
	{
		UV0 = ImVec2(0.0f, 0.0f);
		UV1 = ImVec2(1.0f, 1.0f);

		PreviewTexture = ShaderIcon;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
	}
	else if (Item->GetType() == FE_MESH)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = MeshIcon;
	}
	else if (Item->GetType() == FE_TEXTURE)
	{
		PreviewTexture = RESOURCE_MANAGER.GetTexture(Item->GetObjectID());
		if (PreviewTexture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		{
			PreviewTexture = RESOURCE_MANAGER.NoTexture;
			SmallAdditionTypeIcon = Texture3DIcon;
		}
		else
		{
			SmallAdditionTypeIcon = TextureIcon;
		}
	}
	else if (Item->GetType() == FE_MATERIAL)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = MaterialIcon;
	}
	else if (Item->GetType() == FE_GAMEMODEL)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = GameModelIcon;
	}
	else if (Item->GetType() == FE_POINT_CLOUD)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);
		PreviewTexture = PREVIEW_MANAGER.GetPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = PointCloudIcon;
	}
	else if (Item->GetType() == FE_PREFAB)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = PrefabIcon;
	}
	else if (Item->GetType() == FE_SCENE)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = SceneIcon;
	}
	else if (Item->GetType() == FE_ASSET_PACKAGE)
	{
		UV0 = ImVec2(0.0f, 0.0f);
		UV1 = ImVec2(1.0f, 1.0f);

		PreviewTexture = AssetPackageIcon;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
	}
	else if (Item->GetType() == FE_NATIVE_SCRIPT_MODULE)
	{
		UV0 = ImVec2(0.0f, 0.0f);
		UV1 = ImVec2(1.0f, 1.0f);

		PreviewTexture = NativeScriptModuleIcon;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
	}
}

void FEEditorContentBrowserWindow::RenderFilterMenu()
{
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	float CurrentY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(CurrentY);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 8.0f));
	if (ImGui::ImageButton("GoUpVFSButton", VFSBackIcon->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		VIRTUAL_FILE_SYSTEM.SetCurrentPath(VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));
		NameFilter = "";
	}
	ImGui::PopStyleVar();
	if (!VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(VFSBackButtonInfo.DirectoryPath))
		VFSBackButtonTarget->StickToItem();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	static std::string LastFramePath;
	AllResourcesIDs.clear();
	AllResourcesIDs = VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(VIRTUAL_FILE_SYSTEM.GetCurrentPath());

	float CurrentX = 100.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY + 5);
	ImGui::Text("Filter by type: ");

	CurrentX += 20.0f + 140.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjectTypeFilters.empty() ? SetSelectedStyle(FilterAllTypesButton) : SetDefaultStyle(FilterAllTypesButton);
	FilterAllTypesButton->Render();
	if (FilterAllTypesButton->IsClicked())
		ObjectTypeFilters.clear();

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bool bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_SHADER))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterShaderTypeButton) : SetDefaultStyle(FilterShaderTypeButton);
	FilterShaderTypeButton->Render();
	if (FilterShaderTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_SHADER)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_SHADER));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_TEXTURE))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterTextureTypeButton) : SetDefaultStyle(FilterTextureTypeButton);
	FilterTextureTypeButton->Render();
	if (FilterTextureTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_TEXTURE)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_TEXTURE));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_MESH))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterMeshTypeButton) : SetDefaultStyle(FilterMeshTypeButton);
	FilterMeshTypeButton->Render();
	if (FilterMeshTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_MESH)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_MESH));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_MATERIAL))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterMaterialTypeButton) : SetDefaultStyle(FilterMaterialTypeButton);
	FilterMaterialTypeButton->Render();
	if (FilterMaterialTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_MATERIAL)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_MATERIAL));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_GAMEMODEL))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterGameModelTypeButton) : SetDefaultStyle(FilterGameModelTypeButton);
	FilterGameModelTypeButton->Render();
	if (FilterGameModelTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_GAMEMODEL)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_GAMEMODEL));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_POINT_CLOUD))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterPointCloudTypeButton) : SetDefaultStyle(FilterPointCloudTypeButton);
	FilterPointCloudTypeButton->Render();
	if (FilterPointCloudTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_POINT_CLOUD)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_POINT_CLOUD));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_PREFAB))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterPrefabTypeButton) : SetDefaultStyle(FilterPrefabTypeButton);
	FilterPrefabTypeButton->Render();
	if (FilterPrefabTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_PREFAB)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_PREFAB));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_SCENE))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterSceneTypeButton) : SetDefaultStyle(FilterSceneTypeButton);
	FilterSceneTypeButton->Render();
	if (FilterSceneTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_SCENE)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_SCENE));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_ASSET_PACKAGE))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterAssetPackageTypeButton) : SetDefaultStyle(FilterAssetPackageTypeButton);
	FilterAssetPackageTypeButton->Render();
	if (FilterAssetPackageTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_ASSET_PACKAGE)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_ASSET_PACKAGE));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	bIsCurrentTypeInFilters = false;
	for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
	{
		if (ObjectTypeFilters[i] == FEObjectTypeToString(FE_NATIVE_SCRIPT_MODULE))
		{
			bIsCurrentTypeInFilters = true;
			break;
		}
	}
	bIsCurrentTypeInFilters ? SetSelectedStyle(FilterNativeScriptModuleTypeButton) : SetDefaultStyle(FilterNativeScriptModuleTypeButton);
	FilterNativeScriptModuleTypeButton->Render();
	if (FilterNativeScriptModuleTypeButton->IsClicked())
	{
		if (bIsCurrentTypeInFilters)
		{
			ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), FEObjectTypeToString(FE_NATIVE_SCRIPT_MODULE)), ObjectTypeFilters.end());
		}
		else
		{
			ObjectTypeFilters.push_back(FEObjectTypeToString(FE_NATIVE_SCRIPT_MODULE));
		}
	}

	CurrentX += 48.0f;
	ImGui::SetCursorPosX(CurrentX);
	ImGui::SetCursorPosY(CurrentY - 5);
	static bool bIsPrivateTagsInFilters = true;
	if (bIsPrivateTagsInFilters && ObjectTagNegativeFilters.empty())
	{
		ObjectTagNegativeFilters.push_back(ENGINE_RESOURCE_TAG);
		ObjectTagNegativeFilters.push_back(EDITOR_RESOURCE_TAG);
		ObjectTagNegativeFilters.push_back(PREFAB_SCENE_DESCRIPTION_TAG);
		ObjectTagNegativeFilters.push_back(TERRAIN_SYSTEM_RESOURCE_TAG);
	}

	if (ImGui::Checkbox("Hide private tags", &bIsPrivateTagsInFilters))
	{
		ObjectTagNegativeFilters.clear();
		if (bIsPrivateTagsInFilters)
		{
			ObjectTagNegativeFilters.push_back(ENGINE_RESOURCE_TAG);
			ObjectTagNegativeFilters.push_back(EDITOR_RESOURCE_TAG);
			ObjectTagNegativeFilters.push_back(PREFAB_SCENE_DESCRIPTION_TAG);
			ObjectTagNegativeFilters.push_back(TERRAIN_SYSTEM_RESOURCE_TAG);
		}
	}

	ImGui::SetCursorPosX(100);
	ImGui::SetCursorPosY(CurrentY + 50);
	ImGui::Text("Filter by name: ");

	ImGui::SetCursorPosX(120 + 140);
	ImGui::SetCursorPosY(CurrentY + 47);
	ImGui::InputText("##ContentBrowserNameFilter", &NameFilter);

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);
	ImGui::Separator();

	UpdateFilterForResources();
	UpdateDirectoryDragAndDropTargets();

	// ************** Drag&Drop **************
	if (VIRTUAL_FILE_SYSTEM.GetCurrentPath() != LastFramePath)
	{
		LastFramePath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
		UpdateDirectoryDragAndDropTargets();
	}
	int DirectoryIndex = 0;
	// ************** Drag&Drop END **************

	int IconsPerWindowWidth = (int)(FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl()->Rect().GetWidth() / (ItemIconSize + 8 + 32));
	// Possibly window is minimized anyway ImGui::Columns can't take 0 as columns count!
	if (IconsPerWindowWidth == 0)
		return;

	if (!bContextMenuOpened) ItemUnderMouse = -1;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	ImGui::Columns(IconsPerWindowWidth, "mycolumns3", false);

	for (size_t i = 0; i < FilteredResourcesIDs.size(); i++)
	{
		FEObject* CurrentResource = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[i]);
		if (CurrentResource == nullptr)
			continue;

		ImGui::PushID(int(std::hash<std::string>{}(CurrentResource->GetObjectID())));

		if (ItemInFocus != nullptr && ItemInFocus->GetObjectID() == CurrentResource->GetObjectID())
		{
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
		}
		
 		ImVec2 UV0 = ImVec2(0.0f, 0.0f);
		ImVec2 UV1 = ImVec2(1.0f, 1.0f);

		FETexture* PreviewTexture = nullptr;
		FETexture* SmallAdditionTypeIcon = nullptr;

		ChooseTexturesItem(PreviewTexture, SmallAdditionTypeIcon, UV0, UV1, CurrentResource);

		if (PreviewTexture != nullptr)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 8.0f));
			ImGui::ImageButton((CurrentResource->GetObjectID() + "FilteredContentBrowserButton").c_str(), PreviewTexture->GetTextureID(), ImVec2(ItemIconSize, ItemIconSize), UV0, UV1, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PopStyleVar();
		}
		
		if (CurrentResource->GetType() == FE_NULL && DirectoriesTargets.size() > (size_t)DirectoryIndex)
		{
			if (DirectoriesTargets[DirectoryIndex] != nullptr)
				DirectoriesTargets[DirectoryIndex]->StickToItem();
			DirectoryIndex++;
		}

		if (ImGui::IsItemHovered())
		{
			if (!bContextMenuOpened && !DRAG_AND_DROP_MANAGER.ObjectIsDraged())
			{
				std::string AdditionalTypeInfo;
				if (CurrentResource->GetType() == FE_TEXTURE)
				{
					FETexture* CurrentTexture = RESOURCE_MANAGER.GetTexture(CurrentResource->GetObjectID());
					AdditionalTypeInfo += "\nTexture type: ";
					switch (CurrentTexture->GetType())
					{
						case FE_TEXTURE_TYPE::FE_TEXTURE_1D:   AdditionalTypeInfo += "1D"; break;
						case FE_TEXTURE_TYPE::FE_TEXTURE_2D:   AdditionalTypeInfo += "2D"; break;
						case FE_TEXTURE_TYPE::FE_TEXTURE_3D:   AdditionalTypeInfo += "3D"; break;
						case FE_TEXTURE_TYPE::FE_TEXTURE_CUBE: AdditionalTypeInfo += "Cube"; break;
						default: break;
					}
					std::string InternalFormatString = FETexture::TextureInternalFormatToString(CurrentTexture->GetInternalFormat());
					if (!InternalFormatString.empty())
						AdditionalTypeInfo += ", " + InternalFormatString;
				}
				else if (CurrentResource->GetType() == FE_NATIVE_SCRIPT_MODULE)
				{
					
				}
				else if (CurrentResource->GetType() == FE_POINT_CLOUD)
				{
					FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(CurrentResource->GetObjectID());
					AdditionalTypeInfo += "\nPoint count: ";
					AdditionalTypeInfo += std::to_string(PointCloud->GetPointCount());
				}

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("ID: " + CurrentResource->GetObjectID() +
										"\nTag: " + CurrentResource->GetTag() +
										"\nName: " + CurrentResource->GetName() +
										"\nType: " + FEObjectTypeToString(CurrentResource->GetType()) +
										AdditionalTypeInfo +
										"\nPath: " + VIRTUAL_FILE_SYSTEM.GetCurrentPath()
										).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();

				ItemUnderMouse = int(i);

				if (ImGui::IsMouseDragging(0))
					DRAG_AND_DROP_MANAGER.SetObjectToDrag(CurrentResource, PreviewTexture, UV0, UV1);
			}
		}

		if (SmallAdditionTypeIcon != nullptr)
		{
			ImVec2 CursorPositionBefore = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(CursorPositionBefore.x + 10, CursorPositionBefore.y - 48));
			ImGui::Image(SmallAdditionTypeIcon->GetTextureID(), ImVec2(32, 32));
			ImGui::SetCursorPos(CursorPositionBefore);
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopID();
		
		if (RenameIndex == i)
		{
			FEObject* ObjectToRename = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[RenameIndex]);
			if (ObjectToRename != nullptr)
			{
				if (!bLastFrameRenameEditWasVisible)
				{
					ImGui::SetKeyboardFocusHere(0);
					ImGui::SetFocusID(ImGui::GetID("##NewNameEditor"), FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl());
					ImGui::SetItemDefaultFocus();
					bLastFrameRenameEditWasVisible = true;
				}

				ImGui::SetNextItemWidth(ItemIconSize + 8.0f + 8.0f);
				if (ImGui::InputText("##NewNameEditor", &RenameBuffer, ImGuiInputTextFlags_EnterReturnsTrue) ||
					ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || !ImGui::IsItemFocused())
				{
					if (ObjectToRename->GetType() == FE_NULL)
					{
						std::string PathToDirectory = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
						if (PathToDirectory.back() != '/')
							PathToDirectory += '/';

						PathToDirectory += ObjectToRename->GetName();
						VIRTUAL_FILE_SYSTEM.RenameDirectory(RenameBuffer, PathToDirectory);

						UpdateDirectoryDragAndDropTargets();
					}
					else
					{
						ObjectToRename->SetDirtyFlag(true);
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						ObjectToRename->SetName(RenameBuffer);
					}

					RenameIndex = -1;
				}
			}
		}
		else
		{
			ImVec2 TextSize = ImGui::CalcTextSize(CurrentResource->GetName().c_str());
			if (TextSize.x < ItemIconSize + 8 + 8)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ItemIconSize + 8.0f + 8.0f) / 2.0f - TextSize.x / 2.0f);
				ImGui::Text(CurrentResource->GetName().c_str());
			}
			else
			{
				ImGui::Text(CurrentResource->GetName().c_str());
			}
		}
		
		ImGui::NextColumn();
	}

	if (ImGui::IsMouseDoubleClicked(0) && ItemUnderMouse != -1 && ItemUnderMouse < (int)FilteredResourcesIDs.size() && OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[ItemUnderMouse]) != nullptr)
	{
		FEObject* ClickedItem = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[ItemUnderMouse]);

		if (ClickedItem->GetType() == FE_NULL)
		{
			std::string CurrentPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (CurrentPath.back() != '/')
				CurrentPath += '/';

			CurrentPath += ClickedItem->GetName();
			VIRTUAL_FILE_SYSTEM.SetCurrentPath(CurrentPath);
		}
		else if (ClickedItem->GetType() == FE_MESH)
		{
			std::string MeshInfo = "Vertex count: ";
			MeshInfo += std::to_string(RESOURCE_MANAGER.GetMesh(ClickedItem->GetObjectID())->GetVertexCount());
			MeshInfo += "\n";
			MeshInfo += "Sub material socket: ";
			MeshInfo += RESOURCE_MANAGER.GetMesh(ClickedItem->GetObjectID())->GetMaterialCount() == 2 ? "Yes" : "No";
			MessagePopUp::GetInstance().Show("Mesh info", MeshInfo.c_str());
		}
		else if (ClickedItem->GetType() == FE_MATERIAL)
		{
			EDITOR_MATERIAL_WINDOW.Show(RESOURCE_MANAGER.GetMaterial(ClickedItem->GetObjectID()));
		}
		else if (ClickedItem->GetType() == FE_GAMEMODEL)
		{
			if (!bContextMenuOpened && !EditGameModelPopup::GetInstance().IsVisible())
			{
				EditGameModelPopup::GetInstance().Show(RESOURCE_MANAGER.GetGameModel(ClickedItem->GetObjectID()));
			}
		}
		else if (ClickedItem->GetType() == FE_PREFAB)
		{
			if (!bContextMenuOpened)
			{
				PREFAB_EDITOR_MANAGER.PrepareEditWinow(RESOURCE_MANAGER.GetPrefab(ClickedItem->GetObjectID()));
			}
		}
		else if (ClickedItem->GetType() == FE_SCENE)
		{
			if (!bContextMenuOpened)
			{
				std::string SceneID = ClickedItem->GetObjectID();
				bool bIsAlreadyOpened = EDITOR.GetEditorSceneWindow(SceneID) != nullptr;
				if (!bIsAlreadyOpened)
					EDITOR.CreateEditorWindowForScene(SceneID);
			}
		}
		else if (ClickedItem->GetType() == FE_TEXTURE)
		{
			if (!bContextMenuOpened)
			{
				FETexture* ClickedTexture = RESOURCE_MANAGER.GetTexture(ClickedItem->GetObjectID());
				if (ClickedTexture != nullptr)
				{
					const std::string WindowCaption = "Texture: " + ClickedTexture->GetName();
					FEImGuiWindow* ExistingWindow = FE_IMGUI_WINDOW_MANAGER.GetWindowByCaption(WindowCaption);
					if (ExistingWindow != nullptr)
					{
						ExistingWindow->Show();
					}
					else
					{
						if (ClickedTexture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
						{
							VolumetricTextureViewWindow* NewWindow = new VolumetricTextureViewWindow(ClickedTexture);
							NewWindow->SetCaption(WindowCaption);
							NewWindow->Show();
							EDITOR.RegisterEditorSceneWindow(NewWindow);
						}
						else
						{
							TextureViewWindow* NewWindow = new TextureViewWindow(ClickedTexture);
							NewWindow->SetCaption(WindowCaption);
							NewWindow->Show();
						}
						
						
					}
				}
			}
		}
		else if (ClickedItem->GetType() == FE_ASSET_PACKAGE)
		{
			//
		}
		else if (ClickedItem->GetType() == FE_NATIVE_SCRIPT_MODULE)
		{
			//
		}
	}

	ImGui::Columns(1);
}

void FEEditorContentBrowserWindow::UpdateDirectoryDragAndDropTargets()
{
	for (size_t i = 0; i < DirectoriesTargets.size(); i++)
	{
		delete DirectoriesTargets[i];
	}
	DirectoriesTargets.clear();
	DirectoryDragAndDropInfo.clear();
	AllResourcesIDs.clear();
	AllResourcesIDs = VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	
	UpdateFilterForResources();

	DirectoryDragAndDropInfo.resize(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));
	int SubDirectoryIndex = 0;
	for (size_t i = 0; i < FilteredResourcesIDs.size(); i++)
	{
		FEObject* CurrentResource = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[i]);
		if (CurrentResource == nullptr)
			continue;

		if (CurrentResource->GetType() == FE_NULL)
		{
			DirectoryDragAndDropCallbackInfo info;

			info.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (VIRTUAL_FILE_SYSTEM.GetCurrentPath().back() != '/')
				info.DirectoryPath += "/";

			info.DirectoryPath += CurrentResource->GetName() + "/";
			DirectoryDragAndDropInfo[SubDirectoryIndex] = info;

			if (VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(info.DirectoryPath))
			{
				DirectoriesTargets.push_back(nullptr);
			}
			else
			{
				DirectoriesTargets.push_back(DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL, FE_PREFAB },
					DirectoryDragAndDropCallback, reinterpret_cast<void**>(&DirectoryDragAndDropInfo[SubDirectoryIndex]),
					std::vector<std::string> { "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder" }));
			}
			SubDirectoryIndex++;
		}
	}

	if (VFSBackButtonTarget == nullptr)
	{
		VFSBackButtonInfo.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		VFSBackButtonTarget = DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL, FE_PREFAB },
			DirectoryDragAndDropCallback, reinterpret_cast<void**>(&VFSBackButtonInfo),
			std::vector<std::string> { "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder" });
	}
	else
	{
		VFSBackButtonInfo.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		//VFSBackButton->setNewUserData();
	}
}

void FEEditorContentBrowserWindow::UpdateFilterForResources()
{
	FilteredResourcesIDs.clear();

	std::vector<std::string> TemporaryFilteredList;
	TemporaryFilteredList = AllResourcesIDs;
	if (!AnyFilterActive())
	{
		FilteredResourcesIDs = TemporaryFilteredList;
		return;
	}

	for (size_t i = 0; i < TemporaryFilteredList.size(); i++)
	{
		if (ShouldPassVisibilityFilter(TemporaryFilteredList[i]))
		{
			FilteredResourcesIDs.push_back(TemporaryFilteredList[i]);
		}
	}
}

bool FEEditorContentBrowserWindow::AnyFilterActive()
{ 
	return NameFilter[0] != '\0' || ObjectTypeFilters.size() > 0 || ObjectTagNegativeFilters.size() > 0;
}

bool FEEditorContentBrowserWindow::ShouldPassVisibilityFilter(std::string ObjectID)
{
	FEObject* Object = OBJECT_MANAGER.GetFEObject(ObjectID);
	if (Object == nullptr)
	{
		LOG.Add("Object is nullptr in FEEditorContentBrowserWindow::ShouldPassVisibilityFilter", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_WARNING);
		return false;
	}

	if (!NameFilter.empty())
	{
		if (Object->GetName().find(NameFilter) == -1)
			return false;
	}

	bool bShouldPassTypeFilter = true;
	if (!ObjectTypeFilters.empty())
	{
		bShouldPassTypeFilter = false;
		// Check if object is a folder
		if (Object->GetType() == FE_NULL)
		{
			bShouldPassTypeFilter = true;
		}
		else
		{
			for (size_t i = 0; i < ObjectTypeFilters.size(); i++)
			{
				if (FEObjectTypeToString(Object->GetType()) == ObjectTypeFilters[i])
				{
					bShouldPassTypeFilter = true;
					break;
				}
			}
		}
	}

	if (!bShouldPassTypeFilter)
		return false;
	
	bool bShouldPassNegativeTagFilter = true;
	if (!ObjectTagNegativeFilters.empty())
	{
		// Check if object is not a folder
		if (Object->GetType() != FE_NULL)
		{
			
			for (size_t i = 0; i < ObjectTagNegativeFilters.size(); i++)
			{
				if (Object->GetTag() == ObjectTagNegativeFilters[i])
				{
					bShouldPassNegativeTagFilter = false;
					break;
				}
			}
		}
	}

	if (!bShouldPassNegativeTagFilter)
		return false;

	return true;
}