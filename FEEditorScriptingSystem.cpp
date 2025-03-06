#include "FEEditorScriptingSystem.h"
using namespace FocalEngine;

FEEditorScriptingSystem::FEEditorScriptingSystem()
{
	
}

FEEditorScriptingSystem::~FEEditorScriptingSystem()
{
}

void FEEditorScriptingSystem::Update()
{
	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject == nullptr)
		return;

	std::vector<std::string> IDList = RESOURCE_MANAGER.GetNativeScriptModuleIDList();
	for (size_t i = 0; i < IDList.size(); i++)
	{
		FENativeScriptModule* CurrentModule = RESOURCE_MANAGER.GetNativeScriptModule(IDList[i]);
		if (CurrentModule == nullptr)
			continue;

		if (CurrentModule->GetTag() == ENGINE_RESOURCE_TAG ||
			CurrentModule->GetTag() == EDITOR_RESOURCE_TAG)
			continue;

		if (CurrentModule->GetProject() != nullptr)
			CurrentModule->GetProject()->Update();
	}
}