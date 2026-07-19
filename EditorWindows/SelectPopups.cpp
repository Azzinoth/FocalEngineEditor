#include "SelectPopups.h"

bool SelectFEObjectPopUp::ControlButtonPressed = false;
bool SelectFEObjectPopUp::bOneObjectSelectonMode = true;

SelectFEObjectPopUp::SelectFEObjectPopUp()
{
	PopupCaption = "Select";
	IconButton = new ImGuiImageButton(nullptr);
	IconButton->SetSize(ImVec2(128, 128));
	IconButton->SetUV0(ImVec2(0.0f, 1.0f));
	IconButton->SetUV1(ImVec2(1.0f, 0.0f));
	IconButton->SetFramePadding(8);

	SelectButton = new ImGuiButton("Select");
	SelectButton->SetSize(ImVec2(140, 24));
	SelectButton->SetPosition(ImVec2(500, 35));
	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetSize(ImVec2(140, 24));
	CancelButton->SetPosition(ImVec2(660, 35));

	INPUT.AddKeyCallback(SelectFEObjectPopUp::KeyButtonCallback);
}

SelectFEObjectPopUp::~SelectFEObjectPopUp()
{
	delete SelectButton;
	delete CancelButton;
	delete IconButton;
}

void SelectFEObjectPopUp::FilterOutTags(std::vector<std::string>& FEObjectIDList, std::vector<std::string> ListOfTagsToFilterOut)
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
		std::vector<std::string> TemporaryList;

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
				TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.GetMeshByName("FEPlane")[0]->GetObjectID());

			if (!RESOURCE_MANAGER.GetMeshByName("FECube").empty())
				TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.GetMeshByName("FECube")[0]->GetObjectID());

			if (!RESOURCE_MANAGER.GetMeshByName("FESphere").empty())
				TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.GetMeshByName("FESphere")[0]->GetObjectID());
		}

		if (CurrentType == FE_TEXTURE)
			TemporaryList.insert(TemporaryList.begin(), RESOURCE_MANAGER.NoTexture->GetObjectID());

		if (CurrentType == FE_MATERIAL)
			TemporaryList.insert(TemporaryList.begin(), "18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);
		
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
			if (ItemsList[i]->GetObjectID() == HighlightedObject->GetObjectID())
			{
				AddToSelected(OBJECT_MANAGER.GetFEObject(ItemsList[i]->GetObjectID()));
				break;
			}
		}
	}
}

void SelectFEObjectPopUp::Render()
{
	ImGuiModalPopup::Render();

	ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(40);
		ImGui::Text("Filter: ");
		ImGui::SameLine();

		ImGui::SetCursorPosY(35);
		if (ImGui::InputText("##SelectFEObjectPopUpFilter", &FilterString))
		{
			if (FilterString.empty())
			{
				FilteredItemsList = ItemsList;
			}
			else
			{
				FilteredItemsList.clear();
				for (size_t i = 0; i < ItemsList.size(); i++)
				{
					if (ItemsList[i]->GetName().find(FilterString) != std::string::npos)
					{
						FilteredItemsList.push_back(ItemsList[i]);
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(80);
		ImGui::Columns(5, "selectPopupColumns", false);
		for (size_t i = 0; i < FilteredItemsList.size(); i++)
		{
			ImGui::PushID(FilteredItemsList[i]->GetObjectID().c_str());
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexUnderMouse != -1)
				{
					AddToSelected(OBJECT_MANAGER.GetFEObject(FilteredItemsList[IndexUnderMouse]->GetObjectID()));
					if (!ControlButtonPressed || bOneObjectSelectonMode && SelectedObjects.size() == 1)
					{
						OnSelectAction();
						Close();
					}
				}
			}

			IsSelected(FilteredItemsList[i]) ? SetSelectedStyle(IconButton) : SetDefaultStyle(IconButton);
			IconButton->SetTexture(PREVIEW_MANAGER.GetPreview(FilteredItemsList[i]));
			IconButton->Render();
			if (IconButton->IsClicked())
			{
				AddToSelected(OBJECT_MANAGER.GetFEObject(FilteredItemsList[i]->GetObjectID()));
			}

			if (ImGui::IsItemHovered())
			{
				std::string AdditionalTypeInfo;
				if (FilteredItemsList[i]->GetType() == FE_TEXTURE)
				{
					FETexture* CurrentTexture = RESOURCE_MANAGER.GetTexture(FilteredItemsList[i]->GetObjectID());
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

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("Name: " + FilteredItemsList[i]->GetName() +
										"\nType: " + FEObjectTypeToString(FilteredItemsList[i]->GetType()) +
										AdditionalTypeInfo
										).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			if (IconButton->IsHovered())
			{
				IndexUnderMouse = static_cast<int>(i);
			}

			ImGui::Text(FilteredItemsList[i]->GetName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		SelectButton->Render();
		if (SelectButton->IsClicked())
		{
			OnSelectAction();

			Close();
		}

		CancelButton->Render();
		if (CancelButton->IsClicked())
		{
			Close();
		}

		ImGui::EndPopup();
	}
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
	IndexUnderMouse = -1;
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