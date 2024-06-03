#pragma once

#include "DeletePopups.h"

class RenameFailedPopUp : public ImGuiModalPopup
{
	ImGuiButton* OkButton = nullptr;
public:
	SINGLETON_PUBLIC_PART(RenameFailedPopUp)
	void Render() override;
private:
	SINGLETON_PRIVATE_PART(RenameFailedPopUp)
};

class RenamePopUp : public ImGuiModalPopup
{
	FEObject* ObjToWorkWith;
	char NewName[512];
public:
	SINGLETON_PUBLIC_PART(RenamePopUp)

	void Show(FEObject* ObjToWorkWith);
	void Render() override;
private:
	SINGLETON_PRIVATE_PART(RenamePopUp)
};