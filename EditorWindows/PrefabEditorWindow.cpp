#include "PrefabEditorWindow.h"
#include "SceneGraphWindow.h"
#include "../FEEditor.h"
PrefabEditorWindow* PrefabEditorWindow::Instance = nullptr;
FEPrefab* PrefabEditorWindow::ObjToWorkWith = nullptr;

bool PrefabEditorWindow::AddGameModelTargetCallBack(FEObject* Object, void** EntityPointer)
{
	if (Object == nullptr)
		return false;

	FEGameModel* NewGameModel = reinterpret_cast<FEGameModel*>(Object);
	// FIX ME! It should be here
	//ObjToWorkWith->AddComponent(NewGameModel);

	return true;
}

void PrefabEditorWindow::ShowTransformConfiguration(FETransformComponent* Transform, const int Index)
{
}

void PrefabEditorWindow::AddNewGameModelCallBack(const std::vector<FEObject*> SelectionsResult)
{
	for (int i = 0; i < SelectionsResult.size(); i++)
	{
		// FIX ME! It should be here
		//if (SelectionsResult[i]->GetType() == FE_GAMEMODEL)
		//	ObjToWorkWith->AddComponent(reinterpret_cast<FEGameModel*>(SelectionsResult[i]));
	}
}

PrefabEditorWindow::PrefabEditorWindow()
{
	const std::string TempCaption = "Edit Prefab";
	strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());

	CloseButton = new ImGuiButton("Close");
	CloseButton->SetSize(ImVec2(140, 24));
	CloseButton->SetPosition(ImVec2(800.0 / 2.0 - 140.0 / 2.0, 800.0 - 35.0));

	//addGameModelTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_GAMEMODEL, addGameModelTargetCallBack, nullptr, "Drop to add Game model");
}

PrefabEditorWindow::~PrefabEditorWindow()
{
	delete CloseButton;
}

void PrefabEditorWindow::Show(FEPrefab* Prefab)
{
	if (Prefab->Scene != nullptr)
	{
		CurrentPrefabScene = SCENE_MANAGER.DuplicateScene(Prefab->Scene, "Scene: " + Prefab->GetName());

		FEEntity* Camera = CurrentPrefabScene->CreateEntity("Prefab scene camera");
		CurrentCameraEntity = Camera;
		Camera->AddComponent<FECameraComponent>();
		FECameraComponent& CameraComponent = Camera->GetComponent<FECameraComponent>();
		// FIX ME! It should be model camera but it is not working with transform component
		CameraComponent.Type = 0;
		CameraComponent.DistanceToModel = 10.0;

		CAMERA_SYSTEM.SetMainCamera(Camera);
		FETransformComponent& CameraTransform = Camera->GetComponent<FETransformComponent>();
		CameraTransform.SetPosition(glm::vec3(0.0f, 0.0f, 30.0f));

		FEEntity* SkyDomeEntity = CurrentPrefabScene->CreateEntity("Prefab scene skydome");
		SkyDomeEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(100.0f));
		SkyDomeEntity->AddComponent<FESkyDomeComponent>();

		FEEntity* LightEntity = CurrentPrefabScene->CreateEntity("Prefab scene light");
		LightEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
		FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();
		LightEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
		LightComponent.SetIntensity(4.3f);
		LightComponent.SetCastShadows(false);

		EDITOR.AddEditorScene(CurrentPrefabScene);

		/*Size = ImVec2(800, 800);
		Position = ImVec2(APPLICATION.GetMainWindow()->GetWidth() / 2 - Size.x / 2, APPLICATION.GetMainWindow()->GetHeight() / 2 - Size.y / 2);

		Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

		ObjToWorkWith = Prefab;

		FEImGuiWindow::Show();*/
	}
}

void PrefabEditorWindow::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	if (ImGui::BeginTabBar("##prefabSettings", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Game Models"))
		{
			static bool ContextMenuOpened = false;

			ImGui::BeginChildFrame(ImGui::GetID("GameModels ListBox Child"), ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 500.0f), ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			bool bListBoxHovered = false;
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
			{
				bListBoxHovered = true;
				//addGameModelTarget->setActive(true);
			}

			static bool bOpenContextMenu = false;
			if (ImGui::IsMouseClicked(1))
			{
				if (bListBoxHovered)
				{
					bOpenContextMenu = true;
				}
			}

			if (ImGui::BeginListBox("##GameModels ListBox", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 500.0f)))
			{
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

				for (int i = 0; i < ObjToWorkWith->ComponentsCount(); i++)
				{
					FEGameModel* GameModel = ObjToWorkWith->GetComponent(i)->GameModel;
					if (GameModel == nullptr)
						break;

					ImVec2 PostionBeforeDraw = ImGui::GetCursorPos();

					const ImVec2 TextSize = ImGui::CalcTextSize(GameModel->GetName().c_str());
					ImGui::SetCursorPos(PostionBeforeDraw + ImVec2(ImGui::GetContentRegionAvail().x / 2.0f - TextSize.x / 2.0f, 16));
					ImGui::Text(GameModel->GetName().c_str());
					ImGui::SetCursorPos(PostionBeforeDraw);

					ImGui::PushID(i);
					if (ImGui::Selectable("##item", SelectedGameModel == i ? true : false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x - 0, 64)))
					{
						SelectedGameModel = i;
					}
					ImGui::PopID();

					if (ImGui::IsItemHovered())
						HoveredGameModelItem = i;

					ImGui::SetCursorPos(PostionBeforeDraw);
					ImColor ImageTint = ImGui::IsItemHovered() ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
					FETexture* PreviewTexture = PREVIEW_MANAGER.GetPreview(GameModel->GetObjectID());
					ImGui::Image((void*)static_cast<intptr_t>(PreviewTexture->GetTextureID()), ImVec2(64, 64), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), ImageTint);


					ShowTransformConfiguration(&ObjToWorkWith->GetComponent(i)->Transform, i);
				}

				ImGui::PopFont();
				ImGui::EndListBox();
			}
			

			ImGui::EndChildFrame();
			ImGui::EndTabItem();

			if (bOpenContextMenu)
			{
				bOpenContextMenu = false;
				ImGui::OpenPopup("##layers_listBox_context_menu");
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			if (ImGui::BeginPopup("##layers_listBox_context_menu"))
			{
				ContextMenuOpened = true;

				if (ImGui::MenuItem("Add game model..."))
				{
					SelectFEObjectPopUp::getInstance().Show(FE_GAMEMODEL, AddNewGameModelCallBack);
				}

				if (HoveredGameModelItem != -1)
				{
					FEGameModel* GameModel = ObjToWorkWith->GetComponent(HoveredGameModelItem)->GameModel;
					if (GameModel != nullptr)
					{
						ImGui::Separator();
						const std::string LayerName = GameModel->GetName();
						ImGui::Text((std::string("Actions with ") + LayerName).c_str());
						ImGui::Separator();

						if (ImGui::MenuItem("Remove"))
						{
							ObjToWorkWith->RemoveComponent(HoveredGameModelItem);
						}
					}
				}

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			if (!ContextMenuOpened)
				HoveredGameModelItem = -1;

			ContextMenuOpened = false;
		}
		ImGui::EndTabBar();
	}

	CloseButton->Render();
	if (CloseButton->IsClicked())
	{
		if (CurrentPrefabScene != nullptr)
		{
			SCENE_MANAGER.DeleteScene(CurrentPrefabScene);
			CurrentPrefabScene = nullptr;
		}
			
		Close();
	}
	
	FEImGuiWindow::OnRenderEnd();
}