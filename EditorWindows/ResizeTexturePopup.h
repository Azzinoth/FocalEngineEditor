#pragma once

#include "RenamePopups.h"

class ResizeTexturePopup : public ImGuiModalPopup
{
	FETexture* ObjToWorkWith;
	int NewWidth;
	int NewHeight;
	int Filtration;
	bool bReplaceTexture = false;

	ImGuiButton* CancelButton;
	ImGuiButton* ApplyButton;
	ImVec2 PopupSize = ImVec2(450, 260);
public:
	SINGLETON_PUBLIC_PART(ResizeTexturePopup)

	void Show(FETexture* ObjToWorkWith);
	void Render() override;
private:
	SINGLETON_PRIVATE_PART(ResizeTexturePopup)
};