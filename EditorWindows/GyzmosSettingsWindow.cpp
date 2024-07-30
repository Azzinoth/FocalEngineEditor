#include "GyzmosSettingsWindow.h"
#include "../FEEditor.h"

GyzmosSettingsWindow::GyzmosSettingsWindow()
{
	Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
}

GyzmosSettingsWindow::~GyzmosSettingsWindow()
{
	delete TransformationGizmoButton;
	delete ScaleGizmoButton;
	delete RotateGizmoButton;
}

void GyzmosSettingsWindow::Show()
{
	const std::string TempCaption = "##GizmosSettingsWindow";
	strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());
	Size = ImVec2(146.0f, 48.0f);
	Position = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2.0f - 150.0f / 2.0f, 10.0f);
	FEImGuiWindow::Show();

	if (TransformationGizmoButton == nullptr)
		TransformationGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.TransformationGizmoIcon);

	if (ScaleGizmoButton == nullptr)
		ScaleGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.ScaleGizmoIcon);

	if (RotateGizmoButton == nullptr)
		RotateGizmoButton = new ImGuiImageButton(GIZMO_MANAGER.RotateGizmoIcon);

	float CurrentX = 5.0f;
	TransformationGizmoButton->SetPosition(ImVec2(CurrentX, 5.0f));
	CurrentX += 32.0f + 16.0f;
	ScaleGizmoButton->SetPosition(ImVec2(CurrentX, 5.0f));
	CurrentX += 32.0f + 16.0f;
	RotateGizmoButton->SetPosition(ImVec2(CurrentX, 5.0f));
}

void GyzmosSettingsWindow::Render()
{
	if (!IsVisible())
		return;

	FEScene* CurrentScene = EDITOR.GetFocusedScene();
	if (CurrentScene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(CurrentScene->GetObjectID());
	if (GizmoSceneData == nullptr)
		return;

	FEImGuiWindow::Render();

	GizmoSceneData->GizmosState == TRANSFORM_GIZMOS ? SetSelectedStyle(TransformationGizmoButton) : SetDefaultStyle(TransformationGizmoButton);
	ImGui::PushID(0);
	TransformationGizmoButton->Render();
	if (TransformationGizmoButton->IsClicked())
	{
		if (GizmoSceneData->GizmosState != TRANSFORM_GIZMOS)
		{
			GIZMO_MANAGER.UpdateGizmoState(TRANSFORM_GIZMOS, CurrentScene);
			ImGui::PopID();
			FEImGuiWindow::OnRenderEnd();
			return;
		}
	}
	ImGui::PopID();
	ShowToolTip("Translate objects. key = shift");

	GizmoSceneData->GizmosState == SCALE_GIZMOS ? SetSelectedStyle(ScaleGizmoButton) : SetDefaultStyle(ScaleGizmoButton);
	ImGui::PushID(1);
	ScaleGizmoButton->Render();
	if (ScaleGizmoButton->IsClicked())
	{
		if (GizmoSceneData->GizmosState != SCALE_GIZMOS)
		{
			GIZMO_MANAGER.UpdateGizmoState(SCALE_GIZMOS, CurrentScene);
			ImGui::PopID();
			FEImGuiWindow::OnRenderEnd();
			return;
		}
	}
	ImGui::PopID();
	ShowToolTip("Scale objects. key = shift");

	GizmoSceneData->GizmosState == ROTATE_GIZMOS ? SetSelectedStyle(RotateGizmoButton) : SetDefaultStyle(RotateGizmoButton);
	ImGui::PushID(2);
	RotateGizmoButton->Render();
	if (RotateGizmoButton->IsClicked())
	{
		if (GizmoSceneData->GizmosState != ROTATE_GIZMOS)
		{
			GIZMO_MANAGER.UpdateGizmoState(ROTATE_GIZMOS, CurrentScene);
			ImGui::PopID();
			FEImGuiWindow::OnRenderEnd();
			return;
		}
	}
	ImGui::PopID();
	ShowToolTip("Rotate objects. key = shift");

	FEImGuiWindow::OnRenderEnd();
}