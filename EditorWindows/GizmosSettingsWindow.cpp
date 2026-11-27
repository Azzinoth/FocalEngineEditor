#include "GizmosSettingsWindow.h"
#include "../FEEditor.h"

GizmosSettingsWindow::GizmosSettingsWindow()
{
	Flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground;
	BorderSize = 0.0f;
	Padding = glm::vec2(0.0f);
}

void GizmosSettingsWindow::Show()
{
	const std::string TemporaryCaption = "##GizmosSettingsWindow";
	strcpy_s(Caption, TemporaryCaption.size() + 1, TemporaryCaption.c_str());
	FEImGuiWindow::Show();
}

// Icons are drawn with shapes, so they take any color. Center should be on a pixel border.
static void RenderGizmoIcon(ImDrawList* DrawList, const int GizmoState, const ImVec2 Center, const ImU32 Color)
{
	const float HalfSize = 8.0f;
	const float LineThickness = 2.0f;

	if (GizmoState == TRANSFORM_GIZMOS)
	{
		const float HalfThickness = LineThickness / 2.0f;
		const float ArrowHalfWidth = 4.0f;
		DrawList->AddRectFilled(ImVec2(Center.x - HalfSize, Center.y - HalfThickness), ImVec2(Center.x + HalfSize, Center.y + HalfThickness), Color);
		DrawList->AddRectFilled(ImVec2(Center.x - HalfThickness, Center.y - HalfSize), ImVec2(Center.x + HalfThickness, Center.y + HalfSize), Color);

		const ImVec2 Directions[] = { ImVec2(1.0f, 0.0f), ImVec2(0.0f, 1.0f), ImVec2(-1.0f, 0.0f), ImVec2(0.0f, -1.0f) };
		for (const ImVec2& Direction : Directions)
		{
			const ImVec2 Side = ImVec2(-Direction.y, Direction.x);
			const ImVec2 Tip = ImVec2(Center.x + Direction.x * (HalfSize + 3.0f), Center.y + Direction.y * (HalfSize + 3.0f));
			const ImVec2 Base = ImVec2(Center.x + Direction.x * (HalfSize - 1.0f), Center.y + Direction.y * (HalfSize - 1.0f));
			DrawList->AddTriangleFilled(Tip, ImVec2(Base.x + Side.x * ArrowHalfWidth, Base.y + Side.y * ArrowHalfWidth), ImVec2(Base.x - Side.x * ArrowHalfWidth, Base.y - Side.y * ArrowHalfWidth), Color);
		}
	}
	else if (GizmoState == SCALE_GIZMOS)
	{
		const float Extent = HalfSize + 1.0f;
		const float SmallBoxSize = 7.0f;
		const ImU32 OutlineColor = (Color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(96) << IM_COL32_A_SHIFT);
		DrawList->AddRect(ImVec2(Center.x - Extent, Center.y - Extent), ImVec2(Center.x + Extent, Center.y + Extent), OutlineColor, 1.0f);
		DrawList->AddRectFilled(ImVec2(Center.x - Extent, Center.y + Extent - SmallBoxSize), ImVec2(Center.x - Extent + SmallBoxSize, Center.y + Extent), Color, 1.0f);

		const ImVec2 Tip = ImVec2(Center.x + Extent, Center.y - Extent);
		DrawList->PathLineTo(ImVec2(Center.x - Extent + SmallBoxSize + 1.5f, Center.y + Extent - SmallBoxSize - 1.5f));
		DrawList->PathLineTo(ImVec2(Tip.x - 3.0f, Tip.y + 3.0f));
		DrawList->PathStroke(Color, ImDrawFlags_None, LineThickness);
		DrawList->AddTriangleFilled(Tip, ImVec2(Tip.x, Tip.y + 6.5f), ImVec2(Tip.x - 6.5f, Tip.y), Color);
	}
	else if (GizmoState == ROTATE_GIZMOS)
	{
		DrawList->PathArcTo(Center, HalfSize, 0.15f * IM_PI, 1.5f * IM_PI, 24);
		DrawList->PathStroke(Color, ImDrawFlags_None, LineThickness);
		DrawList->AddTriangleFilled(ImVec2(Center.x + 5.5f, Center.y - HalfSize), ImVec2(Center.x, Center.y - HalfSize + 4.0f), ImVec2(Center.x, Center.y - HalfSize - 4.0f), Color);
		DrawList->AddCircleFilled(Center, 1.8f, Color);
	}
}

void GizmosSettingsWindow::Render()
{
	if (!IsVisible())
		return;

	FEScene* CurrentScene = EDITOR.GetFocusedScene();
	if (CurrentScene == nullptr)
		return;

	FEGizmoSceneData* GizmoSceneData = GIZMO_MANAGER.GetSceneData(CurrentScene->GetID());
	if (GizmoSceneData == nullptr)
		return;

	struct GizmoMode
	{
		int State;
		const char* Name;
		const char* ToolTip;
	};

	const GizmoMode Modes[] = {
		{ TRANSFORM_GIZMOS, "Move", "Translate objects. key = shift" },
		{ SCALE_GIZMOS, "Scale", "Scale objects. key = shift" },
		{ ROTATE_GIZMOS, "Rotate", "Rotate objects. key = shift" }
	};
	const int ModeCount = 3;

	const float Height = 34.0f;
	const float IconAreaSize = 22.0f;
	const float SidePadding = 12.0f;
	const float IconToNameSpacing = 6.0f;

	float Widths[ModeCount] = {};
	float TotalWidth = 0.0f;
	for (int i = 0; i < ModeCount; i++)
	{
		Widths[i] = ceilf(SidePadding + IconAreaSize + IconToNameSpacing + ImGui::CalcTextSize(Modes[i].Name).x + SidePadding + 2.0f);
		TotalWidth += Widths[i];
	}
	Size = ImVec2(TotalWidth, Height);

	// Top right corner of the window of focused scene, below its tab.
	FEEditorSceneWindow* SceneWindow = EDITOR_SCENE_WINDOW_MANAGER.GetSceneWindow(CurrentScene->GetID());
	if (SceneWindow != nullptr && SceneWindow->GetWindow() != nullptr)
	{
		const ImRect SceneArea = SceneWindow->GetWindow()->InnerRect;
		Position = ImVec2(floorf(SceneArea.Max.x - Size.x - 10.0f), floorf(SceneArea.Min.y + 10.0f));
		ImGui::SetNextWindowPos(Position);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	FEImGuiWindow::Render();

	int ClickedState = -1;
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	for (int i = 0; i < ModeCount; i++)
	{
		const bool bSelected = GizmoSceneData->GizmosState == Modes[i].State;
		ImDrawFlags RoundFlags = ImDrawFlags_RoundCornersNone;
		if (i == 0)
		{
			RoundFlags = ImDrawFlags_RoundCornersLeft;
		}
		else if (i == ModeCount - 1)
		{
			RoundFlags = ImDrawFlags_RoundCornersRight;
		}

		const ImVec2 Min = ImGui::GetCursorScreenPos();
		const ImVec2 Max = ImVec2(Min.x + Widths[i], Min.y + Height);
		if (ImGui::InvisibleButton(Modes[i].Name, ImVec2(Widths[i], Height)))
			ClickedState = Modes[i].State;
		const bool bHovered = ImGui::IsItemHovered();
		ShowToolTip(Modes[i].ToolTip);

		ImU32 Fill = bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button;
		if (bSelected)
			Fill = FEEditorStyle::Accent;

		const ImU32 ContentColor = bSelected ? FEEditorStyle::TextOnAccent : FEEditorStyle::Text;
		DrawList->AddRectFilled(Min, Max, Fill, 4.0f, RoundFlags);
		if (!bSelected)
			DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f, 1.0f, RoundFlags);

		RenderGizmoIcon(DrawList, Modes[i].State, ImVec2(floorf(Min.x + SidePadding + IconAreaSize / 2.0f), floorf(Min.y + Height / 2.0f)), ContentColor);
		DrawList->AddText(ImVec2(Min.x + SidePadding + IconAreaSize + IconToNameSpacing, Min.y + (Height - ImGui::GetFontSize()) / 2.0f), ContentColor, Modes[i].Name);

		if (i < ModeCount - 1)
			ImGui::SameLine(0.0f, 0.0f);
	}

	FEImGuiWindow::OnRenderEnd();
	ImGui::PopStyleVar(2);

	if (ClickedState != -1 && ClickedState != GizmoSceneData->GizmosState)
		GIZMO_MANAGER.UpdateGizmoState(ClickedState, CurrentScene);
}
