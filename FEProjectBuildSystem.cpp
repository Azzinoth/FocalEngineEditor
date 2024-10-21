#include "FEProjectBuildSystem.h"
using namespace FocalEngine;

FEProjectBuildSystem::FEProjectBuildSystem()
{
	
}

FEProjectBuildSystem::~FEProjectBuildSystem()
{
}

std::string FEProjectBuildSystem::GetVSProjectName(FEProject* ProjectToBuild)
{
	if (ProjectToBuild == nullptr)
	{
		LOG.Add("FEProjectBuildSystem::GetVSProjectName: ProjectToBuild is nullptr!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return "";
	}

	std::string AppropriateProjectName = ProjectToBuild->GetName();
	for (size_t i = 0; i < AppropriateProjectName.size(); i++)
	{
		if (AppropriateProjectName[i] == ' ')
			AppropriateProjectName[i] = '_';
	}

	if (AppropriateProjectName.empty())
		AppropriateProjectName = "UntitledProject";

	return AppropriateProjectName;
}

bool FEProjectBuildSystem::ReplaceInFile(std::string Path, std::vector<InstructionWhatToReplaceInFile> Instructions)
{
	if (Path.empty())
	{
		LOG.Add("FEProjectBuildSystem::ReplaceInFile: File path is empty", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
		return false;
	}

	std::fstream File(Path, std::ios::in);
	if (!File.is_open())
	{
		LOG.Add("FEProjectBuildSystem::ReplaceInFile: Error opening file " + Path, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
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
		LOG.Add("FEProjectBuildSystem::ReplaceInFile: Error opening file " + Path, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Write the modified content back to the file
	for (size_t i = 0; i < FileContent.size(); i++)
		File << FileContent[i] + "\n";

	File.close();
	return true;
}

bool FEProjectBuildSystem::CopyVisualNodeSystemSubProjectFiles(const std::string& OutputPath)
{
	if (!FILE_SYSTEM.DoesDirectoryExist(OutputPath))
	{
		LOG.Add("FEProjectBuildSystem::CopyVisualNodeSystemSubProjectFiles: OutputPath does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Create all needed folders.
	std::vector<std::string> FoldersToCreate = { "VisualNodeSystem/"};
	for (size_t i = 0; i < FoldersToCreate.size(); i++)
	{
		if (!FILE_SYSTEM.CreateDirectory(OutputPath + FoldersToCreate[i]))
		{
			LOG.Add("FEProjectBuildSystem::CopyVisualNodeSystemSubProjectFiles: Error creating " + FoldersToCreate[i] + " directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	// Place required files in the destination directory.
	std::string EditorPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/";
	std::string VisualNodeSystemPath = EditorPath + "SubSystems/NodeSystem/VisualNodeSystem/";
	std::vector<std::pair<std::string, std::string>> FilesToCopy;
	FilesToCopy.push_back({ VisualNodeSystemPath + "CMakeLists.txt", OutputPath + "VisualNodeSystem/CMakeLists.txt" });

	// Add all source files to the list.
	std::vector<std::string> FilesInMainFolder = FILE_SYSTEM.GetFileList(VisualNodeSystemPath);
	for (size_t i = 0; i < FilesInMainFolder.size(); i++)
	{
		if (FilesInMainFolder[i].substr(FilesInMainFolder[i].size() - 2) == ".h" ||
			FilesInMainFolder[i].substr(FilesInMainFolder[i].size() - 4) == ".hpp" ||
			FilesInMainFolder[i].substr(FilesInMainFolder[i].size() - 4) == ".inl" || 
			FilesInMainFolder[i].substr(FilesInMainFolder[i].size() - 4) == ".cpp" ||
			FilesInMainFolder[i].substr(FilesInMainFolder[i].size() - 2) == ".c")
		{
			FilesToCopy.push_back({ VisualNodeSystemPath + FilesInMainFolder[i], OutputPath + "VisualNodeSystem/" + FilesInMainFolder[i] });
		}
	}
	
	for (size_t i = 0; i < FilesToCopy.size(); i++)
	{
		if (!FILE_SYSTEM.CopyFile(FilesToCopy[i].first, FilesToCopy[i].second))
		{
			LOG.Add("FEProjectBuildSystem::CopyVisualNodeSystemSubProjectFiles: Error copying file " + FilesToCopy[i].first + " to " + FilesToCopy[i].second, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	if (!FILE_SYSTEM.CopyDirectory(VisualNodeSystemPath + "SubSystems/",
								   OutputPath + "VisualNodeSystem/SubSystems/"))
	{
		LOG.Add("FEProjectBuildSystem::CopyVisualNodeSystemSubProjectFiles: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyDirectory(VisualNodeSystemPath + "ThirdParty/",
								   OutputPath + "VisualNodeSystem/ThirdParty/"))
	{
		LOG.Add("FEProjectBuildSystem::CopyVisualNodeSystemSubProjectFiles: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEProjectBuildSystem::BuildExecutable(FEProject* ProjectToBuild)
{
	if (ProjectToBuild == nullptr)
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: ProjectToBuild is nullptr!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	std::string VSProjectDirectory = FILE_SYSTEM.GetCurrentWorkingPath() + "/BuildProjects_Temporary/";
	if (FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		if (!FILE_SYSTEM.DeleteDirectory(VSProjectDirectory))
		{
			LOG.Add("FEProjectBuildSystem::BuildExecutable: Error deleting BuildProjects_Temporary directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	if (!FILE_SYSTEM.CreateDirectory(VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error creating BuildProjects_Temporary directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	VSProjectDirectory += ProjectToBuild->GetID() + "/";
	if (!FILE_SYSTEM.CreateDirectory(VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error creating project directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: VSProjectDirectory does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackage* Resources = ProjectToBuild->SaveResourcesToAssetPackage();
	Resources->SaveToFile(VSProjectDirectory + "/Resources.fepackage");

	// Create all needed folders.
	std::vector<std::string> FoldersToCreate = { "SubSystems/",
												 "SubSystems/FocalEngine",
												 "ScriptModules/",
												 "SubSystems/FocalEngine/ResourceManager",
												 "SubSystems/FocalEngine/SubSystems",
												 "SubSystems/FocalEngine/ThirdParty",
												 "SubSystems/FocalEngine/ThirdParty/openxr",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/GLFW",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/glew2" };

	for (size_t i = 0; i < FoldersToCreate.size(); i++)
	{
		if (!FILE_SYSTEM.CreateDirectory(VSProjectDirectory + FoldersToCreate[i]))
		{
			LOG.Add("FEProjectBuildSystem::InitializeProject: Error creating " + FoldersToCreate[i] + " directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	// Place required files in the destination directory.
	std::string EditorPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/";
	std::vector<std::pair<std::string, std::string>> FilesToCopy;
	FilesToCopy.push_back({ EditorPath + "/Resources/BuildScripts/Main_Template.cpp", VSProjectDirectory + "Main.cpp" });
	FilesToCopy.push_back({ EditorPath + "/Resources/BuildScripts/Main_Template.h", VSProjectDirectory + "Main.h" });
	FilesToCopy.push_back({ EditorPath + "/Resources/BuildScripts/CMakeLists_Template.txt", VSProjectDirectory + "CMakeLists.txt" });
	std::string EnginePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/" + std::string(ENGINE_FOLDER) + "/";
	FilesToCopy.push_back({ EnginePath + "CMakeLists.txt", VSProjectDirectory + "SubSystems/FocalEngine/CMakeLists.txt" });
	FilesToCopy.push_back({ EnginePath + "UpdateTimestamp.cmake", VSProjectDirectory + "SubSystems/FocalEngine/UpdateTimestamp.cmake" });
	FilesToCopy.push_back({ EnginePath + "ResourceManager/Config.h.in", VSProjectDirectory + "SubSystems/FocalEngine/ResourceManager/Config.h.in" });
	FilesToCopy.push_back({ EnginePath + "ResourceManager/Timestamp.h.in", VSProjectDirectory + "SubSystems/FocalEngine/ResourceManager/Timestamp.h.in" });
	FilesToCopy.push_back({ EnginePath + "SubSystems/FEBasicApplication/CMakeLists.txt", VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/CMakeLists.txt" });

	for (size_t i = 0; i < FilesToCopy.size(); i++)
	{
		if (!FILE_SYSTEM.CopyFile(FilesToCopy[i].first, FilesToCopy[i].second))
		{
			LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying file " + FilesToCopy[i].first + " to " + FilesToCopy[i].second, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	if (!FILE_SYSTEM.CopyDirectory(EnginePath + "/SubSystems/FEBasicApplication/ThirdParty/GLFW/lib",
								   VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/GLFW/lib"))
	{
		LOG.Add("FEProjectBuildSystem::InitializeProject: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyDirectory(EnginePath + "/SubSystems/FEBasicApplication/ThirdParty/glew2/lib",
								   VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/glew2/lib"))
	{
		LOG.Add("FEProjectBuildSystem::InitializeProject: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyDirectory(EnginePath + "/ThirdParty/openxr/Lib",
								   VSProjectDirectory + "SubSystems/FocalEngine/ThirdParty/openxr/Lib"))
	{
		LOG.Add("FEProjectBuildSystem::InitializeProject: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!CopyVisualNodeSystemSubProjectFiles(VSProjectDirectory + "/SubSystems/"))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying VisualNodeSystem sub project files", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	FEAssetPackage* EngineResources = RESOURCE_MANAGER.CreatePrivateEngineAssetPackage();
	if (EngineResources == nullptr)
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error creating private engine asset package", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}
	EngineResources->SaveToFile(VSProjectDirectory + "/EngineResources.fepackage");

	if (!InitializeCMakeFileAndScriptFiles(ProjectToBuild, VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::InitializeProject: Error initializing CMakeLists.txt", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Copy the engine files to the temporary directory
	if (!RESOURCE_MANAGER.CopyEngineFiles(true, true, false, VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying engine files", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Build the executable
	if (!ConfigureAndBuildCMake(VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error configuring and building VS Project", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!CreateFinalExecutableDirectory(ProjectToBuild/*, VSProjectDirectory*/))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error creating final executable directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Delete the temporary directory
	if (!FILE_SYSTEM.DeleteDirectory(VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error deleting temporary directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}
	
	return true;
}

bool FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles(FEProject* ProjectToBuild, const std::string& VSProjectDirectory)
{
	if (ProjectToBuild == nullptr)
	{
		LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: ProjectToBuild is nullptr!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (VSProjectDirectory.empty())
	{
		LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: path is empty", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
		return false;
	}

	std::vector<std::string> NativeScriptModuleIDList = RESOURCE_MANAGER.GetNativeScriptModuleIDList();

	std::string CMakeFilePath = VSProjectDirectory + "CMakeLists.txt";

	std::vector<InstructionWhatToReplaceInFile> Instructions;
	InstructionWhatToReplaceInFile CurrentInstruction;
	CurrentInstruction.SubStringInLineToTrigger = "set(PROJECT_NAME PLACE_HOLDER)";
	CurrentInstruction.What = "PLACE_HOLDER";
	CurrentInstruction.ReplaceWith = GetVSProjectName(ProjectToBuild);
	Instructions.push_back(CurrentInstruction);

	std::string SourceFileListString = "";
	std::string ExecutableListString = "";
	std::string FolderListInVSProjectString = "";

	for (size_t i = 0; i < NativeScriptModuleIDList.size(); i++)
	{
		FENativeScriptModule* CurrentModule = RESOURCE_MANAGER.GetNativeScriptModule(NativeScriptModuleIDList[i]);
		if (CurrentModule == nullptr)
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error getting native script module with ID: " + NativeScriptModuleIDList[i], "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}
			
		if (CurrentModule->GetProject() == nullptr)
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error getting project of native script module with ID: " + NativeScriptModuleIDList[i], "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}

		if (!CurrentModule->GetProject()->HasRecoverableVSProjectData())
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Project of native script module with ID: " + NativeScriptModuleIDList[i] + " does not have recoverable VS project data", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}
		
		std::vector<std::string> SourceFileList = CurrentModule->GetProject()->GetSourceFileList();
		if (SourceFileList.empty())
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: SourceFileList is empty for native script module with ID: " + NativeScriptModuleIDList[i], "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}

		std::string CurrentString = "file(GLOB Script_Module_" + CurrentModule->GetObjectID() + "_SRC\n";
		for (size_t i = 0; i < SourceFileList.size(); i++)
		{
			CurrentString += std::string("\t\"") + "ScriptModules/" + CurrentModule->GetObjectID() + "/" + SourceFileList[i] + "\"\n";
		}
		
		CurrentString += ")\n";
		CurrentString += "\n";

		SourceFileListString += CurrentString;

		// Replace the placeholder in add_executable list.
		CurrentString = std::string("\t\t") + "${Script_Module_" + CurrentModule->GetObjectID() + "_SRC}\n";
		ExecutableListString += CurrentString;

		// Add the folder to the list of folders in VS project.
		FolderListInVSProjectString += "source_group(\"Source Files/ScriptModules/" + CurrentModule->GetObjectID() + "/\" FILES ${Script_Module_" + CurrentModule->GetObjectID() + "_SRC})\n";

		// Now we will create the directories and copy the files to the destination directory.
		std::string ScriptModuleDirectory = VSProjectDirectory + "ScriptModules/" + CurrentModule->GetObjectID() + "/";
		if (!FILE_SYSTEM.CreateDirectory(ScriptModuleDirectory))
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error creating directory " + ScriptModuleDirectory, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}

		if (!CurrentModule->GetProject()->ExtractSourceFilesTo(ScriptModuleDirectory))
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error extracting source files to " + ScriptModuleDirectory, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	CurrentInstruction.SubStringInLineToTrigger = "#(PLACE_HOLDER) Here should be list of all script modules sources.";
	CurrentInstruction.What = "#(PLACE_HOLDER) Here should be list of all script modules sources.";
	CurrentInstruction.ReplaceWith = SourceFileListString;
	Instructions.push_back(CurrentInstruction);


	CurrentInstruction.SubStringInLineToTrigger = "		#(PLACE_HOLDER) Here should be list of all script modules.";
	CurrentInstruction.What = "		#(PLACE_HOLDER) Here should be list of all script modules.";
	CurrentInstruction.ReplaceWith = ExecutableListString;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.SubStringInLineToTrigger = "#(PLACE_HOLDER) Here should be list of all script modules and where to place them in VS project.";
	CurrentInstruction.What = "#(PLACE_HOLDER) Here should be list of all script modules and where to place them in VS project.";
	CurrentInstruction.ReplaceWith = FolderListInVSProjectString;
	Instructions.push_back(CurrentInstruction);

	if (!ReplaceInFile(CMakeFilePath, Instructions))
	{
		LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error initializing CMakeLists.txt", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

bool FEProjectBuildSystem::ConfigureAndBuildCMake(const std::string& VSProjectDirectory)
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FEProjectBuildSystem::ConfigureAndBuildCMake: VSProjectDirectory is empty", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
		return false;
	}

	std::string Generator = "Visual Studio 17 2022";

	// CMake configure command.
	std::string ConfigureCommand = "cmake -S \"" + VSProjectDirectory + "\" -B \"" + VSProjectDirectory + "\" -G \"" + Generator + "\"";

	// Execute CMake configure command.
	int ConfigureResult = std::system(ConfigureCommand.c_str());
	if (ConfigureResult != 0)
	{
		LOG.Add("FEProjectBuildSystem::ConfigureAndBuildCMake: Error running CMake configure command", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Construct the CMake build command
	std::string BuildCommand = "cmake --build \"" + VSProjectDirectory + "\" --config Release";

	// Execute CMake build command
	int BuildResult = std::system(BuildCommand.c_str());
	if (BuildResult != 0)
	{
		LOG.Add("FEProjectBuildSystem::ConfigureAndBuildCMake: Error running CMake build command", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	return true;
}

std::string NormalizePathToWindows(const std::string& Path)
{
	std::filesystem::path FSPath(Path);
	std::string Normalized = FSPath.make_preferred().string();

	// Replace remaining forward slashes with backslashes
	std::replace(Normalized.begin(), Normalized.end(), '/', '\\');

	// Remove trailing backslash if present
	if (!Normalized.empty() && Normalized.back() == '\\')
		Normalized.pop_back();
	
	return Normalized;
}

bool FEProjectBuildSystem::CreateFinalExecutableDirectory(FEProject* ProjectToBuild)
{
	std::string ProjectFolder = ProjectToBuild->GetProjectFolder();
	if (!FILE_SYSTEM.DoesDirectoryExist(ProjectFolder))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: ProjectFolder does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	std::string ExecutablePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/BuildProjects_Temporary/" + ProjectToBuild->GetID() + "/Release/" + GetVSProjectName(ProjectToBuild) + ".exe";
	if (!FILE_SYSTEM.DoesFileExist(ExecutablePath))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Executable does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	std::string EngineResourcesPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/BuildProjects_Temporary/" + ProjectToBuild->GetID() + "/EngineResources.fepackage";
	if (!FILE_SYSTEM.DoesFileExist(EngineResourcesPath))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: EngineResources.fepackage does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	std::string ProjectResourcesPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/BuildProjects_Temporary/" + ProjectToBuild->GetID() + "/Resources.fepackage";
	if (!FILE_SYSTEM.DoesFileExist(ProjectResourcesPath))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: ProjectResources.fepackage does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	std::string FinalDirectory = ProjectFolder + "Build/";
	if (FILE_SYSTEM.DoesDirectoryExist(FinalDirectory))
	{
		if (!FILE_SYSTEM.DeleteDirectory(FinalDirectory))
		{
			LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error deleting Build directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	if (!FILE_SYSTEM.CreateDirectory(FinalDirectory))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error creating Build directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Copy all files to the final directory.
	if (!FILE_SYSTEM.CopyFile(ExecutablePath, FinalDirectory + GetVSProjectName(ProjectToBuild) + ".exe"))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying executable", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyFile(EngineResourcesPath, FinalDirectory + "EngineResources.fepackage"))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying EngineResources.fepackage", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyFile(ProjectResourcesPath, FinalDirectory + "Resources.fepackage"))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying ProjectResources.fepackage", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Open the directory in file explorer.
	std::string OpenCommand = "explorer \"" + NormalizePathToWindows(FinalDirectory) + "\"";
	std::system(OpenCommand.c_str());

	return true;
}