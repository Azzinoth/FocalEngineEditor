#pragma once

#include "DebugTextureViewWindow.h"

class ProjectWasModifiedPopUp : public ImGuiModalPopup
{
	FEProject* ObjToWorkWith;
	bool bShouldTerminate = false;
public:
	SINGLETON_PUBLIC_PART(ProjectWasModifiedPopUp)

	void Show(FEProject* Project, bool FullyCloseApplication);
	void Render() override;

private:
	SINGLETON_PRIVATE_PART(ProjectWasModifiedPopUp)
};