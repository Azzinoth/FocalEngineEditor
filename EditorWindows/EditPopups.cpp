#include "EditPopups.h"
#include "../FEEditor.h"

FEMesh** EditGameModelPopup::MeshToModify = nullptr;

FEMaterial** EditGameModelPopup::MaterialToModify = nullptr;
FEMaterial** EditGameModelPopup::BillboardMaterialToModify = nullptr;

static const ImU32 LevelColors[4] = { IM_COL32(0x2B, 0x9F, 0xD9, 255), IM_COL32(0x3B, 0xB5, 0x8C, 255), IM_COL32(0xC9, 0xA2, 0x27, 255), IM_COL32(0xC9, 0x6F, 0x4A, 255) };
static const ImU32 BillboardLevelColor = IM_COL32(0x8E, 0x6B, 0xBF, 255);
static const float SmallFontSize = 16.0f;

static float GetTextWidth(const std::string& Text, const float FontSize)
{
	return ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.0f, Text.c_str()).x;
}

// MaxWidth 0 means that the text is not cut.
static void RenderTextAt(const ImVec2 Position, const ImU32 Color, const std::string& Text, const float FontSize, const float MaxWidth = 0.0f)
{
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	if (MaxWidth > 0.0f)
		DrawList->PushClipRect(ImVec2(Position.x, Position.y - 2.0f), ImVec2(Position.x + MaxWidth, Position.y + FontSize + 4.0f), true);

	DrawList->AddText(ImGui::GetFont(), FontSize, ImVec2(floorf(Position.x), floorf(Position.y)), Color, Text.c_str());
	if (MaxWidth > 0.0f)
		DrawList->PopClipRect();
}

static float GetButtonWidth(const std::string& Name, const float FontSize)
{
	return GetTextWidth(Name, FontSize) + 24.0f;
}

static bool RenderButtonAt(const char* ID, const std::string& Name, const ImVec2 Min, const ImVec2 Size, const float FontSize, const bool bPrimary = false)
{
	ImGui::SetCursorScreenPos(Min);
	const bool bPressed = ImGui::InvisibleButton(ID, Size);
	const bool bHovered = ImGui::IsItemHovered();
	const ImVec2 Max = ImVec2(Min.x + Size.x, Min.y + Size.y);

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	if (bPrimary)
	{
		DrawList->AddRectFilled(Min, Max, bHovered ? FEEditorStyle::PrimaryButtonHovered : FEEditorStyle::PrimaryButton, 4.0f);
	}
	else
	{
		DrawList->AddRectFilled(Min, Max, bHovered ? FEEditorStyle::ButtonHovered : FEEditorStyle::Button, 4.0f);
		DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f);
	}

	RenderTextAt(ImVec2(Min.x + (Size.x - GetTextWidth(Name, FontSize)) / 2.0f, Min.y + (Size.y - FontSize) / 2.0f), FEEditorStyle::Text, Name, FontSize);
	return bPressed;
}

static bool RenderTrashButtonAt(const char* ID, const ImVec2 Min, const float Size)
{
	ImGui::SetCursorScreenPos(Min);
	const bool bPressed = ImGui::InvisibleButton(ID, ImVec2(Size, Size));
	const bool bHovered = ImGui::IsItemHovered();
	const ImVec2 Max = ImVec2(Min.x + Size, Min.y + Size);

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, bHovered ? FEEditorStyle::DangerButton : FEEditorStyle::Button, 4.0f);
	if (!bHovered)
		DrawList->AddRect(Min, Max, FEEditorStyle::ButtonBorder, 4.0f);

	const float X = floorf(Min.x + Size / 2.0f);
	const float Y = floorf(Min.y + Size / 2.0f);
	const ImU32 Color = bHovered ? IM_COL32(255, 255, 255, 255) : FEEditorStyle::TextMuted;
	DrawList->AddRectFilled(ImVec2(X - 3.0f, Y - 9.0f), ImVec2(X + 3.0f, Y - 7.0f), Color);
	DrawList->AddRectFilled(ImVec2(X - 8.0f, Y - 7.0f), ImVec2(X + 8.0f, Y - 5.0f), Color);
	DrawList->AddRectFilled(ImVec2(X - 6.0f, Y - 3.0f), ImVec2(X + 6.0f, Y + 9.0f), Color, 1.0f);
	return bPressed;
}

// Picture of a resource, a click on it is a request to change the resource.
static bool RenderPictureAt(const char* ID, FETexture* Texture, const ImVec2 Min, const float Size)
{
	ImGui::SetCursorScreenPos(Min);
	const bool bPressed = ImGui::InvisibleButton(ID, ImVec2(Size, Size));
	const ImVec2 Max = ImVec2(Min.x + Size, Min.y + Size);

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, Max, FEEditorStyle::Background);
	if (Texture != nullptr)
		DrawList->AddImage(Texture->GetTextureID(), Min, Max, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	DrawList->AddRect(Min, Max, ImGui::IsItemHovered() ? FEEditorStyle::Accent : FEEditorStyle::CardBorder);
	return bPressed;
}

static bool RenderFieldAt(const char* ID, const ImVec2 Min, const float Width, float& Value, const float Speed, const float MinValue, const float MaxValue, const char* Format)
{
	ImGui::SetCursorScreenPos(Min);
	ImGui::SetNextItemWidth(Width);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, FEEditorStyle::Background);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 7.0f));
	const bool bChanged = ImGui::DragFloat(ID, &Value, Speed, MinValue, MaxValue, Format, ImGuiSliderFlags_AlwaysClamp);
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	return bChanged;
}

static void RenderCardAt(const ImVec2 Min, const ImVec2 Size)
{
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	DrawList->AddRectFilled(Min, ImVec2(Min.x + Size.x, Min.y + Size.y), FEEditorStyle::Card, 4.0f);
	DrawList->AddRect(Min, ImVec2(Min.x + Size.x, Min.y + Size.y), FEEditorStyle::CardBorder, 4.0f);
}

static std::string GetDistanceText(const float Distance)
{
	char Buffer[32] = {};
	if (Distance >= 100.0f || fabsf(Distance - roundf(Distance)) < 0.05f)
	{
		sprintf_s(Buffer, "%d m", static_cast<int>(roundf(Distance)));
	}
	else
	{
		sprintf_s(Buffer, "%.1f m", Distance);
	}

	return Buffer;
}

void EditGameModelPopup::ChangeMeshCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (MeshToModify == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MESH)
	{
		FEMesh* SelectedMesh = RESOURCE_MANAGER.GetMesh(SelectionsResult[0]->GetID());
		if (SelectedMesh == nullptr)
			return;

		*MeshToModify = SelectedMesh;
	}
}

void EditGameModelPopup::ChangeMaterialCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetID());
		if (SelectedMaterial == nullptr)
			return;

		*MaterialToModify = SelectedMaterial;
	}
}

void EditGameModelPopup::ChangeBillboardMaterialCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetID());
		if (SelectedMaterial == nullptr)
			return;

		*BillboardMaterialToModify = SelectedMaterial;
	}
}

EditGameModelPopup::EditGameModelPopup()
{
	TemporaryModel = new FEGameModel(nullptr, nullptr, "tempGameModel");
	ObjectToWorkWith = nullptr;
	Flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	SetPadding(glm::vec2(WindowPadding, WindowPadding));
}

EditGameModelPopup::~EditGameModelPopup() {}

void EditGameModelPopup::Show(FEGameModel* GameModel)
{
	if (GameModel != nullptr)
	{
		ObjectToWorkWith = GameModel;

		TemporaryModel->SetMaterial(ObjectToWorkWith->GetMaterial());
		TemporaryModel->SetBillboardMaterial(ObjectToWorkWith->GetBillboardMaterial());
		TemporaryModel->SetScaleFactor(ObjectToWorkWith->GetScaleFactor());
		UpdatedMaterial = ObjectToWorkWith->GetMaterial();
		UpdatedBillboardMaterial = ObjectToWorkWith->GetBillboardMaterial();
		TemporaryModel->SetUsingLOD(ObjectToWorkWith->IsUsingLOD());
		TemporaryModel->SetBillboardZeroRotation(ObjectToWorkWith->GetBillboardZeroRotation());
		TemporaryModel->SetCullDistance(ObjectToWorkWith->GetCullDistance());

		UpdatedLODMeshes.clear();
		for (size_t i = 0; i < ObjectToWorkWith->GetMaxLODCount(); i++)
		{
			TemporaryModel->SetLODMesh(i, ObjectToWorkWith->GetLODMesh(i));
			TemporaryModel->SetIsLODBillboard(i, ObjectToWorkWith->IsLODBillboard(i));
			TemporaryModel->SetLODMaxDrawDistance(i, ObjectToWorkWith->GetLODMaxDrawDistance(i));
			UpdatedLODMeshes.push_back(ObjectToWorkWith->GetLODMesh(i));
		}

		std::string TemporaryCaption = "Edit game model:";
		TemporaryCaption += " " + ObjectToWorkWith->GetName();
		strcpy_s(Caption, TemporaryCaption.size() + 1, TemporaryCaption.c_str());

		Size = GetWindowSize();
		Position = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2 - Size.x / 2, APPLICATION.GetMainWindow()->GetHeight() / 2 - Size.y / 2);
		FEImGuiWindow::Show();

		PREVIEW_MANAGER.CreateGameModelPreview(TemporaryModel, &TemporaryPreview);

		// ************** Drag&Drop **************
		LODMeshCallbackInfo.resize(ObjectToWorkWith->GetMaxLODCount());
		LODMeshTarget.resize(ObjectToWorkWith->GetMaxLODCount());
		for (size_t i = 0; i < ObjectToWorkWith->GetMaxLODCount(); i++)
		{
			LODMeshCallbackInfo[i].LODLevel = static_cast<int>(i);
			LODMeshCallbackInfo[i].Window = this;
			LODMeshTarget[i] = DRAG_AND_DROP_MANAGER.AddTarget(FE_MESH, DragAndDropLODMeshCallback, reinterpret_cast<void**>(&LODMeshCallbackInfo[i]), "Drop to assing LOD" + std::to_string(i) + " mesh");
		}

		MaterialCallbackInfo.bBillboardMaterial = false;
		MaterialCallbackInfo.Window = this;

		BillboardMaterialCallbackInfo.bBillboardMaterial = true;
		BillboardMaterialCallbackInfo.Window = this;

		MaterialTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_MATERIAL, DragAndDropMaterialCallback, reinterpret_cast<void**>(&MaterialCallbackInfo), "Drop to assing material");
		BillboardMaterialTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_MATERIAL, DragAndDropMaterialCallback, reinterpret_cast<void**>(&BillboardMaterialCallbackInfo), "Drop to assing billboard material");
		// ************** Drag&Drop END **************
	}
}

size_t EditGameModelPopup::GetLevelCount()
{
	if (!TemporaryModel->IsUsingLOD())
		return 1;

	return std::max(static_cast<size_t>(1), TemporaryModel->GetLODCount());
}

bool EditGameModelPopup::HasBillboardLevel()
{
	return TemporaryModel->IsUsingLOD() && TemporaryModel->IsLODBillboard(GetLevelCount() - 1);
}

ImVec2 EditGameModelPopup::GetContentSize()
{
	float LevelsColumnHeight = MaterialCardHeight + 16.0f + 50.0f + static_cast<float>(GetLevelCount()) * (LevelRowHeight + 8.0f);
	if (TemporaryModel->IsUsingLOD())
		LevelsColumnHeight += 46.0f + 76.0f;

	const float ResultColumnHeight = HasBillboardLevel() ? 474.0f : 432.0f;
	return ImVec2(ContentWidth, std::max(LevelsColumnHeight, ResultColumnHeight) + FooterHeight);
}

ImVec2 EditGameModelPopup::GetWindowSize()
{
	const ImVec2 ContentSize = GetContentSize();
	return ImVec2(ContentSize.x + WindowPadding * 2.0f, ContentSize.y + WindowPadding * 2.0f + ImGui::GetFrameHeight());
}

// Takes what the selection window or drag and drop has set.
void EditGameModelPopup::ApplyUpdatedResources()
{
	if (UpdatedMaterial != TemporaryModel->GetMaterial())
	{
		TemporaryModel->SetMaterial(UpdatedMaterial);
		PREVIEW_MANAGER.CreateGameModelPreview(TemporaryModel, &TemporaryPreview);
	}

	if (UpdatedBillboardMaterial != TemporaryModel->GetBillboardMaterial())
		TemporaryModel->SetBillboardMaterial(UpdatedBillboardMaterial);

	for (size_t i = 0; i < TemporaryModel->GetMaxLODCount(); i++)
	{
		if (UpdatedLODMeshes[i] == TemporaryModel->GetLODMesh(i))
			continue;

		// New level takes the far half of the distance of the level before it.
		if (i > 0 && TemporaryModel->GetLODMesh(i) == nullptr)
		{
			const float PreviousLevelStart = i > 1 ? TemporaryModel->GetLODMaxDrawDistance(i - 2) : 0.0f;
			TemporaryModel->SetLODMaxDrawDistance(i - 1, (PreviousLevelStart + TemporaryModel->GetCullDistance()) / 2.0f);
		}

		TemporaryModel->SetLODMesh(i, UpdatedLODMeshes[i]);
		PREVIEW_MANAGER.CreateGameModelPreview(TemporaryModel, &TemporaryPreview);
	}
}

// Every level keeps a part of the distance, the last one ends at the cull distance.
void EditGameModelPopup::CorrectLevelDistances()
{
	if (!TemporaryModel->IsUsingLOD())
		return;

	const size_t LevelCount = GetLevelCount();
	const float CullDistance = TemporaryModel->GetCullDistance();
	const float MinLevelSpan = std::min(0.1f, CullDistance / static_cast<float>(LevelCount * 2));
	float LevelStart = 0.0f;
	for (size_t i = 0; i + 1 < LevelCount; i++)
	{
		const float MaxAllowed = CullDistance - MinLevelSpan * static_cast<float>(LevelCount - 1 - i);
		const float Corrected = std::clamp(TemporaryModel->GetLODMaxDrawDistance(i), LevelStart + MinLevelSpan, MaxAllowed);
		if (Corrected != TemporaryModel->GetLODMaxDrawDistance(i))
			TemporaryModel->SetLODMaxDrawDistance(i, Corrected);

		LevelStart = Corrected;
	}

	if (TemporaryModel->GetLODMaxDrawDistance(LevelCount - 1) != CullDistance)
		TemporaryModel->SetLODMaxDrawDistance(LevelCount - 1, CullDistance);
}

void EditGameModelPopup::SetUsingLevels(const bool bUseLevels)
{
	if (bUseLevels == TemporaryModel->IsUsingLOD())
		return;

	if (!bUseLevels)
	{
		for (size_t i = 1; i < TemporaryModel->GetMaxLODCount(); i++)
		{
			UpdatedLODMeshes[i] = nullptr;
			TemporaryModel->SetIsLODBillboard(i, false);
		}

		UpdatedBillboardMaterial = nullptr;
		TemporaryModel->SetBillboardMaterial(nullptr);
	}

	TemporaryModel->SetUsingLOD(bUseLevels);
}

void EditGameModelPopup::ShowMeshSelection(const size_t LevelIndex)
{
	UpdatedLODMeshes[LevelIndex] = TemporaryModel->GetLODMesh(LevelIndex);
	MeshToModify = &UpdatedLODMeshes[LevelIndex];
	SELECT_FEOBJECT_POPUP.Show(FE_MESH, ChangeMeshCallback, UpdatedLODMeshes[LevelIndex]);
}

void EditGameModelPopup::ShowMaterialSelection()
{
	UpdatedMaterial = TemporaryModel->GetMaterial();
	MaterialToModify = &UpdatedMaterial;
	SELECT_FEOBJECT_POPUP.Show(FE_MATERIAL, ChangeMaterialCallback, UpdatedMaterial);
}

// Only materials with the PBR shader can be on a billboard.
void EditGameModelPopup::ShowBillboardMaterialSelection()
{
	UpdatedBillboardMaterial = TemporaryModel->GetBillboardMaterial();

	const std::vector<FEUUID> MaterialList = RESOURCE_MANAGER.GetMaterialIDList();
	std::vector<FEObject*> SuitableMaterials;
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		if (RESOURCE_MANAGER.GetMaterial(MaterialList[i])->Shader->GetID() == FEEngineResourceIDs::PBRShader)
			SuitableMaterials.push_back(RESOURCE_MANAGER.GetMaterial(MaterialList[i]));
	}

	BillboardMaterialToModify = &UpdatedBillboardMaterial;
	SELECT_FEOBJECT_POPUP.Show(FE_MATERIAL, ChangeBillboardMaterialCallback, UpdatedBillboardMaterial, SuitableMaterials);
}

// Picture of the result with the values that belong to the whole game model under it.
void EditGameModelPopup::RenderResultColumn(const ImVec2 Origin)
{
	const float FieldX = Origin.x + 170.0f;
	const float FieldWidth = ResultColumnWidth - 170.0f;

	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	const ImVec2 PictureMax = ImVec2(Origin.x + ResultColumnWidth, Origin.y + ResultColumnWidth);
	DrawList->AddRectFilled(Origin, PictureMax, FEEditorStyle::Background);
	if (TemporaryPreview != nullptr)
		DrawList->AddImage(TemporaryPreview->GetTextureID(), Origin, PictureMax, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
	DrawList->AddRect(Origin, PictureMax, FEEditorStyle::CardBorder);
	RenderTextAt(ImVec2(Origin.x, Origin.y + ResultColumnWidth + 10.0f), FEEditorStyle::TextMuted, "Result, with changes not applied yet", SmallFontSize);

	float ScaleFactor = TemporaryModel->GetScaleFactor();
	RenderTextAt(ImVec2(Origin.x, Origin.y + 347.0f), FEEditorStyle::TextMuted, "Scale", ImGui::GetFontSize());
	if (RenderFieldAt("##ScaleFactor", ImVec2(FieldX, Origin.y + 340.0f), FieldWidth, ScaleFactor, 0.01f, 0.001f, 100000.0f, "%.2f"))
		TemporaryModel->SetScaleFactor(ScaleFactor);

	float CullDistance = TemporaryModel->GetCullDistance();
	RenderTextAt(ImVec2(Origin.x, Origin.y + 389.0f), FEEditorStyle::TextMuted, "Cull distance", ImGui::GetFontSize());
	if (RenderFieldAt("##CullDistance", ImVec2(FieldX, Origin.y + 382.0f), FieldWidth, CullDistance, 1.0f, 0.1f, MaxCullDistance, "%.1f m"))
		TemporaryModel->SetCullDistance(CullDistance);

	if (HasBillboardLevel())
	{
		float ZeroRotation = TemporaryModel->GetBillboardZeroRotation();
		RenderTextAt(ImVec2(Origin.x, Origin.y + 431.0f), FEEditorStyle::TextMuted, "Billboard rotation", ImGui::GetFontSize());
		if (RenderFieldAt("##BillboardZeroRotation", ImVec2(Origin.x + 210.0f, Origin.y + 424.0f), ResultColumnWidth - 210.0f, ZeroRotation, 0.1f, 0.0f, 360.0f, "%.1f"))
			TemporaryModel->SetBillboardZeroRotation(ZeroRotation);
	}
}

void EditGameModelPopup::RenderMaterialCard(const ImVec2 Min, const float Width)
{
	FEMaterial* Material = TemporaryModel->GetMaterial();
	RenderCardAt(Min, ImVec2(Width, MaterialCardHeight));
	if (RenderPictureAt("##MaterialPicture", Material == nullptr ? RESOURCE_MANAGER.NoTexture : PREVIEW_MANAGER.GetMaterialPreview(Material->GetID()), ImVec2(Min.x + 10.0f, Min.y + 10.0f), 72.0f))
		ShowMaterialSelection();
	MaterialTarget->StickToItem();

	const float ButtonHeight = 34.0f;
	const float ButtonWidth = GetButtonWidth("Change...", SmallFontSize);
	const float TextX = Min.x + 96.0f;
	const float TextWidth = Width - 96.0f - ButtonWidth - 28.0f;
	RenderTextAt(ImVec2(TextX, Min.y + 10.0f), FEEditorStyle::TextMuted, "Material", SmallFontSize);
	RenderTextAt(ImVec2(TextX, Min.y + 32.0f), FEEditorStyle::Text, Material == nullptr ? "No material" : Material->GetName(), ImGui::GetFontSize(), TextWidth);
	if (Material != nullptr && Material->Shader != nullptr)
		RenderTextAt(ImVec2(TextX, Min.y + 62.0f), FEEditorStyle::TextMuted, "Shader: " + Material->Shader->GetName(), SmallFontSize, TextWidth);

	if (RenderButtonAt("##ChangeMaterial", "Change...", ImVec2(Min.x + Width - ButtonWidth - 14.0f, Min.y + (MaterialCardHeight - ButtonHeight) / 2.0f), ImVec2(ButtonWidth, ButtonHeight), SmallFontSize))
		ShowMaterialSelection();
}

void EditGameModelPopup::RenderLevelRow(const size_t LevelIndex, const ImVec2 Min, const float Width)
{
	const size_t LevelCount = GetLevelCount();
	const bool bUsingLOD = TemporaryModel->IsUsingLOD();
	const bool bBillboard = bUsingLOD && TemporaryModel->IsLODBillboard(LevelIndex);
	const bool bLastLevel = LevelIndex + 1 == LevelCount;
	FEMesh* Mesh = TemporaryModel->GetLODMesh(LevelIndex);
	FEMaterial* BillboardMaterial = TemporaryModel->GetBillboardMaterial();

	ImGui::PushID(static_cast<int>(LevelIndex));
	RenderCardAt(Min, ImVec2(Width, LevelRowHeight));
	ImGui::GetWindowDrawList()->AddRectFilled(Min, ImVec2(Min.x + 5.0f, Min.y + LevelRowHeight), bBillboard ? BillboardLevelColor : LevelColors[LevelIndex], 4.0f, ImDrawFlags_RoundCornersLeft);

	// A billboard is a flat picture, so its row is about its material.
	bool bChangeRequested = false;
	if (bBillboard)
	{
		bChangeRequested = RenderPictureAt("##LevelPicture", BillboardMaterial == nullptr ? RESOURCE_MANAGER.NoTexture : PREVIEW_MANAGER.GetMaterialPreview(BillboardMaterial->GetID()), ImVec2(Min.x + 15.0f, Min.y + 10.0f), 64.0f);
		BillboardMaterialTarget->StickToItem();
	}
	else
	{
		bChangeRequested = RenderPictureAt("##LevelPicture", Mesh == nullptr ? RESOURCE_MANAGER.NoTexture : PREVIEW_MANAGER.GetMeshPreview(Mesh->GetID()), ImVec2(Min.x + 15.0f, Min.y + 10.0f), 64.0f);
		LODMeshTarget[LevelIndex]->StickToItem();
	}

	std::string Title = "Mesh";
	if (bBillboard)
	{
		Title = "Billboard";
	}
	else if (bUsingLOD)
	{
		Title = "LOD " + std::to_string(LevelIndex);
	}

	std::string Name = Mesh == nullptr ? "No mesh" : Mesh->GetName();
	std::string Fact = Mesh == nullptr ? "" : GetNumberWithSeparators(Mesh->GetVertexCount() / 3) + " triangles";
	if (bBillboard)
	{
		Name = BillboardMaterial == nullptr ? "No material" : BillboardMaterial->GetName();
		Fact = "Faces the camera";
	}

	const float TextX = Min.x + 92.0f;
	const float TextWidth = 250.0f;
	RenderTextAt(ImVec2(TextX, Min.y + 10.0f), FEEditorStyle::TextMuted, Title, SmallFontSize);
	RenderTextAt(ImVec2(TextX, Min.y + 30.0f), FEEditorStyle::Text, Name, ImGui::GetFontSize(), TextWidth);
	RenderTextAt(ImVec2(TextX, Min.y + 58.0f), FEEditorStyle::TextMuted, Fact, SmallFontSize, TextWidth);

	if (bUsingLOD)
	{
		const float DistanceX = Min.x + 356.0f;
		const float LevelStart = LevelIndex == 0 ? 0.0f : TemporaryModel->GetLODMaxDrawDistance(LevelIndex - 1);
		const float MinLevelSpan = std::min(0.1f, TemporaryModel->GetCullDistance() / static_cast<float>(LevelCount * 2));
		RenderTextAt(ImVec2(DistanceX, Min.y + 10.0f), FEEditorStyle::TextMuted, "Shown from " + GetDistanceText(LevelStart) + " to", SmallFontSize);
		if (bLastLevel)
		{
			// The last level is shown up to the cull distance.
			float CullDistance = TemporaryModel->GetCullDistance();
			if (RenderFieldAt("##LevelEnd", ImVec2(DistanceX, Min.y + 36.0f), 120.0f, CullDistance, 1.0f, LevelStart + MinLevelSpan, MaxCullDistance, "%.1f m"))
				TemporaryModel->SetCullDistance(CullDistance);

			RenderTextAt(ImVec2(DistanceX + 130.0f, Min.y + 45.0f), FEEditorStyle::TextMuted, "cull", SmallFontSize);
		}
		else
		{
			float LevelEnd = TemporaryModel->GetLODMaxDrawDistance(LevelIndex);
			const float NextLevelEnd = TemporaryModel->GetLODMaxDrawDistance(LevelIndex + 1);
			if (RenderFieldAt("##LevelEnd", ImVec2(DistanceX, Min.y + 36.0f), 120.0f, LevelEnd, 0.1f, LevelStart + MinLevelSpan, NextLevelEnd - MinLevelSpan, "%.1f m"))
				TemporaryModel->SetLODMaxDrawDistance(LevelIndex, LevelEnd);
		}
	}

	const float ButtonHeight = 34.0f;
	const float ButtonY = Min.y + (LevelRowHeight - ButtonHeight) / 2.0f;
	float ButtonX = Min.x + Width - 14.0f;
	bool bRemoveRequested = false;
	if (bUsingLOD && bLastLevel && LevelIndex > 0)
	{
		ButtonX -= ButtonHeight;
		bRemoveRequested = RenderTrashButtonAt("##RemoveLevel", ImVec2(ButtonX, ButtonY), ButtonHeight);
		ShowToolTip("Remove this level");
		ButtonX -= 8.0f;
	}

	const float ChangeWidth = GetButtonWidth("Change...", SmallFontSize);
	if (RenderButtonAt("##ChangeLevel", "Change...", ImVec2(ButtonX - ChangeWidth, ButtonY), ImVec2(ChangeWidth, ButtonHeight), SmallFontSize))
		bChangeRequested = true;
	ImGui::PopID();

	if (bChangeRequested && bBillboard)
	{
		ShowBillboardMaterialSelection();
	}
	else if (bChangeRequested)
	{
		ShowMeshSelection(LevelIndex);
	}

	if (bRemoveRequested)
	{
		UpdatedLODMeshes[LevelIndex] = nullptr;
		TemporaryModel->SetLODMesh(LevelIndex, nullptr);
		TemporaryModel->SetIsLODBillboard(LevelIndex, false);
		PREVIEW_MANAGER.CreateGameModelPreview(TemporaryModel, &TemporaryPreview);
	}
}

// Shows what part of the distance every level has.
void EditGameModelPopup::RenderDistanceBar(const ImVec2 Min, const ImVec2 Size)
{
	const size_t LevelCount = GetLevelCount();
	const float CullDistance = TemporaryModel->GetCullDistance();
	ImDrawList* DrawList = ImGui::GetWindowDrawList();

	float LevelStart = 0.0f;
	float LastTextEnd = -FLT_MAX;
	for (size_t i = 0; i <= LevelCount; i++)
	{
		const float StartX = Min.x + Size.x * LevelStart / CullDistance;
		if (i < LevelCount)
		{
			const float LevelEnd = i + 1 == LevelCount ? CullDistance : TemporaryModel->GetLODMaxDrawDistance(i);
			const float EndX = Min.x + Size.x * LevelEnd / CullDistance;
			ImDrawFlags Corners = ImDrawFlags_RoundCornersNone;
			if (i == 0)
				Corners |= ImDrawFlags_RoundCornersLeft;

			if (i + 1 == LevelCount)
				Corners |= ImDrawFlags_RoundCornersRight;

			DrawList->AddRectFilled(ImVec2(StartX, Min.y), ImVec2(EndX, Min.y + Size.y), TemporaryModel->IsLODBillboard(i) ? BillboardLevelColor : LevelColors[i], 4.0f, Corners);
			if (i > 0)
				DrawList->AddRectFilled(ImVec2(StartX - 2.0f, Min.y - 4.0f), ImVec2(StartX + 2.0f, Min.y + Size.y + 4.0f), FEEditorStyle::Text, 2.0f);

			LevelStart = LevelEnd;
		}

		// Distance under the border, moved aside if it would cover the one before it.
		const float Distance = i == LevelCount ? CullDistance : (i == 0 ? 0.0f : TemporaryModel->GetLODMaxDrawDistance(i - 1));
		const std::string Text = GetDistanceText(Distance);
		const float TextWidth = GetTextWidth(Text, SmallFontSize);
		float TextX = Min.x + Size.x * Distance / CullDistance - TextWidth / 2.0f;
		TextX = std::max(Min.x, std::min(TextX, Min.x + Size.x - TextWidth));
		TextX = std::max(TextX, LastTextEnd + 10.0f);
		RenderTextAt(ImVec2(TextX, Min.y + Size.y + 10.0f), FEEditorStyle::TextMuted, Text, SmallFontSize);
		LastTextEnd = TextX + TextWidth;
	}
}

void EditGameModelPopup::ApplyChanges()
{
	ObjectToWorkWith->SetDirtyFlag(true);
	ObjectToWorkWith->Mesh = TemporaryModel->Mesh;
	ObjectToWorkWith->SetUsingLOD(TemporaryModel->IsUsingLOD());
	ObjectToWorkWith->SetCullDistance(TemporaryModel->GetCullDistance());

	// Distance is set after the billboard flag, the flag sets the distance of its level to the cull distance.
	for (size_t i = 0; i < TemporaryModel->GetMaxLODCount(); i++)
	{
		ObjectToWorkWith->SetLODMesh(i, TemporaryModel->GetLODMesh(i));
		ObjectToWorkWith->SetIsLODBillboard(i, TemporaryModel->IsLODBillboard(i));
		ObjectToWorkWith->SetLODMaxDrawDistance(i, TemporaryModel->GetLODMaxDrawDistance(i));
	}

	ObjectToWorkWith->SetMaterial(TemporaryModel->GetMaterial());
	ObjectToWorkWith->SetBillboardMaterial(TemporaryModel->GetBillboardMaterial());
	ObjectToWorkWith->SetScaleFactor(TemporaryModel->GetScaleFactor());
	ObjectToWorkWith->SetBillboardZeroRotation(TemporaryModel->GetBillboardZeroRotation());
	PREVIEW_MANAGER.CreateGameModelPreview(ObjectToWorkWith->GetID());
}

void EditGameModelPopup::Render()
{
	if (!IsVisible())
		return;

	ApplyUpdatedResources();
	CorrectLevelDistances();

	Size = GetWindowSize();
	FEImGuiWindow::Render();

	if (ObjectToWorkWith == nullptr)
	{
		FEImGuiWindow::Close();
		return;
	}

	const ImVec2 ContentSize = GetContentSize();
	const ImVec2 Origin = ImGui::GetCursorScreenPos();
	const size_t LevelCount = GetLevelCount();
	const bool bUsingLOD = TemporaryModel->IsUsingLOD();

	RenderResultColumn(Origin);

	const float LevelsX = Origin.x + ResultColumnWidth + 24.0f;
	const float LevelsWidth = ContentWidth - ResultColumnWidth - 24.0f;
	RenderMaterialCard(ImVec2(LevelsX, Origin.y), LevelsWidth);

	float Y = Origin.y + MaterialCardHeight + 16.0f;
	RenderTextAt(ImVec2(LevelsX, Y + 7.0f), FEEditorStyle::Text, "Levels of detail", ImGui::GetFontSize());
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 7.0f));
	ImGui::SetCursorScreenPos(ImVec2(LevelsX + 200.0f, Y));
	if (RenderSwitchButton("One mesh", !bUsingLOD, ImDrawFlags_RoundCornersLeft))
		SetUsingLevels(false);

	ImGui::SameLine(0.0f, 0.0f);
	if (RenderSwitchButton("Several levels", bUsingLOD, ImDrawFlags_RoundCornersRight))
		SetUsingLevels(true);
	ImGui::PopStyleVar();
	Y += 50.0f;

	for (size_t i = 0; i < LevelCount; i++)
	{
		RenderLevelRow(i, ImVec2(LevelsX, Y), LevelsWidth);
		Y += LevelRowHeight + 8.0f;
	}

	if (bUsingLOD)
	{
		const float ButtonHeight = 34.0f;
		if (HasBillboardLevel())
		{
			RenderTextAt(ImVec2(LevelsX, Y + 8.0f), FEEditorStyle::TextDisabled, "A billboard is the last level, remove it to add a level.", SmallFontSize);
		}
		else if (LevelCount >= TemporaryModel->GetMaxLODCount())
		{
			RenderTextAt(ImVec2(LevelsX, Y + 8.0f), FEEditorStyle::TextDisabled, "All levels are used.", SmallFontSize);
		}
		else
		{
			const float AddLevelWidth = GetButtonWidth("+ Add level", SmallFontSize);
			if (RenderButtonAt("##AddLevel", "+ Add level", ImVec2(LevelsX, Y), ImVec2(AddLevelWidth, ButtonHeight), SmallFontSize))
				ShowMeshSelection(LevelCount);

			if (RenderButtonAt("##AddBillboard", "+ Add billboard", ImVec2(LevelsX + AddLevelWidth + 8.0f, Y), ImVec2(GetButtonWidth("+ Add billboard", SmallFontSize), ButtonHeight), SmallFontSize))
			{
				UpdatedLODMeshes[LevelCount] = RESOURCE_MANAGER.GetMesh(FEEngineResourceIDs::PlaneMesh);
				UpdatedBillboardMaterial = RESOURCE_MANAGER.GetMaterial(FEEngineResourceIDs::PBRBaseMaterial);
				TemporaryModel->SetIsLODBillboard(LevelCount, true);
			}
		}

		Y += 46.0f;
		RenderDistanceBar(ImVec2(LevelsX, Y + 6.0f), ImVec2(LevelsWidth, 22.0f));
	}

	const float FooterY = Origin.y + ContentSize.y - FooterHeight + 16.0f;
	ImGui::GetWindowDrawList()->AddLine(ImVec2(Origin.x, FooterY), ImVec2(Origin.x + ContentWidth, FooterY), FEEditorStyle::CardBorder);
	const ImVec2 FooterButtonSize = ImVec2(130.0f, 40.0f);
	const bool bCancel = RenderButtonAt("##Cancel", "Cancel", ImVec2(Origin.x + ContentWidth - FooterButtonSize.x * 2.0f - 10.0f, FooterY + 16.0f), FooterButtonSize, ImGui::GetFontSize());
	const bool bApply = RenderButtonAt("##Apply", "Apply", ImVec2(Origin.x + ContentWidth - FooterButtonSize.x, FooterY + 16.0f), FooterButtonSize, ImGui::GetFontSize(), true);

	ImGui::SetCursorScreenPos(Origin);
	ImGui::Dummy(ContentSize);

	if (bApply)
		ApplyChanges();

	if (bApply || bCancel)
	{
		FEImGuiWindow::Close();
		return;
	}

	FEImGuiWindow::OnRenderEnd();
}

void EditGameModelPopup::Close()
{
	FEImGuiWindow::Close();
}

// ************** Drag&Drop **************
bool EditGameModelPopup::DragAndDropLODMeshCallback(FEObject* Object, void** CallbackInfo)
{
	const MeshTargetCallbackInfo* Info = reinterpret_cast<MeshTargetCallbackInfo*>(CallbackInfo);
	Info->Window->UpdatedLODMeshes[Info->LODLevel] = RESOURCE_MANAGER.GetMesh(Object->GetID());
	Info->Window->TemporaryModel->SetLODMesh(Info->LODLevel, RESOURCE_MANAGER.GetMesh(Object->GetID()));
	PREVIEW_MANAGER.CreateGameModelPreview(Info->Window->TemporaryModel, &Info->Window->TemporaryPreview);
	return true;
}

bool EditGameModelPopup::DragAndDropMaterialCallback(FEObject* Object, void** CallbackInfo)
{
	const MaterialTargetCallbackInfo* Info = reinterpret_cast<MaterialTargetCallbackInfo*>(CallbackInfo);

	if (Info->bBillboardMaterial)
	{
		Info->Window->UpdatedBillboardMaterial = RESOURCE_MANAGER.GetMaterial(Object->GetID());
		Info->Window->TemporaryModel->SetBillboardMaterial(RESOURCE_MANAGER.GetMaterial(Object->GetID()));
	}
	else
	{
		Info->Window->UpdatedMaterial = RESOURCE_MANAGER.GetMaterial(Object->GetID());
		Info->Window->TemporaryModel->SetMaterial(RESOURCE_MANAGER.GetMaterial(Object->GetID()));
		PREVIEW_MANAGER.CreateGameModelPreview(Info->Window->TemporaryModel, &Info->Window->TemporaryPreview);
	}

	return true;
}
// ************** Drag&Drop END **************

FEEntity* EditMaterialWindow::InjectModelViewCamera(FEScene* Scene)
{
	FEEntity* CameraEntity = nullptr;
	std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Model view camera prefab");
	if (CameraPrefab.size() == 0)
	{
		LOG.Add("EditMaterialWindow::InjectModelViewCamera: Camera prefab not found! Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

		CameraEntity = Scene->CreateEntity("Prefab scene camera");
		CameraEntity->AddComponent<FECameraComponent>();
	}
	else
	{
		FEPrefab* CameraPrefabToUse = CameraPrefab[0];
		std::vector<FEEntity*> AddedEntities = SCENE_MANAGER.InstantiatePrefab(CameraPrefabToUse, Scene, true);
		if (AddedEntities.empty())
		{
			LOG.Add("EditMaterialWindow::InjectModelViewCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Prefab scene camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
		else
		{
			CameraEntity = AddedEntities[0];
		}

		if (CameraEntity == nullptr)
		{
			LOG.Add("EditMaterialWindow::InjectModelViewCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Prefab scene camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
	}

	RESOURCE_MANAGER.SetTag(CameraEntity, EDITOR_RESOURCE_TAG);
	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.TryToSetViewportSize(512, 1020);
	CameraComponent.SetDistanceFogEnabled(false);
	FENativeScriptComponent& CameraScript = CameraEntity->GetComponent<FENativeScriptComponent>();
	CameraScript.SetVariableValue("DistanceToModel", 10.0f);
	CAMERA_SYSTEM.SetMainCamera(CameraEntity);

	return CameraEntity;
}

EditMaterialWindow::EditMaterialWindow()
{
	ObjectToWorkWith = nullptr;
	Flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetDefaultColor(FEEditorStyle::ToVec4(FEEditorStyle::DangerButton));
	CancelButton->SetHoveredColor(FEEditorStyle::ToVec4(FEEditorStyle::DangerButtonHovered));
	CancelButton->SetActiveColor(FEEditorStyle::ToVec4(FEEditorStyle::Accent));
	NodeAreaTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_TEXTURE, DragAndDropNodeAreaTargetCallback, reinterpret_cast<void**>(&DragAndDropCallbackInfo), "Drop to add texture");

	PreviewScene = SCENE_MANAGER.CreateScene("MaterialEditor_Scene", FEUUID(), FESceneFlag::Active | FESceneFlag::EditorMode);
	RESOURCE_MANAGER.SetTag(PreviewScene, EDITOR_RESOURCE_TAG);

	PreviewGameModel = new FEGameModel(nullptr, nullptr, "MaterialEditor_Preview_GameModel");
	PreviewGameModel->Mesh = RESOURCE_MANAGER.GetMesh(FEEngineResourceIDs::SphereMesh);

	PreviewEntity = PreviewScene->CreateEntity("MaterialEditor_Scene_PreviewEntity");
	PreviewEntity->AddComponent<FEGameModelComponent>(PreviewGameModel);
	PreviewEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(0.1f));
	RESOURCE_MANAGER.SetTag(PreviewEntity, EDITOR_RESOURCE_TAG);

	PreviewCameraEntity = InjectModelViewCamera(PreviewScene);
	PreviewCameraEntity->SetName("MaterialEditor_Scene_CameraEntity");

	PreviewLightEntity = PreviewScene->CreateEntity("MaterialEditor_Scene_LightEntity");
	PreviewLightEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = PreviewLightEntity->GetComponent<FELightComponent>();
	PreviewLightEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(5.0f);
	LightComponent.SetCastShadows(false);

	FEEntity* SkyDome = PreviewScene->CreateEntity("SkyDome");
	SkyDome->GetComponent<FETransformComponent>().SetScale(glm::vec3(150.0f));
	SkyDome->AddComponent<FESkyDomeComponent>();

	PreviewScene->SetFlag(FESceneFlag::Active, false);
	PreviewScene->SetFlag(FESceneFlag::Renderable, false);
	INPUT.AddMouseButtonCallback(MouseButtonCallback);
}

void EditMaterialWindow::MouseButtonCallback(const int Button, const int Action, int Mods)
{
	EDITOR_MATERIAL_WINDOW.bWindowHovered = false;
	if (ImGui::GetCurrentContext()->HoveredWindow == EDITOR_MATERIAL_WINDOW.GetWindow())
		EDITOR_MATERIAL_WINDOW.bWindowHovered = true;

	if (ImGui::GetIO().WantCaptureMouse && (!EDITOR_MATERIAL_WINDOW.bWindowHovered || !EDITOR_MATERIAL_WINDOW.bCameraOutputHovered))
	{
		EDITOR_MATERIAL_WINDOW.PreviewCameraEntity->GetComponent<FECameraComponent>().SetActive(false);
	}

	if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS && EDITOR_MATERIAL_WINDOW.bWindowHovered && EDITOR_MATERIAL_WINDOW.bCameraOutputHovered)
	{
		EDITOR_MATERIAL_WINDOW.PreviewCameraEntity->GetComponent<FECameraComponent>().SetActive(true);
	}
	else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
	{
		EDITOR_MATERIAL_WINDOW.PreviewCameraEntity->GetComponent<FECameraComponent>().SetActive(false);
	}
}

EditMaterialWindow::~EditMaterialWindow()
{
	delete CancelButton;
	delete IconButton;
}

FEMaterial* EditMaterialWindow::ObjectToWorkWith = nullptr;
VisNodeSys::NodeArea* EditMaterialWindow::MaterialNodeArea = nullptr;
ImVec2 EditMaterialWindow::NodeGridRelativePosition = ImVec2(5, 30);
ImVec2 EditMaterialWindow::WindowPosition = ImVec2(0, 0);
ImVec2 EditMaterialWindow::MousePositionWhenContextMenuWasOpened = ImVec2(0, 0);
FETexture* EditMaterialWindow::TextureForNewNode = nullptr;

void EditMaterialWindow::Show(FEMaterial* Material)
{
	if (Material != nullptr)
	{
		PreviewGameModel->SetMaterial(Material);
		PreviewScene->SetFlag(FESceneFlag::Active | FESceneFlag::Renderable, true);

		TemporaryContainer = RESOURCE_MANAGER.NoTexture;
		ObjectToWorkWith = Material;

		std::string TemporaryCaption = "Edit material:";
		TemporaryCaption += " " + ObjectToWorkWith->GetName();
		strcpy_s(Caption, TemporaryCaption.size() + 1, TemporaryCaption.c_str());
		Size = ImVec2(1512.0f, 1000.0f);
		Position = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2 - Size.x / 2, APPLICATION.GetMainWindow()->GetHeight() / 2 - Size.y / 2);
		FEImGuiWindow::Show();

		IconButton = new ImGuiImageButton(nullptr);
		IconButton->SetSize(ImVec2(128, 128));
		IconButton->SetUV0(ImVec2(0.0f, 0.0f));
		IconButton->SetUV1(ImVec2(1.0f, 1.0f));
		IconButton->SetFramePadding(8);

		MaterialNodeArea = NODE_SYSTEM.CreateNodeArea();
		MaterialNodeArea->SetMainContextMenuFunction(NodeSystemMainContextMenu);
		MaterialNodeArea->AddNodeEventCallback(TextureNodeCallback);

		MaterialNodeArea->SetRenderOffset(ImVec2(-340, 250));

		FEEditorMaterialNode* MaterialNode = new FEEditorMaterialNode(Material);

		ImVec2 PositionOnCanvas;
		PositionOnCanvas.x = NodeGridRelativePosition.x + Size.x - Size.x / 6 - MaterialNode->GetSize().x / 2.0f - 150;
		PositionOnCanvas.y = NodeGridRelativePosition.y + Size.y / 2 - MaterialNode->GetSize().y / 2.0f - 300;
		MaterialNode->SetPosition(PositionOnCanvas);

		MaterialNodeArea->AddNode(MaterialNode);

		// Add all textures of material as a texture nodes
		// Place them in shifted grid.
		int VisualIndex = 0;
		for (size_t i = 0; i < Material->Textures.size(); i++)
		{
			if (Material->Textures[i] == nullptr)
				continue;

			FEEditorTextureSourceNode* NewTextureNode = new FEEditorTextureSourceNode(Material->Textures[i]);

			PositionOnCanvas.x = NodeGridRelativePosition.x + Size.x / 2 - NewTextureNode->GetSize().x / 2.0f - (VisualIndex % 2 == 0 ? (NewTextureNode->GetSize().x + 24.0f) : 0.0f);
			PositionOnCanvas.y = NodeGridRelativePosition.y + VisualIndex / 2.0f * (NewTextureNode->GetSize().y + 24.0f);
			NewTextureNode->SetPosition(PositionOnCanvas);

			MaterialNodeArea->AddNode(NewTextureNode);
			VisualIndex++;

			// We should recreate proper connections.
			if (Material->GetAlbedoMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, MaterialNode, 0);
			}
			else if (Material->GetAlbedoMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, MaterialNode, 6);
			}

			if (Material->GetNormalMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, MaterialNode, 1);
			}
			else if (Material->GetNormalMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, MaterialNode, 7);
			}

			if (Material->GetAOMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetAOMapChannel(), MaterialNode, 2);
			}
			else if (Material->GetAOMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetAOMapChannel(), MaterialNode, 8);
			}

			if (Material->GetRoughnessMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetRoughnessMapChannel(), MaterialNode, 3);
			}
			else if (Material->GetRoughnessMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetRoughnessMapChannel(), MaterialNode, 9);
			}

			if (Material->GetMetalnessMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetMetalnessMapChannel(), MaterialNode, 4);
			}
			else if (Material->GetMetalnessMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetMetalnessMapChannel(), MaterialNode, 10);
			}

			if (Material->GetDisplacementMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetDisplacementMapChannel(), MaterialNode, 5);
			}
			else if (Material->GetDisplacementMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjectToWorkWith->GetDisplacementMapChannel(), MaterialNode, 11);
			}
		}
	}
}

void EditMaterialWindow::Render()
{
	bCameraOutputHovered = false;
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	if (ObjectToWorkWith == nullptr)
	{
		FEImGuiWindow::Close();
		return;
	}

	RENDERER.Render(PreviewScene);

	FETexture* CameraResult = RENDERER.GetCameraResult(PreviewCameraEntity);
	if (CameraResult != nullptr)
	{
		ImGui::SetCursorPosX(-10);
		ImGui::SetCursorPosY(-10);
		
		ImGui::Image(CameraResult->GetTextureID(), ImVec2(512, 1020), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		if (ImGui::IsItemHovered())
		{
			bCameraOutputHovered = true;
		}
	}

	MaterialNodeArea->SetPosition(ImVec2(512 - 10, 0));
	MaterialNodeArea->SetSize(ImVec2(ImGui::GetWindowWidth() - 500, ImGui::GetWindowHeight()));
	MaterialNodeArea->Update();
	NodeAreaTarget->StickToItem();

	WindowPosition = ImGui::GetWindowPos();

	if (ImGui::GetIO().MouseReleased[1])
		MousePositionWhenContextMenuWasOpened = ImGui::GetMousePos();

	CancelButton->SetPosition(ImVec2(CancelButton->GetPosition().x, ImGui::GetWindowHeight() - 44.0f));
	
	CancelButton->Render();
	if (CancelButton->IsClicked())
	{
		FEImGuiWindow::Close();
		return;
	}

	FEImGuiWindow::OnRenderEnd();
}

void EditMaterialWindow::Close()
{
	Stop();
	FEImGuiWindow::Close();
}

void EditMaterialWindow::Stop()
{
	PreviewScene->SetFlag(FESceneFlag::Active, false);
	PreviewScene->SetFlag(FESceneFlag::Renderable, false);
	PreviewGameModel->SetMaterial(nullptr);
}

bool EditMaterialWindow::DragAndDropNodeAreaTargetCallback(FEObject* Object, void** CallbackInfo)
{
	if (ObjectToWorkWith->IsTextureInList(RESOURCE_MANAGER.GetTexture(Object->GetID())))
		return false;

	if (ObjectToWorkWith->GetUsedTexturesCount() == FE_MAX_TEXTURES_PER_MATERIAL)
		return false;

	FEEditorTextureSourceNode* NewNode = new FEEditorTextureSourceNode(RESOURCE_MANAGER.GetTexture(Object->GetID()));

	ImVec2 PositionOnCanvas;
	PositionOnCanvas.x = ImGui::GetMousePos().x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
	PositionOnCanvas.y = ImGui::GetMousePos().y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

	NewNode->SetPosition(PositionOnCanvas);
	MaterialNodeArea->AddNode(NewNode);
	return true;
}

void EditMaterialWindow::NodeSystemMainContextMenu()
{
	if (ImGui::BeginMenu("Add"))
	{
		if (ImGui::MenuItem("Texture node"))
		{
			TextureForNewNode = RESOURCE_MANAGER.NoTexture;

			SELECT_FEOBJECT_POPUP.Show(FE_TEXTURE, TextureNodeCreationCallback);
		}

		if (ImGui::MenuItem("Float node"))
		{
			FEEditorFloatSourceNode* NewNode = new FEEditorFloatSourceNode();

			ImVec2 PositionOnCanvas;
			PositionOnCanvas.x = MousePositionWhenContextMenuWasOpened.x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
			PositionOnCanvas.y = MousePositionWhenContextMenuWasOpened.y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

			NewNode->SetPosition(PositionOnCanvas);
			MaterialNodeArea->AddNode(NewNode);
		}

		ImGui::EndMenu();
	}
}

void EditMaterialWindow::TextureNodeCallback(VisNodeSys::Node* Node, const VisNodeSys::NODE_EVENT EventWithNode)
{
	if (Node == nullptr)
		return;

	if (Node->GetType() != "FEEditorTextureSourceNode")
		return;

	if (EventWithNode != VisNodeSys::DESTROYED && EventWithNode != VisNodeSys::REMOVED)
		return;

	const FEEditorTextureSourceNode* CurrentNode = reinterpret_cast<FEEditorTextureSourceNode*>(Node);
	if (ObjectToWorkWith->IsTextureInList(CurrentNode->GetTexture()))
	{
		ObjectToWorkWith->RemoveTexture(CurrentNode->GetTexture());
	}
}

void EditMaterialWindow::TextureNodeCreationCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() != 1 && SelectionsResult[0]->GetType() != FE_TEXTURE)
		return;

	if (SelectionsResult[0] != nullptr && SelectionsResult[0] != RESOURCE_MANAGER.NoTexture)
	{
		if (!ObjectToWorkWith->IsTextureInList(reinterpret_cast<FETexture*>(SelectionsResult[0])))
		{
			if (ObjectToWorkWith->AddTexture(reinterpret_cast<FETexture*>(SelectionsResult[0])))
			{
				FEEditorTextureSourceNode* NewNode = new FEEditorTextureSourceNode(reinterpret_cast<FETexture*>(SelectionsResult[0]));

				ImVec2 PositionOnCanvas;
				PositionOnCanvas.x = MousePositionWhenContextMenuWasOpened.x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
				PositionOnCanvas.y = MousePositionWhenContextMenuWasOpened.y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

				NewNode->SetPosition(PositionOnCanvas);
				MaterialNodeArea->AddNode(NewNode);
			}
		}
	}
}