#include "FEEditorPluginManager.h"
#include "FEEditorAPIImplementation.h"
using namespace FocalEngine;

FEEditorPluginManager::FEEditorPluginManager()
{
}

FEEditorPluginManager::~FEEditorPluginManager()
{
}

void FEEditorPluginManager::Initialize()
{
	// Editor modules use engine modules, so they are unloaded first on any path that shuts plugins down.
	PLUGIN_MANAGER.SetBeforeShutdownCallback([]() {
		EDITOR_PLUGIN_MANAGER.Shutdown();
	});

	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const PluginManifest& Manifest = Plugins[i].Manifest;
		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			const PluginModule& Module = Manifest.Modules[j];
			if (Module.Type != "Editor")
				continue;

			bool bDependenciesLoaded = true;
			for (size_t k = 0; k < Module.Dependencies.size(); k++)
			{
				if (!PLUGIN_MANAGER.IsModuleLoaded(Module.Dependencies[k]))
				{
					LOG.Add("FEEditorPluginManager::Initialize: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because engine module with ID " + UNIQUE_ID.ToString(Module.Dependencies[k]) + " that it depends on is not loaded", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
					bDependenciesLoaded = false;
					break;
				}
			}

			if (!bDependenciesLoaded)
				continue;

			ActiveEditorPlugin NewPlugin;
			if (!PLUGIN_MANAGER.LoadPluginModule(Plugins[i], j, NewPlugin.Plugin))
				continue;

			NewPlugin.Instance = dynamic_cast<FEEditorPluginCore*>(NewPlugin.Plugin.GetInstance());
			if (NewPlugin.Instance == nullptr)
			{
				LOG.Add("FEEditorPluginManager::Initialize: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because it does not inherit from FEEditorPluginCore", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
				PLUGIN_MANAGER.UnloadPluginModule(NewPlugin.Plugin);
				continue;
			}

			const int APIVersion = NewPlugin.Instance->GetEditorAPIVersion();
			if (APIVersion < FE_EDITOR_API_MIN_VERSION || APIVersion > FE_EDITOR_API_VERSION)
			{
				LOG.Add("FEEditorPluginManager::Initialize: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because it was compiled with editor API version " + std::to_string(APIVersion) + ", editor supports versions from " + std::to_string(FE_EDITOR_API_MIN_VERSION) + " to " + std::to_string(FE_EDITOR_API_VERSION), "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
				PLUGIN_MANAGER.UnloadPluginModule(NewPlugin.Plugin);
				continue;
			}

			NewPlugin.API = new FEEditorAPIImplementation(Manifest.Name, Module.Name, Module.ID);
			if (!NewPlugin.Instance->OnEditorLoad(NewPlugin.API))
			{
				LOG.Add("FEEditorPluginManager::Initialize: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because its OnEditorLoad returned false", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
				delete NewPlugin.API;
				PLUGIN_MANAGER.UnloadPluginModule(NewPlugin.Plugin);
				continue;
			}

			ActiveEditorPlugins.push_back(NewPlugin);
			LOG.Add("FEEditorPluginManager::Initialize: Plugin \"" + Manifest.Name + "\" version " + Manifest.Version + ", editor module \"" + Module.Name + "\" loaded from \"" + NewPlugin.Plugin.GetDLLPath() + "\"", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_INFO);
		}
	}
}

void FEEditorPluginManager::Update(double DeltaTime)
{
	for (size_t i = 0; i < ActiveEditorPlugins.size(); i++)
	{
		ActiveEditorPlugins[i].API->Update();
		ActiveEditorPlugins[i].Instance->OnEditorUpdate(DeltaTime);
	}
}

void FEEditorPluginManager::RenderMenu()
{
	if (ActiveEditorPlugins.empty())
		return;

	if (!ImGui::BeginMenu("Plugins"))
		return;

	for (size_t i = 0; i < ActiveEditorPlugins.size(); i++)
		ActiveEditorPlugins[i].API->RenderMenu();

	ImGui::EndMenu();
}

void FEEditorPluginManager::RenderWindows()
{
	for (size_t i = 0; i < ActiveEditorPlugins.size(); i++)
		ActiveEditorPlugins[i].API->RenderWindows();
}

void FEEditorPluginManager::RenderInspectorSections(FEEntity* Entity)
{
	for (size_t i = 0; i < ActiveEditorPlugins.size(); i++)
		ActiveEditorPlugins[i].API->RenderInspectorSections(Entity);
}

bool FEEditorPluginManager::IsModuleLoaded(const PluginModule& Module) const
{
	if (Module.Type == "Engine")
		return PLUGIN_MANAGER.IsModuleLoaded(Module.ID);

	for (size_t i = 0; i < ActiveEditorPlugins.size(); i++)
	{
		if (ActiveEditorPlugins[i].Plugin.GetModule()->ID == Module.ID)
			return true;
	}

	return false;
}

void FEEditorPluginManager::Shutdown()
{
	for (size_t i = ActiveEditorPlugins.size(); i > 0; i--)
	{
		ActiveEditorPlugin& Plugin = ActiveEditorPlugins[i - 1];
		Plugin.Instance->OnEditorUnload();
		// Registrations hold callbacks that point to module code, so they should be deleted before DLL is unloaded.
		delete Plugin.API;
		PLUGIN_MANAGER.UnloadPluginModule(Plugin.Plugin);
	}

	ActiveEditorPlugins.clear();
}

bool FEEditorPluginManager::CreateEditorPluginProject(std::string& PluginName, const std::string& VSProjectDirectory, bool bEditorOnly)
{
	if (bEditorOnly)
	{
		LOG.Add("FEEditorPluginManager::CreateEditorPluginProject: Editor only plugins are not supported yet", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		return false;
	}

	// Editor module uses engine module through public interface, so it is always generated.
	// Engine does not configure and build project, editor module should be added first.
	if (!PLUGIN_MANAGER.CreatePluginProject(PluginName, VSProjectDirectory, true, false))
	{
		LOG.Add("FEEditorPluginManager::CreateEditorPluginProject: Error creating engine module of plugin", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	std::string ProjectDirectory = VSProjectDirectory;
	if (ProjectDirectory.back() != '/' && ProjectDirectory.back() != '\\')
		ProjectDirectory += '/';

	if (!GenerateEditorModuleSourceFilesFromTemplate(PluginName, ProjectDirectory))
	{
		LOG.Add("FEEditorPluginManager::CreateEditorPluginProject: Error generating editor module source files from template", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	if (!AddEditorModuleToManifest(PluginName, ProjectDirectory))
	{
		LOG.Add("FEEditorPluginManager::CreateEditorPluginProject: Error adding editor module to manifest", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	if (!AddEditorModuleToCMake(ProjectDirectory))
	{
		LOG.Add("FEEditorPluginManager::CreateEditorPluginProject: Error adding editor module to CMake project", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	if (!PLUGIN_MANAGER.ConfigureAndBuildCMake(ProjectDirectory))
		return false;

	return PLUGIN_MANAGER.RunVSProject(PluginName, ProjectDirectory);
}

bool FEEditorPluginManager::GenerateEditorModuleSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory)
{
	std::string EditorDirectory = VSProjectDirectory + "Editor/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EditorDirectory))
	{
		if (!FILE_SYSTEM.MakeDirectory(EditorDirectory))
		{
			LOG.Add("FEEditorPluginManager::GenerateEditorModuleSourceFilesFromTemplate: Error creating " + EditorDirectory + " directory", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	const std::string ModuleName = PluginName + "Editor";
	std::string TemplatesPath = FILE_SYSTEM.GetCurrentWorkingPath() + "/SubSystems/PluginSystem/Templates/";
	std::vector<std::pair<std::string, std::string>> FilesToCopy;
	FilesToCopy.push_back({ TemplatesPath + "PluginEditorModuleTemplate.h", EditorDirectory + ModuleName + ".h" });
	FilesToCopy.push_back({ TemplatesPath + "PluginEditorModuleTemplate.cpp", EditorDirectory + ModuleName + ".cpp" });

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "EDITOR_MODULE_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "EDITOR_MODULE_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = ModuleName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "INTERFACE_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "INTERFACE_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName + "Interface";
	Instructions.push_back(CurrentInstruction);

	for (size_t i = 0; i < FilesToCopy.size(); i++)
	{
		if (!FILE_SYSTEM.CopyFileTo(FilesToCopy[i].first, FilesToCopy[i].second))
		{
			LOG.Add("FEEditorPluginManager::GenerateEditorModuleSourceFilesFromTemplate: Error copying file " + FilesToCopy[i].first + " to " + FilesToCopy[i].second, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}

		if (!FILE_SYSTEM.PerformTextReplacements(FilesToCopy[i].second, Instructions))
		{
			LOG.Add("FEEditorPluginManager::GenerateEditorModuleSourceFilesFromTemplate: Error initializing " + FilesToCopy[i].second, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	// Engine submodule is in SubSystems/FocalEngine, so relative include of FEPluginCore.h in FEEditorAPI.h works there the same way as in editor.
	std::string EditorAPIDirectory = VSProjectDirectory + "SubSystems/EditorAPI/";
	if (!FILE_SYSTEM.DoesDirectoryExist(EditorAPIDirectory))
	{
		if (!FILE_SYSTEM.MakeDirectory(EditorAPIDirectory))
		{
			LOG.Add("FEEditorPluginManager::GenerateEditorModuleSourceFilesFromTemplate: Error creating " + EditorAPIDirectory + " directory", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	const std::string EditorAPIFilePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/SubSystems/PluginSystem/FEEditorAPI.h";
	if (!FILE_SYSTEM.CopyFileTo(EditorAPIFilePath, EditorAPIDirectory + "FEEditorAPI.h"))
	{
		LOG.Add("FEEditorPluginManager::GenerateEditorModuleSourceFilesFromTemplate: Error copying file " + EditorAPIFilePath + " to " + EditorAPIDirectory + "FEEditorAPI.h", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	return true;
}

bool FEEditorPluginManager::AddEditorModuleToManifest(const std::string& PluginName, const std::string& VSProjectDirectory)
{
	const std::string ManifestPath = VSProjectDirectory + PluginName + ".json";
	std::ifstream InputFile(ManifestPath);
	if (!InputFile.is_open())
	{
		LOG.Add("FEEditorPluginManager::AddEditorModuleToManifest: Error opening " + ManifestPath, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	Json::Value Root;
	JSONCPP_STRING Error;
	Json::CharReaderBuilder ReaderBuilder;
	const bool bParsed = Json::parseFromStream(ReaderBuilder, InputFile, &Root, &Error);
	InputFile.close();
	if (!bParsed)
	{
		LOG.Add("FEEditorPluginManager::AddEditorModuleToManifest: Error parsing " + ManifestPath + ": " + Error, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	// At this point manifest contains only engine module.
	Json::Value& Modules = Root["Modules"];
	if (!Modules.isArray() || Modules.size() != 1)
	{
		LOG.Add("FEEditorPluginManager::AddEditorModuleToManifest: Expected only engine module in " + ManifestPath, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	Json::Value EditorModule;
	EditorModule["ID"] = UNIQUE_ID.ToString(UNIQUE_ID.GenerateID());
	EditorModule["Type"] = "Editor";
	EditorModule["Name"] = PluginName + "Editor";
	EditorModule["EntryPointFileName"] = PluginName + "Editor.dll";
	EditorModule["SupportedOS"] = Modules[0]["SupportedOS"];
	EditorModule["Dependencies"].append(Modules[0]["ID"]);
	Modules.append(EditorModule);

	std::ofstream OutputFile(ManifestPath, std::ios::trunc);
	if (!OutputFile.is_open())
	{
		LOG.Add("FEEditorPluginManager::AddEditorModuleToManifest: Error writing " + ManifestPath, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	const Json::StreamWriterBuilder WriterBuilder;
	OutputFile << Json::writeString(WriterBuilder, Root);
	OutputFile.close();

	return PLUGIN_MANAGER.IsValidManifest(ManifestPath);
}

bool FEEditorPluginManager::AddEditorModuleToCMake(const std::string& VSProjectDirectory)
{
	// Plugin CMakeLists.txt includes AdditionalModules.cmake if it exists.
	std::string TemplatePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/SubSystems/PluginSystem/Templates/AdditionalModules_Template.cmake";
	if (!FILE_SYSTEM.CopyFileTo(TemplatePath, VSProjectDirectory + "AdditionalModules.cmake"))
	{
		LOG.Add("FEEditorPluginManager::AddEditorModuleToCMake: Error copying file " + TemplatePath + " to " + VSProjectDirectory + "AdditionalModules.cmake", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	return true;
}