#include "FEPluginBrowser.h"
#include "../FocalEngine/SubSystems/FileSystem/FEZipArchive.h"
// Public folder of CurlPlugin is in include directories only when plugin was installed by CMake.
#if __has_include("CurlPluginInterface.h")
	#include "CurlPluginInterface.h"
	#define FE_CURL_PLUGIN_INTERFACE_AVAILABLE
#endif
using namespace FocalEngine;

FEPluginBrowser::FEPluginBrowser()
{
}

FEPluginBrowser::~FEPluginBrowser()
{
}

static bool ParseJson(const std::string& Text, Json::Value& OutRoot)
{
	Json::CharReaderBuilder Builder;
	std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	std::string Errors;
	return Reader->parse(Text.data(), Text.data() + Text.size(), &OutRoot, &Errors);
}

static std::string GetJsonString(const Json::Value& Object, const char* Key)
{
	if (!Object.isObject() || !Object[Key].isString())
		return "";

	return Object[Key].asString();
}

static std::string GetVersionFromTag(const std::string& Tag)
{
	if (!Tag.empty() && (Tag[0] == 'v' || Tag[0] == 'V'))
		return Tag.substr(1);

	return Tag;
}

static std::string ToLower(std::string Text)
{
	std::transform(Text.begin(), Text.end(), Text.begin(), [](unsigned char Character) { return static_cast<char>(std::tolower(Character)); });
	return Text;
}

void FEPluginBrowser::SetSource(const std::string& NewOwner, const std::vector<std::string>& NewRequiredTopics)
{
	Owner = NewOwner;
	RequiredTopics = NewRequiredTopics;
}

std::string FEPluginBrowser::GetOwner() const
{
	return Owner;
}

std::vector<std::string> FEPluginBrowser::GetRequiredTopics() const
{
	return RequiredTopics;
}

CurlPluginInterface* FEPluginBrowser::GetCurlPlugin()
{
#ifdef FE_CURL_PLUGIN_INTERFACE_AVAILABLE
	CurlPluginInterface* Plugin = PLUGIN_MANAGER.GetPlugin<CurlPluginInterface>(UNIQUE_ID.FromString(CurlPluginInterface::PluginID));
	if (Plugin == nullptr || Plugin->GetInterfaceVersion() != CurlPluginInterface::InterfaceVersion)
		return nullptr;

	return Plugin;
#else
	return nullptr;
#endif
}

bool FEPluginBrowser::IsAvailable()
{
	return GetCurlPlugin() != nullptr;
}

void FEPluginBrowser::SendGetRequest(const std::string& URL, const bool bGitHubAPI, RequestCallback OnFinished)
{
	CurlPluginInterface* CurlPlugin = GetCurlPlugin();
	if (CurlPlugin == nullptr)
	{
		OnFinished(0, "", "CurlPlugin is not loaded");
		return;
	}

#ifdef FE_CURL_PLUGIN_INTERFACE_AVAILABLE
	CurlPluginInterface::Request NewRequest;
	NewRequest.URL = URL;
	NewRequest.UserAgent = "FocalEngineEditor";
	NewRequest.TimeoutInSeconds = 30.0;
	if (bGitHubAPI)
	{
		NewRequest.Headers.push_back({ "Accept", "application/vnd.github+json" });
		NewRequest.Headers.push_back({ "X-GitHub-Api-Version", "2022-11-28" });
	}

	CurlPlugin->SendRequest(NewRequest, [URL, OnFinished](const CurlPluginInterface::Response& Result) {
		if (!Result.bTransferSucceeded)
		{
			OnFinished(0, "", Result.ErrorMessage.empty() ? "Request to " + URL + " failed" : Result.ErrorMessage);
			return;
		}

		if (Result.StatusCode == 200)
		{
			OnFinished(200, Result.Body, "");
			return;
		}

		if ((Result.StatusCode == 403 || Result.StatusCode == 429) && Result.GetHeaderValue("X-RateLimit-Remaining") == "0")
		{
			// Header has time in seconds since epoch when limit is reset, wait time is rounded up to minutes.
			const uint64_t ResetTime = std::strtoull(Result.GetHeaderValue("X-RateLimit-Reset").c_str(), nullptr, 10);
			const uint64_t CurrentTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_SECONDS);
			std::string WaitText = "later";
			if (ResetTime > CurrentTime)
				WaitText = "in " + TIME.TimeToFormattedString((ResetTime - CurrentTime + 59) / 60 * 60, FE_TIME_RESOLUTION_SECONDS);

			OnFinished(Result.StatusCode, "", "GitHub request limit is reached, try again " + WaitText);
			return;
		}

		OnFinished(Result.StatusCode, Result.Body, "Request to " + URL + " failed with HTTP status " + std::to_string(Result.StatusCode));
	});
#endif
}

void FEPluginBrowser::Refresh()
{
	if (State == FEPluginBrowserState::Loading)
		return;

	RefreshNumber++;
	ErrorMessage.clear();
	EntriesBeingLoaded.clear();
	EntryHasRelease.clear();
	ActiveRequestCount = 0;

	if (GetCurlPlugin() == nullptr)
	{
		FailRefresh("CurlPlugin is not loaded");
		return;
	}

	State = FEPluginBrowserState::Loading;
	RequestRepositoryPage(1);
}

void FEPluginBrowser::RequestRepositoryPage(const int Page)
{
	const uint64_t CurrentRefresh = RefreshNumber;
	const std::string URL = "https://api.github.com/users/" + Owner + "/repos?per_page=100&page=" + std::to_string(Page);
	SendGetRequest(URL, true, [this, CurrentRefresh, Page](const int StatusCode, const std::string& Body, const std::string& RequestErrorMessage) {
		if (CurrentRefresh != RefreshNumber)
			return;

		if (!RequestErrorMessage.empty())
		{
			FailRefresh(StatusCode == 404 ? "GitHub user \"" + Owner + "\" was not found" : RequestErrorMessage);
			return;
		}

		Json::Value Root;
		if (!ParseJson(Body, Root) || !Root.isArray())
		{
			FailRefresh("Could not read list of repositories of \"" + Owner + "\"");
			return;
		}

		for (const Json::Value& Repository : Root)
		{
			if (!Repository.isObject() || (Repository["fork"].isBool() && Repository["fork"].asBool()))
				continue;

			bool bHasAllTopics = true;
			for (const std::string& RequiredTopic : RequiredTopics)
			{
				bool bHasTopic = false;
				for (const Json::Value& Topic : Repository["topics"])
				{
					if (Topic.isString() && Topic.asString() == RequiredTopic)
						bHasTopic = true;
				}

				if (!bHasTopic)
					bHasAllTopics = false;
			}

			FEPluginBrowserEntry NewEntry;
			NewEntry.RepositoryName = GetJsonString(Repository, "name");
			NewEntry.RepositoryURL = GetJsonString(Repository, "html_url");
			NewEntry.RepositoryDescription = GetJsonString(Repository, "description");
			if (bHasAllTopics && !NewEntry.RepositoryName.empty())
				EntriesBeingLoaded.push_back(NewEntry);
		}

		if (Root.size() == 100)
		{
			RequestRepositoryPage(Page + 1);
		}
		else
		{
			RequestLatestReleases();
		}
	});
}

void FEPluginBrowser::RequestLatestReleases()
{
	EntryHasRelease.assign(EntriesBeingLoaded.size(), false);
	const uint64_t CurrentRefresh = RefreshNumber;

	// Held until all requests are sent, so refresh is not finished by a request that fails right away.
	ActiveRequestCount++;
	for (size_t i = 0; i < EntriesBeingLoaded.size(); i++)
	{
		const std::string URL = "https://api.github.com/repos/" + Owner + "/" + EntriesBeingLoaded[i].RepositoryName + "/releases/latest";
		ActiveRequestCount++;
		SendGetRequest(URL, true, [this, CurrentRefresh, i](const int StatusCode, const std::string& Body, const std::string& RequestErrorMessage) {
			if (CurrentRefresh != RefreshNumber)
				return;

			// Repository without release is not listed.
			if (StatusCode == 404)
			{
				OnRequestFinished();
				return;
			}

			EntryHasRelease[i] = true;
			FEPluginBrowserEntry& Entry = EntriesBeingLoaded[i];
			Json::Value Root;
			if (!RequestErrorMessage.empty())
			{
				Entry.Problem = "Could not read latest release: " + RequestErrorMessage;
			}
			else if (!ParseJson(Body, Root) || !Root.isObject())
			{
				Entry.Problem = "Could not read latest release";
			}
			else
			{
				Entry.ReleaseTag = GetJsonString(Root, "tag_name");
				Entry.ReleaseURL = GetJsonString(Root, "html_url");
				Entry.ReleaseNotes = GetJsonString(Root, "body");
				Entry.ReleaseNotes.erase(std::remove(Entry.ReleaseNotes.begin(), Entry.ReleaseNotes.end(), '\r'), Entry.ReleaseNotes.end());
				Entry.ReleaseDate = GetJsonString(Root, "published_at").substr(0, 10);

				const std::string ExpectedArchiveName = Entry.RepositoryName + "_" + GetVersionFromTag(Entry.ReleaseTag) + ".zip";
				for (const Json::Value& Asset : Root["assets"])
				{
					if (GetJsonString(Asset, "name") != ExpectedArchiveName)
						continue;

					Entry.ArchiveName = ExpectedArchiveName;
					Entry.ArchiveURL = GetJsonString(Asset, "browser_download_url");
					Entry.ArchiveSize = Asset["size"].isIntegral() ? Asset["size"].asUInt64() : 0;
				}

				if (Entry.ArchiveName.empty())
					Entry.Problem = "Release " + Entry.ReleaseTag + " has no " + ExpectedArchiveName;

				RequestManifest(i);
			}

			OnRequestFinished();
		});
	}

	OnRequestFinished();
}

void FEPluginBrowser::RequestManifest(const size_t EntryIndex)
{
	const FEPluginBrowserEntry& Entry = EntriesBeingLoaded[EntryIndex];
	if (Entry.ReleaseTag.empty())
		return;

	const uint64_t CurrentRefresh = RefreshNumber;
	const std::string URL = "https://raw.githubusercontent.com/" + Owner + "/" + Entry.RepositoryName + "/" + Entry.ReleaseTag + "/" + Entry.RepositoryName + ".json";
	ActiveRequestCount++;
	SendGetRequest(URL, false, [this, CurrentRefresh, EntryIndex](const int StatusCode, const std::string& Body, const std::string& RequestErrorMessage) {
		if (CurrentRefresh != RefreshNumber)
			return;

		FEPluginBrowserEntry& Entry = EntriesBeingLoaded[EntryIndex];
		const std::string ManifestName = Entry.RepositoryName + ".json";
		std::string ManifestProblem;
		if (!RequestErrorMessage.empty())
		{
			ManifestProblem = StatusCode == 404 ? ManifestName + " was not found in repository at " + Entry.ReleaseTag : "Could not read " + ManifestName + ": " + RequestErrorMessage;
		}
		else if (!PLUGIN_MANAGER.ParseManifestText(Body, Entry.Manifest))
		{
			ManifestProblem = ManifestName + " in repository is not a valid plugin manifest";
		}
		else
		{
			Entry.bManifestLoaded = true;
			if (Entry.Manifest.Version != GetVersionFromTag(Entry.ReleaseTag))
				ManifestProblem = ManifestName + " has version " + Entry.Manifest.Version + ", but release is " + Entry.ReleaseTag;
		}

		if (Entry.Problem.empty())
			Entry.Problem = ManifestProblem;

		OnRequestFinished();
	});
}

void FEPluginBrowser::OnRequestFinished()
{
	if (ActiveRequestCount > 0)
		ActiveRequestCount--;

	if (ActiveRequestCount == 0)
		FinishRefresh();
}

void FEPluginBrowser::FinishRefresh()
{
	Entries.clear();
	for (size_t i = 0; i < EntriesBeingLoaded.size(); i++)
	{
		if (EntryHasRelease[i])
			Entries.push_back(EntriesBeingLoaded[i]);
	}

	std::sort(Entries.begin(), Entries.end(), [](const FEPluginBrowserEntry& First, const FEPluginBrowserEntry& Second) {
		return ToLower(First.RepositoryName) < ToLower(Second.RepositoryName);
	});

	EntriesBeingLoaded.clear();
	EntryHasRelease.clear();
	State = FEPluginBrowserState::Loaded;
	LastLoadTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_SECONDS);
}

void FEPluginBrowser::FailRefresh(const std::string& Message)
{
	RefreshNumber++;
	EntriesBeingLoaded.clear();
	EntryHasRelease.clear();
	ActiveRequestCount = 0;
	State = FEPluginBrowserState::Failed;
	ErrorMessage = Message;
	LOG.Add("FEPluginBrowser::Refresh: " + Message, "FE_EDITOR", FE_LOG_WARNING);
}

bool FEPluginBrowser::WasRefreshed() const
{
	return State != FEPluginBrowserState::NotLoaded;
}

bool FEPluginBrowser::IsLoading() const
{
	return State == FEPluginBrowserState::Loading;
}

bool FEPluginBrowser::HasFailed() const
{
	return State == FEPluginBrowserState::Failed;
}

std::string FEPluginBrowser::GetErrorMessage() const
{
	return ErrorMessage;
}

uint64_t FEPluginBrowser::GetLastLoadTime() const
{
	return LastLoadTime;
}

const std::vector<FEPluginBrowserEntry>& FEPluginBrowser::GetEntries() const
{
	return Entries;
}

const FEPluginBrowserEntry* FEPluginBrowser::FindEntry(const FEUUID& PluginID) const
{
	for (size_t i = 0; i < Entries.size(); i++)
	{
		if (Entries[i].bManifestLoaded && Entries[i].Manifest.ID == PluginID)
			return &Entries[i];
	}

	return nullptr;
}

int FEPluginBrowser::CompareVersions(const std::string& First, const std::string& Second)
{
	// Text around number in a part, like "v" in "v1" or "-beta", is ignored.
	auto ReadNextNumber = [](const std::string& Version, size_t& Position) -> unsigned long long {
		while (Position < Version.size() && Version[Position] != '.' && !std::isdigit(static_cast<unsigned char>(Version[Position])))
			Position++;

		unsigned long long Value = 0;
		while (Position < Version.size() && std::isdigit(static_cast<unsigned char>(Version[Position])))
		{
			Value = Value * 10 + static_cast<unsigned long long>(Version[Position] - '0');
			Position++;
		}

		while (Position < Version.size() && Version[Position] != '.')
			Position++;

		if (Position < Version.size())
			Position++;

		return Value;
	};

	size_t FirstPosition = 0;
	size_t SecondPosition = 0;
	while (FirstPosition < First.size() || SecondPosition < Second.size())
	{
		const unsigned long long FirstNumber = ReadNextNumber(First, FirstPosition);
		const unsigned long long SecondNumber = ReadNextNumber(Second, SecondPosition);
		if (FirstNumber != SecondNumber)
			return FirstNumber < SecondNumber ? -1 : 1;
	}

	return 0;
}

const FEPluginBrowserEntry* FEPluginBrowser::FindEntryByName(const std::string& RepositoryName) const
{
	for (size_t i = 0; i < Entries.size(); i++)
	{
		if (Entries[i].RepositoryName == RepositoryName)
			return &Entries[i];
	}

	return nullptr;
}

bool FEPluginBrowser::IsPluginInstalled(const FEUUID& PluginID) const
{
	const std::vector<DiscoveredPlugin>& Plugins = PLUGIN_MANAGER.GetDiscoveredPlugins();
	for (size_t i = 0; i < Plugins.size(); i++)
	{
		if (Plugins[i].Manifest.ID == PluginID)
			return true;
	}

	const FEPluginBrowserEntry* Entry = FindEntry(PluginID);
	return Entry != nullptr && std::find(InstalledPluginNames.begin(), InstalledPluginNames.end(), Entry->RepositoryName) != InstalledPluginNames.end();
}

std::vector<const FEPluginBrowserEntry*> FEPluginBrowser::GetInstallOrder(const FEPluginBrowserEntry& Entry, std::vector<FEUUID>& OutMissingDependencies) const
{
	std::vector<const FEPluginBrowserEntry*> Result;
	std::vector<const FEPluginBrowserEntry*> EntriesInProgress;
	OutMissingDependencies.clear();

	std::function<void(const FEPluginBrowserEntry&)> AddWithDependencies = [&](const FEPluginBrowserEntry& CurrentEntry) {
		// Dependency cycle would never end otherwise.
		if (std::find(Result.begin(), Result.end(), &CurrentEntry) != Result.end() || std::find(EntriesInProgress.begin(), EntriesInProgress.end(), &CurrentEntry) != EntriesInProgress.end())
			return;

		EntriesInProgress.push_back(&CurrentEntry);
		for (const FEUUID& DependencyID : CurrentEntry.Manifest.Dependencies)
		{
			if (IsPluginInstalled(DependencyID))
				continue;

			const FEPluginBrowserEntry* Dependency = FindEntry(DependencyID);
			if (Dependency != nullptr)
			{
				AddWithDependencies(*Dependency);
			}
			else if (std::find(OutMissingDependencies.begin(), OutMissingDependencies.end(), DependencyID) == OutMissingDependencies.end())
			{
				OutMissingDependencies.push_back(DependencyID);
			}
		}

		Result.push_back(&CurrentEntry);
	};

	AddWithDependencies(Entry);
	return Result;
}

void FEPluginBrowser::Install(const std::vector<std::string>& PluginNames)
{
	if (IsInstalling() || PluginNames.empty())
		return;

	InstallNumber++;
	InstallTasks.clear();
	bWaitingForConfirmation = false;

	const std::string PluginDirectory = PLUGIN_MANAGER.GetPluginDirectory();
	for (const std::string& PluginName : PluginNames)
	{
		FEPluginInstallTask NewTask;
		NewTask.PluginName = PluginName;
		const FEPluginBrowserEntry* Entry = FindEntryByName(PluginName);
		if (Entry != nullptr && !Entry->ArchiveURL.empty())
		{
			NewTask.bUpdate = Entry->bManifestLoaded && IsPluginInstalled(Entry->Manifest.ID);
			NewTask.ArchiveURL = Entry->ArchiveURL;
			NewTask.TotalBytes = Entry->ArchiveSize;
			// Same '.' prefix as InstallPluginPackage uses for its temporary folder, plugin manager skips such files and folders.
			NewTask.ArchivePath = PluginDirectory + "/.Downloading_" + Entry->ArchiveName;
		}

		InstallTasks.push_back(NewTask);
	}

	for (size_t i = 0; i < InstallTasks.size(); i++)
	{
		if (InstallTasks[i].ArchiveURL.empty())
		{
			FailInstall(i, "Release archive of " + InstallTasks[i].PluginName + " is not known");
			return;
		}
	}

	if (!FILE_SYSTEM.DoesDirectoryExist(PluginDirectory) && !FILE_SYSTEM.MakeDirectory(PluginDirectory))
	{
		FailInstall(0, "Could not create plugin directory \"" + PluginDirectory + "\"");
		return;
	}

	for (size_t i = 0; i < InstallTasks.size(); i++)
		DownloadArchive(i);
}

void FEPluginBrowser::DownloadArchive(const size_t TaskIndex)
{
	CurlPluginInterface* CurlPlugin = GetCurlPlugin();
	if (CurlPlugin == nullptr)
	{
		FailInstall(TaskIndex, "CurlPlugin is not loaded");
		return;
	}

	FEPluginInstallTask& Task = InstallTasks[TaskIndex];
	DeleteArchive(Task);
	Task.State = FEPluginInstallState::Downloading;

#ifdef FE_CURL_PLUGIN_INTERFACE_AVAILABLE
	const uint64_t CurrentInstall = InstallNumber;
	CurlPluginInterface::Request NewRequest;
	NewRequest.URL = Task.ArchiveURL;
	NewRequest.UserAgent = "FocalEngineEditor";
	NewRequest.DownloadFilePath = Task.ArchivePath;
	NewRequest.OnProgress = [this, CurrentInstall, TaskIndex](const uint64_t DownloadedBytes, const uint64_t TotalBytesToDownload, uint64_t, uint64_t) {
		if (CurrentInstall != InstallNumber)
			return;

		InstallTasks[TaskIndex].DownloadedBytes = DownloadedBytes;
		if (TotalBytesToDownload > 0)
			InstallTasks[TaskIndex].TotalBytes = TotalBytesToDownload;
	};

	Task.RequestID = CurlPlugin->SendRequest(NewRequest, [this, CurrentInstall, TaskIndex](const CurlPluginInterface::Response& Result) {
		if (CurrentInstall != InstallNumber)
			return;

		InstallTasks[TaskIndex].RequestID = 0;
		if (!Result.IsSuccessful())
		{
			const std::string Reason = Result.bTransferSucceeded ? "HTTP status " + std::to_string(Result.StatusCode) : Result.ErrorMessage;
			FailInstall(TaskIndex, "Could not download " + InstallTasks[TaskIndex].ArchiveURL + ": " + Reason);
			return;
		}

		CheckArchive(TaskIndex);
	});
#endif
}

struct FEArchiveReadResult
{
	bool bSuccess = false;
	std::vector<FEZipArchiveFile> Files;
	std::string ErrorMessage;
};

void FEPluginBrowser::CheckArchive(const size_t TaskIndex)
{
	InstallTasks[TaskIndex].State = FEPluginInstallState::CheckingArchive;
	const uint64_t CurrentInstall = InstallNumber;
	const std::string ArchivePath = InstallTasks[TaskIndex].ArchivePath;
	std::shared_ptr<FEArchiveReadResult> ReadResult = std::make_shared<FEArchiveReadResult>();

	// Whole archive is checked, but only EngineCommit.txt is decompressed, it could be in plugin folder inside archive.
	THREAD_POOL.Execute([ArchivePath, ReadResult](void*, void*) {
		ReadResult->bSuccess = FEZipArchive::ReadFiles(ArchivePath, ReadResult->Files, ReadResult->ErrorMessage, [](const std::string& FileName) {
			const size_t SeparatorPosition = FileName.find('/');
			return FileName == "EngineCommit.txt" || (SeparatorPosition != std::string::npos && FileName.substr(SeparatorPosition + 1) == "EngineCommit.txt");
		});
	}, nullptr, nullptr, [this, CurrentInstall, TaskIndex, ReadResult](void*) {
		if (CurrentInstall != InstallNumber)
			return;

		if (!ReadResult->bSuccess)
		{
			FailInstall(TaskIndex, ReadResult->ErrorMessage);
			return;
		}

		FEPluginInstallTask& Task = InstallTasks[TaskIndex];
		Task.EngineCommit.clear();
		for (const FEZipArchiveFile& File : ReadResult->Files)
		{
			// File in the root of archive is preferred over file in a folder.
			if (!Task.EngineCommit.empty() && File.Name.find('/') != std::string::npos)
				continue;

			Task.EngineCommit.assign(File.Data.begin(), File.Data.end());
			Task.EngineCommit.erase(std::remove_if(Task.EngineCommit.begin(), Task.EngineCommit.end(), [](unsigned char Character) { return std::isspace(Character); }), Task.EngineCommit.end());
		}

		Task.State = FEPluginInstallState::Downloaded;
		OnArchiveChecked();
	});
}

void FEPluginBrowser::OnArchiveChecked()
{
	bool bAllBuiltWithThisEngine = true;
	for (const FEPluginInstallTask& Task : InstallTasks)
	{
		if (Task.State != FEPluginInstallState::Downloaded)
			return;

		if (!IsSameEngineCommit(Task.EngineCommit))
			bAllBuiltWithThisEngine = false;
	}

	if (bAllBuiltWithThisEngine)
	{
		InstallPlugin(0);
	}
	else
	{
		bWaitingForConfirmation = true;
	}
}

void FEPluginBrowser::ContinueInstall()
{
	if (!bWaitingForConfirmation)
		return;

	bWaitingForConfirmation = false;
	InstallPlugin(0);
}

void FEPluginBrowser::CancelInstall()
{
	if (!bWaitingForConfirmation)
		return;

	for (const FEPluginInstallTask& Task : InstallTasks)
		DeleteArchive(Task);

	InstallNumber++;
	InstallTasks.clear();
	bWaitingForConfirmation = false;
}

void FEPluginBrowser::InstallPlugin(const size_t TaskIndex)
{
	if (TaskIndex >= InstallTasks.size())
		return;

	InstallTasks[TaskIndex].State = FEPluginInstallState::Installing;
	const uint64_t CurrentInstall = InstallNumber;
	auto OnFinished = [this, CurrentInstall, TaskIndex](const bool bSucceeded, const std::string& ErrorMessage) {
		if (CurrentInstall != InstallNumber)
			return;

		if (!bSucceeded)
		{
			FailInstall(TaskIndex, ErrorMessage);
			return;
		}

		FEPluginInstallTask& Task = InstallTasks[TaskIndex];
		DeleteArchive(Task);
		Task.State = FEPluginInstallState::Installed;
		// Updated plugin is already in installed plugins, plugin manager knows that its update is pending.
		if (!Task.bUpdate)
			InstalledPluginNames.push_back(Task.PluginName);

		InstallPlugin(TaskIndex + 1);
	};

	if (InstallTasks[TaskIndex].bUpdate)
	{
		PLUGIN_MANAGER.UpdatePluginPackageAsync(InstallTasks[TaskIndex].ArchivePath, OnFinished);
	}
	else
	{
		PLUGIN_MANAGER.InstallPluginPackageAsync(InstallTasks[TaskIndex].ArchivePath, OnFinished);
	}
}

void FEPluginBrowser::FailInstall(const size_t TaskIndex, const std::string& Message)
{
	CurlPluginInterface* CurlPlugin = GetCurlPlugin();
	for (size_t i = 0; i < InstallTasks.size(); i++)
	{
		FEPluginInstallTask& Task = InstallTasks[i];
		if (Task.State == FEPluginInstallState::Installed || Task.State == FEPluginInstallState::Failed)
			continue;

#ifdef FE_CURL_PLUGIN_INTERFACE_AVAILABLE
		if (CurlPlugin != nullptr && Task.RequestID != 0)
			CurlPlugin->CancelRequest(Task.RequestID);
#endif
		Task.RequestID = 0;
		Task.State = FEPluginInstallState::Failed;
		Task.ErrorMessage = i == TaskIndex ? Message : "Not installed, because " + InstallTasks[TaskIndex].PluginName + " could not be installed";
		DeleteArchive(Task);
	}

	InstallNumber++;
	bWaitingForConfirmation = false;
	LOG.Add("FEPluginBrowser::Install: " + InstallTasks[TaskIndex].PluginName + ": " + Message, "FE_EDITOR", FE_LOG_ERROR);
}

void FEPluginBrowser::DeleteArchive(const FEPluginInstallTask& Task)
{
	if (!Task.ArchivePath.empty() && FILE_SYSTEM.DoesFileExist(Task.ArchivePath))
		FILE_SYSTEM.RemoveFile(Task.ArchivePath);
}

bool FEPluginBrowser::IsInstalling() const
{
	if (bWaitingForConfirmation)
		return true;

	for (const FEPluginInstallTask& Task : InstallTasks)
	{
		if (Task.State != FEPluginInstallState::Installed && Task.State != FEPluginInstallState::Failed)
			return true;
	}

	return false;
}

bool FEPluginBrowser::IsWaitingForConfirmation() const
{
	return bWaitingForConfirmation;
}

const std::vector<FEPluginInstallTask>& FEPluginBrowser::GetInstallTasks() const
{
	return InstallTasks;
}

const FEPluginInstallTask* FEPluginBrowser::FindInstallTask(const std::string& PluginName) const
{
	for (size_t i = 0; i < InstallTasks.size(); i++)
	{
		if (InstallTasks[i].PluginName == PluginName)
			return &InstallTasks[i];
	}

	return nullptr;
}

const std::vector<std::string>& FEPluginBrowser::GetInstalledPluginNames() const
{
	return InstalledPluginNames;
}

bool FEPluginBrowser::IsSameEngineCommit(const std::string& PluginEngineCommit)
{
	// Engine knows only short hash of its commit, EngineCommit.txt has full hash.
	const std::string EngineCommitHash = ENGINE.GetEngineCommitHash();
	return !EngineCommitHash.empty() && PluginEngineCommit.rfind(EngineCommitHash, 0) == 0;
}
