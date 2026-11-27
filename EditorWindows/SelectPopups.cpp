#include "SelectPopups.h"

bool SelectFEObjectPopUp::ControlButtonPressed = false;
bool SelectFEObjectPopUp::bOneObjectSelectonMode = true;

static std::string GetObjectTypeName(const FE_OBJECT_TYPE Type, const bool bPlural)
{
	switch (Type)
	{
		case FE_TEXTURE: return bPlural ? "textures" : "texture";
		case FE_MESH: return bPlural ? "meshes" : "mesh";
		case FE_MATERIAL: return bPlural ? "materials" : "material";
		case FE_GAMEMODEL: return bPlural ? "game models" : "game model";
		case FE_POINT_CLOUD: return bPlural ? "point clouds" : "point cloud";
		case FE_PREFAB: return bPlural ? "prefabs" : "prefab";
		default: return bPlural ? "objects" : "object";
	}
}

// Name of an object and short facts about it, by its type.
static std::string GetObjectToolTipText(FEObject* Object)
{
	std::string Result = Object->GetName();
	if (Object->GetType() == FE_TEXTURE)
	{
		FETexture* Texture = RESOURCE_MANAGER.GetTexture(Object->GetID());
		if (Texture == nullptr)
			return Result;

		Result += "\n" + std::to_string(Texture->GetWidth()) + " x " + std::to_string(Texture->GetHeight()) + " pixels";
		Result += "\nTexture type: ";
		switch (Texture->GetType())
		{
			case FE_TEXTURE_TYPE::FE_TEXTURE_1D:   Result += "1D"; break;
			case FE_TEXTURE_TYPE::FE_TEXTURE_2D:   Result += "2D"; break;
			case FE_TEXTURE_TYPE::FE_TEXTURE_3D:   Result += "3D"; break;
			case FE_TEXTURE_TYPE::FE_TEXTURE_CUBE: Result += "Cube"; break;
			default: break;
		}

		const std::string InternalFormatString = FETexture::TextureInternalFormatToString(Texture->GetInternalFormat());
		if (!InternalFormatString.empty())
			Result += ", " + InternalFormatString;
	}
	else if (Object->GetType() == FE_MESH)
	{
		FEMesh* Mesh = RESOURCE_MANAGER.GetMesh(Object->GetID());
		// Mesh is rendered as a list of triangles, with three indices or vertices for each one.
		if (Mesh != nullptr)
			Result += "\n" + GetNumberWithSeparators(Mesh->GetVertexCount() / 3) + " triangles";
	}
	else if (Object->GetType() == FE_MATERIAL)
	{
		FEMaterial* Material = RESOURCE_MANAGER.GetMaterial(Object->GetID());
		if (Material != nullptr && Material->Shader != nullptr)
			Result += "\nShader: " + Material->Shader->GetName();
	}
	else if (Object->GetType() == FE_GAMEMODEL)
	{
		FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Object->GetID());
		if (GameModel != nullptr && GameModel->GetMesh() != nullptr)
			Result += "\nMesh: " + GameModel->GetMesh()->GetName();

		if (GameModel != nullptr && GameModel->GetMaterial() != nullptr)
			Result += "\nMaterial: " + GameModel->GetMaterial()->GetName();
	}
	else if (Object->GetType() == FE_POINT_CLOUD)
	{
		FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(Object->GetID());
		if (PointCloud != nullptr)
			Result += "\n" + GetNumberWithSeparators(PointCloud->GetPointCount()) + " points";
	}
	else if (Object->GetType() == FE_PREFAB)
	{
		FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(Object->GetID());
		if (Prefab != nullptr && Prefab->GetScene() != nullptr)
			Result += "\n" + GetNumberWithSeparators(Prefab->GetScene()->GetEntityIDList().size()) + " entities";
	}

	return Result;
}

SelectFEObjectPopUp::SelectFEObjectPopUp()
{
	PopupCaption = "Select";
	INPUT.AddKeyCallback(SelectFEObjectPopUp::KeyButtonCallback);
}

SelectFEObjectPopUp::~SelectFEObjectPopUp() {}

void SelectFEObjectPopUp::FilterOutTags(std::vector<FEUUID>& FEObjectIDList, std::vector<std::string> ListOfTagsToFilterOut)
{
	for (int i = 0; i < FEObjectIDList.size(); i++)
	{
		FEObject* CurrentObject = OBJECT_MANAGER.GetFEObject(FEObjectIDList[i]);

		for (size_t j = 0; j < ListOfTagsToFilterOut.size(); j++)
		{
			if (CurrentObject->GetTag() == ListOfTagsToFilterOut[j])
			{
				FEObjectIDList.erase(FEObjectIDList.begin() + i);
				i--;
				break;
			}
		}
	}
}

void SelectFEObjectPopUp::Show(const FE_OBJECT_TYPE Type, void(*CallBack)(std::vector<FEObject*>), FEObject* HighlightedObject, const std::vector<FEObject*> CustomList)
{
	CurrentType = Type;
	if (CurrentType == FE_NULL)
		return;

	this->HighlightedObject = HighlightedObject;
	this->CallBack = CallBack;
	bShouldOpen = true;

	ItemsList.clear();

	if (CustomList.empty())
	{
		std::vector<FEUUID> TemporaryList;

		switch (CurrentType)
		{
			case FE_TEXTURE:
			{
				TemporaryList = RESOURCE_MANAGER.GetTextureIDList();
				break;
			}

			case FE_MESH:
			{
				TemporaryList = RESOURCE_MANAGER.GetMeshIDList();
				break;
			}
			
			case FE_MATERIAL:
			{
				TemporaryList = RESOURCE_MANAGER.GetMaterialIDList();
				break;
			}
			
			case FE_GAMEMODEL:
			{
				TemporaryList = RESOURCE_MANAGER.GetGameModelIDList();
				break;
			}

			case FE_POINT_CLOUD:
			{
				TemporaryList = RESOURCE_MANAGER.GetPointCloudIDList();
				break;
			}
			
			case FE_PREFAB:
			{
				TemporaryList = RESOURCE_MANAGER.GetPrefabIDList();
				break;
			}
		}

		FilterOutTags(TemporaryList, std::vector<std::string>{ ENGINE_RESOURCE_TAG, EDITOR_RESOURCE_TAG});

		if (CurrentType == FE_MESH)
		{
			if (!RESOURCE_MANAGER.GetMeshByName("FEPlane").empty())
				TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.GetMeshByName("FEPlane")[0]->GetID());

			if (!RESOURCE_MANAGER.GetMeshByName("FECube").empty())
				TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.GetMeshByName("FECube")[0]->GetID());

			if (!RESOURCE_MANAGER.GetMeshByName("FESphere").empty())
				TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.GetMeshByName("FESphere")[0]->GetID());
		}

		if (CurrentType == FE_TEXTURE)
			TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.NoTexture->GetID());

		if (CurrentType == FE_MATERIAL)
			TemporaryList.insert(TemporaryList.begin(), FEEngineResourceIDs::SolidColorMaterial);
		
		for (size_t i = 0; i < TemporaryList.size(); i++)
			ItemsList.push_back(OBJECT_MANAGER.GetFEObject(TemporaryList[i]));
	}
	else
	{
		ItemsList = CustomList;
	}

	FilteredItemsList = ItemsList;
	FilterString = "";

	SelectedObjects.clear();
	if (HighlightedObject != nullptr)
	{
		for (size_t i = 0; i < ItemsList.size(); i++)
		{
			if (ItemsList[i]->GetID() == HighlightedObject->GetID())
			{
				AddToSelected(OBJECT_MANAGER.GetFEObject(ItemsList[i]->GetID()));
				break;
			}
		}
	}
}

// Filter does not look at upper and lower case.
void SelectFEObjectPopUp::UpdateFilteredItems()
{
	if (FilterString.empty())
	{
		FilteredItemsList = ItemsList;
		return;
	}

	FilteredItemsList.clear();
	const std::string LowerCaseFilter = ToLowerCase(FilterString);
	for (size_t i = 0; i < ItemsList.size(); i++)
	{
		if (ToLowerCase(ItemsList[i]->GetName()).find(LowerCaseFilter) != std::string::npos)
			FilteredItemsList.push_back(ItemsList[i]);
	}
}

void SelectFEObjectPopUp::Render()
{
	ImGuiModalPopup::Render();

	const float Padding = 16.0f;
	const float Spacing = 10.0f;
	const int Columns = 6;
	const float ContentWidth = 890.0f;
	const float ItemsHeight = 586.0f;
	const float ItemsPadding = 8.0f;
	const float TilePadding = 6.0f;
	const float SmallFontSize = 16.0f;

	ImGui::SetNextWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Padding, Padding));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Spacing, Spacing));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, FEEditorStyle::Panel);
	ImGui::PushStyleColor(ImGuiCol_Border, FEEditorStyle::PopupBorder);
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, FEEditorStyle::Dim);
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		const ImVec2 Origin = ImGui::GetCursorScreenPos();

		// Header: what is selected, the filter and the number of items.
		ImGui::PushFont(nullptr, 26.0f);
		ImGui::TextUnformatted(("Select " + GetObjectTypeName(CurrentType, false)).c_str());
		ImGui::PopFont();

		const float FilterWidth = 300.0f;
		ImGui::SetCursorScreenPos(ImVec2(Origin.x + ContentWidth - FilterWidth, Origin.y));
		ImGui::SetNextItemWidth(FilterWidth);
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		if (ImGui::InputTextWithHint("##SelectFEObjectPopUpFilter", "Filter by name...", &FilterString))
			UpdateFilteredItems();

		std::string CountText = std::to_string(ItemsList.size()) + " " + GetObjectTypeName(CurrentType, ItemsList.size() != 1);
		if (FilteredItemsList.size() != ItemsList.size())
			CountText = std::to_string(FilteredItemsList.size()) + " of " + CountText;

		ImGui::SetCursorScreenPos(ImVec2(Origin.x, Origin.y + 36.0f));
		ImGui::PushFont(nullptr, SmallFontSize);
		ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
		ImGui::TextUnformatted(CountText.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();

		// Items: previews with names, six in a row.
		bool bSelectRequested = false;
		FEObject* ObjectUnderMouse = nullptr;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, FEEditorStyle::Background);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ItemsPadding, ItemsPadding));
		ImGui::BeginChild("##SelectFEObjectPopUpItems", ImVec2(ContentWidth, ItemsHeight), ImGuiChildFlags_AlwaysUseWindowPadding);
		ImDrawList* DrawList = ImGui::GetWindowDrawList();
		const ImVec2 ItemsOrigin = ImGui::GetCursorScreenPos();
		if (FilteredItemsList.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
			ImGui::TextUnformatted(ItemsList.empty() ? "There is nothing to select." : "Nothing has this text in its name.");
			ImGui::PopStyleColor();
		}

		const float ItemsWidth = ContentWidth - ItemsPadding * 2.0f - ImGui::GetStyle().ScrollbarSize;
		const float TileWidth = floorf((ItemsWidth - static_cast<float>(Columns - 1) * Spacing) / static_cast<float>(Columns));
		const float PreviewSize = TileWidth - TilePadding * 2.0f;
		const float TileHeight = TilePadding + PreviewSize + TilePadding + SmallFontSize + TilePadding;
		for (size_t i = 0; i < FilteredItemsList.size(); i++)
		{
			FEObject* Object = FilteredItemsList[i];
			const ImVec2 Min = ImVec2(ItemsOrigin.x + static_cast<float>(i % Columns) * (TileWidth + Spacing), ItemsOrigin.y + static_cast<float>(i / Columns) * (TileHeight + Spacing));
			const ImVec2 Max = ImVec2(Min.x + TileWidth, Min.y + TileHeight);

			ImGui::SetCursorScreenPos(Min);
			ImGui::PushID(UNIQUE_ID.ToString(Object->GetID()).c_str());
			const bool bPressed = ImGui::InvisibleButton("##Item", ImVec2(TileWidth, TileHeight));
			const bool bHovered = ImGui::IsItemHovered();
			ImGui::PopID();

			if (bHovered)
				ObjectUnderMouse = Object;

			if (bPressed)
				AddToSelected(Object);

			if (bHovered && ImGui::IsMouseDoubleClicked(0))
			{
				AddToSelected(Object);
				if (!ControlButtonPressed || bOneObjectSelectonMode && SelectedObjects.size() == 1)
					bSelectRequested = true;
			}

			const bool bSelected = IsSelected(Object);
			if (bSelected)
			{
				DrawList->AddRectFilled(Min, Max, FEEditorStyle::SelectedItem, 4.0f);
			}
			else if (bHovered)
			{
				DrawList->AddRectFilled(Min, Max, FEEditorStyle::CardHovered, 4.0f);
			}

			const ImVec2 PreviewMin = ImVec2(Min.x + TilePadding, Min.y + TilePadding);
			const ImVec2 PreviewMax = ImVec2(PreviewMin.x + PreviewSize, PreviewMin.y + PreviewSize);
			DrawList->AddImageRounded(PREVIEW_MANAGER.GetPreview(Object)->GetTextureID(), PreviewMin, PreviewMax, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), IM_COL32_WHITE, 4.0f);
			if (bSelected)
				DrawList->AddRect(PreviewMin, PreviewMax, FEEditorStyle::Accent, 4.0f, 2.0f);

			ImGui::PushFont(nullptr, SmallFontSize);
			const std::string ShownName = CutTextToWidth(Object->GetName(), TileWidth - 8.0f);
			DrawList->AddText(ImVec2(floorf(Min.x + (TileWidth - ImGui::CalcTextSize(ShownName.c_str()).x) / 2.0f), PreviewMax.y + TilePadding), bSelected ? FEEditorStyle::Text : FEEditorStyle::TextMuted, ShownName.c_str());
			ImGui::PopFont();
		}

		const size_t RowCount = (FilteredItemsList.size() + Columns - 1) / Columns;
		ImGui::SetCursorScreenPos(ImVec2(ItemsOrigin.x, ItemsOrigin.y + static_cast<float>(RowCount) * (TileHeight + Spacing)));
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();

		if (ObjectUnderMouse != nullptr)
			ImGui::SetTooltip("%s", GetObjectToolTipText(ObjectUnderMouse).c_str());

		// Footer: what is selected and the buttons.
		const ImVec2 FooterMin = ImGui::GetCursorScreenPos();
		const float ButtonWidth = 140.0f;
		const float ButtonHeight = ImGui::GetFrameHeight() + 8.0f;
		std::string SelectionText = "Nothing is selected";
		if (SelectedObjects.size() == 1)
		{
			SelectionText = SelectedObjects[0]->GetName();
		}
		else if (SelectedObjects.size() > 1)
		{
			SelectionText = std::to_string(SelectedObjects.size()) + " selected";
		}

		ImGui::SetCursorScreenPos(ImVec2(FooterMin.x, FooterMin.y + (ButtonHeight - ImGui::GetFontSize()) / 2.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, SelectedObjects.empty() ? FEEditorStyle::TextMuted : FEEditorStyle::Text);
		ImGui::TextUnformatted(CutTextToWidth(SelectionText, ContentWidth - ButtonWidth * 2.0f - 260.0f).c_str());
		ImGui::PopStyleColor();
		if (!bOneObjectSelectonMode)
		{
			ImGui::SameLine(0.0f, 16.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
			ImGui::PushFont(nullptr, SmallFontSize);
			ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
			ImGui::TextUnformatted("Every click adds one more.");
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}

		ImGui::SetCursorScreenPos(ImVec2(FooterMin.x + ContentWidth - ButtonWidth * 2.0f - Spacing, FooterMin.y));
		const bool bCancelRequested = ImGui::Button("Cancel", ImVec2(ButtonWidth, ButtonHeight)) || ImGui::IsKeyPressed(ImGuiKey_Escape);

		ImGui::SameLine(0.0f, Spacing);
		ImGui::BeginDisabled(SelectedObjects.empty());
		ImGui::PushStyleColor(ImGuiCol_Button, FEEditorStyle::PrimaryButton);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, FEEditorStyle::PrimaryButtonHovered);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, FEEditorStyle::PrimaryButtonHovered);
		if (ImGui::Button("Select", ImVec2(ButtonWidth, ButtonHeight)))
			bSelectRequested = true;
		ImGui::PopStyleColor(3);
		ImGui::EndDisabled();

		if (bSelectRequested && !SelectedObjects.empty())
		{
			OnSelectAction();
			Close();
		}
		else if (bCancelRequested)
		{
			Close();
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(4);
}

void SelectFEObjectPopUp::OnSelectAction()
{
	if (CallBack != nullptr)
	{
		CallBack(SelectedObjects);
		CallBack = nullptr;
	}
}

void SelectFEObjectPopUp::Close()
{
	ImGuiModalPopup::Close();
	HighlightedObject = nullptr;
	SelectedObjects.clear();
}

void SelectFEObjectPopUp::KeyButtonCallback(const int Key, int Scancode, const int Action, int Mods)
{
	if ((Key == GLFW_KEY_LEFT_CONTROL || Action == GLFW_KEY_RIGHT_CONTROL) && Action == GLFW_RELEASE)
	{
		ControlButtonPressed = false;
	}
	else if ((Key == GLFW_KEY_LEFT_CONTROL || Action == GLFW_KEY_RIGHT_CONTROL) && Action == GLFW_PRESS)
	{
		ControlButtonPressed = true;
	}
}

bool SelectFEObjectPopUp::IsSelected(const FEObject* Object) const
{
	for (size_t i = 0; i < SelectedObjects.size(); i++)
	{
		if (SelectedObjects[i] == Object)
			return true;
	}
	
	return false;
}

bool SelectFEObjectPopUp::IsOneObjectSelectonMode()
{
	return bOneObjectSelectonMode;
}

void SelectFEObjectPopUp::SetOneObjectSelectonMode(const bool NewValue)
{
	bOneObjectSelectonMode = NewValue;
	if (bOneObjectSelectonMode && SelectedObjects.size() > 1)
		SelectedObjects.resize(1);
}

void SelectFEObjectPopUp::AddToSelected(FEObject* Object)
{
	if (IsSelected(Object))
		return;

	if (bOneObjectSelectonMode && SelectedObjects.size() == 1)
	{
		SelectedObjects[0] = Object;
	}
	else
	{
		SelectedObjects.push_back(Object);
	}
}