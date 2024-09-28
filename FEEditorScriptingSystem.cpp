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

	if (CurrentProject->NativeScriptProject == nullptr)
		return;

	CurrentProject->NativeScriptProject->Update();
}

bool FEEditorScriptingSystem::IsCurrentProjectHaveVSProjectInitialized()
{
	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject == nullptr)
	{
		LOG.Add("FEEditorScriptingSystem::IsCurrentProjectHaveVSProjectInitialized: Current project is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string ProjectPath = CurrentProject->GetProjectFolder();
	if (ProjectPath.empty())
	{
		LOG.Add("FEEditorScriptingSystem::IsCurrentProjectHaveVSProjectInitialized: Project path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (CurrentProject->NativeScriptProject == nullptr)
	{
		LOG.Add("FEEditorScriptingSystem::IsCurrentProjectHaveVSProjectInitialized: Native script project data is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	return CurrentProject->NativeScriptProject->IsVSProjectValid();
}

bool FEEditorScriptingSystem::GenerateNewNativeScriptProject(std::string FirstScriptName)
{
	if (FirstScriptName.empty())
	{
		LOG.Add("FEEditorScriptingSystem::GenerateNewNativeScriptProject: First script name is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	FEProject* CurrentProject = PROJECT_MANAGER.GetCurrent();
	if (CurrentProject == nullptr)
	{
		LOG.Add("FEEditorScriptingSystem::GenerateNewNativeScriptProject: Current project is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (CurrentProject->NativeScriptProject != nullptr)
	{
		LOG.Add("FEEditorScriptingSystem::GenerateNewNativeScriptProject: Native script project already exists", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	// Replace all spaces with underscores.
	for (size_t i = 0; i < FirstScriptName.size(); i++)
	{
		if (FirstScriptName[i] == ' ')
			FirstScriptName[i] = '_';
	}

	CurrentProject->NativeScriptProject = new FENativeScriptProject(CurrentProject);
	if (!CurrentProject->NativeScriptProject->GenerateNewVSProject(FirstScriptName))
	{
		LOG.Add("FEEditorScriptingSystem::GenerateNewNativeScriptProject: Error initializing native script project", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		delete CurrentProject->NativeScriptProject;
		CurrentProject->NativeScriptProject = nullptr;
		return false;
	}

	return true;
}