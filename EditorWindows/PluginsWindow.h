#pragma once

#include "../SubSystems/PluginSystem/FEEditorPluginManager.h"

class FEEditorPluginsWindow
{
	friend class FEEditor;
	SINGLETON_PRIVATE_PART(FEEditorPluginsWindow)

	bool bVisible = false;

	void Render();
	void RenderPluginDetails(const DiscoveredPlugin& Plugin);
	void RenderIDWithCopyButton(const FEUUID& ID);

	std::string FindPluginName(const FEUUID& PluginID) const;
	std::string FindModuleName(const FEUUID& ModuleID) const;
public:
	SINGLETON_PUBLIC_PART(FEEditorPluginsWindow)
};

#define PLUGINS_WINDOW FEEditorPluginsWindow::GetInstance()