#include "ContentBrowserWindow.h"
#include "TextureViewWindow.h"
#include "VolumetricTextureViewWindow.h"
#include "../FEEditor.h"
using namespace FocalEngine;

FEObject* FEEditorContentBrowserWindow::ItemInFocus = nullptr;

// Built-in formats together with formats of registered importers.
static std::vector<FEFileSystem::FileDialogFilter> GetImportFilter()
{
	std::vector<FEFileSystem::FileDialogFilter> Result = ALL_IMPORT_LOAD_FILTER;
	const std::vector<FEResourceImporter> Importers = RESOURCE_MANAGER.GetResourceImporters();
	if (Importers.empty())
		return Result;

	for (const FEResourceImporter& Importer : Importers)
	{
		std::string Patterns;
		for (size_t i = 0; i < Importer.Extensions.size(); i++)
		{
			Patterns += (i == 0 ? "*." : "; *.") + Importer.Extensions[i];
			Result[0].Extensions.push_back(Importer.Extensions[i]);
		}

		Result.push_back({ Importer.Description + " (" + Patterns + ")", Importer.Extensions });
	}

	std::string AllPatterns;
	for (size_t i = 0; i < Result[0].Extensions.size(); i++)
		AllPatterns += (i == 0 ? "*." : "; *.") + Result[0].Extensions[i];

	Result[0].Description = "All files (" + AllPatterns + ")";
	return Result;
}

static void RenderRegisteredExportMenuItems(FEObject* Object)
{
	const std::vector<FEResourceExporter> Exporters = RESOURCE_MANAGER.GetResourceExporters(Object->GetType());
	for (size_t i = 0; i < Exporters.size(); i++)
	{
		ImGui::PushID(static_cast<int>(i));
		if (ImGui::MenuItem((Exporters[i].Description + "...").c_str()))
		{
			const FEResourceExporter Exporter = Exporters[i];
			const std::vector<FEFileSystem::FileDialogFilter> Filter = { { Exporter.Description, Exporter.Extensions } };
			FILE_SYSTEM.SaveFileDialog("", Filter, [Exporter, Object](std::string FilePath, int ChosenFilterIndex) {
				std::string Extension = FILE_SYSTEM.GetFileExtension(FilePath);
				std::transform(Extension.begin(), Extension.end(), Extension.begin(), [](unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
				if (Extension.empty() || std::find(Exporter.Extensions.begin(), Exporter.Extensions.end(), Extension.substr(1)) == Exporter.Extensions.end())
					FilePath += "." + Exporter.Extensions[0];

				return Exporter.Export(Object, FilePath);
			});
		}
		ImGui::PopID();
	}
}

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
		OriginalTexture->SetMipmappingEnabled(true);
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

static std::string GetItemTypeName(FEObject* Item)
{
	switch (Item->GetType())
	{
		case FE_NULL: return "Folder";
		case FE_SHADER: return "Shader";
		case FE_MESH: return "Mesh";
		case FE_TEXTURE:
		{
			FETexture* Texture = RESOURCE_MANAGER.GetTexture(Item->GetID());
			if (Texture != nullptr && Texture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
				return "3D texture";
			if (Texture != nullptr && Texture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_CUBE)
				return "Cube texture";
			return "Texture";
		}
		case FE_MATERIAL: return "Material";
		case FE_GAMEMODEL: return "Game model";
		case FE_POINT_CLOUD: return "Point cloud";
		case FE_PREFAB: return "Prefab";
		case FE_SCENE: return "Scene";
		case FE_ASSET_PACKAGE: return "Asset package";
		case FE_NATIVE_SCRIPT_MODULE: return "Script module";
		default: return "";
	}
}

static ImU32 GetItemTypeColor(const FE_OBJECT_TYPE Type)
{
	switch (Type)
	{
		case FE_SHADER: return FEEditorStyle::TypeShader;
		case FE_MESH: return FEEditorStyle::TypeMesh;
		case FE_TEXTURE: return FEEditorStyle::TypeTexture;
		case FE_MATERIAL: return FEEditorStyle::TypeMaterial;
		case FE_GAMEMODEL: return FEEditorStyle::TypeGameModel;
		case FE_POINT_CLOUD: return FEEditorStyle::TypePointCloud;
		case FE_PREFAB: return FEEditorStyle::TypePrefab;
		case FE_SCENE: return FEEditorStyle::TypeScene;
		case FE_ASSET_PACKAGE: return FEEditorStyle::TypeAssetPackage;
		case FE_NATIVE_SCRIPT_MODULE: return FEEditorStyle::TypeNativeScriptModule;
		default: return FEEditorStyle::TextDisabled;
	}
}

// Icons are white and take this color, folders have no color of their own.
static ImU32 GetItemIconColor(const FE_OBJECT_TYPE Type)
{
	return Type == FE_NULL ? FEEditorStyle::TextMuted : GetItemTypeColor(Type);
}

// Folders, shaders, asset packages and script modules have an icon in place of a preview.
static bool HasPreview(const FE_OBJECT_TYPE Type)
{
	return Type != FE_NULL && Type != FE_SHADER && Type != FE_ASSET_PACKAGE && Type != FE_NATIVE_SCRIPT_MODULE;
}

static bool CanBeDeleted(const FE_OBJECT_TYPE Type)
{
	return Type == FE_NULL || Type == FE_MESH || Type == FE_POINT_CLOUD || Type == FE_TEXTURE || Type == FE_MATERIAL || Type == FE_GAMEMODEL || Type == FE_PREFAB;
}

// Facts about an item as label and value.
static std::vector<std::pair<std::string, std::string>> GetItemFacts(FEObject* Item)
{
	std::vector<std::pair<std::string, std::string>> Result;
	switch (Item->GetType())
	{
		case FE_NULL:
		{
			std::string Path = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (Path.back() != '/')
				Path += '/';

			const size_t Count = VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(Path + Item->GetName()).size();
			Result.push_back({ "Content", Count == 0 ? "Empty" : GetNumberWithSeparators(Count) + (Count == 1 ? " item" : " items") });
			break;
		}
		case FE_TEXTURE:
		{
			FETexture* Texture = RESOURCE_MANAGER.GetTexture(Item->GetID());
			if (Texture == nullptr)
				break;

			std::string Size = std::to_string(Texture->GetWidth()) + " x " + std::to_string(Texture->GetHeight());
			if (Texture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
				Size += " x " + std::to_string(Texture->GetDepth());

			Result.push_back({ "Size", Size });
			Result.push_back({ "Format", TextureViewWindow::GetFormatName(Texture->GetInternalFormat()) });
			break;
		}
		case FE_MESH:
		{
			FEMesh* Mesh = RESOURCE_MANAGER.GetMesh(Item->GetID());
			if (Mesh == nullptr)
				break;

			Result.push_back({ "Vertices", GetNumberWithSeparators(Mesh->GetVertexCount()) });
			Result.push_back({ "Second material", Mesh->GetMaterialCount() == 2 ? "Yes" : "No" });
			break;
		}
		case FE_MATERIAL:
		{
			FEMaterial* Material = RESOURCE_MANAGER.GetMaterial(Item->GetID());
			if (Material != nullptr && Material->Shader != nullptr)
				Result.push_back({ "Shader", Material->Shader->GetName() });
			break;
		}
		case FE_GAMEMODEL:
		{
			FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Item->GetID());
			if (GameModel == nullptr)
				break;

			if (GameModel->GetMesh() != nullptr)
				Result.push_back({ "Mesh", GameModel->GetMesh()->GetName() });
			if (GameModel->GetMaterial() != nullptr)
				Result.push_back({ "Material", GameModel->GetMaterial()->GetName() });
			if (GameModel->IsUsingLOD())
				Result.push_back({ "Levels of detail", std::to_string(GameModel->GetLODCount()) });
			break;
		}
		case FE_POINT_CLOUD:
		{
			FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(Item->GetID());
			if (PointCloud != nullptr)
				Result.push_back({ "Points", GetNumberWithSeparators(PointCloud->GetPointCount()) });
			break;
		}
		case FE_PREFAB:
		{
			FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(Item->GetID());
			if (Prefab != nullptr && Prefab->GetScene() != nullptr)
				Result.push_back({ "Entities", GetNumberWithSeparators(Prefab->GetScene()->GetEntityIDList().size()) });
			break;
		}
		case FE_SCENE:
		{
			FEScene* Scene = SCENE_MANAGER.GetSceneByID(Item->GetID());
			if (Scene != nullptr)
				Result.push_back({ "Entities", GetNumberWithSeparators(Scene->GetEntityIDList().size()) });

			FEScene* StartingScene = SCENE_MANAGER.GetStartingScene();
			if (StartingScene != nullptr && StartingScene->GetID() == Item->GetID())
				Result.push_back({ "Role", "Starting scene" });
			break;
		}
		case FE_NATIVE_SCRIPT_MODULE:
		{
			Result.push_back({ "Scripts", std::to_string(NATIVE_SCRIPT_SYSTEM.GetActiveModuleScriptNameList(Item->GetID()).size()) });
			break;
		}
		default:
			break;
	}

	return Result;
}

// The first line ends after a separator when one is close to its end, the second line is cut.
static void SplitNameInTwoLines(const std::string& Name, const float Width, std::string& FirstLine, std::string& SecondLine)
{
	FirstLine = Name;
	SecondLine = "";
	if (ImGui::CalcTextSize(Name.c_str()).x <= Width)
		return;

	size_t FitCount = Name.size();
	while (FitCount > 1 && ImGui::CalcTextSize(Name.substr(0, FitCount).c_str()).x > Width)
		FitCount--;

	size_t BreakPosition = FitCount;
	for (size_t i = FitCount; i > FitCount / 2; i--)
	{
		const char Character = Name[i - 1];
		if (Character == ' ' || Character == '_' || Character == '-' || Character == '.')
		{
			BreakPosition = i;
			break;
		}
	}

	FirstLine = Name.substr(0, BreakPosition);
	std::string Rest = Name.substr(BreakPosition);
	while (!Rest.empty() && Rest[0] == ' ')
		Rest.erase(0, 1);

	SecondLine = CutTextToWidth(Rest, Width);
}

static void RenderItemToolTip(FEObject* Item, const bool bReadOnly)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 5.0f));
	if (ImGui::BeginTooltip())
	{
		ImGui::TextUnformatted(Item->GetName().c_str());
		ImGui::PushFont(nullptr, 16.0f);
		ImGui::TextColored(FEEditorStyle::ToVec4(FEEditorStyle::TextMuted), "%s", (GetItemTypeName(Item) + (bReadOnly ? ", read only" : "")).c_str());
		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		std::vector<std::pair<std::string, std::string>> Facts = GetItemFacts(Item);
		if (!Item->GetTag().empty())
			Facts.push_back({ "Tag", Item->GetTag() });

		float LabelWidth = 0.0f;
		for (size_t i = 0; i < Facts.size(); i++)
			LabelWidth = std::max(LabelWidth, ImGui::CalcTextSize(Facts[i].first.c_str()).x);

		for (size_t i = 0; i < Facts.size(); i++)
		{
			ImGui::TextColored(FEEditorStyle::ToVec4(FEEditorStyle::TextMuted), "%s", Facts[i].first.c_str());
			ImGui::SameLine(14.0f + LabelWidth + 16.0f);
			ImGui::TextUnformatted(Facts[i].second.c_str());
		}

		if (Item->GetType() != FE_NULL)
		{
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
			ImGui::TextColored(FEEditorStyle::ToVec4(FEEditorStyle::TextDisabled), "%s", UNIQUE_ID.ToString(Item->GetID()).c_str());
		}

		ImGui::PopFont();
		ImGui::EndTooltip();
	}
	ImGui::PopStyleVar(2);
}

// Menu item with a small icon before its name, the icon is white and takes the given color.
static bool RenderMenuItemWithIcon(const char* Label, FETexture* Icon, const ImU32 IconColor)
{
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const bool bPressed = ImGui::MenuItem((std::string("   ") + Label).c_str());
	const float IconSize = 20.0f;
	const float IconY = Min.y + (ImGui::GetItemRectSize().y - IconSize) / 2.0f;
	ImGui::GetWindowDrawList()->AddImage(Icon->GetTextureID(), ImVec2(Min.x + 2.0f, IconY), ImVec2(Min.x + 2.0f + IconSize, IconY + IconSize), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), IconColor);
	return bPressed;
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
		if (CurrentObject != nullptr && CurrentObject->GetID() == Object->GetID())
		{
			ItemIndex = i;
			break;
		}
	}

	ImGuiWindow* ContentBrowserWindow = ImGui::FindWindowByName("Content Browser");
	if (ContentBrowserWindow == nullptr)
		return;

	const size_t ItemRow = ItemIndex / GetItemColumnCount(ContentBrowserWindow->Rect().GetWidth() - 30.0f);
	ContentBrowserWindow->Scroll.y = static_cast<float>(ItemRow) * (ItemCardHeight + ItemCardGap);
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
	ImGui::SetNextWindowSizeConstraints(ImVec2(330.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::BeginPopup("##Content_Browser_Context_Menu"))
	{
		bContextMenuOpened = true;

		FEObject* ObjectUnderMouse = nullptr;
		if (ItemUnderMouse >= 0 && ItemUnderMouse < static_cast<int>(FilteredResourcesIDs.size()))
			ObjectUnderMouse = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[ItemUnderMouse]);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 7.0f));
		if (ItemUnderMouse == -1)
		{
			RenderEmptySpaceMenu();
		}
		else if (ObjectUnderMouse != nullptr)
		{
			RenderItemMenu(ObjectUnderMouse);
		}
		ImGui::PopStyleVar();

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
	RESOURCE_MANAGER.SetTag(AllIcon, EDITOR_RESOURCE_TAG);
}

int FEEditorContentBrowserWindow::GetItemColumnCount(const float Width) const
{
	return std::max(1, static_cast<int>((Width + ItemCardGap) / (ItemCardWidth + ItemCardGap)));
}

FETexture* FEEditorContentBrowserWindow::GetItemPreview(FEObject* Item, ImVec2& UV0, ImVec2& UV1)
{
	UV0 = ImVec2(0.0f, 0.0f);
	UV1 = ImVec2(1.0f, 1.0f);
	switch (Item->GetType())
	{
		case FE_NULL: return FolderIcon;
		case FE_SHADER: return ShaderIcon;
		case FE_ASSET_PACKAGE: return AssetPackageIcon;
		case FE_NATIVE_SCRIPT_MODULE: return NativeScriptModuleIcon;
		case FE_TEXTURE:
		{
			FETexture* Texture = RESOURCE_MANAGER.GetTexture(Item->GetID());
			if (Texture == nullptr || Texture->GetType() != FE_TEXTURE_TYPE::FE_TEXTURE_3D)
				return Texture;

			UV0 = ImVec2(0.0f, 1.0f);
			UV1 = ImVec2(1.0f, 0.0f);
			return PREVIEW_MANAGER.GetTexture3DPreview(Item->GetID());
		}
		case FE_MESH:
		case FE_MATERIAL:
		case FE_GAMEMODEL:
		case FE_POINT_CLOUD:
		case FE_PREFAB:
		case FE_SCENE:
		{
			UV0 = ImVec2(0.0f, 1.0f);
			UV1 = ImVec2(1.0f, 0.0f);
			return PREVIEW_MANAGER.GetPreview(Item->GetID());
		}
		default: return nullptr;
	}
}

FETexture* FEEditorContentBrowserWindow::GetItemTypeIcon(FEObject* Item)
{
	switch (Item->GetType())
	{
		case FE_NULL: return FolderIcon;
		case FE_SHADER: return ShaderSmallIcon;
		case FE_MESH: return MeshIcon;
		case FE_TEXTURE:
		{
			FETexture* Texture = RESOURCE_MANAGER.GetTexture(Item->GetID());
			return Texture != nullptr && Texture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D ? Texture3DIcon : TextureIcon;
		}
		case FE_MATERIAL: return MaterialIcon;
		case FE_GAMEMODEL: return GameModelIcon;
		case FE_POINT_CLOUD: return PointCloudIcon;
		case FE_PREFAB: return PrefabIcon;
		case FE_SCENE: return SceneIcon;
		case FE_ASSET_PACKAGE: return AssetPackageSmallIcon;
		case FE_NATIVE_SCRIPT_MODULE: return NativeScriptModuleSmallIcon;
		default: return nullptr;
	}
}

static float GetFilterChipWidth(const char* Label)
{
	return 10.0f + 20.0f + 6.0f + ImGui::CalcTextSize(Label).x + 12.0f;
}

// Toggle with icon and name, it should be called with font of the chips.
static bool RenderFilterChip(const char* Label, FETexture* Icon, const bool bSelected)
{
	const ImVec2 Size = ImVec2(GetFilterChipWidth(Label), 30.0f);
	const ImVec2 Min = ImGui::GetCursorScreenPos();
	const ImVec2 Max = ImVec2(Min.x + Size.x, Min.y + Size.y);
	const bool bPressed = ImGui::InvisibleButton(Label, Size);
	const bool bHovered = ImGui::IsItemHovered();

	ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
	if (bSelected)
		Fill = bHovered ? FEEditorStyle::AccentHovered : FEEditorStyle::Accent;

	// Icons are white, so they are tinted together with the text.
	const ImU32 ContentColor = bSelected ? FEEditorStyle::TextOnAccent : FEEditorStyle::Text;
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, Fill, Size.y / 2.0f);
	if (!bSelected)
		DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, Size.y / 2.0f);

	DrawList->AddImage(Icon->GetTextureID(), ImVec2(Min.x + 10.0f, Min.y + 5.0f), ImVec2(Min.x + 30.0f, Min.y + 25.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ContentColor);
	DrawList->AddText(ImVec2(Min.x + 36.0f, Min.y + (Size.y - ImGui::GetFontSize()) / 2.0f), ContentColor, Label);
	return bPressed;
}

void FEEditorContentBrowserWindow::RenderFilterMenu()
{
	static std::string LastFramePath;

	const float RowHeight = 34.0f;
	const float RowStartX = ImGui::GetCursorPosX();
	const float RowWidth = ImGui::GetContentRegionAvail().x;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
	if (ImGui::ImageButton("GoUpVFSButton", VFSBackIcon->GetTextureID(), ImVec2(24.0f, 24.0f)))
	{
		VIRTUAL_FILE_SYSTEM.SetCurrentPath(VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));
		NameFilter = "";
	}
	ImGui::PopStyleVar();
	if (!VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(VFSBackButtonInfo.DirectoryPath))
		VFSBackButtonTarget->StickToItem();

	// Current path, every part of it is a button that opens that directory.
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, (RowHeight - ImGui::GetFontSize()) / 2.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
	ImGui::SameLine(0.0f, 8.0f);
	const std::string CurrentPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
	if (ImGui::Button("Content"))
		VIRTUAL_FILE_SYSTEM.SetCurrentPath("/");

	std::string PathToPart;
	size_t PartStart = 0;
	while (PartStart < CurrentPath.size())
	{
		size_t PartEnd = CurrentPath.find('/', PartStart);
		if (PartEnd == std::string::npos)
			PartEnd = CurrentPath.size();

		const std::string Part = CurrentPath.substr(PartStart, PartEnd - PartStart);
		PartStart = PartEnd + 1;
		if (Part.empty())
			continue;

		PathToPart += "/" + Part;
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextDisabled(">");
		ImGui::SameLine(0.0f, 0.0f);
		if (ImGui::Button((Part + "##" + PathToPart).c_str()))
			VIRTUAL_FILE_SYSTEM.SetCurrentPath(PathToPart);
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	struct TypeFilter
	{
		const char* Name;
		FE_OBJECT_TYPE Type;
		FETexture* Icon;
	};

	const TypeFilter TypeFilters[] = {
		{ "Shaders", FE_SHADER, ShaderSmallIcon },
		{ "Textures", FE_TEXTURE, TextureIcon },
		{ "Meshes", FE_MESH, MeshIcon },
		{ "Materials", FE_MATERIAL, MaterialIcon },
		{ "Game models", FE_GAMEMODEL, GameModelIcon },
		{ "Point clouds", FE_POINT_CLOUD, PointCloudIcon },
		{ "Prefabs", FE_PREFAB, PrefabIcon },
		{ "Scenes", FE_SCENE, SceneIcon },
		{ "Asset packages", FE_ASSET_PACKAGE, AssetPackageSmallIcon },
		{ "Script modules", FE_NATIVE_SCRIPT_MODULE, NativeScriptModuleSmallIcon }
	};

	// Type filters and checkbox go to the next line when they reach name filter, that is in the right corner of the first line.
	const float ItemHeight = 30.0f;
	const float ItemSpacing = 6.0f;
	const float NameFilterWidth = 320.0f;
	const float ItemsRight = RowStartX + RowWidth - NameFilterWidth - 16.0f;
	const float ItemsTop = ImGui::GetCursorPosY() + 6.0f;
	float ItemX = RowStartX;
	float ItemY = ItemsTop;
	const auto PlaceItem = [&](const float Width) {
		if (ItemX + Width > ItemsRight && ItemX > RowStartX)
		{
			ItemX = RowStartX;
			ItemY += ItemHeight + ItemSpacing;
		}

		ImGui::SetCursorPos(ImVec2(ItemX, ItemY));
		ItemX += Width + ItemSpacing;
	};

	ImGui::PushFont(nullptr, 16.0f);
	PlaceItem(GetFilterChipWidth("All"));
	if (RenderFilterChip("All", AllIcon, ObjectTypeFilters.empty()))
		ObjectTypeFilters.clear();

	for (const TypeFilter& Filter : TypeFilters)
	{
		const std::string TypeName = FEObjectTypeToString(Filter.Type);
		const bool bSelected = std::find(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), TypeName) != ObjectTypeFilters.end();
		PlaceItem(GetFilterChipWidth(Filter.Name));
		if (RenderFilterChip(Filter.Name, Filter.Icon, bSelected))
		{
			if (bSelected)
			{
				ObjectTypeFilters.erase(std::remove(ObjectTypeFilters.begin(), ObjectTypeFilters.end(), TypeName), ObjectTypeFilters.end());
			}
			else
			{
				ObjectTypeFilters.push_back(TypeName);
			}
		}
	}
	ImGui::PopFont();

	static bool bIsPrivateTagsInFilters = true;
	if (bIsPrivateTagsInFilters && ObjectTagNegativeFilters.empty())
	{
		ObjectTagNegativeFilters.push_back(ENGINE_RESOURCE_TAG);
		ObjectTagNegativeFilters.push_back(EDITOR_RESOURCE_TAG);
		ObjectTagNegativeFilters.push_back(PREFAB_SCENE_DESCRIPTION_TAG);
		ObjectTagNegativeFilters.push_back(TERRAIN_SYSTEM_RESOURCE_TAG);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, (ItemHeight - ImGui::GetFontSize()) / 2.0f));
	const char* PrivateTagsLabel = "Hide private tags";
	ItemX += 10.0f;
	PlaceItem(ItemHeight + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(PrivateTagsLabel).x);
	if (ImGui::Checkbox(PrivateTagsLabel, &bIsPrivateTagsInFilters))
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

	ImGui::SetCursorPos(ImVec2(RowStartX + RowWidth - NameFilterWidth, ItemsTop));
	ImGui::SetNextItemWidth(NameFilterWidth);
	ImGui::InputTextWithHint("##ContentBrowserNameFilter", "Filter by name...", &NameFilter);
	ImGui::PopStyleVar();

	ImGui::SetCursorPos(ImVec2(RowStartX, ItemY + ItemHeight + 8.0f));
	ImGui::Separator();

	if (IsFilteredListOutdated())
		UpdateDirectoryDragAndDropTargets();

	// ************** Drag&Drop **************
	if (VIRTUAL_FILE_SYSTEM.GetCurrentPath() != LastFramePath)
	{
		LastFramePath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
		UpdateDirectoryDragAndDropTargets();
	}
	int DirectoryIndex = 0;
	// ************** Drag&Drop END **************

	if (!bContextMenuOpened) ItemUnderMouse = -1;

	const ImVec2 AreaMin = ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + 5.0f);
	const int ColumnCount = GetItemColumnCount(ImGui::GetContentRegionAvail().x);
	int ShownCount = 0;
	for (size_t i = 0; i < FilteredResourcesIDs.size(); i++)
	{
		FEObject* CurrentResource = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[i]);
		if (CurrentResource == nullptr)
			continue;

		const ImVec2 CardMin = ImVec2(AreaMin.x + (ShownCount % ColumnCount) * (ItemCardWidth + ItemCardGap), AreaMin.y + (ShownCount / ColumnCount) * (ItemCardHeight + ItemCardGap));
		ShownCount++;

		// Cards that are not on the screen only take space.
		if (RenameIndex != static_cast<int>(i) && !ImGui::IsRectVisible(CardMin, ImVec2(CardMin.x + ItemCardWidth, CardMin.y + ItemCardHeight)))
		{
			if (CurrentResource->GetType() == FE_NULL)
				DirectoryIndex++;

			continue;
		}

		RenderItemCard(CurrentResource, static_cast<int>(i), CardMin, DirectoryIndex);
	}

	if (ShownCount > 0)
	{
		const int RowCount = (ShownCount + ColumnCount - 1) / ColumnCount;
		ImGui::SetCursorScreenPos(AreaMin);
		ImGui::Dummy(ImVec2(ItemCardWidth, RowCount * (ItemCardHeight + ItemCardGap)));
	}

	if (ImGui::IsMouseDoubleClicked(0) && !bContextMenuOpened && ItemUnderMouse != -1 && ItemUnderMouse < static_cast<int>(FilteredResourcesIDs.size()))
	{
		FEObject* ClickedItem = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[ItemUnderMouse]);
		if (ClickedItem != nullptr)
			OpenItem(ClickedItem);
	}
}

void FEEditorContentBrowserWindow::RenderItemCard(FEObject* Item, const int Index, const ImVec2 Min, int& DirectoryIndex)
{
	const FE_OBJECT_TYPE Type = Item->GetType();
	const ImVec2 Max = ImVec2(Min.x + ItemCardWidth, Min.y + ItemCardHeight);
	const ImVec2 PictureMax = ImVec2(Max.x, Min.y + ItemCardWidth);
	const bool bRenaming = RenameIndex == Index;

	ImGui::PushID(int(std::hash<FEUUID>{}(Item->GetID())));
	ImGui::SetCursorScreenPos(Min);
	// Rename field is over the card and should get the mouse.
	if (bRenaming)
		ImGui::SetNextItemAllowOverlap();
	ImGui::InvisibleButton("##ItemCard", ImVec2(ItemCardWidth, ItemCardHeight));
	const bool bMouseOver = ImGui::IsItemHovered() && !bContextMenuOpened;

	if (Type == FE_NULL && DirectoriesTargets.size() > static_cast<size_t>(DirectoryIndex))
	{
		if (DirectoriesTargets[DirectoryIndex] != nullptr)
			DirectoriesTargets[DirectoryIndex]->StickToItem();
		DirectoryIndex++;
	}

	ImVec2 UV0;
	ImVec2 UV1;
	FETexture* Preview = GetItemPreview(Item, UV0, UV1);
	if (bMouseOver && !DRAG_AND_DROP_MANAGER.ObjectIsDraged())
	{
		std::string FullPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
		if (FullPath.back() != '/')
			FullPath += '/';
		FullPath += Item->GetName();
		RenderItemToolTip(Item, VIRTUAL_FILE_SYSTEM.IsReadOnly(Item, FullPath));

		ItemUnderMouse = Index;
		if (ImGui::IsMouseDragging(0))
			DRAG_AND_DROP_MANAGER.SetObjectToDrag(Item, Preview, UV0, UV1);
	}

	ImU32 Fill = FEEditorStyle::Card;
	ImU32 Border = FEEditorStyle::CardBorder;
	// Card keeps the hovered look while its menu is opened.
	if (bMouseOver || (bContextMenuOpened && ItemUnderMouse == Index))
	{
		Fill = FEEditorStyle::CardHovered;
		Border = FEEditorStyle::ButtonBorder;
	}

	const bool bInFocus = ItemInFocus != nullptr && ItemInFocus->GetID() == Item->GetID();
	if (bInFocus)
	{
		Fill = FEEditorStyle::SelectedItem;
		Border = FEEditorStyle::Accent;
	}

	const ImU32 TypeColor = GetItemTypeColor(Type);
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, Fill, 6.0f);
	if (Preview != nullptr)
	{
		if (HasPreview(Type))
		{
			DrawList->AddImageRounded(Preview->GetTextureID(), Min, PictureMax, UV0, UV1, IM_COL32_WHITE, 6.0f, ImDrawFlags_RoundCornersTop);

			// Icon of the type on a badge of the type color.
			FETexture* TypeIcon = GetItemTypeIcon(Item);
			const ImVec2 BadgeMin = ImVec2(Min.x + 8.0f, PictureMax.y - 8.0f - 28.0f);
			const ImVec2 BadgeMax = ImVec2(BadgeMin.x + 28.0f, BadgeMin.y + 28.0f);
			DrawList->AddRectFilled(BadgeMin, BadgeMax, TypeColor, 4.0f);
			if (TypeIcon != nullptr)
				DrawList->AddImage(TypeIcon->GetTextureID(), ImVec2(BadgeMin.x + 4.0f, BadgeMin.y + 4.0f), ImVec2(BadgeMax.x - 4.0f, BadgeMax.y - 4.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), FEEditorStyle::Background);
		}
		else
		{
			const float IconSize = 68.0f;
			const ImVec2 IconMin = ImVec2(Min.x + (ItemCardWidth - IconSize) / 2.0f, Min.y + (ItemCardWidth - IconSize) / 2.0f);
			DrawList->AddRectFilled(Min, PictureMax, FEEditorStyle::Panel, 6.0f, ImDrawFlags_RoundCornersTop);
			DrawList->AddImage(Preview->GetTextureID(), IconMin, ImVec2(IconMin.x + IconSize, IconMin.y + IconSize), UV0, UV1, GetItemIconColor(Type));
		}
	}

	DrawList->AddRectFilled(ImVec2(Min.x, PictureMax.y), ImVec2(Max.x, PictureMax.y + 3.0f), TypeColor);
	DrawList->AddRect(Min, Max, Border, 6.0f, 0, bInFocus ? 2.0f : 1.0f);

	ImGui::PushFont(nullptr, 14.0f);
	DrawList->AddText(ImVec2(Min.x + 9.0f, Max.y - 21.0f), FEEditorStyle::TextMuted, GetItemTypeName(Item).c_str());
	ImGui::PopFont();

	ImGui::PopID();

	ImGui::PushFont(nullptr, 16.0f);
	if (bRenaming)
	{
		if (!bLastFrameRenameEditWasVisible)
		{
			ImGui::SetKeyboardFocusHere(0);
			ImGui::SetFocusID(ImGui::GetID("##NewNameEditor"), FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl());
			ImGui::SetItemDefaultFocus();
			bLastFrameRenameEditWasVisible = true;
		}

		ImGui::SetCursorScreenPos(ImVec2(Min.x + 5.0f, PictureMax.y + 8.0f));
		ImGui::SetNextItemWidth(ItemCardWidth - 10.0f);
		ImGui::PushStyleColor(ImGuiCol_Border, FEEditorStyle::Accent);
		const bool bRenameFinished = ImGui::InputText("##NewNameEditor", &RenameBuffer, ImGuiInputTextFlags_EnterReturnsTrue) ||
									 ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || !ImGui::IsItemFocused();
		ImGui::PopStyleColor();
		if (bRenameFinished)
		{
			if (Type == FE_NULL)
			{
				std::string PathToDirectory = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
				if (PathToDirectory.back() != '/')
					PathToDirectory += '/';

				PathToDirectory += Item->GetName();
				VIRTUAL_FILE_SYSTEM.RenameDirectory(RenameBuffer, PathToDirectory);

				UpdateDirectoryDragAndDropTargets();
			}
			else
			{
				Item->SetDirtyFlag(true);
				PROJECT_MANAGER.GetCurrent()->SetModified(true);
				Item->SetName(RenameBuffer);
			}

			RenameIndex = -1;
		}
	}
	else
	{
		std::string FirstLine;
		std::string SecondLine;
		SplitNameInTwoLines(Item->GetName(), ItemCardWidth - 18.0f, FirstLine, SecondLine);
		DrawList->AddText(ImVec2(Min.x + 9.0f, PictureMax.y + 9.0f), FEEditorStyle::Text, FirstLine.c_str());
		if (!SecondLine.empty())
			DrawList->AddText(ImVec2(Min.x + 9.0f, PictureMax.y + 27.0f), FEEditorStyle::Text, SecondLine.c_str());
	}
	ImGui::PopFont();
}

void FEEditorContentBrowserWindow::StartRename(const int Index)
{
	FEObject* Item = OBJECT_MANAGER.GetFEObject(FilteredResourcesIDs[Index]);
	if (Item == nullptr)
		return;

	RenameIndex = Index;
	RenameBuffer = Item->GetName();
	bLastFrameRenameEditWasVisible = false;
}

void FEEditorContentBrowserWindow::OpenItem(FEObject* Item)
{
	if (Item->GetType() == FE_NULL)
	{
		std::string CurrentPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
		if (CurrentPath.back() != '/')
			CurrentPath += '/';

		CurrentPath += Item->GetName();
		VIRTUAL_FILE_SYSTEM.SetCurrentPath(CurrentPath);
	}
	else if (Item->GetType() == FE_MESH)
	{
		std::string MeshInfo = "Vertex count: ";
		MeshInfo += std::to_string(RESOURCE_MANAGER.GetMesh(Item->GetID())->GetVertexCount());
		MeshInfo += "\n";
		MeshInfo += "Sub material socket: ";
		MeshInfo += RESOURCE_MANAGER.GetMesh(Item->GetID())->GetMaterialCount() == 2 ? "Yes" : "No";
		MessagePopUp::GetInstance().Show("Mesh info", MeshInfo.c_str());
	}
	else if (Item->GetType() == FE_MATERIAL)
	{
		EDITOR_MATERIAL_WINDOW.Show(RESOURCE_MANAGER.GetMaterial(Item->GetID()));
	}
	else if (Item->GetType() == FE_GAMEMODEL)
	{
		if (!EditGameModelPopup::GetInstance().IsVisible())
			EditGameModelPopup::GetInstance().Show(RESOURCE_MANAGER.GetGameModel(Item->GetID()));
	}
	else if (Item->GetType() == FE_PREFAB)
	{
		PREFAB_EDITOR_MANAGER.PrepareEditWinow(RESOURCE_MANAGER.GetPrefab(Item->GetID()));
	}
	else if (Item->GetType() == FE_SHADER)
	{
		ShaderEditorWindow::GetInstance().Show(RESOURCE_MANAGER.GetShader(Item->GetID()));
	}
	else if (Item->GetType() == FE_SCENE)
	{
		if (EDITOR.GetEditorSceneWindow(Item->GetID()) == nullptr)
			EDITOR.CreateEditorWindowForScene(Item->GetID());
	}
	else if (Item->GetType() == FE_TEXTURE)
	{
		FETexture* Texture = RESOURCE_MANAGER.GetTexture(Item->GetID());
		if (Texture == nullptr)
			return;

		const std::string WindowCaption = "Texture: " + Texture->GetName();
		FEImGuiWindow* ExistingWindow = FE_IMGUI_WINDOW_MANAGER.GetWindowByCaption(WindowCaption);
		if (ExistingWindow != nullptr)
		{
			ExistingWindow->Show();
		}
		else if (Texture->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
		{
			VolumetricTextureViewWindow* NewWindow = new VolumetricTextureViewWindow(Texture);
			NewWindow->SetCaption(WindowCaption);
			NewWindow->Show();
			EDITOR.RegisterEditorSceneWindow(NewWindow);
		}
		else
		{
			TextureViewWindow* NewWindow = new TextureViewWindow(Texture);
			NewWindow->SetCaption(WindowCaption);
			NewWindow->Show();
		}
	}
}

void FEEditorContentBrowserWindow::RenderEmptySpaceMenu()
{
	ImGui::PushFont(nullptr, 16.0f);
	ImGui::TextColored(FEEditorStyle::ToVec4(FEEditorStyle::TextMuted), "Create");
	ImGui::PopFont();

	if (RenderMenuItemWithIcon("Folder", FolderIcon, GetItemIconColor(FE_NULL)))
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
				StartRename(static_cast<int>(i));
				break;
			}
		}
	}

	if (RenderMenuItemWithIcon("Material", MaterialIcon, GetItemIconColor(FE_MATERIAL)))
	{
		FEMaterial* NewMaterial = RESOURCE_MANAGER.CreateMaterial("");
		if (NewMaterial)
		{
			PROJECT_MANAGER.GetCurrent()->SetModified(true);
			NewMaterial->Shader = RESOURCE_MANAGER.GetShader(FEEngineResourceIDs::PBRShader);

			NewMaterial->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);

			VIRTUAL_FILE_SYSTEM.CreateFile(NewMaterial, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		}
	}

	if (RenderMenuItemWithIcon("Game model", GameModelIcon, GetItemIconColor(FE_GAMEMODEL)))
	{
		FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel();
		PROJECT_MANAGER.GetCurrent()->SetModified(true);
		VIRTUAL_FILE_SYSTEM.CreateFile(NewGameModel, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	}

	if (RenderMenuItemWithIcon("Prefab...", PrefabIcon, GetItemIconColor(FE_PREFAB)))
	{
		SELECT_FEOBJECT_POPUP.SetOneObjectSelectonMode(false);
		SELECT_FEOBJECT_POPUP.Show(FE_GAMEMODEL, CreateNewPrefabCallback);
	}

	if (RenderMenuItemWithIcon("Scene", SceneIcon, GetItemIconColor(FE_SCENE)))
	{
		FEScene* NewScene = SCENE_MANAGER.CreateScene("", FEUUID(), FESceneFlag::EditorMode);
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

	if (RenderMenuItemWithIcon("Script module", NativeScriptModuleSmallIcon, GetItemIconColor(FE_NATIVE_SCRIPT_MODULE)))
	{
		FENativeScriptModule* NewNativeScriptModule = RESOURCE_MANAGER.CreateNativeScriptModule("New NativeScriptModule");
		PROJECT_MANAGER.GetCurrent()->SetModified(true);
		VIRTUAL_FILE_SYSTEM.CreateFile(NewNativeScriptModule, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	}

	if (RenderMenuItemWithIcon("Texture from channels...", TextureIcon, GetItemIconColor(FE_TEXTURE)))
		CombineChannelsToTexturePopUp::GetInstance().Show();

	ImGui::Separator();
	if (ImGui::MenuItem("Import asset..."))
	{
		FILE_SYSTEM.OpenFileDialog(GetImportFilter(), [](const std::string& FilePath) {
			if (FilePath.empty())
				return;

			if (EDITOR.GetFocusedScene() != nullptr)
			{
				const std::vector<FEObject*> LoadedObjects = EDITOR.GetFocusedScene()->ImportAsset(FilePath.c_str());
				VIRTUAL_FILE_SYSTEM.CreateMissingFiles(LoadedObjects, VIRTUAL_FILE_SYSTEM.GetCurrentPath(), true);
				for (size_t i = 0; i < LoadedObjects.size(); i++)
				{
					if (LoadedObjects[i] != nullptr)
					{
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(LoadedObjects[i]);
					}
				}
			}
		});
	}
}

void FEEditorContentBrowserWindow::RenderItemMenu(FEObject* Item)
{
	const FE_OBJECT_TYPE Type = Item->GetType();
	std::string FullPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
	if (FullPath.back() != '/')
		FullPath += '/';
	FullPath += Item->GetName();
	const bool bReadOnly = VIRTUAL_FILE_SYSTEM.IsReadOnly(Item, FullPath);

	// Header: icon, name and type of the item.
	const ImVec2 HeaderMin = ImGui::GetCursorScreenPos();
	const float HeaderWidth = ImGui::GetContentRegionAvail().x;
	ImGui::Dummy(ImVec2(1.0f, 44.0f));
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	FETexture* TypeIcon = GetItemTypeIcon(Item);
	if (TypeIcon != nullptr)
		DrawList->AddImage(TypeIcon->GetTextureID(), ImVec2(HeaderMin.x, HeaderMin.y + 1.0f), ImVec2(HeaderMin.x + 22.0f, HeaderMin.y + 23.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), GetItemIconColor(Type));
	DrawList->AddText(ImVec2(HeaderMin.x + 32.0f, HeaderMin.y), FEEditorStyle::Text, CutTextToWidth(Item->GetName(), HeaderWidth - 32.0f).c_str());
	DrawList->AddText(ImGui::GetFont(), 16.0f, ImVec2(HeaderMin.x + 32.0f, HeaderMin.y + 24.0f), FEEditorStyle::TextMuted, (GetItemTypeName(Item) + (bReadOnly ? ", read only" : "")).c_str());

	// Groups of items are separated, a group without items leaves no separator.
	bool bSeparatorNeeded = true;
	bool bGroupHasItems = false;
	const auto StartMenuItem = [&]() {
		if (bSeparatorNeeded)
			ImGui::Separator();

		bSeparatorNeeded = false;
		bGroupHasItems = true;
	};

	const auto MenuItem = [&](const char* Label, const char* Note = nullptr, const bool bEnabled = true) {
		StartMenuItem();
		return ImGui::MenuItem(Label, Note, false, bEnabled);
	};

	const auto EndMenuGroup = [&]() {
		if (bGroupHasItems)
			bSeparatorNeeded = true;

		bGroupHasItems = false;
	};

	// Main action is the same that double click does.
	if (Type == FE_NULL || Type == FE_TEXTURE)
	{
		if (MenuItem("Open"))
			OpenItem(Item);
	}
	else if (Type == FE_SCENE)
	{
		const bool bAlreadyOpened = EDITOR.GetEditorSceneWindow(Item->GetID()) != nullptr;
		if (MenuItem("Open", bAlreadyOpened ? "opened" : nullptr, !bAlreadyOpened))
			OpenItem(Item);
	}
	else if (Type == FE_SHADER || (!bReadOnly && (Type == FE_MATERIAL || Type == FE_GAMEMODEL || Type == FE_PREFAB)))
	{
		if (MenuItem("Edit"))
			OpenItem(Item);
	}
	else if (Type == FE_NATIVE_SCRIPT_MODULE && !bReadOnly)
	{
		FENativeScriptModule* NativeScriptModule = RESOURCE_MANAGER.GetNativeScriptModule(Item->GetID());
		if (NativeScriptModule == nullptr)
		{
			LOG.Add("FEEditorContentBrowserWindow::RenderItemMenu: Native script module is nullptr", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_WARNING);
		}
		else if (NativeScriptModule->GetProject() != nullptr && NativeScriptModule->GetProject()->IsVSProjectFolderValidAndIntact())
		{
			if (MenuItem("Open VS project"))
			{
				if (!NativeScriptModule->GetProject()->RunVSProject())
					LOG.Add("FEEditorContentBrowserWindow::RenderItemMenu: Failed to run Visual Studio project!", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_ERROR);
			}
		}
		else if (NativeScriptModule->GetProject()->HasRecoverableVSProjectData())
		{
			if (MenuItem("Recover VS project"))
			{
				std::string ProjectFolder = PROJECT_MANAGER.GetCurrent()->GetProjectFolder();
				NativeScriptModule->GetProject()->SetWorkingDirectory(ProjectFolder);
				if (!NativeScriptModule->GetProject()->RecreateVSProject())
					LOG.Add("FEEditorContentBrowserWindow::RenderItemMenu: Failed to regenerate Visual Studio project!", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_ERROR);
			}
		}
		else if (MenuItem("Create VS project"))
		{
			std::string ProjectFolder = PROJECT_MANAGER.GetCurrent()->GetProjectFolder();
			NativeScriptModule->GetProject()->SetWorkingDirectory(ProjectFolder);

			// First script is named after the module, without "Module" at the end, so "New NativeScriptModule" gives "NewNativeScript".
			std::string ModuleName = NativeScriptModule->GetName();
			const std::string ModuleSuffix = "Module";
			if (ModuleName.size() > ModuleSuffix.size() && ModuleName.compare(ModuleName.size() - ModuleSuffix.size(), ModuleSuffix.size(), ModuleSuffix) == 0)
				ModuleName.erase(ModuleName.size() - ModuleSuffix.size());

			std::string FirstScriptName = NATIVE_SCRIPT_SYSTEM.ConvertToCorrectScriptClassName(ModuleName);
			if (FirstScriptName.empty())
				FirstScriptName = "NewNativeScript";

			if (!NativeScriptModule->GetProject()->CreateNewVSProject(FirstScriptName))
				LOG.Add("FEEditorContentBrowserWindow::RenderItemMenu: Failed to create Visual Studio project!", "FE_EDITOR_CONTENT_BROWSER", FE_LOG_ERROR);
		}
	}

	if (!bReadOnly && MenuItem("Rename"))
		StartRename(ItemUnderMouse);

	if (Type == FE_SCENE && MenuItem("Duplicate"))
	{
		FEScene* NewScene = SCENE_MANAGER.DuplicateScene(Item->GetID(), "", [](FEEntity* EntityToCheck) {
			return !(EntityToCheck->GetTag() == EDITOR_RESOURCE_TAG);
		});

		if (NewScene != nullptr)
		{
			PROJECT_MANAGER.GetCurrent()->SetModified(true);
			VIRTUAL_FILE_SYSTEM.CreateFile(NewScene, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		}
	}
	EndMenuGroup();

	if (Type == FE_TEXTURE)
	{
		FETexture* Texture = RESOURCE_MANAGER.GetTexture(Item->GetID());
		if (MenuItem("Resize..."))
			ResizeTexturePopup::GetInstance().Show(Texture);

		if (MenuItem("Split channels into textures"))
		{
			const std::vector<FETexture*> NewTextures = RESOURCE_MANAGER.ChannelsToFETextures(Texture);

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

		if (MenuItem("Add transparency from mask..."))
		{
			TemporaryTexture = Texture;
			SELECT_FEOBJECT_POPUP.Show(FE_TEXTURE, AddTransparencyToTextureCallback);
		}

		if (MenuItem("Export as PNG..."))
		{
			// The dialog already adds ".png" when the user does not type an extension.
			FILE_SYSTEM.SaveFileDialog("", TEXTURE_LOAD_FILTER, [Texture](std::string FilePath, int ChosenFilterIndex) {
				if (FILE_SYSTEM.GetFileExtension(FilePath) != ".png")
					FilePath += ".png";

				return RESOURCE_MANAGER.ExportFETextureToPNG(Texture, FilePath);
			});
		}
	}
	else if (Type == FE_MESH)
	{
		StartMenuItem();
		if (ImGui::BeginMenu("Export"))
		{
			FEMesh* MeshToExport = RESOURCE_MANAGER.GetMesh(Item->GetID());
			if (ImGui::MenuItem("OBJ..."))
			{
				FILE_SYSTEM.SaveFileDialog("", OBJ_LOAD_FILTER, [MeshToExport](std::string FilePath, int ChosenFilterIndex) {
					if (FilePath.find(".obj") == std::string::npos)
						FilePath += ".obj";

					return RESOURCE_MANAGER.ExportFEMeshToOBJ(MeshToExport, FilePath);
				});
			}

			if (ImGui::MenuItem("PLY..."))
			{
				FILE_SYSTEM.SaveFileDialog("", PLY_FILTER, [MeshToExport](std::string FilePath, int ChosenFilterIndex) {
					if (FilePath.find(".ply") == std::string::npos)
						FilePath += ".ply";

					return RESOURCE_MANAGER.ExportFEMeshToPLY(MeshToExport, FilePath);
				});
			}

			RenderRegisteredExportMenuItems(MeshToExport);
			ImGui::EndMenu();
		}
	}
	else if (Type == FE_POINT_CLOUD)
	{
		FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(Item->GetID());

		StartMenuItem();
		if (ImGui::BeginMenu("Export"))
		{
			if (ImGui::MenuItem("PLY..."))
			{
				FILE_SYSTEM.SaveFileDialog("", PLY_FILTER, [PointCloud](std::string FilePath, int ChosenFilterIndex) {
					if (FilePath.find(".ply") == std::string::npos)
						FilePath += ".ply";

					return RESOURCE_MANAGER.ExportFEPointCloudToPLY(PointCloud, FilePath);
				});
			}

			RenderRegisteredExportMenuItems(PointCloud);
			ImGui::EndMenu();
		}

		bool bUseAdvancedRendering = PointCloud->IsAdvancedRenderingEnabled();
		if (ImGui::MenuItem("Advanced rendering", nullptr, &bUseAdvancedRendering))
			PointCloud->SetAdvancedRenderingEnabled(bUseAdvancedRendering);
	}
	else if (Type == FE_GAMEMODEL && !bReadOnly)
	{
		if (MenuItem("Create prefab from it"))
		{
			FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Item->GetID());
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
	else if (Type == FE_SCENE)
	{
		FEScene* StartingScene = SCENE_MANAGER.GetStartingScene();
		bool bIsSceneStarting = StartingScene != nullptr && StartingScene->GetID() == Item->GetID();

		StartMenuItem();
		if (ImGui::MenuItem("Starting scene", nullptr, &bIsSceneStarting))
		{
			if (bIsSceneStarting)
				SCENE_MANAGER.SetStartingScene(Item->GetID());
		}
	}
	else if (Type == FE_NATIVE_SCRIPT_MODULE && !bReadOnly)
	{
		FENativeScriptModule* NativeScriptModule = RESOURCE_MANAGER.GetNativeScriptModule(Item->GetID());
		if (NativeScriptModule != nullptr && MenuItem("Save module as..."))
		{
			FILE_SYSTEM.SaveFileDialog("", NATIVE_SCRIPT_MODULE_FILTER, [NativeScriptModule](std::string FilePath, int ChosenFilterIndex) {
				if (FILE_SYSTEM.GetFileExtension(FilePath) != ".nativescriptmodule")
					FilePath += ".nativescriptmodule";

				RESOURCE_MANAGER.SaveFENativeScriptModule(NativeScriptModule, FilePath);
				return true;
			});
		}
	}
	EndMenuGroup();

	if (!bReadOnly && CanBeDeleted(Type))
	{
		StartMenuItem();
		ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::Danger);
		const bool bDeletePressed = ImGui::MenuItem("Delete");
		ImGui::PopStyleColor();
		if (bDeletePressed)
		{
			if (Type == FE_NULL)
			{
				DeleteDirectoryPopup::GetInstance().Show(Item->GetName());
			}
			else if (Type == FE_MESH)
			{
				DeleteMeshPopup::GetInstance().Show(RESOURCE_MANAGER.GetMesh(Item->GetID()));
			}
			else if (Type == FE_POINT_CLOUD)
			{
				DeletePointCloudPopup::GetInstance().Show(RESOURCE_MANAGER.GetPointCloud(Item->GetID()));
			}
			else if (Type == FE_TEXTURE)
			{
				DeleteTexturePopup::GetInstance().Show(RESOURCE_MANAGER.GetTexture(Item->GetID()));
			}
			else if (Type == FE_MATERIAL)
			{
				DeleteMaterialPopup::GetInstance().Show(RESOURCE_MANAGER.GetMaterial(Item->GetID()));
			}
			else if (Type == FE_GAMEMODEL)
			{
				DeleteGameModelPopup::GetInstance().Show(RESOURCE_MANAGER.GetGameModel(Item->GetID()));
			}
			else if (Type == FE_PREFAB)
			{
				DeletePrefabPopup::GetInstance().Show(RESOURCE_MANAGER.GetPrefab(Item->GetID()));
			}
		}
	}
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
			DirectoryDragAndDropCallbackInfo CallbackInfo;

			CallbackInfo.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (VIRTUAL_FILE_SYSTEM.GetCurrentPath().back() != '/')
				CallbackInfo.DirectoryPath += "/";

			CallbackInfo.DirectoryPath += CurrentResource->GetName() + "/";
			DirectoryDragAndDropInfo[SubDirectoryIndex] = CallbackInfo;

			if (VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(CallbackInfo.DirectoryPath))
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

bool FEEditorContentBrowserWindow::IsFilteredListOutdated()
{
	// Names of objects can be changed outside of this window.
	if (!NameFilter.empty())
		return true;

	return NameFilter != LastUsedNameFilter ||
		   ObjectTypeFilters != LastUsedObjectTypeFilters ||
		   ObjectTagNegativeFilters != LastUsedObjectTagNegativeFilters ||
		   AllResourcesIDs != VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
}

void FEEditorContentBrowserWindow::UpdateFilterForResources()
{
	LastUsedNameFilter = NameFilter;
	LastUsedObjectTypeFilters = ObjectTypeFilters;
	LastUsedObjectTagNegativeFilters = ObjectTagNegativeFilters;

	FilteredResourcesIDs.clear();

	std::vector<FEUUID> TemporaryFilteredList;
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

bool FEEditorContentBrowserWindow::ShouldPassVisibilityFilter(const FEUUID& ObjectID)
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