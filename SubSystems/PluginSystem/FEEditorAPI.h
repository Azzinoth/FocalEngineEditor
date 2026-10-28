#pragma once
#include "../FocalEngine/SubSystems/PluginSystem/FEPluginCore.h"

#define FE_EDITOR_API_VERSION 2
#define FE_EDITOR_API_MIN_VERSION 1

namespace FocalEngine
{
	class FEEntity;
	class FEScene;
	class FEEditorPluginCore;

	// Implemented by the editor, every editor module gets its own object, it is valid until OnEditorUnload returns.
	class FEEditorAPI
	{
	public:
		virtual int GetVersion() const = 0;

		// Menu items are placed in Plugins-><Plugin name>, '/' in Path creates submenus, OnClick is called at the beginning of the next frame.
		virtual FEUUID AddMenuItem(const std::string& Path, std::function<void()> OnClick) = 0;
		virtual FEUUID AddWindow(const std::string& Key, const std::string& Title, std::function<void()> DrawContent) = 0;
		virtual void SetWindowVisible(const FEUUID& WindowID, bool bVisible) = 0;
		virtual bool IsWindowVisible(const FEUUID& WindowID) const = 0;
		virtual FEUUID AddInspectorSection(const std::string& Title, std::function<bool(FEEntity*)> IsShownFor, std::function<void(FEEntity*)> DrawContent) = 0;
		virtual void Remove(const FEUUID& RegistrationID) = 0;

		virtual bool IsProjectOpen() const = 0;
		virtual FEScene* GetFocusedScene() const = 0;
		virtual FEEntity* GetSelectedEntity() const = 0;
		virtual void SetSelectedEntity(FEEntity* Entity) = 0;
		virtual void MarkProjectModified() = 0;

		// Returns nullptr if editor module of that plugin is not loaded.
		// Module that uses editor module of other plugin should list it in manifest dependencies, so it is loaded first.
		virtual FEEditorPluginCore* GetEditorModuleCore(const FEUUID& PluginID) const = 0;

		// T is usually public editor interface that plugin ships in its own header.
		template<typename T>
		T* GetEditorModule(const FEUUID& PluginID) const
		{
			return dynamic_cast<T*>(GetEditorModuleCore(PluginID));
		}
	protected:
		~FEEditorAPI() = default;
	};

	class FEEditorPluginCore : public FEPluginCore
	{
	public:
		virtual int GetEditorAPIVersion() const { return FE_EDITOR_API_VERSION; }

		// Called after editor is initialized, before any project is opened. Return false to unload the module.
		virtual bool OnEditorLoad(FEEditorAPI* Editor) = 0;
		virtual void OnEditorUnload() {}
		// Called every frame inside ImGui frame, also when no project is open.
		// Drawing should be wrapped in own ImGui::Begin/End, otherwise it ends up in the editor toolbar.
		virtual void OnEditorUpdate(double DeltaTime) {}
	private:
		bool OnLoad() final { return true; }
		void OnUnload() final {}
		void OnUpdate(double DeltaTime) final {}
		void OnBeforeRender() final {}
		void OnAfterRender() final {}
		void OnSceneAdded(FEScene* Scene) final {}
		void OnSceneFlagsChanged(FEScene* Scene, FESceneFlag PreviousFlags) final {}
		void OnSceneDeleting(FEScene* Scene) final {}
		void OnCameraRenderStage(FECameraRenderStage Stage, FEEntity* CameraEntity) final {}
	};
}