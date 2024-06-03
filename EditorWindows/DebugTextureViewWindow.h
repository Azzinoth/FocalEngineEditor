#pragma once

#include "PrefabEditorWindow.h"

class DebugTextureViewWindow : public FEImGuiWindow
{
	ImGuiButton* CloseButton = nullptr;
	std::function<FETexture* ()> TextureToView;
public:
	DebugTextureViewWindow(std::function<FETexture* ()> Func);
	~DebugTextureViewWindow();

	void Render() override;
};