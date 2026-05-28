#pragma once

#include "PrefabEditorManager.h"

class TextureViewWindow : public FEImGuiWindow
{
	FETexture* TextureToView = nullptr;
	std::string SourceTextureObjectID;
	FETexture* TemporaryFlipbookTexture = nullptr;
	int FlipbookColumns = 0;
	int FlipbookRows = 0;
	bool bSourceIsVolumetric = false;
	bool bFlipbookConversionFailed = false;

	// If texture is not 8-bit, we should display range that is actually used in the texture, otherwise the image will look completely black or white.
	FETexture* DisplayTexture = nullptr;
	bool bUsesDisplayRange = false;
	float DataMinValue = 0.0f;
	float DataMaxValue = 1.0f;
	float DisplayMinValue = 0.0f;
	float DisplayMaxValue = 1.0f;

	bool bShowRed = true;
	bool bShowGreen = true;
	bool bShowBlue = true;
	bool bShowAlpha = false;

	float ZoomFactor = 1.0f;
	bool bFitToWindow = true;
	ImVec2 PanOffset = ImVec2(0.0f, 0.0f);

	static constexpr float ToolbarHeight = 36.0f;
	static constexpr float DetailsPanelWidth = 360.0f;
	static constexpr float MinimumZoom = 0.05f;
	static constexpr float MaximumZoom = 16.0f;

	void RenderToolbar();
	void RenderImageCanvas(const ImVec2& CanvasSize);
	void RenderDetailsPanel(const ImVec2& PanelSize);

	bool RenderChannelToggle(const char* Label, bool& Toggle, const ImVec4& ActiveColor);
	std::string ReadableSize(size_t Bytes) const;

	void BuildDisplayTexture(FETexture* RawDataSource);
	void RebuildDisplayTexture(FETexture* RawDataSource);
	FETexture* GetRawDataSourceTexture() const;

public:
	TextureViewWindow(FETexture* InTextureToView);
	~TextureViewWindow();

	void Render() override;
};