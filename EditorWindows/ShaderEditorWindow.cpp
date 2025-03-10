#include "ShaderEditorWindow.h"
#include "../FEEditor.h"

ShaderDebugWindow::ShaderDebugWindow()
{
	Flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
	Editor.SetShowWhitespaces(false);
	Editor.SetReadOnly(true);
	Size = ImVec2(800, 600);
	CloseButton = new ImGuiButton("Close");
	UpdateButton = new ImGuiButton("Update");
	Editor.SetPalette(TextEditor::GetLightPalette());
	Editor.SetColorizerEnable(false);
}

ShaderDebugWindow::~ShaderDebugWindow()
{
	delete CloseButton;
	delete UpdateButton;

	ShaderToWorkWith = nullptr;
}

void ShaderDebugWindow::Show(FEShader* Shader, std::string Caption)
{
	bUpdateNeeded = true;
	ShaderToWorkWith = Shader;

	SelectedDebugData = "";
	OccurrenceList.clear();
	Data = nullptr;
	DataDump.clear();

	if (Caption.empty())
		Caption = "Text view";

	strcpy_s(this->Caption, Caption.size() + 1, Caption.c_str());
	FEImGuiWindow::Show();
}

void ShaderDebugWindow::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	if (bUpdateNeeded)
	{
		Data = ShaderToWorkWith->GetDebugData();
		if (!Data->empty())
		{
			DataDump.clear();
			OccurrenceList.clear();
			bUpdateNeeded = false;
			for (size_t i = 0; i < Data->size(); i++)
			{
				std::string DebugItemsCount = std::to_string(Data->operator[](i)[0]);
				for (size_t j = 0; j < DebugItemsCount.size(); j++)
				{
					if (DebugItemsCount[j] == '.')
					{
						DebugItemsCount.erase(DebugItemsCount.begin() + j, DebugItemsCount.end());
						break;
					}
				}

				OccurrenceList.push_back("occurrence with " + DebugItemsCount + " debug items");
				DataDump.push_back(Data->operator[](i));
			}

			SelectedDebugData = OccurrenceList[0];
			std::string text;
			const std::vector<std::string> DebugVariables = ShaderToWorkWith->GetDebugVariables();
			for (size_t i = 1; i < DataDump[0].size(); i++)
			{
				const float t = DataDump[0][i];
				text += DebugVariables[(i - 1) % DebugVariables.size()];
				text += " : ";
				text += std::to_string(t);
				if (i < DataDump[0].size() - 1)
					text += "\n";
			}
			Editor.SetText(text);
		}
	}

	if (ImGui::BeginCombo("Shader occurrence", SelectedDebugData.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t n = 0; n < OccurrenceList.size(); n++)
		{
			ImGui::PushID(static_cast<int>(n));
			const bool is_selected = (SelectedDebugData == OccurrenceList[n]);
			if (ImGui::Selectable(OccurrenceList[n].c_str(), is_selected))
			{
				SelectedDebugData = OccurrenceList[n];
				std::string text;
				std::vector<std::string> DebugVariables = ShaderToWorkWith->GetDebugVariables();
				for (size_t i = 1; i < DataDump[n].size(); i++)
				{
					const float t = DataDump[n][i];
					text += DebugVariables[(i - 1) % DebugVariables.size()];
					text += " : ";
					text += std::to_string(t);
					if (i < DataDump[n].size() - 1)
						text += "\n";
				}
				Editor.SetText(text);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();

			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	CloseButton->Render();
	if (CloseButton->IsClicked())
	{
		FEImGuiWindow::Close();
	}

	UpdateButton->Render();
	if (UpdateButton->IsClicked())
	{
		bUpdateNeeded = true;
	}

	Editor.Render("TextEditor", ImVec2(FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl()->Size.x, 250));
	FEImGuiWindow::OnRenderEnd();
}

ShaderEditorWindow::ShaderEditorWindow()
{
	Flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
	CurrentEditor = &VertexShaderEditor;
	VertexShaderEditor.SetShowWhitespaces(false);
	VertexShaderEditor.SetPalette(TextEditor::GetLightPalette());
	TessControlShaderEditor.SetShowWhitespaces(false);
	TessControlShaderEditor.SetPalette(TextEditor::GetLightPalette());
	TessEvalShaderEditor.SetShowWhitespaces(false);
	TessEvalShaderEditor.SetPalette(TextEditor::GetLightPalette());
	GeometryShaderEditor.SetShowWhitespaces(false);
	GeometryShaderEditor.SetPalette(TextEditor::GetLightPalette());
	FragmentShaderEditor.SetShowWhitespaces(false);
	FragmentShaderEditor.SetPalette(TextEditor::GetLightPalette());
	ComputeShaderEditor.SetShowWhitespaces(false);
	ComputeShaderEditor.SetPalette(TextEditor::GetLightPalette());

	Size = ImVec2(800, 650);
	CompileButton = new ImGuiButton("Compile");

	CloseButton = new ImGuiButton("Close");
	CloseButton->SetDefaultColor((ImVec4)ImColor(0.6f, 0.24f, 0.24f));
	CloseButton->SetHoveredColor((ImVec4)ImColor(0.7f, 0.21f, 0.21f));
	CloseButton->SetHoveredColor((ImVec4)ImColor(0.8f, 0.16f, 0.16f));
}

ShaderEditorWindow::~ShaderEditorWindow()
{
	delete CompileButton;
}

void ShaderEditorWindow::Show(FEShader* Shader)
{
	ShaderToEdit = Shader;
	std::string TempCaption = "Edit shader: ";
	TempCaption += ShaderToEdit->GetName();
	strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());

	FEImGuiWindow::Show();
	CurrentEditor->SetText("");
	CurrentEditor = nullptr;

	if (ShaderToEdit->GetVertexShaderText() != nullptr)
	{
		VertexShaderEditor.SetText(ShaderToEdit->GetVertexShaderText());
		bVertexShaderUsed = true;
		CurrentEditor = &VertexShaderEditor;
	}
	else
	{
		VertexShaderEditor.SetText("");
		bVertexShaderUsed = false;
	}

	if (ShaderToEdit->GetFragmentShaderText() != nullptr)
	{
		FragmentShaderEditor.SetText(ShaderToEdit->GetFragmentShaderText());
		bFragmentShaderUsed = true;
		if (CurrentEditor == nullptr)
			CurrentEditor = &FragmentShaderEditor;
	}
	else
	{
		FragmentShaderEditor.SetText("");
		bFragmentShaderUsed = false;
	}

	if (ShaderToEdit->GetTessControlShaderText() != nullptr)
	{
		TessControlShaderEditor.SetText(ShaderToEdit->GetTessControlShaderText());
		bTessControlShaderUsed = true;
		if (CurrentEditor == nullptr)
			CurrentEditor = &TessControlShaderEditor;
	}
	else
	{
		TessControlShaderEditor.SetText("");
		bTessControlShaderUsed = false;
	}

	if (ShaderToEdit->GetTessEvalShaderText() != nullptr)
	{
		TessEvalShaderEditor.SetText(ShaderToEdit->GetTessEvalShaderText());
		bTessEvalShaderUsed = true;
		if (CurrentEditor == nullptr)
			CurrentEditor = &TessEvalShaderEditor;
	}
	else
	{
		TessEvalShaderEditor.SetText("");
		bTessEvalShaderUsed = false;
	}

	if (ShaderToEdit->GetGeometryShaderText() != nullptr)
	{
		GeometryShaderEditor.SetText(ShaderToEdit->GetGeometryShaderText());
		bGeometryShaderUsed = true;
		if (CurrentEditor == nullptr)
			CurrentEditor = &GeometryShaderEditor;
	}
	else
	{
		GeometryShaderEditor.SetText("");
		bGeometryShaderUsed = false;
	}

	if (ShaderToEdit->GetComputeShaderText() != nullptr)
	{
		ComputeShaderEditor.SetText(ShaderToEdit->GetComputeShaderText());
		bComputeShaderUsed = true;
		if (CurrentEditor == nullptr)
			CurrentEditor = &ComputeShaderEditor;
	}
	else
	{
		ComputeShaderEditor.SetText("");
		bComputeShaderUsed = false;
	}
}

void ShaderEditorWindow::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Edit"))
		{
			bool ro = CurrentEditor->IsReadOnly();
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
				CurrentEditor->SetReadOnly(ro);
			ImGui::Separator();

			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && CurrentEditor->CanUndo()))
				CurrentEditor->Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && CurrentEditor->CanRedo()))
				CurrentEditor->Redo();

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, CurrentEditor->HasSelection()))
				CurrentEditor->Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && CurrentEditor->HasSelection()))
				CurrentEditor->Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && CurrentEditor->HasSelection()))
				CurrentEditor->Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
				CurrentEditor->Paste();

			ImGui::Separator();

			if (ImGui::MenuItem("Select all", nullptr, nullptr))
				CurrentEditor->SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(CurrentEditor->GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
				CurrentEditor->SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette"))
				CurrentEditor->SetPalette(TextEditor::GetLightPalette());
			if (ImGui::MenuItem("Retro blue palette"))
				CurrentEditor->SetPalette(TextEditor::GetRetroBluePalette());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	const auto cpos = CurrentEditor->GetCursorPosition();
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, CurrentEditor->GetTotalLines(),
		CurrentEditor->IsOverwrite() ? "Ovr" : "Ins",
		CurrentEditor->CanUndo() ? "*" : " ",
		CurrentEditor->GetLanguageDefinition().mName.c_str(), "none");

	ImGui::PushStyleColor(ImGuiCol_TabActive, static_cast<ImVec4>(ImColor::ImColor(0.4f, 0.9f, 0.4f, 1.0f)));
	if (ImGui::BeginTabBar("##Shaders Editors", ImGuiTabBarFlags_None))
	{
		if (bVertexShaderUsed)
		{
			if (ImGui::BeginTabItem("Vertex Shader"))
			{
				ActiveTab = 0;
				CurrentEditor = &VertexShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (bTessControlShaderUsed)
		{
			if (ImGui::BeginTabItem("Tessalation Control Shader"))
			{
				ActiveTab = 1;
				CurrentEditor = &TessControlShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (bTessEvalShaderUsed)
		{
			if (ImGui::BeginTabItem("Tessalation Evaluation Shader"))
			{
				ActiveTab = 2;
				CurrentEditor = &TessEvalShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (bGeometryShaderUsed)
		{
			if (ImGui::BeginTabItem("Geometry Shader"))
			{
				ActiveTab = 3;
				CurrentEditor = &GeometryShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (bFragmentShaderUsed)
		{
			if (ImGui::BeginTabItem("Fragment Shader"))
			{
				ActiveTab = 4;
				CurrentEditor = &FragmentShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (bComputeShaderUsed)
		{
			if (ImGui::BeginTabItem("Compute Shader"))
			{
				ActiveTab = 5;
				CurrentEditor = &ComputeShaderEditor;
				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}
	ImGui::PopStyleColor();
	
	ImGuiWindow* CurrentWindow = FE_IMGUI_WINDOW_MANAGER.GetCurrentWindowImpl();
	CurrentEditor->Render("Editor", ImVec2(CurrentWindow->Size.x - 40, CurrentWindow->Size.y - 190));

	const float CurrentYPosition = ImGui::GetCursorPosY() + 15;
	CompileButton->SetPosition(ImVec2(ImGui::GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f, CurrentYPosition));
	CompileButton->Render();
	if (CompileButton->IsClicked())
	{
		delete DummyShader;

		DummyShader = new FEShader("dummyShader", 
			bVertexShaderUsed ? VertexShaderEditor.GetText().c_str() : nullptr,
			bFragmentShaderUsed ? FragmentShaderEditor.GetText().c_str() : nullptr,
			bTessControlShaderUsed ? TessControlShaderEditor.GetText().c_str() : nullptr,
			bTessEvalShaderUsed ? TessEvalShaderEditor.GetText().c_str() : nullptr,
			bGeometryShaderUsed ? GeometryShaderEditor.GetText().c_str() : nullptr,
			bComputeShaderUsed ? ComputeShaderEditor.GetText().c_str() : nullptr,
			true);

		// TODO: Verify if this is needed.
		DummyShader->AddUniformsFromShader(ShaderToEdit);

		std::string Errors = DummyShader->GetCompilationErrors();
		Errors += "\n";
		Errors += DummyShader->GetLinkErrors();
		Errors += "\n";
		Errors += DummyShader->GetValidateErrors();

		if (!Errors.empty())
		{
			JustTextWindowObj.Show(Errors, "Shader have error(s)!");
		}
		else
		{
			FEShader* ReCompiledShader = new FEShader(ShaderToEdit->GetName(), 
				bVertexShaderUsed ? VertexShaderEditor.GetText().c_str() : nullptr,
				bFragmentShaderUsed ? FragmentShaderEditor.GetText().c_str() : nullptr,
				bTessControlShaderUsed ? TessControlShaderEditor.GetText().c_str() : nullptr,
				bTessEvalShaderUsed ? TessEvalShaderEditor.GetText().c_str() : nullptr,
				bGeometryShaderUsed ? GeometryShaderEditor.GetText().c_str() : nullptr,
				bComputeShaderUsed ? ComputeShaderEditor.GetText().c_str() : nullptr);

			// TODO: Verify if this is needed.
			ReCompiledShader->AddUniformsFromShader(ShaderToEdit);

			RESOURCE_MANAGER.ReplaceShader(ShaderToEdit->GetObjectID(), ReCompiledShader);

			if (ShaderToEdit->IsDebugRequest())
			{
				ShaderDebugWindow::GetInstance().Show(ShaderToEdit, "Shader debug info");
			}
		}
	}

	CloseButton->SetPosition(ImVec2(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f, CurrentYPosition));
	CloseButton->Render();
	if (CloseButton->IsClicked())
	{
		FEImGuiWindow::Close();
	}

	FEImGuiWindow::OnRenderEnd();
}

void ShaderEditorWindow::ReplaceShader(FEShader* OldShader, FEShader* NewShader)
{
	std::vector<std::string> MaterialList = RESOURCE_MANAGER.GetMaterialIDList();
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		FEMaterial* TempMaterial = RESOURCE_MANAGER.GetMaterial(MaterialList[i]);
		if (TempMaterial->Shader->GetNameHash() == OldShader->GetNameHash())
		{
			TempMaterial->Shader = NewShader;
		}
	}

	MaterialList = RESOURCE_MANAGER.GetEnginePrivateMaterialIDList();
	for (size_t i = 0; i < MaterialList.size(); i++)
	{
		FEMaterial* TempMaterial = RESOURCE_MANAGER.GetMaterial(MaterialList[i]);
		if (TempMaterial->Shader->GetNameHash() == OldShader->GetNameHash())
		{
			TempMaterial->Shader = NewShader;
		}
	}

	if (EDITOR.GetFocusedScene() != nullptr)
	{
		FEScene* CurrentScene = EDITOR.GetFocusedScene();
		const std::vector<std::string> TerrainList = CurrentScene->GetEntityIDListWithComponent<FETerrainComponent>();
		for (size_t i = 0; i < TerrainList.size(); i++)
		{
			FEEntity* TempTerrain = CurrentScene->GetEntity(TerrainList[i]);
			FETerrainComponent& TerrainComponent = TempTerrain->GetComponent<FETerrainComponent>();
			if (TerrainComponent.Shader->GetNameHash() == OldShader->GetNameHash())
			{
				TerrainComponent.Shader = NewShader;
			}
		}
	}
}