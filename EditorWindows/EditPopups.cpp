#include "EditPopups.h"
#include "../FEEditor.h"

EditGameModelPopup* EditGameModelPopup::Instance = nullptr;
FEMesh** EditGameModelPopup::MeshToModify = nullptr;

FEMaterial** EditGameModelPopup::MaterialToModify = nullptr;
FEMaterial** EditGameModelPopup::BillboardMaterialToModify = nullptr;

void EditGameModelPopup::ChangeMeshCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (MeshToModify == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MESH)
	{
		FEMesh* SelectedMesh = RESOURCE_MANAGER.GetMesh(SelectionsResult[0]->GetObjectID());
		if (SelectedMesh == nullptr)
			return;

		*MeshToModify = SelectedMesh;
	}
}

void EditGameModelPopup::ChangeMaterialCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		*MaterialToModify = SelectedMaterial;
	}
}

void EditGameModelPopup::ChangeBillboardMaterialCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		*BillboardMaterialToModify = SelectedMaterial;
	}
}

EditGameModelPopup::EditGameModelPopup()
{
	TempModel = new FEGameModel(nullptr, nullptr, "tempGameModel");
	ObjToWorkWith = nullptr;
	Flags = ImGuiWindowFlags_NoResize;

	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	CancelButton->SetHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	CancelButton->SetActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));

	ApplyButton = new ImGuiButton("Apply");
	ChangeMaterialButton = new ImGuiButton("Change Material");
	ChangeBillboardMaterialButton = new ImGuiButton("Change Billboard Material");
	DeleteLODMeshButton = new ImGuiButton("X");
	AddBillboard = new ImGuiButton("Add Billboard");

	LODGroups = new FERangeConfigurator();
	LODGroups->SetSize(ImVec2(870.0f, 40.0f));
	LODGroups->SetPosition(ImVec2(920.0f / 2.0f - 870.0f / 2.0f, 650.0f));

	LODColors.push_back(ImColor(0, 255, 0, 255));
	LODColors.push_back(ImColor(0, 0, 255, 255));
	LODColors.push_back(ImColor(0, 255, 255, 255));
	LODColors.push_back(ImColor(0, 255, 125, 255));
}

EditGameModelPopup::~EditGameModelPopup()
{
	delete CancelButton;
	delete ApplyButton;
	delete ChangeMaterialButton;
	delete ChangeBillboardMaterialButton;

	for (size_t i = 0; i < ChangeLODMeshButton.size(); i++)
	{
		delete ChangeLODMeshButton[i];
	}

	delete DeleteLODMeshButton;
	delete AddBillboard;
	delete LODGroups;
}

void EditGameModelPopup::Show(FEGameModel* GameModel)
{
	if (GameModel != nullptr)
	{
		ObjToWorkWith = GameModel;
		Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		TempModel->SetMaterial(ObjToWorkWith->GetMaterial());
		TempModel->SetScaleFactor(ObjToWorkWith->GetScaleFactor());
		UpdatedMaterial = ObjToWorkWith->GetMaterial();
		UpdatedBillboardMaterial = ObjToWorkWith->GetBillboardMaterial();
		TempModel->SetUsingLOD(ObjToWorkWith->IsUsingLOD());
		TempModel->SetBillboardZeroRotaion(ObjToWorkWith->GetBillboardZeroRotaion());

		ChangeLODMeshButton.clear();
		if (ObjToWorkWith->IsUsingLOD())
		{
			ChangeLODMeshButton.resize(ObjToWorkWith->GetMaxLODCount());
			for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
			{
				const std::string ButtonName = "Change LOD" + std::to_string(i) + " Mesh";
				ChangeLODMeshButton[i] = new ImGuiButton(ButtonName);
				ChangeLODMeshButton[i]->SetSize(ImVec2(200, 35));
			}
		}
		else
		{
			ChangeLODMeshButton.resize(1);
			ChangeLODMeshButton[0] = new ImGuiButton("Change Mesh");
			ChangeLODMeshButton[0]->SetSize(ImVec2(200, 35));
			ChangeLODMeshButton[0]->SetPosition(ImVec2(Size.x / 2.0f - Size.x / 4.0f - ChangeLODMeshButton[0]->GetSize().x / 2, 35 + 340.0f));
		}

		UpdatedLODMeshs.clear();
		for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
		{
			TempModel->SetLODMesh(i, ObjToWorkWith->GetLODMesh(i));
			TempModel->SetLODMaxDrawDistance(i, ObjToWorkWith->GetLODMaxDrawDistance(i));
			TempModel->SetIsLODBillboard(i, ObjToWorkWith->IsLODBillboard(i));
			UpdatedLODMeshs.push_back(ObjToWorkWith->GetLODMesh(i));
		}
		TempModel->SetCullDistance(ObjToWorkWith->GetCullDistance());

		if (ObjToWorkWith->IsUsingLOD())
		{
			SwitchMode(HAS_LOD_MODE);
		}
		else
		{
			SwitchMode(NO_LOD_MODE);
		}

		std::string TempCaption = "Edit game model:";
		TempCaption += " " + ObjToWorkWith->GetName();
		strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());

		Position = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2 - Size.x / 2, APPLICATION.GetMainWindow()->GetHeight() / 2 - Size.y / 2);
		FEImGuiWindow::Show();

		PREVIEW_MANAGER.CreateGameModelPreview(TempModel, &TempPreview);

		ChangeMaterialButton->SetSize(ImVec2(200, 35));
		ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeMaterialButton->GetSize().x / 2, 35 + 340.0f));
		ChangeBillboardMaterialButton->SetSize(ImVec2(270, 35));
		ChangeBillboardMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeBillboardMaterialButton->GetSize().x / 2, 35 + 340.0f));

		DeleteLODMeshButton->SetSize(ImVec2(24, 25));
		DeleteLODMeshButton->SetDefaultColor(ImVec4(0.9f, 0.5f, 0.5f, 1.0f));

		AddBillboard->SetSize(ImVec2(200, 35));

		ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
		CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));

		// ************** LOD Groups **************
		LODGroups->Clear();
		float Previous = 0.0f;
		for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
		{
			if (ObjToWorkWith->GetLODMesh(i) != nullptr)
			{
				LODGroups->AddRange(((ObjToWorkWith->GetLODMaxDrawDistance(i)) - Previous) / ObjToWorkWith->GetCullDistance(), std::string("LOD") + std::to_string(i), "", LODColors[i]);
				Previous = ObjToWorkWith->GetLODMaxDrawDistance(i);
			}
		}
		// ************** LOD Groups END **************

		// ************** Drag&Drop **************
		LODMeshCallbackInfo.resize(ObjToWorkWith->GetMaxLODCount());
		LODMeshTarget.resize(ObjToWorkWith->GetMaxLODCount());
		for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
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

void EditGameModelPopup::SwitchMode(const int ToMode)
{
	switch (ToMode)
	{
		case NO_LOD_MODE:
		{
			Size.x = NO_LOD_WINDOW_WIDTH;
			Size.y = NO_LOD_WINDOW_HEIGHT;
			CurrentMode = NO_LOD_MODE;
			ChangeLODMeshButton[0]->SetCaption("Change Mesh");
			ChangeLODMeshButton[0]->SetPosition(ImVec2(Size.x / 2.0f - Size.x / 4.0f - ChangeLODMeshButton[0]->GetSize().x / 2, 35 + 340.0f));
			ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeMaterialButton->GetSize().x / 2, 35 + 340.0f));
			ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
			CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));

			for (size_t i = 1; i < TempModel->GetMaxLODCount(); i++)
			{
				UpdatedLODMeshs[i] = nullptr;
				TempModel->SetLODMesh(i, nullptr);
			}

			UpdatedBillboardMaterial = nullptr;
			TempModel->SetBillboardMaterial(nullptr);

			LODGroups->Clear();
			LODGroups->AddRange((ObjToWorkWith->GetLODMaxDrawDistance(0)) / ObjToWorkWith->GetCullDistance(), "LOD0", "", ImColor(0, 255, 0, 255));
			break;
		}

		case HAS_LOD_MODE:
		{
			ChangeLODMeshButton.resize(ObjToWorkWith->GetMaxLODCount());
			for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
			{
				const std::string ButtonName = "Change LOD" + std::to_string(i) + " Mesh";
				ChangeLODMeshButton[i] = new ImGuiButton(ButtonName);
				ChangeLODMeshButton[i]->SetSize(ImVec2(200, 35));
			}

			Size.x = 920.0f;
			Size.y = 880.0f;
			CurrentMode = HAS_LOD_MODE;
			ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2.0f - Size.x / 4.0f - ChangeMaterialButton->GetSize().x / 2, 35.0f + 340.0f + 200.0f));
			ChangeBillboardMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeBillboardMaterialButton->GetSize().x / 2, 35 + 340.0f + 200.0f));
			ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
			CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));
			break;
		}
		default:
			break;
	}
}

void EditGameModelPopup::DisplayLODGroups()
{
	const ImVec2 TextSize = ImGui::CalcTextSize("LOD Groups: ");
	ImGui::SetCursorPosX(Size.x / 2 - TextSize.x / 2);
	ImGui::SetCursorPosY(ChangeBillboardMaterialButton->GetPosition().y + ChangeBillboardMaterialButton->GetSize().y + 10.0f);
	ImGui::Text("LOD Groups: ");

	LODGroups->Render();

	float TotalRangeSpan = 0.0f;
	for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
	{
		FERangeRecord* Record = LODGroups->GetRangesRecord(i);
		if (Record == nullptr)
			break;

		const float RangeSpan = Record->GetRangeSpan();
		TotalRangeSpan += RangeSpan;

		if (TempModel->GetLODMesh(i) == nullptr)
			break;

		if (TempModel->IsLODBillboard(i))
		{
			Record->SetCaption(std::string("Billboard"));
		}
		else
		{
			Record->SetCaption(std::string("LOD") + std::to_string(i));
		}

		TempModel->SetLODMaxDrawDistance(i, TempModel->GetCullDistance() * TotalRangeSpan);
		std::string NewToolTip;
		if (i == 0)
		{
			NewToolTip = Record->GetCaption() + "(0 - " + std::to_string(static_cast<int>(TempModel->GetLODMaxDrawDistance(i))) + "m) " + std::to_string(RangeSpan * 100.0f) + "%";
		}
		else
		{
			int EndDistance = static_cast<int>(TempModel->GetLODMaxDrawDistance(i));
			if (i == TempModel->GetMaxLODCount() - 1 || TempModel->GetLODMesh(i + 1) == nullptr)
				EndDistance = static_cast<int>(TempModel->GetCullDistance());
			NewToolTip = Record->GetCaption() + "(" + std::to_string(static_cast<int>(TempModel->GetLODMaxDrawDistance(i - 1))) + " - " + std::to_string(EndDistance) + "m) " + std::to_string(RangeSpan * 100.0f) + "%";
		}

		Record->SetToolTipText(NewToolTip);
	}

	ImGui::SetCursorPosY(LODGroups->GetPosition().y + LODGroups->GetSize().y + 10.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
	ImGui::Text("cullDistance:");
	float CurrentCullRange = TempModel->GetCullDistance();
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	ImGui::DragFloat("##cullDistance", &CurrentCullRange, 1.0f, 0.1f, 5000.0f);
	TempModel->SetCullDistance(CurrentCullRange);
}

void EditGameModelPopup::Render()
{
	if (!IsVisible())
		return;

	if (TempModel->IsUsingLOD())
	{
		SwitchMode(HAS_LOD_MODE);
	}
	else
	{
		SwitchMode(NO_LOD_MODE);
	}

	ImGui::SetNextWindowSize(Size);
	FEImGuiWindow::Render();

	// if we change something we will update preview.
	if (UpdatedMaterial != TempModel->GetMaterial())
	{
		TempModel->SetMaterial(UpdatedMaterial);
		PREVIEW_MANAGER.CreateGameModelPreview(TempModel, &TempPreview);
	}

	if (UpdatedBillboardMaterial != TempModel->GetBillboardMaterial())
	{
		TempModel->SetBillboardMaterial(UpdatedBillboardMaterial);
	}

	for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
	{
		if (UpdatedLODMeshs[i] != TempModel->GetLODMesh(i))
		{
			if (TempModel->GetLODMesh(i) == nullptr)
			{
				LODGroups->AddRange((TempModel->GetLODMaxDrawDistance(i)) / TempModel->GetCullDistance(), std::string("LOD") + std::to_string(i), "", LODColors[i]);
			}

			TempModel->SetLODMesh(i, UpdatedLODMeshs[i]);
			PREVIEW_MANAGER.CreateGameModelPreview(TempModel, &TempPreview);
		}
	}

	if (ObjToWorkWith == nullptr)
	{
		FEImGuiWindow::Close();
		return;
	}

	const float BaseY = 35.0f;
	const float CurrentY = BaseY;

	bool bLODActive = TempModel->IsUsingLOD();
	ImGui::SetCursorPosY(CurrentY);
	ImGui::Checkbox("have LOD levels", &bLODActive);
	TempModel->SetUsingLOD(bLODActive);
	/*if (tempModel->IsUsingLOD())
	{
		switchMode(HAS_LOD_MODE);
	}
	else
	{
		switchMode(NO_LOD_MODE);
	}*/

	ImVec2 TextSize = ImGui::CalcTextSize("Preview of game model:");
	ImGui::SetCursorPosX(Size.x / 2 - TextSize.x / 2);
	ImGui::SetCursorPosY(CurrentY + 30);
	ImGui::Text("Preview of game model:");
	ImGui::SetCursorPosX(Size.x / 2 - 128 / 2);
	ImGui::SetCursorPosY(CurrentY + 50);
	// FIX ME!
	//ImGui::Image((void*)static_cast<intptr_t>(TempPreview->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	if (CurrentMode == NO_LOD_MODE)
	{
		ImGui::Separator();
		TextSize = ImGui::CalcTextSize("Mesh component:");
		ImGui::SetCursorPosX(Size.x / 4 - TextSize.x / 2);
		ImGui::Text("Mesh component:");
		ImGui::SetCursorPosX(Size.x / 4 - 128 / 2);
		ImGui::SetCursorPosY(CurrentY + 210.0f);
		ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMeshPreview(TempModel->Mesh->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		LODMeshTarget[0]->StickToItem();

		ChangeLODMeshButton[0]->Render();
		if (ChangeLODMeshButton[0]->IsClicked())
		{
			UpdatedLODMeshs[0] = TempModel->GetLODMesh(0);

			MeshToModify = &UpdatedLODMeshs[0];
			SelectFEObjectPopUp::getInstance().Show(FE_MESH, ChangeMeshCallBack, UpdatedLODMeshs[0]);
		}

		TextSize = ImGui::CalcTextSize("Material component:");
		ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - TextSize.x / 2);
		ImGui::SetCursorPosY(CurrentY + 187.0f);
		ImGui::Text("Material component:");
		ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - 128 / 2);
		ImGui::SetCursorPosY(CurrentY + 210.0f);
		ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMaterialPreview(TempModel->Material->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		MaterialTarget->StickToItem();
		ChangeMaterialButton->Render();
		if (ChangeMaterialButton->IsClicked())
		{
			UpdatedMaterial = TempModel->GetMaterial();

			MaterialToModify = &UpdatedMaterial;
			SelectFEObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeMaterialCallBack, UpdatedMaterial);
		}
	}
	else if (CurrentMode == HAS_LOD_MODE)
	{
		ImGui::Separator();

		const float BaseXPosition = Size.x / 2.0f - Size.x / 4.0f;
		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			const float CurrentXPosition = BaseXPosition + (Size.x / 4.0f) * i - Size.x / 8.0f;
			if (TempModel->GetLODMesh(i) == nullptr)
			{
				if (TempModel->IsLODBillboard(i - 1))
					break;

				ChangeLODMeshButton[i]->SetCaption(std::string("Add LOD") + std::to_string(i));
				ChangeLODMeshButton[i]->SetPosition(ImVec2(CurrentXPosition - ChangeLODMeshButton[i]->GetSize().x / 2, CurrentY + 210.0f + 128.0f / 2.0f - 10.0f - ChangeLODMeshButton[i]->GetSize().y/*- changeLODMeshButton[i]->getSize().y / 2.0f*/));

				ChangeLODMeshButton[i]->Render();
				if (ChangeLODMeshButton[i]->IsClicked())
				{
					UpdatedLODMeshs[i] = TempModel->GetLODMesh(i);

					MeshToModify = &UpdatedLODMeshs[i];
					SelectFEObjectPopUp::getInstance().Show(FE_MESH, ChangeMeshCallBack, UpdatedLODMeshs[i]);
				}

				AddBillboard->SetPosition(ImVec2(CurrentXPosition - AddBillboard->GetSize().x / 2, CurrentY + 210.0f + 128.0f / 2.0f + 10.0f / 2.0f));
				AddBillboard->Render();
				if (AddBillboard->IsClicked())
				{
					UpdatedLODMeshs[i] = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
					UpdatedBillboardMaterial = RESOURCE_MANAGER.GetMaterial("61649B9E0F08013Q3939316C"/*"FEPBRBaseMaterial"*/);
					TempModel->SetIsLODBillboard(i, true);
				}

				break;
			}
			else
			{
				std::string Caption = (std::string("LOD") + std::to_string(i)) + ":";
				if (TempModel->IsLODBillboard(i))
					Caption = "Billboard:";

				TextSize = ImGui::CalcTextSize(Caption.c_str());
				ImGui::SetCursorPosX(CurrentXPosition - TextSize.x / 2.0f);
				ImGui::SetCursorPosY(CurrentY + 187.0f);
				ImGui::Text(Caption.c_str());
				ImGui::SetCursorPosX(CurrentXPosition - 128 / 2);
				ImGui::SetCursorPosY(CurrentY + 210.0f);

				ImGui::Image((void*)static_cast<intptr_t>(TempModel->GetLODMesh(i) == nullptr
					                                          ? RESOURCE_MANAGER.NoTexture->GetTextureID()
					                                          : PREVIEW_MANAGER.GetMeshPreview(TempModel->GetLODMesh(i)->GetObjectID())->GetTextureID()),
														  ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				LODMeshTarget[i]->StickToItem();

				ChangeLODMeshButton[i]->SetCaption(std::string("Change LOD") + std::to_string(i) + " Mesh");
				if (TempModel->IsLODBillboard(i))
					ChangeLODMeshButton[i]->SetCaption(std::string("Change Billboard"));
				ChangeLODMeshButton[i]->SetPosition(ImVec2(CurrentXPosition - ChangeLODMeshButton[i]->GetSize().x / 2, 35 + 340.0f));

				if (IsLastSetupLOD(i) && i > 0)
				{
					DeleteLODMeshButton->SetPosition(ImVec2(CurrentXPosition + 80.0f, 215.0f));
					DeleteLODMeshButton->Render();
					if (DeleteLODMeshButton->IsClicked())
					{
						UpdatedLODMeshs[i] = nullptr;
						TempModel->SetLODMesh(i, nullptr);
						TempModel->SetIsLODBillboard(i, false);
						LODGroups->DeleteRange(i);
					}
				}
			}

			ChangeLODMeshButton[i]->Render();
			if (ChangeLODMeshButton[i]->IsClicked())
			{
				UpdatedLODMeshs[i] = TempModel->GetLODMesh(i);

				MeshToModify = &UpdatedLODMeshs[i];
				SelectFEObjectPopUp::getInstance().Show(FE_MESH, ChangeMeshCallBack, UpdatedLODMeshs[i]);
			}
		}

		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			if (IsLastSetupLOD(i))
			{
				if (TempModel->IsLODBillboard(i))
				{
					TextSize = ImGui::CalcTextSize("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - Size.x / 4 - TextSize.x / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 187.0f);
					ImGui::Text("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - Size.x / 4 - 128 / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 210.0f);
					ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMaterialPreview(TempModel->GetMaterial()->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					MaterialTarget->StickToItem();

					ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 - Size.x / 4 - ChangeMaterialButton->GetSize().x / 2, BaseY + 340.0f + 200.0f));
					ChangeMaterialButton->Render();
					if (ChangeMaterialButton->IsClicked())
					{
						MaterialToModify = &UpdatedMaterial;
						SelectFEObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeMaterialCallBack, UpdatedMaterial);
					}

					TextSize = ImGui::CalcTextSize("Billboard Material component:");
					ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - TextSize.x / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 187.0f);
					ImGui::Text("Billboard Material component:");
					ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - 128 / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 210.0f);
					ImGui::Image((void*)static_cast<intptr_t>(TempModel->GetBillboardMaterial() == nullptr
						                                          ? RESOURCE_MANAGER.NoTexture->GetTextureID()
						                                          : PREVIEW_MANAGER.GetMaterialPreview(TempModel->GetBillboardMaterial()->GetObjectID())->
						                                          GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					BillboardMaterialTarget->StickToItem();

					ChangeBillboardMaterialButton->Render();
					if (ChangeBillboardMaterialButton->IsClicked())
					{
						UpdatedBillboardMaterial = TempModel->GetBillboardMaterial();

						const std::vector<std::string> TempMaterialList = RESOURCE_MANAGER.GetMaterialList();
						std::vector<FEObject*> FinalMaterialList;
						for (size_t j = 0; j < TempMaterialList.size(); j++)
						{
							if (RESOURCE_MANAGER.GetMaterial(TempMaterialList[j])->Shader->GetObjectID() == "0800253C242B05321A332D09"/*"FEPBRShader"*/)
							{
								FinalMaterialList.push_back(RESOURCE_MANAGER.GetMaterial(TempMaterialList[j]));
							}
						}

						BillboardMaterialToModify = &UpdatedBillboardMaterial;
						SelectFEObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeBillboardMaterialCallBack, UpdatedBillboardMaterial, FinalMaterialList);
					}
				}
				else
				{
					TextSize = ImGui::CalcTextSize("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - TextSize.x / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 187.0f);
					ImGui::Text("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - 128 / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 210.0f);
					ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMaterialPreview(TempModel->GetMaterial()->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

					ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 - ChangeMaterialButton->GetSize().x / 2, BaseY + 340.0f + 200.0f));
					ChangeMaterialButton->Render();
					if (ChangeMaterialButton->IsClicked())
					{
						MaterialToModify = &UpdatedMaterial;
						SelectFEObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeMaterialCallBack, UpdatedMaterial);
					}
				}

				break;
			}
		}

		// ************** LOD Groups **************
		ImGui::Separator();
		DisplayLODGroups();

		bool bBillboard = false;
		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			if (TempModel->IsLODBillboard(i))
				bBillboard = true;
		}

		if (bBillboard)
		{
			Size.y = 830.0f + 40.0f;
			ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
			CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));

			/*ImGui::Text("Billboard Scale:");
			float billboardScale = tempModel->getBillboardScale();
			ImGui::SameLine();
			ImGui::DragFloat("##Billboard Scale", &billboardScale, 0.1f);
			tempModel->setBillboardScale(billboardScale);*/

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			ImGui::Text("Billboard Zero Rotation:");
			float ZeroRotation = TempModel->GetBillboardZeroRotaion();
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
			ImGui::DragFloat("##Billboard Zero Rotation", &ZeroRotation, 0.1f, 0.0f, 360.0f);
			TempModel->SetBillboardZeroRotaion(ZeroRotation);
		}
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
	ImGui::Text("Scale Factor:");
	float ScaleFactor = TempModel->GetScaleFactor();
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	ImGui::SetNextItemWidth(200);
	ImGui::DragFloat("##Scale Factor", &ScaleFactor, 0.1f);
	TempModel->SetScaleFactor(ScaleFactor);

	ImGui::SetCursorPosY(Size.y - 50.0f);
	ImGui::Separator();
	ImGui::SetItemDefaultFocus();
	ApplyButton->Render();
	if (ApplyButton->IsClicked())
	{
		ObjToWorkWith->SetDirtyFlag(true);
		ObjToWorkWith->Mesh = TempModel->Mesh;
		ObjToWorkWith->SetUsingLOD(TempModel->IsUsingLOD());

		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			ObjToWorkWith->SetLODMesh(i, TempModel->GetLODMesh(i));
			ObjToWorkWith->SetLODMaxDrawDistance(i, TempModel->GetLODMaxDrawDistance(i));
			ObjToWorkWith->SetIsLODBillboard(i, TempModel->IsLODBillboard(i));
		}

		ObjToWorkWith->SetCullDistance(TempModel->GetCullDistance());
		ObjToWorkWith->SetMaterial(TempModel->GetMaterial());
		ObjToWorkWith->SetBillboardMaterial(TempModel->GetBillboardMaterial());
		ObjToWorkWith->SetScaleFactor(TempModel->GetScaleFactor());
		ObjToWorkWith->SetBillboardZeroRotaion(TempModel->GetBillboardZeroRotaion());
		PREVIEW_MANAGER.CreateGameModelPreview(ObjToWorkWith->GetObjectID());

		FEImGuiWindow::Close();
		return;
	}

	ImGui::SameLine();
	CancelButton->Render();
	if (CancelButton->IsClicked())
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
	Info->Window->UpdatedLODMeshs[Info->LODLevel] = RESOURCE_MANAGER.GetMesh(Object->GetObjectID());
	Info->Window->TempModel->SetLODMesh(Info->LODLevel, RESOURCE_MANAGER.GetMesh(Object->GetObjectID()));
	PREVIEW_MANAGER.CreateGameModelPreview(Info->Window->TempModel, &Info->Window->TempPreview);
	return true;
}

bool EditGameModelPopup::DragAndDropMaterialCallback(FEObject* Object, void** CallbackInfo)
{
	const MaterialTargetCallbackInfo* Info = reinterpret_cast<MaterialTargetCallbackInfo*>(CallbackInfo);

	if (Info->bBillboardMaterial)
	{
		Info->Window->UpdatedBillboardMaterial = RESOURCE_MANAGER.GetMaterial(Object->GetObjectID());
		Info->Window->TempModel->SetBillboardMaterial(RESOURCE_MANAGER.GetMaterial(Object->GetObjectID()));
	}
	else
	{
		Info->Window->UpdatedMaterial = RESOURCE_MANAGER.GetMaterial(Object->GetObjectID());
		Info->Window->TempModel->SetMaterial(RESOURCE_MANAGER.GetMaterial(Object->GetObjectID()));
		PREVIEW_MANAGER.CreateGameModelPreview(Info->Window->TempModel, &Info->Window->TempPreview);
	}

	return true;
}
// ************** Drag&Drop END **************

bool EditGameModelPopup::IsLastSetupLOD(const size_t LODIndex)
{
	if (LODIndex >= TempModel->GetMaxLODCount())
		return false;

	if (LODIndex == TempModel->GetMaxLODCount() - 1)
		return true;

	if (TempModel->IsLODBillboard(LODIndex))
		return true;

	for (size_t i = LODIndex + 1; i < TempModel->GetMaxLODCount(); i++)
	{
		if (TempModel->GetLODMesh(i) != nullptr)
			return false;
	}

	return true;
}

EditMaterialWindow* EditMaterialWindow::Instance = nullptr;

EditMaterialWindow::EditMaterialWindow()
{
	ObjToWorkWith = nullptr;
	Flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	CancelButton->SetHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	CancelButton->SetActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));
	NodeAreaTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_TEXTURE, DragAndDropnodeAreaTargetCallback, reinterpret_cast<void**>(&DragAndDropCallbackInfo), "Drop to add texture");

	PreviewScene = SCENE_MANAGER.CreateScene("MaterialEditor_Scene");
	RESOURCE_MANAGER.SetTag(PreviewScene, EDITOR_RESOURCE_TAG);

	PreviewGameModel = new FEGameModel(nullptr, nullptr, "MaterialEditor_Preview_GameModel");
	PreviewGameModel->Mesh = RESOURCE_MANAGER.GetMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);

	PreviewEntity = PreviewScene->CreateEntity("MaterialEditor_Scene_PreviewEntity");
	PreviewEntity->AddComponent<FEGameModelComponent>(PreviewGameModel);
	PreviewEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
	PreviewEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(0.1f));
	RESOURCE_MANAGER.SetTag(PreviewEntity, EDITOR_RESOURCE_TAG);

	PreviewCameraEntity = PreviewScene->CreateEntity("MaterialEditor_Scene_CameraEntity");
	PreviewCameraEntity->AddComponent<FECameraComponent>();
	FECameraComponent& CameraComponent = PreviewCameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.Type = 1;
	CameraComponent.DistanceToModel = 10.0;
	CameraComponent.SetRenderTargetSize(512, 1020);
	CameraComponent.SetDistanceFogEnabled(false);
	CAMERA_SYSTEM.SetMainCamera(PreviewCameraEntity);

	PreviewLightEntity = PreviewScene->CreateEntity("MaterialEditor_Scene_LightEntity");
	PreviewLightEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = PreviewLightEntity->GetComponent<FELightComponent>();
	PreviewLightEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(5.0f);
	LightComponent.SetCastShadows(false);

	FEEntity* SkyDome = PreviewScene->CreateEntity("SkyDome");
	SkyDome->GetComponent<FETransformComponent>().SetScale(glm::vec3(150.0f));
	SkyDome->AddComponent<FESkyDomeComponent>();

	SCENE_MANAGER.DeactivateScene(PreviewScene);
	ENGINE.AddMouseButtonCallback(MouseButtonCallback);
}

void EditMaterialWindow::MouseButtonCallback(const int Button, const int Action, int Mods)
{
	EDITOR_MATERIAL_WINDOW.bWindowHovered = false;
	if (ImGui::GetCurrentContext()->HoveredWindow == EDITOR_MATERIAL_WINDOW.GetWindow())
		EDITOR_MATERIAL_WINDOW.bWindowHovered = true;

	if (ImGui::GetIO().WantCaptureMouse && (!EDITOR_MATERIAL_WINDOW.bWindowHovered || !EDITOR_MATERIAL_WINDOW.bCameraOutputHovered))
	{
		CAMERA_SYSTEM.SetIsIndividualInputActive(EDITOR_MATERIAL_WINDOW.PreviewCameraEntity, false);
	}

	if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_PRESS && EDITOR_MATERIAL_WINDOW.bWindowHovered && EDITOR_MATERIAL_WINDOW.bCameraOutputHovered)
	{
		CAMERA_SYSTEM.SetIsIndividualInputActive(EDITOR_MATERIAL_WINDOW.PreviewCameraEntity, true);
	}
	else if (Button == GLFW_MOUSE_BUTTON_2 && Action == GLFW_RELEASE)
	{
		CAMERA_SYSTEM.SetIsIndividualInputActive(EDITOR_MATERIAL_WINDOW.PreviewCameraEntity, false);
	}
}

EditMaterialWindow::~EditMaterialWindow()
{
	delete CancelButton;
	delete IconButton;
}

FEMaterial* EditMaterialWindow::ObjToWorkWith = nullptr;
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
		SCENE_MANAGER.ActivateScene(PreviewScene);

		TempContainer = RESOURCE_MANAGER.NoTexture;
		ObjToWorkWith = Material;

		std::string TempCaption = "Edit material:";
		TempCaption += " " + ObjToWorkWith->GetName();
		strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());
		Size = ImVec2(1512.0f, 1000.0f);
		Position = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2 - Size.x / 2, APPLICATION.GetMainWindow()->GetHeight() / 2 - Size.y / 2);
		FEImGuiWindow::Show();

		IconButton = new ImGuiImageButton(nullptr);
		IconButton->SetSize(ImVec2(128, 128));
		IconButton->SetUV0(ImVec2(0.0f, 0.0f));
		IconButton->SetUV1(ImVec2(1.0f, 1.0f));
		IconButton->SetFramePadding(8);

		MaterialNodeArea = NODE_SYSTEM.CreateNodeArea();
		MaterialNodeArea->SetMainContextMenuFunc(NodeSystemMainContextMenu);
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
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetAOMapChannel(), MaterialNode, 2);
			}
			else if (Material->GetAOMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetAOMapChannel(), MaterialNode, 8);
			}

			if (Material->GetRoughnessMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetRoughnessMapChannel(), MaterialNode, 3);
			}
			else if (Material->GetRoughnessMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetRoughnessMapChannel(), MaterialNode, 9);
			}

			if (Material->GetMetalnessMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetMetalnessMapChannel(), MaterialNode, 4);
			}
			else if (Material->GetMetalnessMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetMetalnessMapChannel(), MaterialNode, 10);
			}

			if (Material->GetDisplacementMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetDisplacementMapChannel(), MaterialNode, 5);
			}
			else if (Material->GetDisplacementMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetDisplacementMapChannel(), MaterialNode, 11);
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

	if (ObjToWorkWith == nullptr)
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
		
		ImGui::Image((void*)(intptr_t)CameraResult->GetTextureID(), ImVec2(512, 1020), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
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
	SCENE_MANAGER.DeactivateScene(PreviewScene);
	PreviewGameModel->SetMaterial(nullptr);
}

bool EditMaterialWindow::DragAndDropnodeAreaTargetCallback(FEObject* Object, void** CallbackInfo)
{
	if (ObjToWorkWith->IsTextureInList(RESOURCE_MANAGER.GetTexture(Object->GetObjectID())))
		return false;

	if (ObjToWorkWith->GetUsedTexturesCount() == FE_MAX_TEXTURES_PER_MATERIAL)
		return false;

	FEEditorTextureSourceNode* NewNode = new FEEditorTextureSourceNode(RESOURCE_MANAGER.GetTexture(Object->GetObjectID()));

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

			SelectFEObjectPopUp::getInstance().Show(FE_TEXTURE, TextureNodeCreationCallback);
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
	if (ObjToWorkWith->IsTextureInList(CurrentNode->GetTexture()))
	{
		ObjToWorkWith->RemoveTexture(CurrentNode->GetTexture());
	}
}

void EditMaterialWindow::TextureNodeCreationCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() != 1 && SelectionsResult[0]->GetType() != FE_TEXTURE)
		return;

	if (SelectionsResult[0] != nullptr && SelectionsResult[0] != RESOURCE_MANAGER.NoTexture)
	{
		if (!ObjToWorkWith->IsTextureInList(reinterpret_cast<FETexture*>(SelectionsResult[0])))
		{
			if (ObjToWorkWith->AddTexture(reinterpret_cast<FETexture*>(SelectionsResult[0])))
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