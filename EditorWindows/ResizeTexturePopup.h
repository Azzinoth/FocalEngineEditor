#pragma once

#include "DeletePopups.h"

class ResizeTexturePopup : public ImGuiModalPopup
{
	FETexture* ObjectToWorkWith;
	int NewWidth;
	int NewHeight;
	int Filtration;
	bool bReplaceTexture = true;

	const int MinSize = 4;
	const int MaxSize = 8196;
public:
	SINGLETON_PUBLIC_PART(ResizeTexturePopup)

	void Show(FETexture* ObjectToWorkWith);
	void Render() override;
private:
	SINGLETON_PRIVATE_PART(ResizeTexturePopup)
};