#pragma once

#include "../FocalEngine/FEngine.h"
using namespace FocalEngine;

class CurlPluginInterface;

struct FEPluginBrowserEntry
{
	std::string RepositoryName;
	std::string RepositoryURL;
	std::string RepositoryDescription;

	std::string ReleaseTag;
	std::string ReleaseURL;
	std::string ReleaseNotes;
	// YYYY-MM-DD
	std::string ReleaseDate;

	// Release asset <RepositoryName>_<Version>.zip, empty if release does not have it.
	std::string ArchiveName;
	std::string ArchiveURL;
	uint64_t ArchiveSize = 0;

	// Manifest is read from repository at release tag, so it is known before archive is downloaded.
	bool bManifestLoaded = false;
	PluginManifest Manifest;

	// Not empty if plugin can not be installed from this release.
	std::string Problem;
};

enum class FEPluginInstallState
{
	Downloading,
	CheckingArchive,
	Downloaded,
	Installing,
	Installed,
	Failed
};

struct FEPluginInstallTask
{
	std::string PluginName;
	// Plugin is already installed, it is replaced with new version.
	bool bUpdate = false;
	std::string ArchiveURL;
	std::string ArchivePath;
	FEPluginInstallState State = FEPluginInstallState::Downloading;
	uint64_t RequestID = 0;
	uint64_t DownloadedBytes = 0;
	uint64_t TotalBytes = 0;
	// Content of EngineCommit.txt from archive, empty if archive does not have it.
	std::string EngineCommit;
	std::string ErrorMessage;
};

// Lists plugins that are published as GitHub releases, requests are sent through CurlPlugin.
class FEPluginBrowser
{
	SINGLETON_PRIVATE_PART(FEPluginBrowser)

	std::string Owner = "Azzinoth";
	std::vector<std::string> RequiredTopics = { "focal-engine", "plugin" };

	enum class FEPluginBrowserState
	{
		NotLoaded,
		Loading,
		Loaded,
		Failed
	};

	FEPluginBrowserState State = FEPluginBrowserState::NotLoaded;
	std::string ErrorMessage;
	std::vector<FEPluginBrowserEntry> Entries;
	uint64_t LastLoadTime = 0;

	// Responses of the current refresh are ignored after next refresh started.
	uint64_t RefreshNumber = 0;
	size_t ActiveRequestCount = 0;
	std::vector<FEPluginBrowserEntry> EntriesBeingLoaded;
	// Entries without release are removed when refresh is finished.
	std::vector<bool> EntryHasRelease;

	using RequestCallback = std::function<void(int StatusCode, const std::string& Body, const std::string& RequestErrorMessage)>;
	CurlPluginInterface* GetCurlPlugin();
	void SendGetRequest(const std::string& URL, bool bGitHubAPI, RequestCallback OnFinished);

	void RequestRepositoryPage(int Page);
	void RequestLatestReleases();
	void RequestManifest(size_t EntryIndex);
	void OnRequestFinished();
	void FinishRefresh();
	void FailRefresh(const std::string& Message);

	// Callbacks of the current install are ignored after it failed or was canceled.
	uint64_t InstallNumber = 0;
	std::vector<FEPluginInstallTask> InstallTasks;
	bool bWaitingForConfirmation = false;
	std::vector<std::string> InstalledPluginNames;

	void DownloadArchive(size_t TaskIndex);
	void CheckArchive(size_t TaskIndex);
	void OnArchiveChecked();
	void InstallPlugin(size_t TaskIndex);
	// Plugins of current install that are not installed yet are not installed at all.
	void FailInstall(size_t TaskIndex, const std::string& Message);
	void DeleteArchive(const FEPluginInstallTask& Task);
	bool IsPluginInstalled(const FEUUID& PluginID) const;
public:
	SINGLETON_PUBLIC_PART(FEPluginBrowser)

	// Plugins are public repositories of owner that have all required topics and a release.
	void SetSource(const std::string& NewOwner, const std::vector<std::string>& NewRequiredTopics);
	std::string GetOwner() const;
	std::vector<std::string> GetRequiredTopics() const;

	// Browser needs CurlPlugin, it is installed when editor is configured.
	bool IsAvailable();
	void Refresh();

	bool WasRefreshed() const;
	bool IsLoading() const;
	// List of the last successful refresh is kept when the next one fails.
	bool HasFailed() const;
	std::string GetErrorMessage() const;
	// Seconds since epoch, 0 if list was never loaded.
	uint64_t GetLastLoadTime() const;

	const std::vector<FEPluginBrowserEntry>& GetEntries() const;
	const FEPluginBrowserEntry* FindEntry(const FEUUID& PluginID) const;
	const FEPluginBrowserEntry* FindEntryByName(const std::string& RepositoryName) const;

	// Plugins that should be installed together with this one, dependencies first and plugin itself last.
	// OutMissingDependencies gets dependencies that are not installed and are not in the list.
	std::vector<const FEPluginBrowserEntry*> GetInstallOrder(const FEPluginBrowserEntry& Entry, std::vector<FEUUID>& OutMissingDependencies) const;
	// All archives are downloaded first, then plugins are installed in the given order, installed plugins are updated.
	// Plugins are loaded on next start.
	void Install(const std::vector<std::string>& PluginNames);
	bool IsInstalling() const;
	// Some downloaded archive was built with other engine commit, ContinueInstall or CancelInstall should be called.
	bool IsWaitingForConfirmation() const;
	void ContinueInstall();
	void CancelInstall();
	const std::vector<FEPluginInstallTask>& GetInstallTasks() const;
	const FEPluginInstallTask* FindInstallTask(const std::string& PluginName) const;
	const std::vector<std::string>& GetInstalledPluginNames() const;

	static bool IsSameEngineCommit(const std::string& PluginEngineCommit);

	// Compares dot separated numbers, "v" before them is ignored, returns negative, zero or positive value like strcmp.
	static int CompareVersions(const std::string& First, const std::string& Second);
};

#define PLUGIN_BROWSER FEPluginBrowser::GetInstance()
