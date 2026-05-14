#pragma once

#include "ShaderEditorWindow.h"

class GizmosSettingsWindow : public FEImGuiWindow
{
	ImGuiImageButton* TransformationGizmoButton = nullptr;
	ImGuiImageButton* ScaleGizmoButton = nullptr;
	ImGuiImageButton* RotateGizmoButton = nullptr;
public:
	GizmosSettingsWindow();
	~GizmosSettingsWindow();

	void Show() override;
	void Render() override;
};
inline GizmosSettingsWindow GizmosSettingsWindowObject;