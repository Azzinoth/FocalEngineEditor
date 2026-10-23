#pragma once
#include "../SubSystems/FocalEngine/SubSystems/PluginSystem/FEPluginConnector.h"
#include "../SubSystems/EditorAPI/FEEditorAPI.h"
using namespace FocalEngine;

class EDITOR_MODULE_NAME_PLACE_HOLDER : public FEEditorPluginCore
{
	FEEditorAPI* Editor = nullptr;
public:
	bool OnEditorLoad(FEEditorAPI* EditorAPI) override;
	void OnEditorUnload() override;
	void OnEditorUpdate(double DeltaTime) override;
};