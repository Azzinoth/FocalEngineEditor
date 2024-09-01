#include "PrefabEditorManager.h"
#include "SceneGraphWindow.h"
#include "../FEEditor.h"

FEPrefabSceneEditorWindow::FEPrefabSceneEditorWindow(FEScene* Scene) : FEEditorSceneWindow(Scene, false)
{
	bSelfContained = false;

	ApplyButton = new ImGuiButton("Apply");
	ApplyButton->SetSize(ImVec2(140, 24));
	ApplyButton->SetDefaultColor(ImVec4(0.2f, 0.8f, 0.2f, 1.0f));

	CloseButton = new ImGuiButton("Close");
	CloseButton->SetSize(ImVec2(140, 24));
	CloseButton->SetDefaultColor(ImVec4(0.8f, 0.2f, 0.2f, 1.0f));

	AcceptedTypes.clear();
	AcceptedTypes.push_back(FE_GAMEMODEL);
	ToolTipTexts.clear();
	ToolTipTexts.push_back("Drop to add to scene");
	CurrentDragAndDropCallback = FEPrefabSceneEditorWindow::DragAndDropCallBack;
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

bool FEPrefabSceneEditorWindow::DragAndDropCallBack(FEObject* Object, void** UserData)
{
	if (EDITOR.GetFocusedScene() == nullptr)
		return false;

	if (CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene()) == nullptr)
		return false;

	if (UserData == nullptr)
		return false;

	FEEditorSceneWindow* EditorSceneWindow = reinterpret_cast<FEEditorSceneWindow*>(UserData);
	if (EditorSceneWindow->GetScene() == nullptr)
		return false;

	if (Object->GetType() == FE_GAMEMODEL)
	{
		FEGameModel* GameModel = RESOURCE_MANAGER.GetGameModel(Object->GetObjectID());

		FETransformComponent& CameraTransformComponent = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene())->GetComponent<FETransformComponent>();
		FECameraComponent& CameraComponent = CAMERA_SYSTEM.GetMainCameraEntity(EDITOR.GetFocusedScene())->GetComponent<FECameraComponent>();

		FEEntity* Entity = EditorSceneWindow->GetScene()->CreateEntity(Object->GetName());
		Entity->AddComponent<FEGameModelComponent>(GameModel);

		SELECTED.SetSelected(Entity);
		PROJECT_MANAGER.GetCurrent()->SetModified(true);

		return true;

	}

	return false;
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

FEEntity* FEPrefabEditorManager::InjectModelViewCamera(FEScene* Scene)
{
	FEEntity* CameraEntity = nullptr;
	std::vector<FEPrefab*> CameraPrefab = RESOURCE_MANAGER.GetPrefabByName("Model view camera prefab");
	if (CameraPrefab.size() == 0)
	{
		LOG.Add("FEPrefabEditorManager::InjectModelViewCamera: Camera prefab not found! Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

		CameraEntity = Scene->CreateEntity("Prefab scene camera");
		CameraEntity->AddComponent<FECameraComponent>();
	}
	else
	{
		FEPrefab* CameraPrefabToUse = CameraPrefab[0];
		std::vector<FEEntity*> AddedEntities = SCENE_MANAGER.InstantiatePrefab(CameraPrefabToUse, Scene, true);
		if (AddedEntities.empty())
		{
			LOG.Add("FEPrefabEditorManager::InjectModelViewCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Prefab scene camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
		else
		{
			CameraEntity = AddedEntities[0];
		}

		if (CameraEntity == nullptr)
		{
			LOG.Add("FEPrefabEditorManager::InjectModelViewCamera: Camera prefab was not instantiated correctly. Inserting camera manually.", "FE_LOG_LOADING", FE_LOG_WARNING);

			CameraEntity = Scene->CreateEntity("Prefab scene camera");
			CameraEntity->AddComponent<FECameraComponent>();
		}
	}

	RESOURCE_MANAGER.SetTag(CameraEntity, EDITOR_RESOURCE_TAG);
	CAMERA_SYSTEM.SetMainCamera(CameraEntity);

	return CameraEntity;
}

void FEPrefabEditorManager::PrepareEditWinow(FEPrefab* Prefab)
{
	if (Prefab->GetScene() == nullptr)
		return;

	// We don't want to open the same prefab scene twice
	if (PrefabWindows.find(Prefab) != PrefabWindows.end())
		return;

	FEScene* CurrentPrefabScene = SCENE_MANAGER.DuplicateScene(Prefab->GetScene(), "Scene: " + Prefab->GetName());
	CurrentPrefabScene->SetFlag(FESceneFlag::Active | FESceneFlag::EditorMode | FESceneFlag::Renderable, true);

	// Because by default camera is looking at 0,0,0 we need to place "empty" entity at 0,0,0.
	// To ensure that scene AABB would include some entity at 0,0,0.
	FEEntity* EmptyEntity = CurrentPrefabScene->CreateEntity("Empty entity");

	FEAABB SceneAABB = CurrentPrefabScene->GetSceneAABB([](FEEntity* Entity) -> bool {
		if (Entity->GetTag() == EDITOR_RESOURCE_TAG)
			return false;

		if (Entity->HasComponent<FESkyDomeComponent>())
			return false;

		if (Entity->HasComponent<FECameraComponent>())
			return false;

		return true;
	});

	CurrentPrefabScene->DeleteEntity(EmptyEntity);

	FEEntity* Camera = InjectModelViewCamera(CurrentPrefabScene);
	FECameraComponent& CameraComponent = Camera->GetComponent<FECameraComponent>();
	CameraComponent.SetSSAOEnabled(false);
	
	// We want to make sure that distance to model is at least 1.0
	float CalculatedDistance = SceneAABB.GetLongestAxisLength() * 2.5f;
	FENativeScriptComponent& ScriptComponent = Camera->GetComponent<FENativeScriptComponent>();
	ScriptComponent.SetVariableValue("DistanceToModel", CalculatedDistance > 1.0f ? SceneAABB.GetLongestAxisLength() * 2.5f : 1.0f);
	
	// To make sure that next scene FEAABB calculation will include correct camera position.
	CAMERA_SYSTEM.IndividualUpdate(Camera, 0.0);
	
	FEEntity* SkyDomeEntity = CurrentPrefabScene->CreateEntity("Prefab scene skydome");
	RESOURCE_MANAGER.SetTag(SkyDomeEntity, EDITOR_RESOURCE_TAG);
	SkyDomeEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(100.0f));
	SkyDomeEntity->AddComponent<FESkyDomeComponent>();

	FEEntity* LightEntity = CurrentPrefabScene->CreateEntity("Prefab scene light");
	RESOURCE_MANAGER.SetTag(LightEntity, EDITOR_RESOURCE_TAG);
	LightEntity->AddComponent<FELightComponent>(FE_DIRECTIONAL_LIGHT);
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();
	LightEntity->GetComponent<FETransformComponent>().SetRotation(glm::vec3(-40.0f, 10.0f, 0.0f));
	LightComponent.SetIntensity(4.3f);
	SceneAABB = CurrentPrefabScene->GetSceneAABB([](FEEntity* Entity) -> bool {
		if (Entity->GetTag() == EDITOR_RESOURCE_TAG && !Entity->HasComponent<FECameraComponent>())
			return false;

		if (Entity->HasComponent<FESkyDomeComponent>())
			return false;

		return true;
	});
	LightComponent.SetShadowCoverage(SceneAABB.GetLongestAxisLength() * 2);
	LightComponent.SetCastShadows(true);

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
	Prefab->GetScene()->Clear();

	SCENE_MANAGER.ImportSceneAsNode(ModifiedScene, Prefab->GetScene(), Prefab->GetScene()->SceneGraph.GetRoot(), [](FEEntity* EntityToCheck) {
		return !(EntityToCheck->GetTag() == EDITOR_RESOURCE_TAG);
	});

	Prefab->SetDirtyFlag(true);
	PREVIEW_MANAGER.GetPrefabPreview(Prefab->GetObjectID());
}