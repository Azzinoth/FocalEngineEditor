#pragma once

#include "FEEditor.h"
using namespace FocalEngine;

class FEEditorScriptingSystem
{
public:
	SINGLETON_PUBLIC_PART(FEEditorScriptingSystem)

	void Update();
	bool IsCurrentProjectHaveVSProjectInitialized();
	bool GenerateNewNativeScriptProject(std::string FirstScriptName);
private:
	SINGLETON_PRIVATE_PART(FEEditorScriptingSystem)
};

#define EDITOR_SCRIPTING_SYSTEM FEEditorScriptingSystem::GetInstance()
