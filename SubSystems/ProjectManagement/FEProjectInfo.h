#pragma once

#include "FEProject.h"

class FEProjectInfo
{
	friend class FEProjectManager;
public:
	FEUUID GetID() const;
	const std::string& GetName() const;
	const std::string& GetFolder() const;
	const std::string& GetDisplayFolder() const;
	bool IsMissing() const;

	uint64_t GetLastOpenedTime() const;
	uint64_t GetAddedTime() const;

	// Empty if project was not saved with that information.
	const std::string& GetCreatedWith() const;
	const std::string& GetSavedWith() const;
	const std::vector<std::string>& GetPluginNames() const;

	FETexture* GetThumbnail();
private:
	FEProjectInfo(const std::string& Folder);
	~FEProjectInfo();

	FEUUID ID;
	std::string Name;
	std::string Folder;
	std::string DisplayFolder;
	bool bMissing = false;
	uint64_t LastOpenedTime = 0;
	uint64_t AddedTime = 0;
	std::string CreatedWith;
	std::string SavedWith;
	std::vector<std::string> PluginNames;
	FETexture* Thumbnail = nullptr;
	bool bThumbnailLoaded = false;

	void SetFolder(const std::string& NewValue);
	void Refresh();
	void ReleaseThumbnail();
};