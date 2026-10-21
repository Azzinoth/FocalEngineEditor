#pragma once
#include "../SubSystems/FocalEngine/SubSystems/PluginSystem/FEPluginConnector.h"
#include "../SubSystems/EditorAPI/FEEditorAPI.h"
#include "../Public/INTERFACE_NAME_PLACE_HOLDER.h"
using namespace FocalEngine;

class EDITOR_MODULE_NAME_PLACE_HOLDER : public FEEditorPluginCore
{
	using EngineModuleInterface = INTERFACE_NAME_PLACE_HOLDER;

	// Editor module is loaded after engine module and unloaded before it, so engine module is always available.
	EngineModuleInterface* GetEngineModule() { return PLUGIN_MANAGER.GetPlugin<EngineModuleInterface>(UNIQUE_ID.FromString(EngineModuleInterface::PluginID)); }

	FEEditorAPI* Editor = nullptr;
public:
	bool OnEditorLoad(FEEditorAPI* EditorAPI) override;
	void OnEditorUnload() override;
	void OnEditorUpdate(double DeltaTime) override;
};