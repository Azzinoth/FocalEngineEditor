#include "TextureViewWindow.h"
#include "../SubSystems/ProjectManagment/FEProjectManager.h"

namespace
{
	struct ChannelMappingData
	{
		GLuint TextureID;
		GLint Mapping[4];
	};

	void ApplyChannelMappingCallback(const ImDrawList* ParentList, const ImDrawCmd* Command)
	{
		const ChannelMappingData* Data = static_cast<const ChannelMappingData*>(Command->UserCallbackData);

		GLint PreviouslyBound = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &PreviouslyBound);

		glBindTexture(GL_TEXTURE_2D, Data->TextureID);
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, Data->Mapping);
		glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(PreviouslyBound));
	}

	bool FormatHasAlphaChannel(GLint InternalFormat)
	{
		switch (InternalFormat)
		{
			case GL_RGBA:
			case GL_RGBA16F:
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
				return true;
			default:
				return false;
		}
	}

	bool FormatIsSingleChannel(GLint InternalFormat)
	{
		switch (InternalFormat)
		{
			case GL_RED:
			case GL_R8:
			case GL_R16:
			case GL_R16F:
			case GL_R32F:
				return true;
			default:
				return false;
		}
	}

	void ComputeChannelMapping(GLint Out[4], bool ShowRed, bool ShowGreen, bool ShowBlue, bool ShowAlpha)
	{
		const int RGBCount = (ShowRed ? 1 : 0) + (ShowGreen ? 1 : 0) + (ShowBlue ? 1 : 0);
		const bool OnlyAlpha = ShowAlpha && !ShowRed && !ShowGreen && !ShowBlue;

		if (OnlyAlpha)
		{
			Out[0] = GL_ALPHA;
			Out[1] = GL_ALPHA;
			Out[2] = GL_ALPHA;
			Out[3] = GL_ONE;
			return;
		}

		if (RGBCount == 1 && !ShowAlpha)
		{
			const GLint SingleChannel = ShowRed ? GL_RED : (ShowGreen ? GL_GREEN : GL_BLUE);
			Out[0] = SingleChannel;
			Out[1] = SingleChannel;
			Out[2] = SingleChannel;
			Out[3] = GL_ONE;
			return;
		}

		Out[0] = ShowRed ? GL_RED : GL_ZERO;
		Out[1] = ShowGreen ? GL_GREEN : GL_ZERO;
		Out[2] = ShowBlue ? GL_BLUE : GL_ZERO;
		Out[3] = ShowAlpha ? GL_ALPHA : GL_ONE;
	}
}

TextureViewWindow::TextureViewWindow(FETexture* InTextureToView)
{
	// 3D (volumetric) textures are viewed in VolumetricTextureViewWindow, not here.
	if (InTextureToView != nullptr && InTextureToView->GetType() == FE_TEXTURE_TYPE::FE_TEXTURE_3D)
	{
		LOG.Add("TextureViewWindow does not support 3D textures; use VolumetricTextureViewWindow instead.", "FE_LOG_GENERAL", FE_LOG_WARNING);
		InTextureToView = nullptr;
	}

	if (InTextureToView != nullptr)
	{
		SourceTextureObjectID = InTextureToView->GetObjectID();
		const GLint SourceInternalFormat = InTextureToView->GetInternalFormat();
		bShowAlpha = FormatHasAlphaChannel(SourceInternalFormat);
		if (FormatIsSingleChannel(SourceInternalFormat))
		{
			bShowGreen = false;
			bShowBlue = false;
		}

		TextureToView = InTextureToView;

		if (SourceInternalFormat == GL_R16 || SourceInternalFormat == GL_R32F)
		{
			BuildDisplayTexture(TextureToView);
			if (DisplayTexture != nullptr)
			{
				bUsesDisplayRange = true;
				TextureToView = DisplayTexture;
			}
		}
	}

	std::string TemporaryCaption = InTextureToView != nullptr ? InTextureToView->GetName() : std::string("Texture View");
	strcpy_s(Caption, TemporaryCaption.size() + 1, TemporaryCaption.c_str());

	Size = ImVec2(1100, 720);
	Position = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2 - Size.x / 2, APPLICATION.GetMainWindow()->GetHeight() / 2 - Size.y / 2);

	Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
	bHaveCloseButton = true;
}

TextureViewWindow::~TextureViewWindow()
{
	if (DisplayTexture != nullptr)
	{
		RESOURCE_MANAGER.DeleteFETexture(DisplayTexture);
		DisplayTexture = nullptr;
	}
}

FETexture* TextureViewWindow::GetRawDataSourceTexture() const
{
	return RESOURCE_MANAGER.GetTexture(SourceTextureObjectID);
}

void TextureViewWindow::BuildDisplayTexture(FETexture* RawDataSource)
{
	if (RawDataSource == nullptr)
		return;

	const GLint RawFormat = RawDataSource->GetInternalFormat();
	if (RawFormat != GL_R16 && RawFormat != GL_R32F)
		return;

	size_t RawSize = 0;
	unsigned char* RawData = RawDataSource->GetRawData(&RawSize);
	if (RawData == nullptr || RawSize == 0)
		return;

	const int Width = RawDataSource->GetWidth();
	const int Height = RawDataSource->GetHeight();
	const size_t PixelCount = static_cast<size_t>(Width) * static_cast<size_t>(Height);

	float MinFound = std::numeric_limits<float>::infinity();
	float MaxFound = -std::numeric_limits<float>::infinity();

	if (RawFormat == GL_R16)
	{
		const unsigned short* Pixels = reinterpret_cast<const unsigned short*>(RawData);
		for (size_t i = 0; i < PixelCount; i++)
		{
			const float Value = static_cast<float>(Pixels[i]);
			if (Value < MinFound)
				MinFound = Value;
			if (Value > MaxFound)
				MaxFound = Value;
		}
	}
	else
	{
		const float* Pixels = reinterpret_cast<const float*>(RawData);
		for (size_t i = 0; i < PixelCount; i++)
		{
			const float Value = Pixels[i];
			if (Value < MinFound)
				MinFound = Value;
			if (Value > MaxFound)
				MaxFound = Value;
		}
	}

	if (!std::isfinite(MinFound) || !std::isfinite(MaxFound) || MaxFound <= MinFound)
	{
		MinFound = 0.0f;
		MaxFound = (RawFormat == GL_R16) ? 65535.0f : 1.0f;
	}

	DataMinValue = MinFound;
	DataMaxValue = MaxFound;
	DisplayMinValue = MinFound;
	DisplayMaxValue = MaxFound;

	unsigned char* DisplayData = new unsigned char[PixelCount];
	const float Range = std::max(DisplayMaxValue - DisplayMinValue, 1e-6f);

	if (RawFormat == GL_R16)
	{
		const unsigned short* Pixels = reinterpret_cast<const unsigned short*>(RawData);
		for (size_t i = 0; i < PixelCount; i++)
		{
			const float Normalized = (static_cast<float>(Pixels[i]) - DisplayMinValue) / Range;
			const float Clamped = std::max(0.0f, std::min(1.0f, Normalized));
			DisplayData[i] = static_cast<unsigned char>(Clamped * 255.0f + 0.5f);
		}
	}
	else
	{
		const float* Pixels = reinterpret_cast<const float*>(RawData);
		for (size_t i = 0; i < PixelCount; i++)
		{
			const float Normalized = (Pixels[i] - DisplayMinValue) / Range;
			const float Clamped = std::max(0.0f, std::min(1.0f, Normalized));
			DisplayData[i] = static_cast<unsigned char>(Clamped * 255.0f + 0.5f);
		}
	}

	delete[] RawData;

	DisplayTexture = RESOURCE_MANAGER.RawDataToFETexture(DisplayData, Width, Height, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
	if (DisplayTexture != nullptr)
	{
		DisplayTexture->SetName(RawDataSource->GetName() + "_DisplayCopy");
		DisplayTexture->SetFilterType(RawDataSource->GetFilterType());
	}

	delete[] DisplayData;
}

void TextureViewWindow::RebuildDisplayTexture(FETexture* RawDataSource)
{
	if (RawDataSource == nullptr || DisplayTexture == nullptr)
		return;

	const GLint RawFormat = RawDataSource->GetInternalFormat();
	if (RawFormat != GL_R16 && RawFormat != GL_R32F)
		return;

	size_t RawSize = 0;
	unsigned char* RawData = RawDataSource->GetRawData(&RawSize);
	if (RawData == nullptr || RawSize == 0)
		return;

	const int Width = RawDataSource->GetWidth();
	const int Height = RawDataSource->GetHeight();
	const size_t PixelCount = static_cast<size_t>(Width) * static_cast<size_t>(Height);

	unsigned char* DisplayData = new unsigned char[PixelCount];
	const float Range = std::max(DisplayMaxValue - DisplayMinValue, 1e-6f);

	if (RawFormat == GL_R16)
	{
		const unsigned short* Pixels = reinterpret_cast<const unsigned short*>(RawData);
		for (size_t i = 0; i < PixelCount; i++)
		{
			const float Normalized = (static_cast<float>(Pixels[i]) - DisplayMinValue) / Range;
			const float Clamped = std::max(0.0f, std::min(1.0f, Normalized));
			DisplayData[i] = static_cast<unsigned char>(Clamped * 255.0f + 0.5f);
		}
	}
	else
	{
		const float* Pixels = reinterpret_cast<const float*>(RawData);
		for (size_t i = 0; i < PixelCount; i++)
		{
			const float Normalized = (Pixels[i] - DisplayMinValue) / Range;
			const float Clamped = std::max(0.0f, std::min(1.0f, Normalized));
			DisplayData[i] = static_cast<unsigned char>(Clamped * 255.0f + 0.5f);
		}
	}

	delete[] RawData;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, DisplayTexture->GetTextureID());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_RED, GL_UNSIGNED_BYTE, DisplayData);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	delete[] DisplayData;
}

bool TextureViewWindow::RenderChannelToggle(const char* Label, bool& Toggle, const ImVec4& ActiveColor)
{
	const ImVec4 InactiveColor = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
	const ImVec4 ButtonColor = Toggle ? ActiveColor : InactiveColor;
	const ImVec4 HoverColor = ImVec4(ButtonColor.x + 0.15f, ButtonColor.y + 0.15f, ButtonColor.z + 0.15f, 1.0f);

	ImGui::PushStyleColor(ImGuiCol_Button, ButtonColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HoverColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ButtonColor);
	ImGui::PushStyleColor(ImGuiCol_Text, Toggle ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.85f, 0.85f, 0.85f, 1.0f));

	const bool bClicked = ImGui::Button(Label, ImVec2(28.0f, 24.0f));
	if (bClicked)
		Toggle = !Toggle;

	ImGui::PopStyleColor(4);
	return bClicked;
}

std::string TextureViewWindow::ReadableSize(size_t Bytes)
{
	const double KB = 1024.0;
	const double MB = KB * 1024.0;
	char Buffer[64];

	if (Bytes >= static_cast<size_t>(MB))
	{
		snprintf(Buffer, sizeof(Buffer), "%.2f MB", Bytes / MB);
		return std::string(Buffer);
	}

	if (Bytes >= static_cast<size_t>(KB))
	{
		snprintf(Buffer, sizeof(Buffer), "%.2f KB", Bytes / KB);
		return std::string(Buffer);
	}

	snprintf(Buffer, sizeof(Buffer), "%zu B", Bytes);
	return std::string(Buffer);
}

size_t TextureViewWindow::EstimateBytesPerPixel(GLint InternalFormat)
{
	switch (InternalFormat)
	{
		case GL_RED: return 1;
		case GL_R16: return 2;
		case GL_RG16F: return 4;
		case GL_RGB: return 3;
		case GL_RGBA: return 4;
		case GL_RGBA16F: return 8;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return 1;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return 1;
		case GL_DEPTH_COMPONENT32: return 4;
		case GL_DEPTH24_STENCIL8: return 4;
		default: return 4;
	}
}

void TextureViewWindow::RenderToolbar()
{
	ImGui::BeginChild("##TextureViewToolbar", ImVec2(0.0f, ToolbarHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::AlignTextToFramePadding();

	RenderChannelToggle("R", bShowRed, ImVec4(0.95f, 0.25f, 0.25f, 1.0f));
	ImGui::SameLine(0.0f, 4.0f);
	RenderChannelToggle("G", bShowGreen, ImVec4(0.30f, 0.85f, 0.30f, 1.0f));
	ImGui::SameLine(0.0f, 4.0f);
	RenderChannelToggle("B", bShowBlue, ImVec4(0.30f, 0.55f, 0.95f, 1.0f));
	ImGui::SameLine(0.0f, 4.0f);
	RenderChannelToggle("A", bShowAlpha, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));

	ImGui::SameLine(0.0f, 24.0f);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Zoom");
	ImGui::SameLine();

	ImGui::SetNextItemWidth(180.0f);
	int ZoomPercent = static_cast<int>(ZoomFactor * 100.0f + 0.5f);
	if (ImGui::SliderInt("##ZoomSlider", &ZoomPercent, static_cast<int>(MinimumZoom * 100.0f), static_cast<int>(MaximumZoom * 100.0f), "%d%%"))
	{
		ZoomFactor = ZoomPercent / 100.0f;
		bFitToWindow = false;
	}

	ImGui::SameLine();
	if (ImGui::Button("-", ImVec2(24.0f, 24.0f)))
	{
		ZoomFactor = std::max(MinimumZoom, ZoomFactor * 0.8f);
		bFitToWindow = false;
	}

	ImGui::SameLine(0.0f, 4.0f);
	if (ImGui::Button("+", ImVec2(24.0f, 24.0f)))
	{
		ZoomFactor = std::min(MaximumZoom, ZoomFactor * 1.25f);
		bFitToWindow = false;
	}

	ImGui::SameLine(0.0f, 4.0f);
	if (ImGui::Button("Fit", ImVec2(40.0f, 24.0f)))
	{
		bFitToWindow = true;
		PanOffset = ImVec2(0.0f, 0.0f);
	}

	ImGui::SameLine(0.0f, 4.0f);
	if (ImGui::Button("100%", ImVec2(54.0f, 24.0f)))
	{
		ZoomFactor = 1.0f;
		bFitToWindow = false;
		PanOffset = ImVec2(0.0f, 0.0f);
	}

	if (bUsesDisplayRange)
	{
		ImGui::SameLine(0.0f, 24.0f);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Range");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(220.0f);
		float SliderSpeed = std::max((DataMaxValue - DataMinValue) / 1000.0f, 1e-4f);
		ImGui::DragFloatRange2("##DisplayRange", &DisplayMinValue, &DisplayMaxValue, SliderSpeed, DataMinValue, DataMaxValue, "Min: %.1f", "Max: %.1f");
		const bool bRangeReleased = ImGui::IsItemDeactivatedAfterEdit();

		ImGui::SameLine(0.0f, 4.0f);
		bool bRequestRebuild = bRangeReleased;
		if (ImGui::Button("Auto", ImVec2(48.0f, 24.0f)))
		{
			DisplayMinValue = DataMinValue;
			DisplayMaxValue = DataMaxValue;
			bRequestRebuild = true;
		}

		if (bRequestRebuild)
			RebuildDisplayTexture(GetRawDataSourceTexture());
	}

	ImGui::EndChild();
}

void TextureViewWindow::RenderImageCanvas(const ImVec2& CanvasSize)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
	ImGui::BeginChild("##TextureViewCanvas", CanvasSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	if (TextureToView == nullptr)
	{
		const char* Message = "No texture available.";
		const ImVec2 InnerSizeEmpty = ImGui::GetContentRegionAvail();
		const ImVec2 TextSize = ImGui::CalcTextSize(Message);
		ImGui::SetCursorPos(ImVec2((InnerSizeEmpty.x - TextSize.x) * 0.5f, (InnerSizeEmpty.y - TextSize.y) * 0.5f));
		ImGui::TextColored(ImVec4(0.95f, 0.5f, 0.5f, 1.0f), "%s", Message);
		ImGui::EndChild();
		ImGui::PopStyleColor();
		return;
	}

	const ImVec2 InnerSize = ImGui::GetContentRegionAvail();
	const float TextureWidth = static_cast<float>(TextureToView->GetWidth());
	const float TextureHeight = static_cast<float>(TextureToView->GetHeight());

	if (bFitToWindow && TextureWidth > 0.0f && TextureHeight > 0.0f)
	{
		const float FitScaleX = InnerSize.x / TextureWidth;
		const float FitScaleY = InnerSize.y / TextureHeight;
		ZoomFactor = std::max(MinimumZoom, std::min(FitScaleX, FitScaleY));
	}

	const ImVec2 DrawSize = ImVec2(TextureWidth * ZoomFactor, TextureHeight * ZoomFactor);
	const ImVec2 CanvasMin = ImGui::GetCursorScreenPos();
	const ImVec2 CenterOffset = ImVec2((InnerSize.x - DrawSize.x) * 0.5f, (InnerSize.y - DrawSize.y) * 0.5f);
	const ImVec2 ImageMin = ImVec2(CanvasMin.x + CenterOffset.x + PanOffset.x, CanvasMin.y + CenterOffset.y + PanOffset.y);
	const ImVec2 ImageMax = ImVec2(ImageMin.x + DrawSize.x, ImageMin.y + DrawSize.y);

	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	const ImU32 CheckerColorA = IM_COL32(80, 80, 80, 255);
	const ImU32 CheckerColorB = IM_COL32(120, 120, 120, 255);
	const float CheckerSize = 12.0f;
	const ImVec2 CheckerStart = DrawList->GetClipRectMin();
	const ImVec2 CheckerEnd = DrawList->GetClipRectMax();
	for (float Y = CheckerStart.y; Y < CheckerEnd.y; Y += CheckerSize)
	{
		for (float X = CheckerStart.x; X < CheckerEnd.x; X += CheckerSize)
		{
			const int CellX = static_cast<int>((X - CheckerStart.x) / CheckerSize);
			const int CellY = static_cast<int>((Y - CheckerStart.y) / CheckerSize);
			const ImU32 CellColor = ((CellX + CellY) & 1) == 0 ? CheckerColorA : CheckerColorB;
			const ImVec2 CellEnd = ImVec2(std::min(X + CheckerSize, CheckerEnd.x), std::min(Y + CheckerSize, CheckerEnd.y));
			DrawList->AddRectFilled(ImVec2(X, Y), CellEnd, CellColor);
		}
	}

	GLuint TextureID = TextureToView->GetTextureID();
	ChannelMappingData SetMapping;
	SetMapping.TextureID = TextureID;
	ComputeChannelMapping(SetMapping.Mapping, bShowRed, bShowGreen, bShowBlue, bShowAlpha);

	ChannelMappingData ResetMapping;
	ResetMapping.TextureID = TextureID;
	ResetMapping.Mapping[0] = GL_RED;
	ResetMapping.Mapping[1] = GL_GREEN;
	ResetMapping.Mapping[2] = GL_BLUE;
	ResetMapping.Mapping[3] = GL_ALPHA;

	DrawList->AddCallback(ApplyChannelMappingCallback, &SetMapping, sizeof(ChannelMappingData));
	DrawList->AddImage(static_cast<ImTextureID>(TextureID), ImageMin, ImageMax, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
	DrawList->AddCallback(ApplyChannelMappingCallback, &ResetMapping, sizeof(ChannelMappingData));

	ImGui::InvisibleButton("##CanvasInteract", InnerSize);
	const bool bCanvasHovered = ImGui::IsItemHovered();
	const bool bCanvasActive = ImGui::IsItemActive();

	if (bCanvasHovered)
	{
		const float WheelDelta = ImGui::GetIO().MouseWheel;
		if (WheelDelta != 0.0f)
		{
			const float ZoomStep = WheelDelta > 0.0f ? 1.1f : (1.0f / 1.1f);
			ZoomFactor = std::max(MinimumZoom, std::min(MaximumZoom, ZoomFactor * ZoomStep));
			bFitToWindow = false;
		}
	}

	if (bCanvasActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
	{
		const ImVec2 Delta = ImGui::GetIO().MouseDelta;
		PanOffset.x += Delta.x;
		PanOffset.y += Delta.y;
		bFitToWindow = false;
	}

	ImGui::EndChild();
	ImGui::PopStyleColor();
}

void TextureViewWindow::RenderDetailsPanel(const ImVec2& PanelSize)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f, 0.16f, 0.16f, 1.0f));
	ImGui::BeginChild("##TextureViewDetails", PanelSize, true);

	FETexture* SourceTexture = RESOURCE_MANAGER.GetTexture(SourceTextureObjectID);
	if (SourceTexture == nullptr && TextureToView != nullptr)
		SourceTexture = TextureToView;
	if (SourceTexture == nullptr)
	{
		ImGui::EndChild();
		ImGui::PopStyleColor();
		return;
	}

	const int Width = SourceTexture->GetWidth();
	const int Height = SourceTexture->GetHeight();
	const GLint InternalFormat = SourceTexture->GetInternalFormat();
	const std::string ResolvedFormatString = FETexture::TextureInternalFormatToString(InternalFormat);
	const std::string FormatString = ResolvedFormatString.empty() ? ("Unknown (" + std::to_string(InternalFormat) + ")") : ResolvedFormatString;
	const size_t ResourceSizeBytes = static_cast<size_t>(Width) * static_cast<size_t>(Height) * EstimateBytesPerPixel(InternalFormat);

	std::string ChannelsString;
	if (bShowRed)
		ChannelsString += "R";
	if (bShowGreen)
		ChannelsString += "G";
	if (bShowBlue)
		ChannelsString += "B";
	if (bShowAlpha)
		ChannelsString += "A";
	if (ChannelsString.empty())
		ChannelsString = "(none)";

	auto DrawSectionHeader = [](const char* Title)
	{
		ImGui::TextColored(ImVec4(0.95f, 0.90f, 0.0f, 1.0f), "%s", Title);
		ImGui::Separator();
		ImGui::Spacing();
	};

	auto DrawRow = [](const char* Label, const std::string& Value)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "%s", Label);
		ImGui::TableSetColumnIndex(1);
		ImGui::TextWrapped("%s", Value.c_str());
	};

	const char* AllLabels[] = {
		"Name:", "Object ID:", "Dimensions:", "Format:",
		"Resource size:", "Data range:", "Display range:", "Zoom:", "Channels:"
	};
	float LabelColumnWidth = 0.0f;
	for (const char* CurrentLabel : AllLabels)
	{
		const float LabelWidth = ImGui::CalcTextSize(CurrentLabel).x;
		if (LabelWidth > LabelColumnWidth)
			LabelColumnWidth = LabelWidth;
	}
	LabelColumnWidth += ImGui::GetStyle().CellPadding.x * 2.0f + 8.0f;

	const ImGuiTableFlags TableFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoBordersInBody;

	DrawSectionHeader("Details");
	if (ImGui::BeginTable("##DetailsTable", 2, TableFlags))
	{
		ImGui::TableSetupColumn("##Label", ImGuiTableColumnFlags_WidthFixed, LabelColumnWidth);
		ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);

		DrawRow("Name:", SourceTexture->GetName());
		DrawRow("Object ID:", SourceTexture->GetObjectID());

		DrawRow("Dimensions:", std::to_string(Width) + " x " + std::to_string(Height));
		DrawRow("Format:", FormatString);
		DrawRow("Resource size:", ReadableSize(ResourceSizeBytes));

		if (bUsesDisplayRange)
		{
			char RangeBuffer[64];
			snprintf(RangeBuffer, sizeof(RangeBuffer), "%.1f - %.1f", DataMinValue, DataMaxValue);
			DrawRow("Data range:", RangeBuffer);
			snprintf(RangeBuffer, sizeof(RangeBuffer), "%.1f - %.1f", DisplayMinValue, DisplayMaxValue);
			DrawRow("Display range:", RangeBuffer);
		}

		ImGui::EndTable();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	DrawSectionHeader("View");
	if (ImGui::BeginTable("##ViewTable", 2, TableFlags))
	{
		ImGui::TableSetupColumn("##Label", ImGuiTableColumnFlags_WidthFixed, LabelColumnWidth);
		ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);

		DrawRow("Zoom:", std::to_string(static_cast<int>(ZoomFactor * 100.0f + 0.5f)) + "%");
		DrawRow("Channels:", ChannelsString);

		ImGui::EndTable();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	DrawSectionHeader("Sampling");

	const char* FilterOptions[] = { "Nearest", "Linear" };
	int CurrentFilter = static_cast<int>(SourceTexture->GetFilterType());
	if (ImGui::Combo("Filtering", &CurrentFilter, FilterOptions, IM_ARRAYSIZE(FilterOptions)))
	{
		SourceTexture->SetFilterType(static_cast<FE_TEXTURE_MINMAG_FILTER_TYPE>(CurrentFilter));
		if (DisplayTexture != nullptr)
			DisplayTexture->SetFilterType(static_cast<FE_TEXTURE_MINMAG_FILTER_TYPE>(CurrentFilter));
	}

	const char* WrapOptions[] = { "Repeat", "Mirrored repeat", "Clamp to edge", "Clamp to border" };

	int CurrentWrapU = static_cast<int>(SourceTexture->GetUWrapType());
	if (ImGui::Combo("Wrap U", &CurrentWrapU, WrapOptions, IM_ARRAYSIZE(WrapOptions)))
		SourceTexture->SetUWrapType(static_cast<FE_TEXTURE_WRAP_TYPE>(CurrentWrapU));

	int CurrentWrapV = static_cast<int>(SourceTexture->GetVWrapType());
	if (ImGui::Combo("Wrap V", &CurrentWrapV, WrapOptions, IM_ARRAYSIZE(WrapOptions)))
		SourceTexture->SetVWrapType(static_cast<FE_TEXTURE_WRAP_TYPE>(CurrentWrapV));

	// Offer 2D flipbook => 3D conversion.
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	DrawSectionHeader("Convert to 3D");

	// R16/R32F keep their format; GL_RED is 8-bit single channel; RGBA / DXT1 / DXT5 are
	// reduced to a single 8-bit red channel (GetRawData decompresses the DXT formats to RGBA8).
	const bool bFormatSupported = (InternalFormat == GL_R16 || InternalFormat == GL_R32F || InternalFormat == GL_RED ||
								   InternalFormat == GL_RGBA || InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT || InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);

	ImGui::TextWrapped("Interpret this texture as a flipbook grid and build a 3D texture from its slices.");
	ImGui::Spacing();

	ImGui::SetNextItemWidth(120.0f);
	ImGui::InputInt("Columns", &ConvertColumns);
	ImGui::SetNextItemWidth(120.0f);
	ImGui::InputInt("Rows", &ConvertRows);

	if (ConvertColumns < 1)
		ConvertColumns = 1;

	if (ConvertRows < 1)
		ConvertRows = 1;

	const bool bColumnsDivide = (Width % ConvertColumns) == 0;
	const bool bRowsDivide = (Height % ConvertRows) == 0;
	const bool bCanConvert = bFormatSupported && bColumnsDivide && bRowsDivide;

	if (!bFormatSupported)
	{
		ImGui::TextColored(ImVec4(0.95f, 0.5f, 0.5f, 1.0f), "This texture's format cannot be converted (use R16, R32F, R8/RED, RGBA, or DXT1/DXT5).");
	}
	else if (!bColumnsDivide || !bRowsDivide)
	{
		ImGui::TextColored(ImVec4(0.95f, 0.5f, 0.5f, 1.0f), "Width must divide evenly by Columns and Height by Rows.");
	}
	else
	{
		ImGui::TextDisabled("%d slices of %d x %d", ConvertColumns* ConvertRows, Width / ConvertColumns, Height / ConvertRows);
	}

	ImGui::Spacing();
	ImGui::BeginDisabled(!bCanConvert);
	if (ImGui::Button("Convert to 3D Texture", ImVec2(-1.0f, 0.0f)))
	{
		FETexture* ResultTexture = RESOURCE_MANAGER.ConvertFlipbook2DTo3DTexture(SourceTexture, ConvertColumns, ConvertRows);
		if (ResultTexture != nullptr)
		{
			bConvertSucceeded = true;

			const bool bAddedToContentBrowser = VIRTUAL_FILE_SYSTEM.CreateFile(ResultTexture, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
			if (PROJECT_MANAGER.GetCurrent() != nullptr)
			{
				PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(ResultTexture);
				PROJECT_MANAGER.GetCurrent()->SetModified(true);
			}

			ConvertStatusMessage = "Created 3D texture \"" + ResultTexture->GetName() + "\" (" + std::to_string(ConvertColumns * ConvertRows) + " slices).";
		}
		else
		{
			bConvertSucceeded = false;
			ConvertStatusMessage = "Conversion failed. See the log for details.";
		}
	}
	ImGui::EndDisabled();

	if (!ConvertStatusMessage.empty())
	{
		const ImVec4 StatusColor = bConvertSucceeded ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f) : ImVec4(0.95f, 0.5f, 0.5f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, StatusColor);
		ImGui::TextWrapped("%s", ConvertStatusMessage.c_str());
		ImGui::PopStyleColor();
	}
	

	ImGui::EndChild();
	ImGui::PopStyleColor();
}

void TextureViewWindow::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	FETexture* SourceTexture = RESOURCE_MANAGER.GetTexture(SourceTextureObjectID);
	if (SourceTexture == nullptr)
	{
		Close();
		FEImGuiWindow::OnRenderEnd();
		return;
	}

	if (DisplayTexture != nullptr)
	{
		TextureToView = DisplayTexture;
	}
	else
	{
		TextureToView = SourceTexture;
	}

	if (GetUserRequestedClose())
	{
		Close();
		FEImGuiWindow::OnRenderEnd();
		return;
	}

	RenderToolbar();

	const ImVec2 ContentArea = ImGui::GetContentRegionAvail();
	const float CanvasWidth = std::max(64.0f, ContentArea.x - DetailsPanelWidth - 8.0f);
	const ImVec2 CanvasSize = ImVec2(CanvasWidth, ContentArea.y);
	const ImVec2 PanelSize = ImVec2(DetailsPanelWidth, ContentArea.y);

	RenderImageCanvas(CanvasSize);
	ImGui::SameLine(0.0f, 8.0f);
	RenderDetailsPanel(PanelSize);

	FEImGuiWindow::OnRenderEnd();
}