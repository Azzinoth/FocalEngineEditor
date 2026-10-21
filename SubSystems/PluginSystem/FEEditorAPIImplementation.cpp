#include "FEEditorAPIImplementation.h"
#include "../../FEEditor.h"

FEEditorAPIImplementation::FEEditorAPIImplementation(const std::string& PluginName, const std::string& ModuleName, const FEUUID& ModuleID)
	: PluginName(PluginName), ModuleName(ModuleName), ModuleID(ModuleID)
{
}

int FEEditorAPIImplementation::GetVersion() const
{
	return FE_EDITOR_API_VERSION;
}

FEUUID FEEditorAPIImplementation::AddMenuItem(const std::string& Path, std::function<void()> OnClick)
{
	MenuItemRegistration Item;
	std::stringstream PathStream(Path);
	std::string Part;
	while (std::getline(PathStream, Part, '/'))
	{
		if (!Part.empty())
			Item.Path.push_back(Part);
	}

	if (Item.Path.empty() || !OnClick)
	{
		LOG.Add("FEEditorAPIImplementation::AddMenuItem: Module \"" + ModuleName + "\" tried to add menu item with empty path or callback", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		return FEUUID();
	}

	Item.ID = UNIQUE_ID.GenerateID();
	Item.OnClick = OnClick;
	MenuItems.push_back(Item);
	return Item.ID;
}

FEUUID FEEditorAPIImplementation::AddWindow(const std::string& Key, const std::string& Title, std::function<void()> DrawContent)
{
	if (Key.empty() || !DrawContent)
	{
		LOG.Add("FEEditorAPIImplementation::AddWindow: Module \"" + ModuleName + "\" tried to add window with empty key or draw callback", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		return FEUUID();
	}

	for (const WindowRegistration& Window : Windows)
	{
		if (!Window.bRemoved && Window.Key == Key)
		{
			LOG.Add("FEEditorAPIImplementation::AddWindow: Module \"" + ModuleName + "\" already has window with key \"" + Key + "\"", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
			return FEUUID();
		}
	}

	WindowRegistration Window;
	Window.ID = UNIQUE_ID.GenerateID();
	Window.Key = Key;
	Window.Title = Title.empty() ? Key : Title;
	Window.DrawContent = DrawContent;
	Windows.push_back(Window);
	return Window.ID;
}

void FEEditorAPIImplementation::SetWindowVisible(const FEUUID& WindowID, const bool bVisible)
{
	for (WindowRegistration& Window : Windows)
	{
		if (!Window.bRemoved && Window.ID == WindowID)
			Window.bVisible = bVisible;
	}
}

bool FEEditorAPIImplementation::IsWindowVisible(const FEUUID& WindowID) const
{
	for (const WindowRegistration& Window : Windows)
	{
		if (!Window.bRemoved && Window.ID == WindowID)
			return Window.bVisible;
	}

	return false;
}

FEUUID FEEditorAPIImplementation::AddInspectorSection(const std::string& Title, std::function<bool(FEEntity*)> IsShownFor, std::function<void(FEEntity*)> DrawContent)
{
	if (Title.empty() || !DrawContent)
	{
		LOG.Add("FEEditorAPIImplementation::AddInspectorSection: Module \"" + ModuleName + "\" tried to add inspector section with empty title or draw callback", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		return FEUUID();
	}

	InspectorSectionRegistration Section;
	Section.ID = UNIQUE_ID.GenerateID();
	Section.Title = Title;
	Section.IsShownFor = IsShownFor;
	Section.DrawContent = DrawContent;
	InspectorSections.push_back(Section);
	return Section.ID;
}

void FEEditorAPIImplementation::Remove(const FEUUID& RegistrationID)
{
	for (MenuItemRegistration& Item : MenuItems)
	{
		if (Item.ID == RegistrationID)
			Item.bRemoved = true;
	}

	for (WindowRegistration& Window : Windows)
	{
		if (Window.ID == RegistrationID)
			Window.bRemoved = true;
	}

	for (InspectorSectionRegistration& Section : InspectorSections)
	{
		if (Section.ID == RegistrationID)
			Section.bRemoved = true;
	}
}

bool FEEditorAPIImplementation::IsProjectOpen() const
{
	return PROJECT_MANAGER.GetCurrent() != nullptr;
}

FEScene* FEEditorAPIImplementation::GetFocusedScene() const
{
	return EDITOR.GetFocusedScene();
}

FEEntity* FEEditorAPIImplementation::GetSelectedEntity() const
{
	return SELECTED.GetSelected(EDITOR.GetFocusedScene());
}

void FEEditorAPIImplementation::SetSelectedEntity(FEEntity* Entity)
{
	if (Entity == nullptr)
	{
		SELECTED.Clear(EDITOR.GetFocusedScene());
		return;
	}

	SELECTED.SetSelected(Entity);
}

void FEEditorAPIImplementation::MarkProjectModified()
{
	if (PROJECT_MANAGER.GetCurrent() != nullptr)
		PROJECT_MANAGER.GetCurrent()->SetModified(true);
}

void FEEditorAPIImplementation::Update()
{
	for (MenuItemRegistration& Item : MenuItems)
	{
		if (!Item.bClicked || Item.bRemoved)
			continue;

		Item.bClicked = false;
		Item.OnClick();
	}

	MenuItems.remove_if([](const MenuItemRegistration& Item) { return Item.bRemoved; });
	Windows.remove_if([](const WindowRegistration& Window) { return Window.bRemoved; });
	InspectorSections.remove_if([](const InspectorSectionRegistration& Section) { return Section.bRemoved; });
}

void FEEditorAPIImplementation::RenderMenu()
{
	// Module ID keeps menus of different plugins with the same name apart.
	if (!ImGui::BeginMenu((PluginName + "##" + UNIQUE_ID.ToString(ModuleID)).c_str()))
		return;

	bool bEmpty = true;
	for (WindowRegistration& Window : Windows)
	{
		if (Window.bRemoved)
			continue;

		bEmpty = false;
		ImGui::PushID(UNIQUE_ID.ToString(Window.ID).c_str());
		ImGui::MenuItem(Window.Title.c_str(), nullptr, &Window.bVisible);
		ImGui::PopID();
	}

	for (MenuItemRegistration& Item : MenuItems)
	{
		if (Item.bRemoved)
			continue;

		bEmpty = false;
		// BeginMenu with label that was already used in this frame appends to that submenu.
		size_t OpenedMenus = 0;
		while (OpenedMenus + 1 < Item.Path.size() && ImGui::BeginMenu(Item.Path[OpenedMenus].c_str()))
			OpenedMenus++;

		if (OpenedMenus + 1 == Item.Path.size())
		{
			ImGui::PushID(UNIQUE_ID.ToString(Item.ID).c_str());
			if (ImGui::MenuItem(Item.Path.back().c_str()))
				Item.bClicked = true;
			ImGui::PopID();
		}

		for (size_t i = 0; i < OpenedMenus; i++)
			ImGui::EndMenu();
	}

	if (bEmpty)
		ImGui::MenuItem("No items", nullptr, false, false);

	ImGui::EndMenu();
}

void FEEditorAPIImplementation::RenderWindows()
{
	for (WindowRegistration& Window : Windows)
	{
		if (Window.bRemoved || !Window.bVisible)
			continue;

		// Part after ### is window ID, so title could change without losing window layout.
		if (ImGui::Begin((Window.Title + "###" + UNIQUE_ID.ToString(ModuleID) + "." + Window.Key).c_str(), &Window.bVisible))
			Window.DrawContent();
		ImGui::End();
	}
}

void FEEditorAPIImplementation::RenderInspectorSections(FEEntity* Entity)
{
	for (InspectorSectionRegistration& Section : InspectorSections)
	{
		if (Section.bRemoved || (Section.IsShownFor && !Section.IsShownFor(Entity)))
			continue;

		ImGui::PushID(UNIQUE_ID.ToString(Section.ID).c_str());
		if (ImGui::CollapsingHeader(Section.Title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			Section.DrawContent(Entity);
		ImGui::PopID();
	}
}