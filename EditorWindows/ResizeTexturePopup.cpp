#include "ResizeTexturePopup.h"

ResizeTexturePopup::ResizeTexturePopup()
{
	PopupCaption = "Resize texture";
	ObjectToWorkWith = nullptr;
}

ResizeTexturePopup::~ResizeTexturePopup() {}

void ResizeTexturePopup::Show(FETexture* ObjectToWorkWith)
{
	if (ObjectToWorkWith == nullptr)
		return;
	bShouldOpen = true;
	this->ObjectToWorkWith = ObjectToWorkWith;

	NewWidth = ObjectToWorkWith->GetWidth();
	NewHeight = ObjectToWorkWith->GetHeight();
}

void ResizeTexturePopup::Render()
{
	ImGuiModalPopup::Render();

	const bool bOpened = BeginStyledPopup(PopupCaption.c_str());
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		ImGuiModalPopup::Close();
	}
	else if (bOpened)
	{
		RenderPopupHeading("Resize texture");
		RenderPopupCard(ObjectToWorkWith, ObjectToWorkWith->GetName(), { "Texture, " + std::to_string(ObjectToWorkWith->GetWidth()) + " x " + std::to_string(ObjectToWorkWith->GetHeight()) + " now" });

		const float LabelWidth = 110.0f;
		const float FieldWidth = 120.0f;
		RenderRowLabel("New size", LabelWidth);
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::DragInt("##Width", &NewWidth, 1.0f, MinSize, MaxSize);
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, FEEditorStyle::TextMuted);
		ImGui::TextUnformatted("x");
		ImGui::PopStyleColor();
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::DragInt("##Height", &NewHeight, 1.0f, MinSize, MaxSize);

		// A typed value is not limited by the field.
		NewWidth = std::clamp(NewWidth, MinSize, MaxSize);
		NewHeight = std::clamp(NewHeight, MinSize, MaxSize);

		RenderRowLabel("Result", LabelWidth);
		if (RenderSwitchButton("Replace this texture", bReplaceTexture, ImDrawFlags_RoundCornersLeft))
			bReplaceTexture = true;

		ImGui::SameLine(0.0f, 0.0f);
		if (RenderSwitchButton("New texture", !bReplaceTexture, ImDrawFlags_RoundCornersRight))
			bReplaceTexture = false;

		RenderPopupText("Pixels are taken from the nearest ones, there is no smoothing for now.");

		const int PressedButton = RenderPopupButtons({ { "Cancel", FEPopupButtonKind::Normal }, { "Resize", FEPopupButtonKind::Primary } });
		if (PressedButton == 1)
		{
			if (bReplaceTexture)
			{
				RESOURCE_MANAGER.ResizeTexture(ObjectToWorkWith, NewWidth, NewHeight, 0/*filtration*/);
				PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(ObjectToWorkWith);
			}
			else
			{
				unsigned char* Result = RESOURCE_MANAGER.ResizeTextureRawData(ObjectToWorkWith, NewWidth, NewHeight, 0/*filtration*/);
				FETexture* NewTexture = RESOURCE_MANAGER.RawDataToFETexture(Result, NewWidth, NewHeight);
				VIRTUAL_FILE_SYSTEM.CreateFile(NewTexture, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTexture);
				delete[] Result;
			}
		}

		if (PressedButton != -1)
		{
			ObjectToWorkWith = nullptr;
			ImGuiModalPopup::Close();
		}
	}
	EndStyledPopup(bOpened);
}