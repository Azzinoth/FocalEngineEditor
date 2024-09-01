#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEEditorScriptingSystem
{
public:
	SINGLETON_PUBLIC_PART(FEEditorScriptingSystem)

	void Update();
private:
	SINGLETON_PRIVATE_PART(FEEditorScriptingSystem)
	
	uint64_t LastWriteTime = 0;
	FENativeScriptModule* ExternalEditorActiveModule = nullptr;
};

#define EDITOR_SCRIPTING_SYSTEM FEEditorScriptingSystem::GetInstance()
