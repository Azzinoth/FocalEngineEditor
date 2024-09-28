#include "FENativeScriptProject.h"
#include "FEProject.h"
#include <shellapi.h>

std::string FENativeScriptProject::GetCorrectProjectName()
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::GetCorrectProjectName: Parent is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return "";
	}

	std::string ProjectName = Parent->GetName();
	std::string CorrectProjectName = "";
	for (size_t i = 0; i < ProjectName.size(); i++)
	{
		if (ProjectName[i] == ' ')
			CorrectProjectName += "_";
		else
			CorrectProjectName += ProjectName[i];
	}

	return CorrectProjectName;
}

FENativeScriptProject::FENativeScriptProject(FEProject* Parent)
{
	this->Parent = Parent;
	ProjectPath = Parent->GetProjectFolder() + "NativeScriptProject/";
}

FENativeScriptProject::~FENativeScriptProject()
{
	if (PackageData != nullptr)
		delete PackageData;
}

bool FENativeScriptProject::Load(Json::Value Root, std::string FolderPath)
{
	if (!Root.isMember("ModuleID"))
	{
		LOG.Add("FENativeScriptProject::Load: ModuleID is missing!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	ModuleID = Root["ModuleID"].asCString();

	if (!FILE_SYSTEM.DoesDirectoryExist(FolderPath))
	{
		LOG.Add("FENativeScriptProject::Load: Folder does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.DoesFileExist(FolderPath + "NativeScriptProjectData.feassetpackage"))
	{
		LOG.Add("FENativeScriptProject::Load: File does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	PackageData = new FEAssetPackage();
	if (!PackageData->LoadFromFile(FolderPath + "NativeScriptProjectData.feassetpackage"))
	{
		LOG.Add("FENativeScriptProject::Load: Failed to load native script project data!", "FE_LOG_LOADING", FE_LOG_ERROR);
		delete PackageData;
		PackageData = nullptr;
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> SourceFilesList = PackageData->GetEntryList();
	for (size_t i = 0; i < SourceFilesList.size(); i++)
	{
		SourceFileList.push_back(SourceFilesList[i].Name);
	}

	SetFileTracking();
	return true;
}

void FENativeScriptProject::Save(Json::Value& Root, std::string FolderPath)
{
	Root["ModuleID"] = ModuleID;

	if (!FILE_SYSTEM.DoesDirectoryExist(FolderPath))
	{
		LOG.Add("FENativeScriptProject::Save: Folder does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	if (PackageData == nullptr)
	{
		PackageData = new FEAssetPackage();
		for (size_t i = 0; i < SourceFileList.size(); i++)
		{
			std::string FullFilePath = FolderPath + "NativeScriptProject/" + SourceFileList[i];
			if (!FILE_SYSTEM.DoesFileExist(FullFilePath))
			{
				LOG.Add("FENativeScriptProject::Save: File does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
				continue;
			}

			FEAssetPackageEntryIntializeData EntryData;
			EntryData.Name = SourceFileList[i];
			EntryData.Type = "Text";
			PackageData->ImportAssetFromFile(FullFilePath, EntryData);
		}
	}
	else
	{
		std::vector<FEAssetPackageAssetInfo> OldSourceFilesList = PackageData->GetEntryList();
		for (size_t i = 0; i < SourceFileList.size(); i++)
		{
			std::string FullFilePath = FolderPath + "NativeScriptProject/" + SourceFileList[i];
			if (!FILE_SYSTEM.DoesFileExist(FullFilePath))
			{
				LOG.Add("FENativeScriptProject::Save: File does not exist!", "FE_LOG_LOADING", FE_LOG_ERROR);
				continue;
			}

			for (size_t j = 0; j < OldSourceFilesList.size(); j++)
			{
				if (OldSourceFilesList[j].Name == SourceFileList[i])
				{
					if (!PackageData->UpdateAssetFromFile(OldSourceFilesList[j].ID, FullFilePath))
					{
						LOG.Add("FENativeScriptProject::Save: Failed to update asset!", "FE_LOG_LOADING", FE_LOG_ERROR);
					}
					break;
				}
			}
		}
	}

	PackageData->SaveToFile(FolderPath + "NativeScriptProjectData.feassetpackage");
}

bool FENativeScriptProject::GenerateScriptModule()
{
	std::string DebugDllPath = DebugDllFileData.Path;
	std::string DebugPdbPath = DebugPdbFileData.Path;
	std::string ReleaseDllPath = ReleaseDllFileData.Path;

	FENativeScriptModule* NewNativeScriptModule = RESOURCE_MANAGER.CreateNativeScriptModule(DebugDllPath, DebugPdbPath, ReleaseDllPath);
	if (NewNativeScriptModule == nullptr)
	{
		LOG.Add("FENativeScriptProject::GenerateScriptModule: Error creating native script module after external project files update.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (LastGeneratedScriptModule != nullptr)
	{
		NATIVE_SCRIPT_SYSTEM.UpdateNativeScriptModule(LastGeneratedScriptModule->GetObjectID(), NewNativeScriptModule->GetObjectID());
	}
	else
	{
		NATIVE_SCRIPT_SYSTEM.ActivateNativeScriptModule(NewNativeScriptModule);
	}

	LastGeneratedScriptModule = NewNativeScriptModule;
	return true;
}

bool FENativeScriptProject::IsVSProjectValid()
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::IsVSProjectValid: Parent is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (ProjectPath.empty())
	{
		LOG.Add("FENativeScriptProject::IsVSProjectValid: Path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(ProjectPath))
	{
		LOG.Add("FENativeScriptProject::IsVSProjectValid: Path does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<std::string> InitialFilesToCheck = {
		"CMakeLists.txt",
		"SubSystems/FocalEngine/FENativeScriptConnector.h", "SubSystems/FocalEngine/FENativeScriptConnector.cpp",
		"FocalEngine.lib", "FEBasicApplication.lib",
		"BuildManagement/EnsureBuildCompletion.cmake",
		"BuildManagement/DebugBuildActions.cmake", "BuildManagement/ReleaseBuildActions.cmake"
	};

	for (size_t i = 0; i < InitialFilesToCheck.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(ProjectPath + InitialFilesToCheck[i]))
		{
			LOG.Add("FENativeScriptProject::IsVSProjectValid: File " + InitialFilesToCheck[i] + " does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
			return false;
		}
	}

	std::string AppropriateProjectName = GetCorrectProjectName();
	std::vector<std::string> VSProjectFilesToCheck = {
		AppropriateProjectName + ".sln", AppropriateProjectName + ".vcxproj", AppropriateProjectName + ".vcxproj.filters"
	};

	for (size_t i = 0; i < VSProjectFilesToCheck.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(ProjectPath + VSProjectFilesToCheck[i]))
		{
			LOG.Add("FENativeScriptProject::IsVSProjectValid: File " + VSProjectFilesToCheck[i] + " does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
			return false;
		}
	}

	if (SourceFileList.empty())
		return false;

	for (size_t i = 0; i < SourceFileList.size(); i++)
	{
		if (!FILE_SYSTEM.DoesFileExist(ProjectPath + SourceFileList[i]))
		{
			LOG.Add("FENativeScriptProject::IsVSProjectValid: File " + SourceFileList[i] + " does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
			return false;
		}
	}

	return true;
}

bool FENativeScriptProject::RegenerateVSProject()
{
	if (PackageData == nullptr)
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: PackageData is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = PackageData->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: Asset package is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: Parent is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (ProjectPath.empty())
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: Project path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.CreateDirectory(ProjectPath))
	{
		LOG.Add("FENativeScriptProject::RegenerateVSProject: Error creating directory", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<std::string> SourceFileFullPathList;
	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		PackageData->ExportAssetToFile(AssetPackageContent[i].ID, ProjectPath + AssetPackageContent[i].Name);
		SourceFileFullPathList.push_back(ProjectPath + AssetPackageContent[i].Name);
	}

	if (!InitializeProject(SourceFileFullPathList))
		return false;

	if (!ConfigureAndBuildCMake())
		return false;

	// FIX ME! that should not be part of this function.
	return RunProjectVSSolution();
}

bool FENativeScriptProject::InitializeProject(std::vector<std::string> SourceFileFullPathList)
{
	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Parent is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (ProjectPath.empty())
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Project path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (SourceFileFullPathList.empty())
	{
		LOG.Add("FENativeScriptProject::InitializeProject: source file list are empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!SourceFileFullPathList.empty())
	{
		for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
		{
			if (!FILE_SYSTEM.DoesFileExist(SourceFileFullPathList[i]))
			{
				LOG.Add("FENativeScriptProject::InitializeProject: Source file " + SourceFileFullPathList[i] + " does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
				return false;
			}
		}
	}

	// Create all needed folders.
	std::vector<std::string> FoldersToCreate = { "SubSystems/", "SubSystems/FocalEngine", "BuildManagement/" };
	for (size_t i = 0; i < FoldersToCreate.size(); i++)
	{
		if (!FILE_SYSTEM.CreateDirectory(ProjectPath + FoldersToCreate[i]))
		{
			LOG.Add("FENativeScriptProject::InitializeProject: Error creating " + FoldersToCreate[i] + " directory", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	// Place required files in the destination directory.
	std::string EditorFolder = FILE_SYSTEM.GetCurrentWorkingPath();
	std::vector<std::pair<std::string, std::string>> FilesToCopy;
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/BuildManagement/EnsureBuildCompletion.cmake", ProjectPath + "BuildManagement/EnsureBuildCompletion.cmake" });
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/BuildManagement/DebugBuildActions.cmake", ProjectPath + "BuildManagement/DebugBuildActions.cmake" });
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/BuildManagement/ReleaseBuildActions.cmake", ProjectPath + "BuildManagement/ReleaseBuildActions.cmake" });
	FilesToCopy.push_back({ EditorFolder + "/UserScripts/NativeScriptProjectData/CMakeLists.txt", ProjectPath + "CMakeLists.txt" });
	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/FENativeScriptConnector.h", ProjectPath + "SubSystems/FocalEngine/FENativeScriptConnector.h" });
	FilesToCopy.push_back({ EnginePath + "/Resources/UserScriptsData/FENativeScriptConnector.cpp", ProjectPath + "SubSystems/FocalEngine/FENativeScriptConnector.cpp" });

	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		FilesToCopy.push_back({ SourceFileFullPathList[i], ProjectPath + FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]) });
		if (FilesToCopy.back().first == FilesToCopy.back().second)
			FilesToCopy.pop_back();
	}

	for (size_t i = 0; i < FilesToCopy.size(); i++)
	{
		if (!FILE_SYSTEM.CopyFile(FilesToCopy[i].first, FilesToCopy[i].second))
		{
			LOG.Add("FENativeScriptProject::InitializeProject: Error copying file " + FilesToCopy[i].first + " to " + FilesToCopy[i].second, "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	SourceFileList.clear();
	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		SourceFileList.push_back(FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]));
	}

	if (!InitializeCMakeFileAndScriptFiles(SourceFileFullPathList))
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Error initializing CMakeLists.txt", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!UpdateEngineFiles())
	{
		LOG.Add("FENativeScriptProject::InitializeProject: Error updating engine files in user native script project", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	SetFileTracking();
	return true;
}

bool FENativeScriptProject::InitializeCMakeFileAndScriptFiles(std::vector<std::string> SourceFileFullPathList)
{
	if (ProjectPath.empty())
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (ModuleID.empty())
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: DLL module ID is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string ProjectName = GetCorrectProjectName();

	std::string CMakeFilePath = ProjectPath + "CMakeLists.txt";

	std::vector<InstructionWhatToReplaceInFile> Instructions;
	InstructionWhatToReplaceInFile CurrentInstruction;
	CurrentInstruction.SubStringInLineToTrigger = "set(PROJECT_NAME PLACE_HOLDER)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ProjectName;
	Instructions.push_back(CurrentInstruction);

	std::string SourceFilesList = "file(GLOB Main_SRC\n";
	for (size_t i = 0; i < SourceFileFullPathList.size(); i++)
	{
		SourceFilesList += "\t\"" + FILE_SYSTEM.GetFileName(SourceFileFullPathList[i]) + "\"\n";
	}
	SourceFilesList += ")";

	CurrentInstruction.SubStringInLineToTrigger = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.What = "file(GLOB Main_SRC PLACE_HOLDER)";
	CurrentInstruction.ReplaceWith = SourceFilesList;
	Instructions.push_back(CurrentInstruction);

	if (!ReplaceInFile(CMakeFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing CMakeLists.txt", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "--target PLACE_HOLDER --config Release";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ProjectName;
	Instructions.push_back(CurrentInstruction);

	std::string DebugBuildActionsCMake = ProjectPath + "BuildManagement/DebugBuildActions.cmake";
	if (!ReplaceInFile(DebugBuildActionsCMake, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing BuildManagement/DebugBuildActions.cmake", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "--target PLACE_HOLDER --config Debug";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ProjectName;
	Instructions.push_back(CurrentInstruction);

	std::string ReleaseBuildActionsCMake = ProjectPath + "BuildManagement/ReleaseBuildActions.cmake";
	if (!ReplaceInFile(ReleaseBuildActionsCMake, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing BuildManagement/ReleaseBuildActions.cmake", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "SET_MODULE_ID(\"PLACE_HOLDER\");";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ModuleID;
	Instructions.push_back(CurrentInstruction);

	std::string FENativeScriptConnectorHeaderFilePath = ProjectPath + "SubSystems/FocalEngine/FENativeScriptConnector.h";
	if (!ReplaceInFile(FENativeScriptConnectorHeaderFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::InitializeCMakeFileAndScriptFiles: Error initializing FENativeScriptConnector.h", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FENativeScriptProject::ReplaceInFile(std::string Path, std::vector<InstructionWhatToReplaceInFile> Instructions)
{
	if (Path.empty())
	{
		LOG.Add("FENativeScriptProject::ReplaceInFile: File path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::fstream File(Path, std::ios::in);
	if (!File.is_open())
	{
		LOG.Add("FENativeScriptProject::ReplaceInFile: Error opening file " + Path, "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<std::string> FileContent;
	std::string Line;
	while (std::getline(File, Line))
		FileContent.push_back(Line);

	File.close();

	for (size_t i = 0; i < FileContent.size(); i++)
	{
		for (size_t j = 0; j < Instructions.size(); j++)
		{
			if (FileContent[i].find(Instructions[j].SubStringInLineToTrigger) != std::string::npos)
			{
				FileContent[i].replace(FileContent[i].find(Instructions[j].What), Instructions[j].What.size(), Instructions[j].ReplaceWith);
			}
		}
	}

	File.open(Path, std::ios::out | std::ios::trunc);
	if (!File.is_open())
	{
		LOG.Add("FENativeScriptProject::ReplaceInFile: Error opening file " + Path, "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Write the modified content back to the file
	for (size_t i = 0; i < FileContent.size(); i++)
		File << FileContent[i] + "\n";

	File.close();
	return true;
}

bool FENativeScriptProject::UpdateEngineFiles()
{
	if (ProjectPath.empty())
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: ProjectPath is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(ProjectPath))
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: ProjectPath does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	FEAssetPackage* EngineHeadersPackage = CreateEngineHeadersAssetPackage();
	if (EngineHeadersPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error creating engine headers asset package.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!UnPackEngineHeadersAssetPackage(EngineHeadersPackage, ProjectPath + "SubSystems/FocalEngine/"))
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error unpacking engine headers asset package.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackage* EngineLIBPackage = CreateEngineLIBAssetPackage();
	if (EngineLIBPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error creating engine lib asset package.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!UnPackEngineLIBAssetPackage(EngineLIBPackage, ProjectPath))
	{
		LOG.Add("FENativeScriptProject::UpdateEngineFiles: Error unpacking engine lib asset package.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

FEAssetPackage* FENativeScriptProject::CreateEngineHeadersAssetPackage()
{
	FEAssetPackage* EngineHeadersAssetPackage = new FEAssetPackage();
	EngineHeadersAssetPackage->SetName("EngineHeaders");
	if (EngineHeadersAssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::CreateEngineHeadersAssetPackage: Error creating asset package", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FENativeScriptProject::CreateEngineHeadersAssetPackage: Engine folder does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return nullptr;
	}

	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath, true);
	// After having all files in the engine folder, we need to filter out only the header files.
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].find(".h") != std::string::npos || AllFiles[i].find(".inl") != std::string::npos)
		{
			FEAssetPackageEntryIntializeData EntryData;
			// Also since FEAssetPackage does not support folders, we need to save folder structure in the file name.
			// But we will erase the engine folder path from the file name.
			EntryData.Name = AllFiles[i].substr(EnginePath.size());
			EntryData.Type = "Text";
			EntryData.Tag = EDITOR_RESOURCE_TAG;
			EntryData.Comment = "Engine header file";

			EngineHeadersAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	return EngineHeadersAssetPackage;
}

bool FENativeScriptProject::UnPackEngineHeadersAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Asset package is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Destination path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Destination path does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Asset package is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		std::string LocalPath = std::filesystem::path(AssetPackageContent[i].Name).parent_path().string();
		// Since we are not using folders in FEAssetPackage, we need to create all folders in the file path.
		// First we need to get chain of folders.
		std::vector<std::string> FolderChain;
		try
		{
			std::filesystem::path Directory(LocalPath);
			while (!Directory.string().empty())
			{
				if (!FolderChain.empty())
				{
					if (FolderChain.back() == Directory.string())
						break;
				}
				FolderChain.push_back(Directory.string());
				Directory = Directory.parent_path();
			}

			std::reverse(FolderChain.begin(), FolderChain.end());
		}
		catch (const std::exception& Exception)
		{
			LOG.Add("Error in FENativeScriptProject::UnPackEngineHeadersAssetPackage: " + std::string(Exception.what()), "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}

		// Then we will go from the root folder to the last folder and create them if they do not exist.
		for (size_t i = 0; i < FolderChain.size(); i++)
		{
			std::string FinalPath = Path + FolderChain[i];
			if (!FILE_SYSTEM.DoesDirectoryExist(FinalPath))
			{
				if (!FILE_SYSTEM.CreateDirectory(FinalPath))
				{
					LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Error creating directory " + FinalPath, "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
					return false;
				}
			}
		}

		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

FEAssetPackage* FENativeScriptProject::CreateEngineLIBAssetPackage()
{
	FEAssetPackage* EngineLIBAssetPackage = new FEAssetPackage();
	EngineLIBAssetPackage->SetName("EngineHeaders");
	if (EngineLIBAssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::CreateEngineLIBAssetPackage: Error creating asset package", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return nullptr;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EnginePath))
	{
		LOG.Add("FENativeScriptProject::CreateEngineLIBAssetPackage: Engine folder does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return nullptr;
	}

	std::vector<std::string> AllFiles = FILE_SYSTEM.GetFilesInDirectory(EnginePath, true);

	std::vector<std::string> DebugStrings;
	// After having all files in the engine folder, we need to filter out only the lib files.
	for (size_t i = 0; i < AllFiles.size(); i++)
	{
		if (AllFiles[i].find(".lib") != std::string::npos)
		{
			// FIX ME! Currently projects would need only debug lib files. Is it correct?
			// And only FocalEngine.lib and FEBasicApplication.lib are needed.
			if (AllFiles[i].find("FocalEngine.lib") == std::string::npos && AllFiles[i].find("FEBasicApplication.lib") == std::string::npos)
				continue;

			FEAssetPackageEntryIntializeData EntryData;
			EntryData.Name = FILE_SYSTEM.GetFileName(AllFiles[i]);
			DebugStrings.push_back(EntryData.Name);
			EntryData.Type = "BINARY";
			EntryData.Tag = EDITOR_RESOURCE_TAG;
			EntryData.Comment = "Engine lib file";

			EngineLIBAssetPackage->ImportAssetFromFile(AllFiles[i], EntryData);
		}
	}

	return EngineLIBAssetPackage;
}

bool FENativeScriptProject::UnPackEngineLIBAssetPackage(FEAssetPackage* AssetPackage, std::string Path)
{
	if (AssetPackage == nullptr)
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Asset package is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Path.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Destination path is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(Path))
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Destination path does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::vector<FEAssetPackageAssetInfo> AssetPackageContent = AssetPackage->GetEntryList();
	if (AssetPackageContent.empty())
	{
		LOG.Add("FENativeScriptProject::UnPackEngineLIBAssetPackage: Asset package is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	for (size_t i = 0; i < AssetPackageContent.size(); i++)
	{
		// Now we are ready to write the file.
		if (!AssetPackage->ExportAssetToFile(AssetPackageContent[i].ID, Path + AssetPackageContent[i].Name))
		{
			LOG.Add("FENativeScriptProject::UnPackEngineHeadersAssetPackage: Error exporting asset " + AssetPackageContent[i].ID + " to " + Path + AssetPackageContent[i].Name, "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
			return false;
		}
	}

	return true;
}

bool FENativeScriptProject::ConfigureAndBuildCMake()
{
	if (ProjectPath.empty())
	{
		LOG.Add("FENativeScriptProject::RunCMake: ProjectPath is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string Generator = "Visual Studio 17 2022";

	// CMake configure command.
	std::string ConfigureCommand = "cmake -S \"" + ProjectPath + "\" -B \"" + ProjectPath + "\" -G \"" + Generator + "\"";

	// Execute CMake configure command.
	int ConfigureResult = std::system(ConfigureCommand.c_str());
	if (ConfigureResult != 0)
	{
		LOG.Add("FENativeScriptProject::RunCMake: Error running CMake configure command", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	// Construct the CMake build command
	std::string BuildCommand = "cmake --build \"" + ProjectPath + "\" --config Debug";

	// Execute CMake build command
	int BuildResult = std::system(BuildCommand.c_str());
	if (BuildResult != 0)
	{
		LOG.Add("FENativeScriptProject::RunCMake: Error running CMake build command", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FENativeScriptProject::RunProjectVSSolution()
{
	if (ProjectPath.empty())
	{
		LOG.Add("FENativeScriptProject::RunProjectVSSolution: ProjectPath is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string ProjectName = GetCorrectProjectName();
	if (ProjectName.empty())
	{
		LOG.Add("FENativeScriptProject::RunProjectVSSolution: Project name is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string SolutionPath = ProjectPath + ProjectName + ".sln";

	// Use ShellExecute to open the solution file
	HINSTANCE Result = ShellExecuteA(
		NULL,
		"open",
		SolutionPath.c_str(),
		NULL,
		NULL,
		SW_SHOWNORMAL
	);

	// Check if ShellExecute was successful
	if ((INT_PTR)Result <= 32)
	{
		LOG.Add("FENativeScriptProject::RunProjectVSSolution: Error running solution", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

void FENativeScriptProject::SetFileTracking()
{
	std::string AppropriateProjectName = GetCorrectProjectName();

	DebugDllFileData.Path = ProjectPath + "Debug/" + AppropriateProjectName + ".dll";
	DebugDllFileData.WriteTime = FILE_SYSTEM.GetFileLastWriteTime(DebugDllFileData.Path);

	DebugPdbFileData.Path = ProjectPath + "Debug/" + AppropriateProjectName + ".pdb";
	DebugPdbFileData.WriteTime = FILE_SYSTEM.GetFileLastWriteTime(DebugPdbFileData.Path);

	ReleaseDllFileData.Path = ProjectPath + "Release/" + AppropriateProjectName + ".dll";
	ReleaseDllFileData.WriteTime = FILE_SYSTEM.GetFileLastWriteTime(ReleaseDllFileData.Path);
}

bool FENativeScriptProject::GenerateScriptFilesFromTemplate(std::string ScriptName)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(ProjectPath))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: ProjectPath does not exist", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (ScriptName.empty())
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Script name is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER);
	std::string TemplateHeaderFilePath = EnginePath + "/Resources/UserScriptsData/NativeScriptTemplate.h";
	std::string TemplateCPPFilePath = EnginePath + "/Resources/UserScriptsData/NativeScriptTemplate.cpp";

	if (!FILE_SYSTEM.CopyFile(TemplateHeaderFilePath, ProjectPath + ScriptName + ".h"))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error copying file " + TemplateHeaderFilePath + " to " + ProjectPath + ScriptName + ".h", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyFile(TemplateCPPFilePath, ProjectPath + ScriptName + ".cpp"))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error copying file " + TemplateCPPFilePath + " to " + ProjectPath + ScriptName + ".cpp", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	std::vector<InstructionWhatToReplaceInFile> Instructions;
	InstructionWhatToReplaceInFile CurrentInstruction;
	CurrentInstruction.SubStringInLineToTrigger = "SET_MODULE_ID(\"PLACE_HOLDER\")";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = "2342HA";
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "class PLACE_HOLDER : public FENativeScriptCore";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "REGISTER_SCRIPT(PLACE_HOLDER)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "REGISTER_SCRIPT_FIELD(PLACE_HOLDER, int, ExampleVariable)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	std::string ScriptHeaderFilePath = ProjectPath + ScriptName + ".h";
	if (!ReplaceInFile(ScriptHeaderFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error initializing " + ScriptName + ".h", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	CurrentInstruction.SubStringInLineToTrigger = "#include \"NativeScriptTemplate.h\"";
	CurrentInstruction.What = "NativeScriptTemplate";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "void PLACE_HOLDER::Awake()";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "void PLACE_HOLDER::OnDestroy()";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "void PLACE_HOLDER::OnUpdate(double DeltaTime)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = ScriptName;
	Instructions.push_back(CurrentInstruction);

	std::string ScriptSourceFilePath = ProjectPath + ScriptName + ".cpp";
	if (!ReplaceInFile(ScriptSourceFilePath, Instructions))
	{
		LOG.Add("FENativeScriptProject::GenerateScriptFilesFromTemplate: Error initializing " + ScriptName + ".cpp", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FENativeScriptProject::GenerateNewVSProject(std::string FirstScriptName)
{
	if (FirstScriptName.empty())
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: First script name is empty", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Parent == nullptr)
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: Parent is nullptr", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	// Replace all spaces with underscores.
	for (size_t i = 0; i < FirstScriptName.size(); i++)
	{
		if (FirstScriptName[i] == ' ')
			FirstScriptName[i] = '_';
	}

	if (FILE_SYSTEM.DoesDirectoryExist(ProjectPath))
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: VS project already exists", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.CreateDirectory(ProjectPath))
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: Error creating directory", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	ModuleID = APPLICATION.GetUniqueHexID();

	if (!GenerateScriptFilesFromTemplate(FirstScriptName))
	{
		LOG.Add("FENativeScriptProject::GenerateNewVSProject: Error generating script files from template", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!InitializeProject({ ProjectPath + FirstScriptName + ".h", ProjectPath + FirstScriptName + ".cpp" }))
		return false;

	if (!ConfigureAndBuildCMake())
		return false;

	// FIX ME! that should not be part of this function.
	return RunProjectVSSolution();
}

bool FENativeScriptProject::IsFileChanged(const TrackedFileData& FileData)
{
	uint64_t NewWriteTime = FILE_SYSTEM.GetFileLastWriteTime(FileData.Path);
	if (NewWriteTime != FileData.WriteTime && NewWriteTime != 0)
	{
		uint64_t Differece = NewWriteTime >= FileData.WriteTime ? NewWriteTime - FileData.WriteTime : FileData.WriteTime - NewWriteTime;
		if (Differece > DifferenceThreshold)
			return true;
	}

	return false;
}

void FENativeScriptProject::UpdateTrackedFileWriteTime(TrackedFileData& FileData)
{
	uint64_t NewTimeStamp = FILE_SYSTEM.GetFileLastWriteTime(FileData.Path);

	if (NewTimeStamp > FileData.WriteTime)
		FileData.WriteTime = NewTimeStamp;
}

bool FENativeScriptProject::Update()
{
	if (!IsVSProjectValid())
		return false;

	bool CheckForReload = false;
	if (IsFileChanged(DebugDllFileData) && IsFileChanged(DebugPdbFileData) || IsFileChanged(ReleaseDllFileData))
		CheckForReload = true;

	if (!CheckForReload)
		return false;

	// Waiting for external build system to finish.
	FILE_SYSTEM.WaitForFileAccess(ProjectPath + "BuildManagement/Force_Build_Finished.txt", 1000);

	if (FILE_SYSTEM.DoesFileExist(ProjectPath + "BuildManagement/Force_Build_Finished.txt"))
	{
		FILE_SYSTEM.DeleteFile(ProjectPath + "BuildManagement/Force_Build_Finished.txt");
	}
	else
	{
		LOG.Add("FENativeScriptProject::Update: Force_Build_Finished.txt does not exist, script reload aborted.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
			
		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	// Waiting for all files to be accessible.
	if (!FILE_SYSTEM.WaitForFileAccess(DebugDllFileData.Path, 2000))
	{
		LOG.Add("FENativeScriptProject::Update: File " + DebugDllFileData.Path + "does not exist, script reload aborted.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);
			
		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	if (!FILE_SYSTEM.WaitForFileAccess(DebugPdbFileData.Path, 2000))
	{
		LOG.Add("FENativeScriptProject::Update: File " + DebugPdbFileData.Path + "does not exist, script reload aborted.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);

		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	if (!FILE_SYSTEM.WaitForFileAccess(ReleaseDllFileData.Path, 2000))
	{
		LOG.Add("FENativeScriptProject::Update: File " + ReleaseDllFileData.Path + "does not exist, script reload aborted.", "FE_EDITOR_SCRIPT_SYSTEM", FE_LOG_WARNING);

		UpdateTrackedFileWriteTime(DebugDllFileData);
		UpdateTrackedFileWriteTime(DebugPdbFileData);
		UpdateTrackedFileWriteTime(ReleaseDllFileData);

		return false;
	}

	bool bResult = GenerateScriptModule();

	UpdateTrackedFileWriteTime(DebugDllFileData);
	UpdateTrackedFileWriteTime(DebugPdbFileData);
	UpdateTrackedFileWriteTime(ReleaseDllFileData);

	return bResult;
}