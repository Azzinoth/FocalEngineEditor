#include "RenamePopups.h"

RenameFailedPopUp* RenameFailedPopUp::Instance = nullptr;

RenameFailedPopUp::RenameFailedPopUp()
{
	PopupCaption = "Invalid name";
	OkButton = new ImGuiButton("OK");
	OkButton->SetDefaultColor(ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	OkButton->SetHoveredColor(ImVec4(0.95f, 0.90f, 0.0f, 1.0f));
	OkButton->SetPosition(ImVec2(0, 0));
}

RenameFailedPopUp::~RenameFailedPopUp()
{
	delete OkButton;
}

void RenameFailedPopUp::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text("Entered name is incorrect !");

		OkButton->SetPosition(ImVec2(ImGui::GetWindowWidth() / 2.0f - 120.0f / 2.0f, ImGui::GetCursorPosY() + 10.0f));
		OkButton->Render();
		if (OkButton->IsClicked())
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

RenamePopUp* RenamePopUp::Instance = nullptr;

RenamePopUp::RenamePopUp()
{
	PopupCaption = "Rename object";
	ObjToWorkWith = nullptr;
}

void RenamePopUp::Show(FEObject* ObjToWorkWith)
{
	bShouldOpen = true;
	this->ObjToWorkWith = ObjToWorkWith;
	strcpy_s(NewName, ObjToWorkWith->GetName().size() + 1, ObjToWorkWith->GetName().c_str());
}

void RenamePopUp::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		if (ObjToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGui::EndPopup();
			return;
		}

		ImGui::SetWindowPos(ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, APPLICATION.GetMainWindow()->GetHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text("New object name :");
		ImGui::InputText("##New object name", NewName, IM_ARRAYSIZE(NewName));

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
		if (ImGui::Button("Apply", ImVec2(120, 0)))
		{
			std::string OldName = ObjToWorkWith->GetName();
			// if new name is acceptable
			if (strlen(NewName) > 0)
			{
				ObjToWorkWith->SetDirtyFlag(true);
				PROJECT_MANAGER.GetCurrent()->SetModified(true);

				ObjToWorkWith->SetName(NewName);

				ImGuiModalPopup::Close();
				strcpy_s(NewName, "");
			}
			else
			{
				ObjToWorkWith = nullptr;
				ImGuiModalPopup::Close();
				RenameFailedPopUp::getInstance().Show();
			}
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ObjToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}
