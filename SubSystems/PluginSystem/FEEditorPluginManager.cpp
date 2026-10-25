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

	struct PendingModule
	{
		const DiscoveredPlugin* Plugin = nullptr;
		size_t ModuleIndex = 0;
	};

	std::vector<PendingModule> PendingModules;
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		for (size_t j = 0; j < Plugins[i].Manifest.Modules.size(); j++)
		{
			if (Plugins[i].Manifest.Modules[j].Type == "Editor")
				PendingModules.push_back({ &Plugins[i], j });
		}
	}

	// Editor modules could depend on editor modules of other plugins, so every pass loads modules whose dependencies are already loaded.
	bool bAnyModuleProcessed = true;
	while (bAnyModuleProcessed && !PendingModules.empty())
	{
		bAnyModuleProcessed = false;
		for (size_t i = 0; i < PendingModules.size();)
		{
			const PluginModule& Module = PendingModules[i].Plugin->Manifest.Modules[PendingModules[i].ModuleIndex];
			bool bDependenciesLoaded = true;
			for (size_t j = 0; j < Module.Dependencies.size(); j++)
			{
				if (!IsModuleLoaded(Module.Dependencies[j]))
				{
					bDependenciesLoaded = false;
					break;
				}
			}

			if (!bDependenciesLoaded)
			{
				i++;
				continue;
			}

			LoadEditorModule(*PendingModules[i].Plugin, PendingModules[i].ModuleIndex);
			PendingModules.erase(PendingModules.begin() + i);
			bAnyModuleProcessed = true;
		}
	}

	for (size_t i = 0; i < PendingModules.size(); i++)
	{
		const PluginManifest& Manifest = PendingModules[i].Plugin->Manifest;
		const PluginModule& Module = Manifest.Modules[PendingModules[i].ModuleIndex];
		for (size_t j = 0; j < Module.Dependencies.size(); j++)
		{
			if (IsModuleLoaded(Module.Dependencies[j]))
				continue;

			LOG.Add("FEEditorPluginManager::Initialize: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because module \"" + GetModuleDisplayName(Module.Dependencies[j]) + "\" that it depends on is not loaded", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
			break;
		}
	}
}

bool FEEditorPluginManager::LoadEditorModule(const DiscoveredPlugin& Plugin, size_t ModuleIndex)
{
	const PluginManifest& Manifest = Plugin.Manifest;
	const PluginModule& Module = Manifest.Modules[ModuleIndex];

	ActiveEditorPlugin NewPlugin;
	if (!PLUGIN_MANAGER.LoadPluginModule(Plugin, ModuleIndex, NewPlugin.Plugin))
		return false;

	NewPlugin.Instance = dynamic_cast<FEEditorPluginCore*>(NewPlugin.Plugin.GetInstance());
	if (NewPlugin.Instance == nullptr)
	{
		LOG.Add("FEEditorPluginManager::LoadEditorModule: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because it does not inherit from FEEditorPluginCore", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		PLUGIN_MANAGER.UnloadPluginModule(NewPlugin.Plugin);
		return false;
	}

	const int APIVersion = NewPlugin.Instance->GetEditorAPIVersion();
	if (APIVersion < FE_EDITOR_API_MIN_VERSION || APIVersion > FE_EDITOR_API_VERSION)
	{
		LOG.Add("FEEditorPluginManager::LoadEditorModule: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because it was compiled with editor API version " + std::to_string(APIVersion) + ", editor supports versions from " + std::to_string(FE_EDITOR_API_MIN_VERSION) + " to " + std::to_string(FE_EDITOR_API_VERSION), "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		PLUGIN_MANAGER.UnloadPluginModule(NewPlugin.Plugin);
		return false;
	}

	NewPlugin.API = new FEEditorAPIImplementation(Manifest.Name, Module.Name, Module.ID);
	if (!NewPlugin.Instance->OnEditorLoad(NewPlugin.API))
	{
		LOG.Add("FEEditorPluginManager::LoadEditorModule: Editor module \"" + Module.Name + "\" of plugin \"" + Manifest.Name + "\" is not loaded, because its OnEditorLoad returned false", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		delete NewPlugin.API;
		PLUGIN_MANAGER.UnloadPluginModule(NewPlugin.Plugin);
		return false;
	}

	ActiveEditorPlugins.push_back(NewPlugin);
	LOG.Add("FEEditorPluginManager::LoadEditorModule: Plugin \"" + Manifest.Name + "\" version " + Manifest.Version + ", editor module \"" + Module.Name + "\" loaded from \"" + NewPlugin.Plugin.GetDLLPath() + "\"", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_INFO);
	return true;
}

std::string FEEditorPluginManager::GetModuleDisplayName(const FEUUID& ModuleID) const
{
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		const PluginManifest& Manifest = Plugins[i].Manifest;
		for (size_t j = 0; j < Manifest.Modules.size(); j++)
		{
			if (Manifest.Modules[j].ID == ModuleID)
				return Manifest.Name + "/" + Manifest.Modules[j].Name;
		}
	}

	return UNIQUE_ID.ToString(ModuleID);
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

bool FEEditorPluginManager::IsModuleLoaded(const FEUUID& ModuleID) const
{
	if (PLUGIN_MANAGER.IsModuleLoaded(ModuleID))
		return true;

	for (size_t i = 0; i < ActiveEditorPlugins.size(); i++)
	{
		if (ActiveEditorPlugins[i].Plugin.GetModule()->ID == ModuleID)
			return true;
	}

	return false;
}

FEEditorPluginCore* FEEditorPluginManager::GetEditorModule(const FEUUID& PluginID) const
{
	for (size_t i = 0; i < ActiveEditorPlugins.size(); i++)
	{
		if (ActiveEditorPlugins[i].Plugin.GetManifest()->ID == PluginID)
			return ActiveEditorPlugins[i].Instance;
	}

	return nullptr;
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

bool FEEditorPluginManager::CreatePluginProject(std::string& PluginName, const std::string& VSProjectDirectory, const FEEditorPluginProjectSettings& Settings)
{
	if (VSProjectDirectory.empty())
	{
		LOG.Add("FEEditorPluginManager::CreatePluginProject: VSProjectDirectory is empty", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_WARNING);
		return false;
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(VSProjectDirectory) && !FILE_SYSTEM.MakeDirectory(VSProjectDirectory))
	{
		LOG.Add("FEEditorPluginManager::CreatePluginProject: Error creating " + VSProjectDirectory + " directory", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	FEPluginProjectSettings EngineSettings;
	EngineSettings.bAddMainModule = Settings.Type != FEPluginProjectType::Editor;
	// Editor module uses engine module through its public interface.
	EngineSettings.bAddPublicInterface = Settings.Type == FEPluginProjectType::EngineAndEditor || Settings.bAddPublicInterface;
	// Project is configured and built only after editor finished changing it.
	EngineSettings.bConfigureAndBuild = false;
	EngineSettings.Description = Settings.Description;
	if (Settings.bEngineModuleWorksWithoutGraphics)
		EngineSettings.SupportedGraphicsModes = { FEGraphicsMode::OnScreen, FEGraphicsMode::Offscreen, FEGraphicsMode::None };

	if (!PLUGIN_MANAGER.CreatePluginProject(PluginName, VSProjectDirectory, EngineSettings))
	{
		LOG.Add("FEEditorPluginManager::CreatePluginProject: Error creating plugin project", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	std::string ProjectDirectory = VSProjectDirectory;
	if (ProjectDirectory.back() != '/' && ProjectDirectory.back() != '\\')
		ProjectDirectory += '/';

	if (Settings.Type != FEPluginProjectType::Engine)
	{
		if (!GenerateEditorModuleSourceFilesFromTemplate(PluginName, ProjectDirectory, Settings))
		{
			LOG.Add("FEEditorPluginManager::CreatePluginProject: Error generating editor module source files from template", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}

		if (!AddEditorModuleToManifest(PluginName, ProjectDirectory))
		{
			LOG.Add("FEEditorPluginManager::CreatePluginProject: Error adding editor module to manifest", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}

		// Plugin ID is read from manifest, so it is generated only after manifest is complete.
		if (Settings.bAddEditorPublicInterface && !GenerateEditorPublicInterfaceFromTemplate(PluginName, ProjectDirectory))
		{
			LOG.Add("FEEditorPluginManager::CreatePluginProject: Error generating editor public interface from template", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}

		if (!AddEditorModuleToCMake(ProjectDirectory))
		{
			LOG.Add("FEEditorPluginManager::CreatePluginProject: Error adding editor module to CMake project", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	if (!PLUGIN_MANAGER.ConfigureAndBuildCMake(ProjectDirectory))
		return false;

	return PLUGIN_MANAGER.RunVSProject(PluginName, ProjectDirectory);
}

bool FEEditorPluginManager::GenerateEditorModuleSourceFilesFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory, const FEEditorPluginProjectSettings& Settings)
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

	// Template is editor module without engine module, other parts are added only when they are needed.
	const bool bHasEngineModule = Settings.Type == FEPluginProjectType::EngineAndEditor;
	const std::string EngineInterfaceName = PluginName + "Interface";
	const std::string EditorInterfaceName = PluginName + "EditorInterface";

	std::string PublicIncludes;
	if (bHasEngineModule)
		PublicIncludes += "\n#include \"../Public/" + EngineInterfaceName + ".h\"";

	if (Settings.bAddEditorPublicInterface)
		PublicIncludes += "\n#include \"../Public/Editor/" + EditorInterfaceName + ".h\"";

	if (!PublicIncludes.empty())
	{
		const std::string EditorAPIInclude = "#include \"../SubSystems/EditorAPI/FEEditorAPI.h\"";
		CurrentInstruction.ContextPattern = EditorAPIInclude;
		CurrentInstruction.TargetText = EditorAPIInclude;
		CurrentInstruction.ReplacementText = EditorAPIInclude + PublicIncludes;
		Instructions.push_back(CurrentInstruction);
	}

	// Rules are applied one after another, so class name is already replaced at this point.
	if (Settings.bAddEditorPublicInterface)
	{
		CurrentInstruction.ContextPattern = " : public FEEditorPluginCore";
		CurrentInstruction.TargetText = " : public FEEditorPluginCore";
		CurrentInstruction.ReplacementText = " : public FEEditorPluginCore, public " + EditorInterfaceName;
		Instructions.push_back(CurrentInstruction);
	}

	if (bHasEngineModule)
	{
		const std::string EditorMember = "FEEditorAPI* Editor = nullptr;";
		CurrentInstruction.ContextPattern = EditorMember;
		CurrentInstruction.TargetText = EditorMember;
		CurrentInstruction.ReplacementText = EditorMember + "\n\n"
			"\tusing EngineModuleInterface = " + EngineInterfaceName + ";\n"
			"\t// Editor module is loaded after engine module and unloaded before it, so engine module is always available.\n"
			"\tEngineModuleInterface* GetEngineModule() { return PLUGIN_MANAGER.GetPlugin<EngineModuleInterface>(UNIQUE_ID.FromString(EngineModuleInterface::PluginID)); }";
		Instructions.push_back(CurrentInstruction);
	}

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

	// At this point manifest contains only engine module, or no modules if plugin has no engine module.
	Json::Value& Modules = Root["Modules"];
	if (!Modules.isArray() || Modules.size() > 1)
	{
		LOG.Add("FEEditorPluginManager::AddEditorModuleToManifest: Expected at most engine module in " + ManifestPath, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	Json::Value EditorModule;
	EditorModule["ID"] = UNIQUE_ID.ToString(UNIQUE_ID.GenerateID());
	EditorModule["Type"] = "Editor";
	EditorModule["Name"] = PluginName + "Editor";
	EditorModule["EntryPointFileName"] = PluginName + "Editor.dll";
	EditorModule["SupportedOS"] = Modules.empty() ? Root["SupportedOS"] : Modules[0]["SupportedOS"];
	EditorModule["Dependencies"] = Json::Value(Json::arrayValue);
	if (!Modules.empty())
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

bool FEEditorPluginManager::GenerateEditorPublicInterfaceFromTemplate(const std::string& PluginName, const std::string& VSProjectDirectory)
{
	const std::string ManifestPath = VSProjectDirectory + PluginName + ".json";
	PluginManifest Manifest;
	if (!PLUGIN_MANAGER.ParseManifest(ManifestPath, Manifest))
	{
		LOG.Add("FEEditorPluginManager::GenerateEditorPublicInterfaceFromTemplate: Error parsing " + ManifestPath, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	// Native script projects get Public folder of every plugin on include path, editor interface is kept in subfolder, because scripts can not use it.
	const std::vector<std::string> DirectoriesToCreate = { VSProjectDirectory + "Public/", VSProjectDirectory + "Public/Editor/" };
	for (size_t i = 0; i < DirectoriesToCreate.size(); i++)
	{
		if (!FILE_SYSTEM.DoesDirectoryExist(DirectoriesToCreate[i]) && !FILE_SYSTEM.MakeDirectory(DirectoriesToCreate[i]))
		{
			LOG.Add("FEEditorPluginManager::GenerateEditorPublicInterfaceFromTemplate: Error creating " + DirectoriesToCreate[i] + " directory", "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
			return false;
		}
	}

	const std::string InterfaceName = PluginName + "EditorInterface";
	const std::string TemplatePath = FILE_SYSTEM.GetCurrentWorkingPath() + "/SubSystems/PluginSystem/Templates/PluginEditorInterfaceTemplate.h";
	const std::string InterfaceFilePath = DirectoriesToCreate.back() + InterfaceName + ".h";
	if (!FILE_SYSTEM.CopyFileTo(TemplatePath, InterfaceFilePath))
	{
		LOG.Add("FEEditorPluginManager::GenerateEditorPublicInterfaceFromTemplate: Error copying file " + TemplatePath + " to " + InterfaceFilePath, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	std::vector<FEFileSystem::TextReplacementRule> Instructions;
	FEFileSystem::TextReplacementRule CurrentInstruction;
	CurrentInstruction.ContextPattern = "INTERFACE_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "INTERFACE_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = InterfaceName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "PLUGIN_NAME_PLACE_HOLDER";
	CurrentInstruction.TargetText = "PLUGIN_NAME_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = PluginName;
	Instructions.push_back(CurrentInstruction);

	CurrentInstruction.ContextPattern = "PLUGIN_ID_PLACE_HOLDER";
	CurrentInstruction.TargetText = "PLUGIN_ID_PLACE_HOLDER";
	CurrentInstruction.ReplacementText = UNIQUE_ID.ToString(Manifest.ID);
	Instructions.push_back(CurrentInstruction);

	if (!FILE_SYSTEM.PerformTextReplacements(InterfaceFilePath, Instructions))
	{
		LOG.Add("FEEditorPluginManager::GenerateEditorPublicInterfaceFromTemplate: Error initializing " + InterfaceFilePath, "FE_EDITOR_PLUGIN_MANAGER", FE_LOG_ERROR);
		return false;
	}

	return true;
}