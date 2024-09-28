#pragma once
#include "../FEngine.h"
using namespace FocalEngine;

struct TrackedFileData
{
	std::string Path;
	uint64_t WriteTime;
};

class FENativeScriptProject
{
	friend class FEProject;

	FEProject* Parent = nullptr;
	std::string ProjectPath = "";
	std::vector<std::string> SourceFileList;
	FEAssetPackage* PackageData = nullptr;

	TrackedFileData DebugDllFileData;
	TrackedFileData DebugPdbFileData;
	TrackedFileData ReleaseDllFileData;

	const uint64_t DifferenceThreshold = 1'000'000;

	FENativeScriptModule* LastGeneratedScriptModule = nullptr;

	bool InitializeProject(std::vector<std::string> SourceFileFullPathList);
	bool ConfigureAndBuildCMake();
	bool RunProjectVSSolution();

	bool InitializeCMakeFileAndScriptFiles(std::vector<std::string> SourceFileFullPathList);

	struct InstructionWhatToReplaceInFile
	{
		std::string SubStringInLineToTrigger;
		std::string What;
		std::string ReplaceWith;
	};

	bool ReplaceInFile(std::string FilePath, std::vector<InstructionWhatToReplaceInFile> Instructions);
	bool UpdateEngineFiles();

	FEAssetPackage* CreateEngineHeadersAssetPackage();
	bool UnPackEngineHeadersAssetPackage(FEAssetPackage* AssetPackage, std::string Path);

	FEAssetPackage* CreateEngineLIBAssetPackage();
	bool UnPackEngineLIBAssetPackage(FEAssetPackage* AssetPackage, std::string Path);

	void SetFileTracking();
	bool GenerateScriptFilesFromTemplate(std::string ScriptName);

	bool IsFileChanged(const TrackedFileData& FileData);
	void UpdateTrackedFileWriteTime(TrackedFileData& FileData);
public:
	FENativeScriptProject(FEProject* Parent);
	~FENativeScriptProject();

	bool GenerateNewVSProject(std::string FirstScriptName);
	bool IsVSProjectValid();
	bool RegenerateVSProject();

	bool Load(Json::Value Root, std::string FolderPath);
	void Save(Json::Value& Root, std::string FolderPath);

	bool GenerateScriptModule();

	std::string GetCorrectProjectName();
	std::string ModuleID = "";

	bool Update();
};