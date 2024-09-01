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
	//std::string StartOfPath = "D:/Script__08_28_2024/Debug/";
	////std::string StartOfPath = "C:/Users/kberegovyi/Downloads/Script__08_27_2024/Debug/";

	//uint64_t NewWriteTime = FILE_SYSTEM.GetFileLastWriteTime(StartOfPath + "UserScriptTest.dll");
	//if (NewWriteTime != LastWriteTime && NewWriteTime != 0)
	//{
	//	// FIX ME! It is called 2 times.
	//	uint64_t Differece = NewWriteTime >= LastWriteTime ? NewWriteTime - LastWriteTime : LastWriteTime - NewWriteTime;
	//	LastWriteTime = NewWriteTime;

	//	if (Differece > 1'000'000)
	//	{
	//		//std::string NewWriteTimeStr = TIME.NanosecondTimeStampToDate(FILE_SYSTEM.GetFileLastWriteTime(StartOfPath + "UserScriptTest.dll"));
	//		//std::string OldWriteTimeStr = TIME.NanosecondTimeStampToDate(LastWriteTime);

	//		LastWriteTime = NewWriteTime;

	//		if (!FILE_SYSTEM.DoesDirectoryExist(FILE_SYSTEM.GetCurrentWorkingPath() + "/NativeScriptsTemp"))
	//			FILE_SYSTEM.CreateDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "/NativeScriptsTemp");

	//		FILE_SYSTEM.WaitForFileAccess(StartOfPath + "UserScriptTest.dll", 2000);
	//		FILE_SYSTEM.WaitForFileAccess(StartOfPath + "UserScriptTest.pdb", 2000);

	//		FENativeScriptModule* NewNativeScriptModule = RESOURCE_MANAGER.CreateNativeScriptModule(StartOfPath + "UserScriptTest.dll", StartOfPath + "UserScriptTest.pdb");
	//		if (ExternalEditorActiveModule != nullptr)
	//		{
	//			NATIVE_SCRIPT_SYSTEM.UpdateNativeScriptModule(ExternalEditorActiveModule->GetObjectID(), NewNativeScriptModule->GetObjectID());
	//		}
	//		else
	//		{
	//			NATIVE_SCRIPT_SYSTEM.ActivateNativeScriptModule(NewNativeScriptModule);
	//		}

	//		ExternalEditorActiveModule = NewNativeScriptModule;
	//		
	//		int y = 0;
	//		y++;
	//	}
	//}
}
