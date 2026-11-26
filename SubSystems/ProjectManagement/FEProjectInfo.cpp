#include "FEProjectInfo.h"

FEProjectInfo::FEProjectInfo(const std::string& Folder)
{
	SetFolder(Folder);
}

FEProjectInfo::~FEProjectInfo()
{
	ReleaseThumbnail();
}

FEUUID FEProjectInfo::GetID() const
{
	return ID;
}

const std::string& FEProjectInfo::GetName() const
{
	return Name;
}

const std::string& FEProjectInfo::GetFolder() const
{
	return Folder;
}

const std::string& FEProjectInfo::GetDisplayFolder() const
{
	return DisplayFolder;
}

bool FEProjectInfo::IsMissing() const
{
	return bMissing;
}

uint64_t FEProjectInfo::GetLastOpenedTime() const
{
	return LastOpenedTime;
}

uint64_t FEProjectInfo::GetAddedTime() const
{
	return AddedTime;
}

const std::string& FEProjectInfo::GetCreatedWith() const
{
	return CreatedWith;
}

const std::string& FEProjectInfo::GetSavedWith() const
{
	return SavedWith;
}

const std::vector<std::string>& FEProjectInfo::GetPluginNames() const
{
	return PluginNames;
}

FETexture* FEProjectInfo::GetThumbnail()
{
	if (!bThumbnailLoaded)
	{
		bThumbnailLoaded = true;
		const std::string FilePath = Folder + "\\Thumbnail.texture";
		if (!bMissing && FILE_SYSTEM.DoesFileExist(FilePath))
			Thumbnail = RESOURCE_MANAGER.LoadFETextureUnmanagedWithNewID(FilePath);
	}

	return Thumbnail;
}

void FEProjectInfo::ReleaseThumbnail()
{
	if (Thumbnail != nullptr)
		delete Thumbnail;

	Thumbnail = nullptr;
	bThumbnailLoaded = false;
}

void FEProjectInfo::SetFolder(const std::string& NewValue)
{
	ReleaseThumbnail();
	Folder = FILE_SYSTEM.GetNormalizedPath(NewValue);
	DisplayFolder = Folder;

	const std::string EditorFolder = FILE_SYSTEM.GetNormalizedPath(FILE_SYSTEM.GetCurrentWorkingPath());
	if (FILE_SYSTEM.IsPathInsideDirectory(Folder, EditorFolder))
	{
		DisplayFolder = Folder.substr(EditorFolder.size());
		if (DisplayFolder[0] == '\\' || DisplayFolder[0] == '/')
			DisplayFolder.erase(0, 1);
	}
}

// Result looks like "Editor 0.1.0 (ab44c70), engine 0.2.4 (232ca66)".
static std::string GetVersionText(const Json::Value& VersionData)
{
	std::string Result;
	if (!VersionData.isObject())
		return Result;

	const std::string Parts[] = { "Editor", "Engine" };
	for (const std::string& Part : Parts)
	{
		if (!VersionData.isMember(Part))
			continue;

		const Json::Value& PartData = VersionData[Part];
		Result += Result.empty() ? "Editor " : ", engine ";
		Result += PartData["Version"].asString();
		const std::string Commit = PartData["Commit"].asString();
		if (!Commit.empty())
			Result += " (" + Commit + ")";
	}

	return Result;
}

void FEProjectInfo::Refresh()
{
	Json::Value Root;
	const bool bWasMissing = bMissing;
	bMissing = !FEProject::ReadProjectFile(Folder, Root);
	if (bMissing != bWasMissing)
		ReleaseThumbnail();

	if (bMissing)
		return;

	ID = UNIQUE_ID.FromString(Root["ID"].asString());
	Name = Root["Name"].asString();
	CreatedWith = GetVersionText(Root["CreatedWith"]);
	SavedWith = GetVersionText(Root["SavedWith"]);

	PluginNames.clear();
	for (const Json::Value& PluginData : Root["Plugins"])
		PluginNames.push_back(PluginData["Name"].asString());
}