#pragma once

#include "ShaderEditorWindow.h"

class GizmosSettingsWindow : public FEImGuiWindow
{
public:
	GizmosSettingsWindow();

	void Show() override;
	void Render() override;
};
inline GizmosSettingsWindow GizmosSettingsWindowObject;