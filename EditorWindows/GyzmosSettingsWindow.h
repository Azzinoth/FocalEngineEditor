#pragma once

#include "ShaderEditorWindow.h"

class GyzmosSettingsWindow : public FEImGuiWindow
{
	ImGuiImageButton* TransformationGizmoButton = nullptr;
	ImGuiImageButton* ScaleGizmoButton = nullptr;
	ImGuiImageButton* RotateGizmoButton = nullptr;
public:
	GyzmosSettingsWindow();
	~GyzmosSettingsWindow();

	void Show() override;
	void Render() override;
};
static GyzmosSettingsWindow GyzmosSettingsWindowObject;