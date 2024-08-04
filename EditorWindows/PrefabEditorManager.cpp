#include "PrefabEditorManager.h"
#include "SceneGraphWindow.h"
#include "../FEEditor.h"
FEPrefabEditorManager* FEPrefabEditorManager::Instance = nullptr;


FEPrefabSceneEditorWindow::FEPrefabSceneEditorWindow(FEScene* Scene) : FEEditorSceneWindow(Scene, false)
{
	bSelfContained = false;

	ApplyButton = new ImGuiButton("Apply");
	ApplyButton->SetSize(ImVec2(140, 24));
	ApplyButton->SetDefaultColor(ImVec4(0.2f, 0.8f, 0.2f, 1.0f));

	CloseButton = new ImGuiButton("Close");
	CloseButton->SetSize(ImVec2(140, 24));
	CloseButton->SetDefaultColor(ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
}

FEPrefabSceneEditorWindow::~FEPrefabSceneEditorWindow()
{
	delete CloseButton;
	SCENE_MANAGER.DeleteScene(Scene);

	if (!PREFAB_EDITOR_MANAGER.bClearing)
	{
		auto WindowIterator = PREFAB_EDITOR_MANAGER.PrefabWindows.begin();
		while (WindowIterator != PREFAB_EDITOR_MANAGER.PrefabWindows.end())
		{
			if (WindowIterator->second == this)
			{
				PREFAB_EDITOR_MANAGER.PrefabWindows.erase(WindowIterator);
				break;
			}

			WindowIterator++;
		}
	}
}

void FEPrefabSceneEditorWindow::Render()
{
	FEEditorSceneWindow::Render();

	if (!IsVisible())
		return;

	ApplyButton->SetSize(ImVec2(GetWindow()->ContentRegionRect.GetWidth() / 10.0f, GetWindow()->Rect().GetHeight() / 20.0f));
	ApplyButton->SetPosition(ImVec2(5.0f, GetWindow()->Rect().GetHeight() - ApplyButton->GetSize().y - 5.0f));
	ApplyButton->Render();
	if (ApplyButton->IsClicked())
	{
		PREFAB_EDITOR_MANAGER.ApplyModificationsToPrefabScene(this);
		//bWaitingForRemoval = true;
		//Close();
	}

	CloseButton->SetSize(ApplyButton->GetSize());
	CloseButton->SetPosition(ImVec2(GetWindow()->ContentRegionRect.GetWidth() - CloseButton->GetSize().x - 5.0f,
									GetWindow()->Rect().GetHeight() - CloseButton->GetSize().y - 5.0f));
	CloseButton->Render();
	if (CloseButton->IsClicked())
	{
		bWaitingForRemoval = true;
		Close();
	}

	FEEditorSceneWindow::OnRenderEnd();
}

FEPrefabEditorManager::FEPrefabEditorManager()
{
	
}

FEPrefabEditorManager::~FEPrefabEditorManager()
{
	Clear();
}

void FEPrefabEditorManager::Clear()
{
	bClearing = true;

	auto WindowIterator = PrefabWindows.begin();
	while (WindowIterator != PrefabWindows.end())
	{
		delete WindowIterator->second;
		WindowIterator = PrefabWindows.erase(WindowIterator);
	}

	PrefabWindows.clear();
	bClearing = false;
}

void FEPrefabEditorManager::PrepareEditWinow(FEPrefab* Prefab)
{
	if (Prefab->Scene == nullptr)
		return;

	// We don't want to open the same prefab scene twice
	if (PrefabWindows.find(Prefab) != PrefabWindows.end())
		return;

	FEScene* CurrentPrefabScene = SCENE_MANAGER.DuplicateScene(Prefab->Scene, "Scene: " + Prefab->GetName());

	FEEntity* Camera = CurrentPrefabScene->CreateEntity("Prefab scene camera");
	Camera->GetComponent<FETagComponent>().SetTag(EDITOR_SCENE_TAG);
	Camera->AddComponent<FECameraComponent>();
	FECameraComponent& CameraComponent = Camera->GetComponent<FECameraComponent>();
	CameraComponent.Type = 1;
	CameraComponent.DistanceToModel = 10.0;
	CameraComponent.SetSSAOEnabled(false);

	CAMERA_SYSTEM.SetMainCamera(Camera);
	FETransformComponent& CameraTransform = Camera->GetComponent<FETransformComponent>();

	FEEntity* SkyDomeEntity = CurrentPrefabScene->CreateEntity("Prefab scene skydome");
	SkyDomeEntity->GetComponent<FETagComponent>().SetTag(EDITOR_SCENE_TAG);
	SkyDomeEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(100.0f));
	SkyDomeEntity->AddComponent<FESkyDomeComponent>();

	FEEntity* LightEntity = CurrentPrefabScene->CreateEntity("Prefab scene light");
	LightEntity->GetComponent<FETagComponent>().SetTag(EDITOR_SCENE_TAG);
	LightEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();
	LightEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(4.3f);
	LightComponent.SetCastShadows(false);

	PrefabWindows[Prefab] = new FEPrefabSceneEditorWindow(CurrentPrefabScene);
	EDITOR.AddCustomEditorScene(PrefabWindows[Prefab]);
}

void FEPrefabEditorManager::ApplyModificationsToPrefabScene(FEPrefabSceneEditorWindow* Window)
{
	FEPrefab* Prefab = nullptr;

	auto WindowIterator = PrefabWindows.begin();
	while (WindowIterator != PrefabWindows.end())
	{
		if (WindowIterator->second == Window)
		{
			Prefab = WindowIterator->first;
			break;
		}

		WindowIterator++;
	}

	if (Prefab == nullptr)
		return;

	FEScene* ModifiedScene = Window->GetScene();
	Prefab->Scene->Clear();

	SCENE_MANAGER.ImportSceneAsNode(ModifiedScene, Prefab->Scene, Prefab->Scene->SceneGraph.GetRoot(), [](FEEntity* EntityToCheck) {
		return !(EntityToCheck->GetComponent<FETagComponent>().GetTag() == EDITOR_SCENE_TAG);
	});
}