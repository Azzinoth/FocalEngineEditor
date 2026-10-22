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
		if (!FILE_SYSTEM.MakeDirectory(OutputPath + FoldersToCreate[i]))
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
		if (!FILE_SYSTEM.CopyFileTo(FilesToCopy[i].first, FilesToCopy[i].second))
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

	if (!FILE_SYSTEM.CopyDirectory(VisualNodeSystemPath + "StandardNodes/",
								   OutputPath + "VisualNodeSystem/StandardNodes/"))
	{
		LOG.Add("FEProjectBuildSystem::CopyVisualNodeSystemSubProjectFiles: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyDirectory(VisualNodeSystemPath + "VersionInfo/",
								   OutputPath + "VisualNodeSystem/VersionInfo/"))
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

	if (!FILE_SYSTEM.MakeDirectory(VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error creating BuildProjects_Temporary directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	VSProjectDirectory += UNIQUE_ID.ToString(ProjectToBuild->GetID()) + "/";
	if (!FILE_SYSTEM.MakeDirectory(VSProjectDirectory))
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
												 "SubSystems/FocalEngine/Core",
												 "SubSystems/FocalEngine/Core/VersionInfo",
												 "SubSystems/FocalEngine/SubSystems",
												 "SubSystems/FocalEngine/ThirdParty",
												 "SubSystems/FocalEngine/ThirdParty/openxr",
												 "SubSystems/FocalEngine/ThirdParty/laszip",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication/VersionInfo",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/GLFW",
												 "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/glew2" };

	for (size_t i = 0; i < FoldersToCreate.size(); i++)
	{
		if (!FILE_SYSTEM.MakeDirectory(VSProjectDirectory + FoldersToCreate[i]))
		{
			LOG.Add("FEProjectBuildSystem::BuildExecutable: Error creating " + FoldersToCreate[i] + " directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
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
	FilesToCopy.push_back({ EnginePath + "ResourceManager/Config.h.in", VSProjectDirectory + "SubSystems/FocalEngine/ResourceManager/Config.h.in" });
	FilesToCopy.push_back({ EnginePath + "Core/VersionInfo/UpdateProjectVersion.cmake", VSProjectDirectory + "SubSystems/FocalEngine/Core/VersionInfo/UpdateProjectVersion.cmake" });
	FilesToCopy.push_back({ EnginePath + "Core/VersionInfo/ProjectVersion.h.in", VSProjectDirectory + "SubSystems/FocalEngine/Core/VersionInfo/ProjectVersion.h.in" });
	FilesToCopy.push_back({ EnginePath + "SubSystems/FEBasicApplication/CMakeLists.txt", VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/CMakeLists.txt" });
	FilesToCopy.push_back({ EnginePath + "SubSystems/FEBasicApplication/VersionInfo/UpdateProjectVersion.cmake", VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/VersionInfo/UpdateProjectVersion.cmake" });
	FilesToCopy.push_back({ EnginePath + "SubSystems/FEBasicApplication/VersionInfo/ProjectVersion.h.in", VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/VersionInfo/ProjectVersion.h.in" });

	for (size_t i = 0; i < FilesToCopy.size(); i++)
	{
		if (!FILE_SYSTEM.CopyFileTo(FilesToCopy[i].first, FilesToCopy[i].second))
		{
			LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying file " + FilesToCopy[i].first + " to " + FilesToCopy[i].second, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}
	}

	if (!FILE_SYSTEM.CopyDirectory(EnginePath + "/SubSystems/FEBasicApplication/ThirdParty/GLFW/lib",
								   VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/GLFW/lib"))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyDirectory(EnginePath + "/SubSystems/FEBasicApplication/ThirdParty/glew2/lib",
								   VSProjectDirectory + "SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/glew2/lib"))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyDirectory(EnginePath + "/ThirdParty/openxr/Lib",
								   VSProjectDirectory + "SubSystems/FocalEngine/ThirdParty/openxr/Lib"))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyDirectory(EnginePath + "/ThirdParty/laszip/lib",
								   VSProjectDirectory + "SubSystems/FocalEngine/ThirdParty/laszip/lib"))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error copying directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
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

	StaticPlugins = FindStaticPlugins();

	if (!InitializeCMakeFileAndScriptFiles(ProjectToBuild, VSProjectDirectory))
	{
		LOG.Add("FEProjectBuildSystem::BuildExecutable: Error initializing CMakeLists.txt", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
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

	if (!CreateFinalExecutableDirectory(ProjectToBuild))
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

	std::vector<FEUUID> NativeScriptModuleIDList = RESOURCE_MANAGER.GetNativeScriptModuleIDList();

	std::string CMakeFilePath = VSProjectDirectory + "CMakeLists.txt";

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "set(PROJECT_NAME PLACE_HOLDER)";
	CurrentInstruction.TargetText = "PLACE_HOLDER";
	CurrentInstruction.ReplacementText = GetVSProjectName(ProjectToBuild);
	Instructions.push_back(CurrentInstruction);

	std::string SourceFileListString = "";
	std::string ExecutableListString = "";
	std::string FolderListInVSProjectString = "";

	for (size_t i = 0; i < NativeScriptModuleIDList.size(); i++)
	{
		FENativeScriptModule* CurrentModule = RESOURCE_MANAGER.GetNativeScriptModule(NativeScriptModuleIDList[i]);
		if (CurrentModule == nullptr)
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error getting native script module with ID: " + UNIQUE_ID.ToString(NativeScriptModuleIDList[i]), "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}
			
		if (CurrentModule->GetProject() == nullptr)
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error getting project of native script module with ID: " + UNIQUE_ID.ToString(NativeScriptModuleIDList[i]), "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}

		if (!CurrentModule->GetProject()->HasRecoverableVSProjectData())
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Project of native script module with ID: " + UNIQUE_ID.ToString(NativeScriptModuleIDList[i]) + " does not have recoverable VS project data", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}
		
		std::vector<std::string> SourceFileList = CurrentModule->GetProject()->GetSourceFileList();
		if (SourceFileList.empty())
		{
			LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: SourceFileList is empty for native script module with ID: " + UNIQUE_ID.ToString(NativeScriptModuleIDList[i]), "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}

		std::string CurrentString = "file(GLOB Script_Module_" + UNIQUE_ID.ToString(CurrentModule->GetID()) + "_SRC\n";
		for (size_t i = 0; i < SourceFileList.size(); i++)
		{
			CurrentString += std::string("\t\"") + "ScriptModules/" + UNIQUE_ID.ToString(CurrentModule->GetID()) + "/" + SourceFileList[i] + "\"\n";
		}
		
		CurrentString += ")\n";
		CurrentString += "\n";

		SourceFileListString += CurrentString;

		// Replace the placeholder in add_executable list.
		CurrentString = std::string("\t\t") + "${Script_Module_" + UNIQUE_ID.ToString(CurrentModule->GetID()) + "_SRC}\n";
		ExecutableListString += CurrentString;

		// Add the folder to the list of folders in VS project.
		FolderListInVSProjectString += "source_group(\"Source Files/ScriptModules/" + UNIQUE_ID.ToString(CurrentModule->GetID()) + "/\" FILES ${Script_Module_" + UNIQUE_ID.ToString(CurrentModule->GetID()) + "_SRC})\n";

		// Now we will create the directories and copy the files to the destination directory.
		std::string ScriptModuleDirectory = VSProjectDirectory + "ScriptModules/" + UNIQUE_ID.ToString(CurrentModule->GetID()) + "/";
		if (!FILE_SYSTEM.MakeDirectory(ScriptModuleDirectory))
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

	CurrentInstruction.ContextPattern = "#(PLACE_HOLDER) Here should be list of all script modules sources.";
	CurrentInstruction.TargetText = "#(PLACE_HOLDER) Here should be list of all script modules sources.";
	CurrentInstruction.ReplacementText = SourceFileListString;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "		#(PLACE_HOLDER) Here should be list of all script modules.";
	CurrentInstruction.TargetText = "		#(PLACE_HOLDER) Here should be list of all script modules.";
	CurrentInstruction.ReplacementText = ExecutableListString;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "#(PLACE_HOLDER) Here should be list of all script modules and where to place them in VS project.";
	CurrentInstruction.TargetText = "#(PLACE_HOLDER) Here should be list of all script modules and where to place them in VS project.";
	CurrentInstruction.ReplacementText = FolderListInVSProjectString;
	Instructions.push_back(CurrentInstruction);

	std::string StaticPluginListString = "";
	for (size_t i = 0; i < StaticPlugins.size(); i++)
	{
		// Backslash is escape character in CMake.
		std::string PluginFolderPath = FILE_SYSTEM.GetAbsolutePath(StaticPlugins[i].FolderPath);
		std::replace(PluginFolderPath.begin(), PluginFolderPath.end(), '\\', '/');

		const std::string& PluginName = StaticPlugins[i].Manifest.Name;
		StaticPluginListString += "include(\"" + PluginFolderPath + "/Static/" + PluginName + ".cmake\")\n";
		StaticPluginListString += "target_link_libraries(${PROJECT_NAME} PRIVATE " + PluginName + "::Static)\n";

		// Native scripts could use plugin API.
		if (FILE_SYSTEM.DoesDirectoryExist(PluginFolderPath + "/Public"))
			StaticPluginListString += "target_include_directories(${PROJECT_NAME} PRIVATE \"" + PluginFolderPath + "/Public\")\n";
	}

	CurrentInstruction.ContextPattern = "#(PLACE_HOLDER) Here should be list of static plugins.";
	CurrentInstruction.TargetText = "#(PLACE_HOLDER) Here should be list of static plugins.";
	CurrentInstruction.ReplacementText = StaticPluginListString;
	Instructions.push_back(CurrentInstruction);

	if (!FILE_SYSTEM.PerformTextReplacements(CMakeFilePath, Instructions))
	{
		LOG.Add("FEProjectBuildSystem::InitializeCMakeFileAndScriptFiles: Error initializing CMakeLists.txt", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}
	Instructions.clear();

	return true;
}

static void AddFileLinesToLog(const std::string& FilePath, const std::string& StartPattern)
{
	std::vector<std::string> Lines;
	std::ifstream File(FilePath);
	std::string Line;
	while (std::getline(File, Line))
	{
		const size_t FirstCharacter = Line.find_first_not_of(" \t");
		if (FirstCharacter != std::string::npos)
			Lines.push_back(Line.substr(FirstCharacter));
	}

	size_t StartIndex = 0;
	for (size_t i = 0; i < Lines.size(); i++)
	{
		if (Lines[i].find(StartPattern) != std::string::npos)
		{
			StartIndex = i;
			break;
		}
	}

	for (size_t i = StartIndex; i < Lines.size(); i++)
		LOG.Add(Lines[i], "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
}

bool FEProjectBuildSystem::ConfigureAndBuildCMake(const std::string& VSProjectDirectory)
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FEProjectBuildSystem::ConfigureAndBuildCMake: VSProjectDirectory is empty", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
		return false;
	}

	std::string Generator = "Visual Studio 18 2026";

	// Output of CMake goes to log files, so errors could be shown in editor log.
	const std::string ConfigureLogPath = VSProjectDirectory + "ConfigureLog.txt";
	std::string ConfigureCommand = "cmake -S \"" + VSProjectDirectory + "\" -B \"" + VSProjectDirectory + "\" -G \"" + Generator + "\" > \"" + ConfigureLogPath + "\" 2>&1";

	// Execute CMake configure command.
	int ConfigureResult = std::system(ConfigureCommand.c_str());
	if (ConfigureResult != 0)
	{
		LOG.Add("FEProjectBuildSystem::ConfigureAndBuildCMake: Error running CMake configure command, full output is in " + ConfigureLogPath, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		AddFileLinesToLog(ConfigureLogPath, "CMake Error");
		return false;
	}

	// MSBuild also writes only errors to a separate file.
	const std::string BuildLogPath = VSProjectDirectory + "BuildLog.txt";
	const std::string ErrorLogPath = VSProjectDirectory + "BuildErrors.txt";
	std::string BuildCommand = "cmake --build \"" + VSProjectDirectory + "\" --config Release -- /fl \"/flp:ErrorsOnly;LogFile=" + ErrorLogPath + "\" > \"" + BuildLogPath + "\" 2>&1";

	// Execute CMake build command
	int BuildResult = std::system(BuildCommand.c_str());
	if (BuildResult != 0)
	{
		LOG.Add("FEProjectBuildSystem::ConfigureAndBuildCMake: Error running CMake build command, full output is in " + BuildLogPath, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		AddFileLinesToLog(ErrorLogPath, "");
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

	std::string ExecutablePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/BuildProjects_Temporary/" + UNIQUE_ID.ToString(ProjectToBuild->GetID()) + "/Release/" + GetVSProjectName(ProjectToBuild) + ".exe";
	if (!FILE_SYSTEM.DoesFileExist(ExecutablePath))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Executable does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	std::string EngineResourcesPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/BuildProjects_Temporary/" + UNIQUE_ID.ToString(ProjectToBuild->GetID()) + "/EngineResources.fepackage";
	if (!FILE_SYSTEM.DoesFileExist(EngineResourcesPath))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: EngineResources.fepackage does not exist!", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	std::string ProjectResourcesPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/BuildProjects_Temporary/" + UNIQUE_ID.ToString(ProjectToBuild->GetID()) + "/Resources.fepackage";
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

	if (!FILE_SYSTEM.MakeDirectory(FinalDirectory))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error creating Build directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	// Copy all files to the final directory.
	if (!FILE_SYSTEM.CopyFileTo(ExecutablePath, FinalDirectory + GetVSProjectName(ProjectToBuild) + ".exe"))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying executable", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyFileTo(EngineResourcesPath, FinalDirectory + "EngineResources.fepackage"))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying EngineResources.fepackage", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!FILE_SYSTEM.CopyFileTo(ProjectResourcesPath, FinalDirectory + "Resources.fepackage"))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying ProjectResources.fepackage", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	if (!StaticPlugins.empty() && !FILE_SYSTEM.MakeDirectory(FinalDirectory + "Plugins/"))
	{
		LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error creating Plugins directory", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
		return false;
	}

	for (size_t i = 0; i < StaticPlugins.size(); i++)
	{
		const DiscoveredPlugin& Plugin = StaticPlugins[i];
		const std::string PluginFinalDirectory = FinalDirectory + "Plugins/" + Plugin.Manifest.Name + "/";
		if (!FILE_SYSTEM.MakeDirectory(PluginFinalDirectory))
		{
			LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error creating directory " + PluginFinalDirectory, "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
			return false;
		}

		// Plugin manager of the game finds plugins that are linked into executable by their manifests.
		std::vector<std::string> PluginFiles = FILE_SYSTEM.GetFileList(Plugin.FolderPath);
		for (size_t j = 0; j < PluginFiles.size(); j++)
		{
			const std::string FilePath = Plugin.FolderPath + "/" + PluginFiles[j];
			if (FILE_SYSTEM.GetFileExtension(PluginFiles[j]) != ".json" || !PLUGIN_MANAGER.IsValidManifest(FilePath))
				continue;

			if (!FILE_SYSTEM.CopyFileTo(FilePath, PluginFinalDirectory + PluginFiles[j]))
			{
				LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying manifest of plugin \"" + Plugin.Manifest.Name + "\"", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
				return false;
			}
		}

		// Executable itself imports DLLs that plugins depend on, so they should be next to it.
		const std::string DependencyFolderPath = Plugin.FolderPath + "/Release/";
		std::vector<std::string> DependencyFiles = FILE_SYSTEM.GetFileList(DependencyFolderPath);
		for (size_t j = 0; j < DependencyFiles.size(); j++)
		{
			if (FILE_SYSTEM.DoesFileExist(FinalDirectory + DependencyFiles[j]))
			{
				LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: " + DependencyFiles[j] + " of plugin \"" + Plugin.Manifest.Name + "\" is not copied, because file with the same name is already in the build directory", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
				continue;
			}

			if (!FILE_SYSTEM.CopyFileTo(DependencyFolderPath + DependencyFiles[j], FinalDirectory + DependencyFiles[j]))
			{
				LOG.Add("FEProjectBuildSystem::CreateFinalExecutableDirectory: Error copying " + DependencyFiles[j] + " of plugin \"" + Plugin.Manifest.Name + "\"", "FE_BUILD_EXECUTABLE", FE_LOG_ERROR);
				return false;
			}
		}
	}

	// Open the directory in file explorer.
	std::string OpenCommand = "explorer \"" + NormalizePathToWindows(FinalDirectory) + "\"";
	std::system(OpenCommand.c_str());

	return true;
}

std::vector<DiscoveredPlugin> FEProjectBuildSystem::FindStaticPlugins()
{
	std::vector<DiscoveredPlugin> Result;
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const PluginManifest& Manifest = Plugins[i].Manifest;
		const std::string StaticFolderPath = Plugins[i].FolderPath + "/Static/";

		bool bHasEngineModule = false;
		bool bAllFilesFound = FILE_SYSTEM.DoesFileExist(StaticFolderPath + Manifest.Name + ".cmake");
		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			// Only engine modules are part of the game.
			if (Manifest.Modules[j].Type != "Engine")
				continue;

			bHasEngineModule = true;
			if (!FILE_SYSTEM.DoesFileExist(StaticFolderPath + Manifest.Modules[j].Name + "_Release.lib"))
				bAllFilesFound = false;
		}

		if (!bHasEngineModule)
			continue;

		if (!bAllFilesFound)
		{
			LOG.Add("FEProjectBuildSystem::FindStaticPlugins: Plugin \"" + Manifest.Name + "\" is not included in the game, because its Static folder does not have link file or static library of every engine module", "FE_BUILD_EXECUTABLE", FE_LOG_WARNING);
			continue;
		}

		Result.push_back(Plugins[i]);
	}

	return Result;
}