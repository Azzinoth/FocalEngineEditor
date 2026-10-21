#include "EDITOR_MODULE_NAME_PLACE_HOLDER.h"

// DO NOT CHANGE THIS LINE.
// The plugin manager uses it to create and destroy the plugin instance.
REGISTER_PLUGIN(EDITOR_MODULE_NAME_PLACE_HOLDER)

// Windows, menu items and inspector sections should be added here through Editor, they are removed automatically when module is unloaded.
bool EDITOR_MODULE_NAME_PLACE_HOLDER::OnEditorLoad(FEEditorAPI* EditorAPI)
{
	Editor = EditorAPI;
	return true;
}

void EDITOR_MODULE_NAME_PLACE_HOLDER::OnEditorUnload()
{

}

void EDITOR_MODULE_NAME_PLACE_HOLDER::OnEditorUpdate(double DeltaTime)
{

}