#include "ProjectWasModifiedPopUp.h"
#include "../FEEditor.h"

ProjectWasModifiedPopUp::ProjectWasModifiedPopUp()
{
	PopupCaption = "Save project ?";
	ObjectToWorkWith = nullptr;
}

void ProjectWasModifiedPopUp::Show(FEProject* Project, const bool FullyCloseApplication)
{
	bShouldOpen = true;
	ObjectToWorkWith = Project;
	bShouldTerminate = FullyCloseApplication;
}

void ProjectWasModifiedPopUp::Render()
{
	ImGuiModalPopup::Render();

	bool bCloseProject = false;
	const bool bOpened = BeginStyledPopup(PopupCaption.c_str());
	if (bOpened && ObjectToWorkWith == nullptr)
	{
		EndStyledPopup(bOpened);
		APPLICATION.GetMainWindow()->Terminate();
		return;
	}

	if (bOpened)
	{
		RenderPopupHeading("Save changes before closing?");
		RenderPopupCard(nullptr, ObjectToWorkWith->GetName(), { ObjectToWorkWith->GetProjectFolder(), "Has changes that are not saved." });
		RenderPopupText("They are lost if the project is closed without saving.");

		const int PressedButton = RenderPopupButtons({ { "Cancel", FEPopupButtonKind::Normal }, { "Don't save", FEPopupButtonKind::DangerText }, { "Save", FEPopupButtonKind::Primary } });
		if (PressedButton == 2)
			PROJECT_MANAGER.GetCurrent()->SaveProject();

		if (PressedButton == 1 || PressedButton == 2)
		{
			ObjectToWorkWith = nullptr;
			bCloseProject = true;
		}

		if (PressedButton == 0 || PressedButton == 2)
			APPLICATION.GetMainWindow()->CancelClose();

		if (PressedButton != -1)
			ImGuiModalPopup::Close();
	}
	EndStyledPopup(bOpened);

	if (bCloseProject)
	{
		PROJECT_MANAGER.CloseCurrentProject();
		if (bShouldTerminate)
		{
			ENGINE.Terminate();
			return;
		}
		else if (EDITOR.IsRestartNeededForProject(""))
		{
			EDITOR.RestartWithProject("");
		}
	}
}