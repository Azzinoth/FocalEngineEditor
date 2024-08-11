#include "ContentBrowserWindow.h"
#include "../FEEditor.h"
using namespace FocalEngine;

FEEditorContentBrowserWindow* FEEditorContentBrowserWindow::Instance = nullptr;
FEObject* FEEditorContentBrowserWindow::ItemInFocus = nullptr;

FEEditorContentBrowserWindow::FEEditorContentBrowserWindow()
{
	strcpy_s(FilterForResources, "");

	ENGINE.AddMouseButtonCallback(&FEEditorContentBrowserWindow::MouseButtonCallback);
}

static FETexture* TempTexture = nullptr;
static void AddTransparencyToTextureCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (TempTexture == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_TEXTURE)
	{
		FETexture* OriginalTexture = TempTexture;

		FETexture* NewTexture = RESOURCE_MANAGER.CreateTextureWithTransparency(OriginalTexture, reinterpret_cast<FETexture*>(SelectionsResult[0]));
		if (NewTexture == nullptr)
		{
			TempTexture = nullptr;
			return;
		}

		unsigned char* NewRawData = NewTexture->GetRawData();
		const int MaxDimention = std::max(OriginalTexture->GetWidth(), OriginalTexture->GetHeight());
		const size_t MipCount = size_t(floor(log2(MaxDimention)) + 1);
		OriginalTexture->UpdateRawData(NewRawData, MipCount);
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
		PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(OriginalTexture);

		delete[] NewRawData;
		RESOURCE_MANAGER.DeleteFETexture(NewTexture);
	}

	TempTexture = nullptr;
}

static void CreateNewPrefabCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (!SelectionsResult.empty() && SelectionsResult[0]->GetType() == FE_GAMEMODEL)
	{
		// FIX ME!
		FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab();

		if (SelectionsResult.size() > 1)
		{
			for (int i = 1; i < SelectionsResult.size(); i++)
			{
				// FIX ME! It should be here
				//if (SelectionsResult[i]->GetType() == FE_GAMEMODEL)
				//	NewPrefab->AddComponent(reinterpret_cast<FEGameModel*>(SelectionsResult[i]));
			}
		}
		
		PROJECT_MANAGER.GetCurrent()->SetModified(true);
		VIRTUAL_FILE_SYSTEM.CreateFile(NewPrefab, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	}

	SelectFEObjectPopUp::getInstance().SetOneObjectSelectonMode(true);
}

void FEEditorContentBrowserWindow::MouseButtonCallback(int Button, int Action, int Mods)
{
	ItemInFocus = nullptr;
}

void FEEditorContentBrowserWindow::OpenItemParentFolder(FEObject* Object)
{
	VIRTUAL_FILE_SYSTEM.SetCurrentPath(VIRTUAL_FILE_SYSTEM.LocateFile(Object));
	ImGui::SetWindowFocus("Content Browser");
	strcpy_s(FilterForResources, "");
	ItemInFocus = Object;

	const auto Content = VIRTUAL_FILE_SYSTEM.GetDirectoryContent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	size_t TotalItemCount = Content.size();

	size_t ItemIndex = 0;
	for (size_t i = 0; i < Content.size(); i++)
	{
		if (Content[i]->GetObjectID() == Object->GetObjectID())
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
		ImGui::OpenPopup("##context_menu");

	bShouldOpenContextMenu = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		bShouldOpenContextMenu = true;

		if (ItemUnderMouse == -1)
		{
			if (ImGui::MenuItem("Import Asset..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, ALL_IMPORT_LOAD_FILTER, 3);
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
					const std::vector<FEObject*> LoadedObjects = RESOURCE_MANAGER.ImportOBJ(FilePath.c_str(), true);
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

					for (size_t i = 0; i < FilteredResources.size(); i++)
					{
						if (FilteredResources[i]->GetName() == NewDirectoryName)
						{
							RenameIndex = int(i);
							strcpy_s(RenameBuffer, FilteredResources[i]->GetName().size() + 1, FilteredResources[i]->GetName().c_str());
							bLastFrameRenameEditWasVisiable = false;
							break;
						}
					}
				}

				if (ImGui::BeginMenu("Texture"))
				{
					if (ImGui::MenuItem("Combine channels..."))
						CombineChannelsToTexturePopUp::getInstance().Show();

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
					SelectFEObjectPopUp::getInstance().SetOneObjectSelectonMode(false);
					SelectFEObjectPopUp::getInstance().Show(FE_GAMEMODEL, CreateNewPrefabCallBack);
				}

				ImGui::EndMenu();
			}
		}
		else
		{
			std::string FullPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (FullPath.back() != '/')
				FullPath += '/';
			FullPath += FilteredResources[ItemUnderMouse]->GetName();

			const bool ReadOnlyItem = VIRTUAL_FILE_SYSTEM.IsReadOnly(FilteredResources[ItemUnderMouse], FullPath);

			if (ReadOnlyItem)
				ImGui::MenuItem("Read Only");
			
			if (!ReadOnlyItem)
			{
				if (ImGui::MenuItem("Rename"))
				{
					RenameIndex = ItemUnderMouse;

					strcpy_s(RenameBuffer, FilteredResources[ItemUnderMouse]->GetName().size() + 1, FilteredResources[ItemUnderMouse]->GetName().c_str());
					bLastFrameRenameEditWasVisiable = false;
				}
			}

			if (FilteredResources[ItemUnderMouse]->GetType() == FE_MATERIAL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					EDITOR_MATERIAL_WINDOW.Show(RESOURCE_MANAGER.GetMaterial(FilteredResources[ItemUnderMouse]->GetObjectID()));
				}
			}

			if (FilteredResources[ItemUnderMouse]->GetType() == FE_GAMEMODEL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					EditGameModelPopup::getInstance().Show(RESOURCE_MANAGER.GetGameModel(FilteredResources[ItemUnderMouse]->GetObjectID()));
				}

				if (ImGui::MenuItem("Create Prefab out of this Game Model"))
				{
					// FIX ME !
					/*FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(RESOURCE_MANAGER.GetGameModel(FilteredResources[ItemUnderMouse]->GetObjectID()));
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					VIRTUAL_FILE_SYSTEM.CreateFile(NewPrefab, VIRTUAL_FILE_SYSTEM.GetCurrentPath());*/
				}
			}

			if (FilteredResources[ItemUnderMouse]->GetType() == FE_PREFAB)
			{
				if (ImGui::MenuItem("Edit"))
				{
					PREFAB_EDITOR_MANAGER.PrepareEditWinow(RESOURCE_MANAGER.GetPrefab(FilteredResources[ItemUnderMouse]->GetObjectID()));
				}
			}

			if (FilteredResources[ItemUnderMouse]->GetType() == FE_SHADER)
			{
				if (ImGui::MenuItem("Edit"))
				{
					ShaderEditorWindow::getInstance().Show(RESOURCE_MANAGER.GetShader(FilteredResources[ItemUnderMouse]->GetObjectID()));
				}
			}

			if (!ReadOnlyItem)
			{
				if (ImGui::MenuItem("Delete"))
				{
					if (FilteredResources[ItemUnderMouse]->GetType() == FE_NULL)
					{
						DeleteDirectoryPopup::getInstance().Show(FilteredResources[ItemUnderMouse]->GetName());
					}
					else if (FilteredResources[ItemUnderMouse]->GetType() == FE_MESH)
					{
						DeleteMeshPopup::getInstance().Show(RESOURCE_MANAGER.GetMesh(FilteredResources[ItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResources[ItemUnderMouse]->GetType() == FE_TEXTURE)
					{
						DeleteTexturePopup::getInstance().Show(RESOURCE_MANAGER.GetTexture(FilteredResources[ItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResources[ItemUnderMouse]->GetType() == FE_MATERIAL)
					{
						DeleteMaterialPopup::getInstance().Show(RESOURCE_MANAGER.GetMaterial(FilteredResources[ItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResources[ItemUnderMouse]->GetType() == FE_GAMEMODEL)
					{
						DeleteGameModelPopup::getInstance().Show(RESOURCE_MANAGER.GetGameModel(FilteredResources[ItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResources[ItemUnderMouse]->GetType() == FE_PREFAB)
					{
						DeletePrefabPopup::getInstance().Show(RESOURCE_MANAGER.GetPrefab(FilteredResources[ItemUnderMouse]->GetObjectID()));
					}
				}
			}

			if (FilteredResources[ItemUnderMouse]->GetType() == FE_TEXTURE)
			{
				if (ImGui::BeginMenu("Convert"))
				{
					if (ImGui::MenuItem("Texture channels to individual textures"))
					{
						const std::vector<FETexture*> NewTextures = RESOURCE_MANAGER.ChannelsToFETextures(RESOURCE_MANAGER.GetTexture(FilteredResources[ItemUnderMouse]->GetObjectID()));

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
						FETexture* TextureToResize = RESOURCE_MANAGER.GetTexture(FilteredResources[ItemUnderMouse]->GetObjectID());
						ResizeTexturePopup::getInstance().Show(TextureToResize);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Add transparency"))
				{
					if (ImGui::MenuItem("Choose transparency mask"))
					{
						TempTexture = reinterpret_cast<FETexture*>(FilteredResources[ItemUnderMouse]);
						SelectFEObjectPopUp::getInstance().Show(FE_TEXTURE, AddTransparencyToTextureCallBack);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Export"))
				{
					FETexture* TextureToExport = RESOURCE_MANAGER.GetTexture(FilteredResources[ItemUnderMouse]->GetObjectID());

					if (ImGui::MenuItem("as PNG"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, TEXTURE_LOAD_FILTER, 1);

						if (!FilePath.empty())
						{
							FilePath += ".png";
							RESOURCE_MANAGER.ExportFETextureToPNG(TextureToExport, FilePath.c_str());
						}
					}

					ImGui::EndMenu();
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
	AllResources.clear();
	FilteredResources.clear();
	ItemUnderMouse = -1;
	RenameIndex = -1;
	strcpy_s(FilterForResources, "");
}

void FEEditorContentBrowserWindow::InitializeResources()
{
	FolderIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/folderIcon.png", "folderIcon");
	RESOURCE_MANAGER.SetTag(FolderIcon, EDITOR_RESOURCE_TAG);

	ShaderIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/shaderIcon.png", "shaderIcon");
	RESOURCE_MANAGER.SetTag(ShaderIcon, EDITOR_RESOURCE_TAG);

	VFSBackIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/VFSBackIcon.png", "VFSBackIcon");
	RESOURCE_MANAGER.SetTag(VFSBackIcon, EDITOR_RESOURCE_TAG);

	TextureIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/textureContentBrowserIcon.png", "textureContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(TextureIcon, EDITOR_RESOURCE_TAG);

	MeshIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/meshContentBrowserIcon.png", "meshContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(MeshIcon, EDITOR_RESOURCE_TAG);

	MaterialIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/materialContentBrowserIcon.png", "materialContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(MaterialIcon, EDITOR_RESOURCE_TAG);

	GameModelIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/gameModelContentBrowserIcon.png", "gameModelContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(GameModelIcon, EDITOR_RESOURCE_TAG);

	PrefabIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/prefabContentBrowserIcon.png", "prefabContentBrowserIcon");
	RESOURCE_MANAGER.SetTag(PrefabIcon, EDITOR_RESOURCE_TAG);

	AllIcon = RESOURCE_MANAGER.LoadPNGTexture("Resources/Images/allContentBrowserIcon.png", "allIcon");
	FilterAllTypesButton = new ImGuiImageButton(AllIcon);
	RESOURCE_MANAGER.SetTag(AllIcon, EDITOR_RESOURCE_TAG);
	FilterAllTypesButton->SetSize(ImVec2(32, 32));

	FilterTextureTypeButton = new ImGuiImageButton(TextureIcon);
	FilterTextureTypeButton->SetSize(ImVec2(32, 32));

	FilterMeshTypeButton = new ImGuiImageButton(MeshIcon);
	FilterMeshTypeButton->SetSize(ImVec2(32, 32));

	FilterMaterialTypeButton = new ImGuiImageButton(MaterialIcon);
	FilterMaterialTypeButton->SetSize(ImVec2(32, 32));

	FilterGameModelTypeButton = new ImGuiImageButton(GameModelIcon);
	FilterGameModelTypeButton->SetSize(ImVec2(32, 32));

	FilterPrefabTypeButton = new ImGuiImageButton(PrefabIcon);
	FilterPrefabTypeButton->SetSize(ImVec2(32, 32));
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

		PreviewTexture = PREVIEW_MANAGER.GetMeshPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = MeshIcon;
	}
	else if (Item->GetType() == FE_TEXTURE)
	{
		PreviewTexture = RESOURCE_MANAGER.GetTexture(Item->GetObjectID());
		SmallAdditionTypeIcon = TextureIcon;
	}
	else if (Item->GetType() == FE_MATERIAL)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetMaterialPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = MaterialIcon;
	}
	else if (Item->GetType() == FE_GAMEMODEL)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetGameModelPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = GameModelIcon;
	}
	else if (Item->GetType() == FE_PREFAB)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetPrefabPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = PrefabIcon;
	}
}

void FEEditorContentBrowserWindow::RenderFilterMenu()
{
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	float CurrentY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(CurrentY);
	if (ImGui::ImageButton((void*)(intptr_t)VFSBackIcon->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		VIRTUAL_FILE_SYSTEM.SetCurrentPath(VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));
		strcpy_s(FilterForResources, "");
	}
	VFSBackButtonTarget->StickToItem();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	static std::string LastFramePath;
	AllResources.clear();
	AllResources = VIRTUAL_FILE_SYSTEM.GetDirectoryContent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());

	ImGui::SetCursorPosX(100);
	ImGui::SetCursorPosY(CurrentY + 5);
	ImGui::Text("Filter by type: ");

	ImGui::SetCursorPosX(120 + 140);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResources.empty() ? SetSelectedStyle(FilterAllTypesButton) : SetDefaultStyle(FilterAllTypesButton);
	FilterAllTypesButton->Render();
	if (FilterAllTypesButton->IsClicked())
		ObjTypeFilterForResources = "";

	ImGui::SetCursorPosX(120 + 140 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResources == FEObjectTypeToString(FE_TEXTURE) ? SetSelectedStyle(FilterTextureTypeButton) : SetDefaultStyle(FilterTextureTypeButton);
	FilterTextureTypeButton->Render();
	if (FilterTextureTypeButton->IsClicked())
		ObjTypeFilterForResources = FEObjectTypeToString(FE_TEXTURE);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResources == FEObjectTypeToString(FE_MESH) ? SetSelectedStyle(FilterMeshTypeButton) : SetDefaultStyle(FilterMeshTypeButton);
	FilterMeshTypeButton->Render();
	if (FilterMeshTypeButton->IsClicked())
		ObjTypeFilterForResources = FEObjectTypeToString(FE_MESH);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResources == FEObjectTypeToString(FE_MATERIAL) ? SetSelectedStyle(FilterMaterialTypeButton) : SetDefaultStyle(FilterMaterialTypeButton);
	FilterMaterialTypeButton->Render();
	if (FilterMaterialTypeButton->IsClicked())
		ObjTypeFilterForResources = FEObjectTypeToString(FE_MATERIAL);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResources == FEObjectTypeToString(FE_GAMEMODEL) ? SetSelectedStyle(FilterGameModelTypeButton) : SetDefaultStyle(FilterGameModelTypeButton);
	FilterGameModelTypeButton->Render();
	if (FilterGameModelTypeButton->IsClicked())
		ObjTypeFilterForResources = FEObjectTypeToString(FE_GAMEMODEL);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48 + 48 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResources == FEObjectTypeToString(FE_PREFAB) ? SetSelectedStyle(FilterPrefabTypeButton) : SetDefaultStyle(FilterPrefabTypeButton);
	FilterPrefabTypeButton->Render();
	if (FilterPrefabTypeButton->IsClicked())
		ObjTypeFilterForResources = FEObjectTypeToString(FE_PREFAB);

	ImGui::SetCursorPosX(100);
	ImGui::SetCursorPosY(CurrentY + 50);
	ImGui::Text("Filter by name: ");

	ImGui::SetCursorPosX(120 + 140);
	ImGui::SetCursorPosY(CurrentY + 47);
	ImGui::InputText("##filter", FilterForResources, IM_ARRAYSIZE(FilterForResources));

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

	int IconsPerWindowWidth = (int)(ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (ItemIconSize + 8 + 32));
	// Possibly window is minimized anyway ImGui::Columns can't take 0 as columns count!
	if (IconsPerWindowWidth == 0)
		return;

	if (!bShouldOpenContextMenu) ItemUnderMouse = -1;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	ImGui::Columns(IconsPerWindowWidth, "mycolumns3", false);

	for (size_t i = 0; i < FilteredResources.size(); i++)
	{
		ImGui::PushID(int(std::hash<std::string>{}(FilteredResources[i]->GetObjectID())));

		if (ItemInFocus != nullptr && ItemInFocus->GetObjectID() == FilteredResources[i]->GetObjectID())
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
		
 		ImVec2 uv0 = ImVec2(0.0f, 0.0f);
		ImVec2 uv1 = ImVec2(1.0f, 1.0f);

		FETexture* PreviewTexture = nullptr;
		FETexture* SmallAdditionTypeIcon = nullptr;

		ChooseTexturesItem(PreviewTexture, SmallAdditionTypeIcon, uv0, uv1, FilteredResources[i]);

		if (PreviewTexture != nullptr)
			ImGui::ImageButton((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(ItemIconSize, ItemIconSize), uv0, uv1, 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		
		if (FilteredResources[i]->GetType() == FE_NULL && DirectoriesTargets.size() > (size_t)DirectoryIndex)
			DirectoriesTargets[DirectoryIndex++]->StickToItem();

		if (ImGui::IsItemHovered())
		{
			if (!bShouldOpenContextMenu && !DRAG_AND_DROP_MANAGER.ObjectIsDraged())
			{
				std::string AdditionalTypeInfo;
				if (FilteredResources[i]->GetType() == FE_TEXTURE)
				{
					AdditionalTypeInfo += "\nTexture type: ";
					AdditionalTypeInfo += FETexture::TextureInternalFormatToString(RESOURCE_MANAGER.GetTexture(FilteredResources[i]->GetObjectID())->GetInternalFormat());
				}

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("ID: " + FilteredResources[i]->GetObjectID() +
										"\nTag: " + FilteredResources[i]->GetTag() +
										"\nName: " + FilteredResources[i]->GetName() +
										"\nType: " + FEObjectTypeToString(FilteredResources[i]->GetType()) +
										AdditionalTypeInfo +
										"\nPath: " + VIRTUAL_FILE_SYSTEM.GetCurrentPath()
										).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();

				ItemUnderMouse = int(i);

				if (ImGui::IsMouseDragging(0))
					DRAG_AND_DROP_MANAGER.SetObjectToDrag(FilteredResources[i], PreviewTexture, uv0, uv1);
			}
		}

		if (SmallAdditionTypeIcon != nullptr)
		{
			ImVec2 CursorPosBefore = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(CursorPosBefore.x + 10, CursorPosBefore.y - 48));
			ImGui::Image((void*)(intptr_t)SmallAdditionTypeIcon->GetTextureID(), ImVec2(32, 32));
			ImGui::SetCursorPos(CursorPosBefore);
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopID();
		
		if (RenameIndex == i)
		{
			if (!bLastFrameRenameEditWasVisiable)
			{
				ImGui::SetKeyboardFocusHere(0);
				ImGui::SetFocusID(ImGui::GetID("##newNameEditor"), ImGui::GetCurrentWindow());
				ImGui::SetItemDefaultFocus();
				bLastFrameRenameEditWasVisiable = true;
			}

			ImGui::SetNextItemWidth(ItemIconSize + 8.0f + 8.0f);
			if (ImGui::InputText("##newNameEditor", RenameBuffer, IM_ARRAYSIZE(RenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue) ||
				ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##newNameEditor"))
			{
				if (FilteredResources[RenameIndex]->GetType() == FE_NULL)
				{
					std::string PathToDirectory = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
					if (PathToDirectory.back() != '/')
						PathToDirectory += '/';

					PathToDirectory += FilteredResources[RenameIndex]->GetName();
					VIRTUAL_FILE_SYSTEM.RenameDirectory(RenameBuffer, PathToDirectory);

					UpdateDirectoryDragAndDropTargets();
				}
				else
				{
					FilteredResources[RenameIndex]->SetDirtyFlag(true);
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					FilteredResources[RenameIndex]->SetName(RenameBuffer);
				}
				
				RenameIndex = -1;
			}
		}
		else
		{
			ImVec2 TextSize = ImGui::CalcTextSize(FilteredResources[i]->GetName().c_str());
			if (TextSize.x < ItemIconSize + 8 + 8)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ItemIconSize + 8.0f + 8.0f) / 2.0f - TextSize.x / 2.0f);
				ImGui::Text(FilteredResources[i]->GetName().c_str());
			}
			else
			{
				ImGui::Text(FilteredResources[i]->GetName().c_str());
			}
		}
		
		ImGui::NextColumn();
	}

	if (ImGui::IsMouseDoubleClicked(0) && ItemUnderMouse != -1)
	{
		if (FilteredResources[ItemUnderMouse]->GetType() == FE_NULL)
		{
			std::string CurrentPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (CurrentPath.back() != '/')
				CurrentPath += '/';

			CurrentPath += FilteredResources[ItemUnderMouse]->GetName();
			VIRTUAL_FILE_SYSTEM.SetCurrentPath(CurrentPath);
		}
		else if (FilteredResources[ItemUnderMouse]->GetType() == FE_MESH)
		{
			std::string MeshInfo = "Vertex count: ";
			MeshInfo += std::to_string(RESOURCE_MANAGER.GetMesh(FilteredResources[ItemUnderMouse]->GetObjectID())->GetVertexCount());
			MeshInfo += "\n";
			MeshInfo += "Sub material socket: ";
			MeshInfo += RESOURCE_MANAGER.GetMesh(FilteredResources[ItemUnderMouse]->GetObjectID())->GetMaterialCount() == 2 ? "Yes" : "No";
			MessagePopUp::getInstance().Show("Mesh info", MeshInfo.c_str());
		}
		else if (FilteredResources[ItemUnderMouse]->GetType() == FE_MATERIAL)
		{
			EDITOR_MATERIAL_WINDOW.Show(RESOURCE_MANAGER.GetMaterial(FilteredResources[ItemUnderMouse]->GetObjectID()));
		}
		else if (FilteredResources[ItemUnderMouse]->GetType() == FE_GAMEMODEL)
		{
			if (!bShouldOpenContextMenu && !EditGameModelPopup::getInstance().IsVisible())
			{
				EditGameModelPopup::getInstance().Show(RESOURCE_MANAGER.GetGameModel(FilteredResources[ItemUnderMouse]->GetObjectID()));
			}
		}
		else if (FilteredResources[ItemUnderMouse]->GetType() == FE_PREFAB)
		{
			if (!bShouldOpenContextMenu)
			{
				PREFAB_EDITOR_MANAGER.PrepareEditWinow(RESOURCE_MANAGER.GetPrefab(FilteredResources[ItemUnderMouse]->GetObjectID()));
			}
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
	AllResources.clear();
	AllResources = VIRTUAL_FILE_SYSTEM.GetDirectoryContent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());

	UpdateFilterForResources();

	DirectoryDragAndDropInfo.resize(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));
	int SubDirectoryIndex = 0;
	for (size_t i = 0; i < FilteredResources.size(); i++)
	{
		if (FilteredResources[i]->GetType() == FE_NULL)
		{
			DirectoryDragAndDropCallbackInfo info;

			info.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (VIRTUAL_FILE_SYSTEM.GetCurrentPath().back() != '/')
				info.DirectoryPath += "/";

			info.DirectoryPath += FilteredResources[i]->GetName() + "/";
			DirectoryDragAndDropInfo[SubDirectoryIndex] = info;

			DirectoriesTargets.push_back(DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL, FE_PREFAB },
				DirectoryDragAndDropCallback, reinterpret_cast<void**>(&DirectoryDragAndDropInfo[SubDirectoryIndex]),
				std::vector<std::string> { "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder" }));
			SubDirectoryIndex++;
		}
	}

	if (VFSBackButtonTarget == nullptr)
	{
		VFSBackButtoninfo.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		VFSBackButtonTarget = DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL, FE_PREFAB },
			DirectoryDragAndDropCallback, reinterpret_cast<void**>(&VFSBackButtoninfo),
			std::vector<std::string> { "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder" });
	}
	else
	{
		VFSBackButtoninfo.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		//VFSBackButton->setNewUserData();
	}
}

void FEEditorContentBrowserWindow::UpdateFilterForResources()
{
	FilteredResources.clear();

	// Filter by name.
	if (strlen(FilterForResources) == 0)
	{
		FilteredResources = AllResources;
	}
	else
	{
		for (size_t i = 0; i < AllResources.size(); i++)
		{
			if (AllResources[i]->GetName().find(FilterForResources) != -1)
			{
				FilteredResources.push_back(AllResources[i]);
			}
		}
	}

	// Filter by type.
	if (!ObjTypeFilterForResources.empty())
	{
		std::vector<FEObject*> FinalFilteredList;
		for (size_t i = 0; i < FilteredResources.size(); i++)
		{
			if (FEObjectTypeToString(FilteredResources[i]->GetType()) == ObjTypeFilterForResources ||
				// Add folders
				FilteredResources[i]->GetType() == FE_NULL)
			{
				FinalFilteredList.push_back(FilteredResources[i]);
			}
		}

		FilteredResources = FinalFilteredList;
	}
}