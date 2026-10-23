#pragma once

#include "../FocalEngine/FEngine.h"
#include "FEEditorAPI.h"
using namespace FocalEngine;

class FEEditorAPIImplementation : public FEEditorAPI
{
	friend class FEEditorPluginManager;

	struct MenuItemRegistration
	{
		FEUUID ID;
		std::vector<std::string> Path;
		std::function<void()> OnClick;
		bool bClicked = false;
		bool bRemoved = false;
	};

	struct WindowRegistration
	{
		FEUUID ID;
		std::string Key;
		std::string Title;
		std::function<void()> DrawContent;
		bool bVisible = false;
		bool bRemoved = false;
	};

	struct InspectorSectionRegistration
	{
		FEUUID ID;
		std::string Title;
		std::function<bool(FEEntity*)> IsShownFor;
		std::function<void(FEEntity*)> DrawContent;
		bool bRemoved = false;
	};

	std::string PluginName;
	std::string ModuleName;
	FEUUID ModuleID;

	// std::list, because module could add registrations from its callbacks while lists are iterated.
	// For the same reason removed registrations are only marked and erased later in Update.
	std::list<MenuItemRegistration> MenuItems;
	std::list<WindowRegistration> Windows;
	std::list<InspectorSectionRegistration> InspectorSections;

	void Update();
	void RenderMenu();
	void RenderWindows();
	void RenderInspectorSections(FEEntity* Entity);
public:
	FEEditorAPIImplementation(const std::string& PluginName, const std::string& ModuleName, const FEUUID& ModuleID);

	int GetVersion() const override;

	FEUUID AddMenuItem(const std::string& Path, std::function<void()> OnClick) override;
	FEUUID AddWindow(const std::string& Key, const std::string& Title, std::function<void()> DrawContent) override;
	void SetWindowVisible(const FEUUID& WindowID, bool bVisible) override;
	bool IsWindowVisible(const FEUUID& WindowID) const override;
	FEUUID AddInspectorSection(const std::string& Title, std::function<bool(FEEntity*)> IsShownFor, std::function<void(FEEntity*)> DrawContent) override;
	void Remove(const FEUUID& RegistrationID) override;

	bool IsProjectOpen() const override;
	FEScene* GetFocusedScene() const override;
	FEEntity* GetSelectedEntity() const override;
	void SetSelectedEntity(FEEntity* Entity) override;
	void MarkProjectModified() override;

	FEEditorPluginCore* GetEditorModuleCore(const FEUUID& PluginID) const override;
};