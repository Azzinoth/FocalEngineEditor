#include "FEEditorVirtualFileSystem.h"
using namespace FocalEngine;

FEVFSFile::FEVFSFile()
{

}

FEVFSFile::FEVFSFile(std::string DataID, FEVFSDirectory* InDirectory)
{
	this->DataID = DataID;
}

bool FEVFSFile::IsReadOnly()
{
	return bReadOnly;
}

void FEVFSFile::SetReadOnly(const bool NewValue)
{
	bReadOnly = NewValue;
}

FEVFSDirectory::FEVFSDirectory() : FEObject(FE_NULL, "")
{
	Parent = nullptr;
}

FEVFSDirectory::~FEVFSDirectory()
{
	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		delete SubDirectories[i];
	}

	SubDirectories.clear();
	Files.clear();
}

bool FEVFSDirectory::HasFile(const FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVFSDirectory::HasFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	for (size_t i = 0; i < Files.size(); i++)
	{
		if (Files[i].DataID == File->GetObjectID())
			return true;
	}

	return false;
}

bool FEVFSDirectory::AddSubDirectory(const std::string Name, const std::string ForceObjectID)
{
	if (HasSubDirectory(Name))
		return false;

	FEVFSDirectory* NewDirectory = new FEVFSDirectory();
	if (!ForceObjectID.empty())
		NewDirectory->SetIDOfUnTyped(ForceObjectID);
	NewDirectory->SetName(Name);
	NewDirectory->Parent = this;
	SubDirectories.push_back(NewDirectory);

	return true;
}

bool FEVFSDirectory::HasSubDirectory(const std::string SubDirectory)
{
	if (SubDirectory.empty())
		return false;

	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		if (SubDirectories[i]->GetName() == SubDirectory)
			return true;
	}

	return false;
}

FEVFSDirectory* FEVFSDirectory::GetSubDirectory(const std::string SubDirectory)
{
	if (SubDirectory.empty())
		return nullptr;

	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		if (SubDirectories[i]->GetName() == SubDirectory)
			return SubDirectories[i];
	}

	return nullptr;
}

void FEVFSDirectory::Clear()
{
	for (size_t i = 0; i < SubDirectories.size(); i++)
	{
		delete SubDirectories[i];
	}

	SubDirectories.clear();
	Files.clear();
}

bool FEVFSDirectory::DeleteFile(const FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVFSDirectory::deleteFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	for (size_t i = 0; i < Files.size(); i++)
	{
		if (File->GetObjectID() == Files[i].DataID && !Files[i].IsReadOnly())
		{
			Files.erase(Files.begin() + i, Files.begin() + i + 1);
			return true;
		}
	}

	return false;
}

bool FEVFSDirectory::AddFile(FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVFSDirectory::addFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	Files.push_back(FEVFSFile(File->GetObjectID(), this));
	return true;
}

bool FEVFSDirectory::IsReadOnly()
{
	return bReadOnly;
}

void FEVFSDirectory::SetReadOnly(const bool NewValue)
{
	bReadOnly = NewValue;
}

FEVirtualFileSystem::FEVirtualFileSystem()
{
	Root = new FEVFSDirectory();
	Root->Parent = nullptr;
	Root->SetName("/");
}

FEVirtualFileSystem::~FEVirtualFileSystem()
{
	Clear();
	delete Root;
	Root = nullptr;
}

bool FEVirtualFileSystem::IsPathCorrect(std::string Path)
{
	if (Path.empty() || Path[0] != '/')
		return false;

	Path.erase(0, 1);

	// it is root directory
	if (Path.empty())
		return true;

	std::vector<std::string> TokenizedPath;
	std::istringstream Iss(Path);
	std::string Token;
	while (std::getline(Iss, Token, '/'))
	{
		TokenizedPath.push_back(Token);
	}

	FEVFSDirectory* CurrentDirectory = Root;
	for (size_t i = 0; i < TokenizedPath.size(); i++)
	{
		// This is not directory and it is last token it could be file.
		if (CurrentDirectory->GetSubDirectory(TokenizedPath[i]) == nullptr && i == TokenizedPath.size() - 1)
		{
			for (size_t j = 0; j < CurrentDirectory->Files.size(); j++)
			{
				FEObject* FileObject = OBJECT_MANAGER.GetFEObject(CurrentDirectory->Files[j].DataID);
				if (FileObject != nullptr && FileObject->GetName() == TokenizedPath[i])
				{
					// Trailing slash indicates a directory, if it is a file it should not have trailing slash.	
					if (!Path.empty() && Path.back() == '/')
						return false;

					return true;
				}
			}

			return false;
		}

		CurrentDirectory = CurrentDirectory->GetSubDirectory(TokenizedPath[i]);

		if (CurrentDirectory == nullptr)
			return false;
	}

	return true;
}

FEVFSDirectory* FEVirtualFileSystem::PathToDirectory(std::string Path)
{
	if (!IsPathCorrect(Path))
		return nullptr;

	if (Path[0] == '/')
		Path.erase(0, 1);

	// it is root directory
	if (Path.empty())
		return Root;

	std::vector<std::string> TokenizedPath;
	std::istringstream Iss(Path);
	std::string Token;
	while (std::getline(Iss, Token, '/'))
	{
		TokenizedPath.push_back(Token);
	}

	FEVFSDirectory* CurrentDirectory = Root;
	for (size_t i = 0; i < TokenizedPath.size(); i++)
	{
		// This is not directory and it is last token it could be file.
		if (CurrentDirectory->GetSubDirectory(TokenizedPath[i]) == nullptr && i == TokenizedPath.size() - 1)
		{
			for (size_t j = 0; j < CurrentDirectory->Files.size(); j++)
			{
				// If that the case we return last valid directory.
				FEObject* FileObject = OBJECT_MANAGER.GetFEObject(CurrentDirectory->Files[j].DataID);
				if (FileObject != nullptr && FileObject->GetName() == TokenizedPath[i])
					return CurrentDirectory;
			}
		}

		CurrentDirectory = CurrentDirectory->GetSubDirectory(TokenizedPath[i]);
		if (CurrentDirectory == nullptr)
			return nullptr;
	}

	return CurrentDirectory;
}

bool FEVirtualFileSystem::CreateFile(FEObject* Data, const std::string Path)
{
	if (Data == nullptr)
	{
		LOG.Add("Data is nullptr in function FEVirtualFileSystem::CreateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (Data->GetType() != FE_SHADER &&
		Data->GetType() != FE_TEXTURE &&
		Data->GetType() != FE_MESH &&
		Data->GetType() != FE_MATERIAL &&
		Data->GetType() != FE_GAMEMODEL &&
		Data->GetType() != FE_POINT_CLOUD &&
		Data->GetType() != FE_PREFAB &&
		Data->GetType() != FE_SCENE &&
		Data->GetType() != FE_ASSET_PACKAGE &&
		Data->GetType() != FE_NATIVE_SCRIPT_MODULE)
	{
		LOG.Add("Data type is not supported in function FEVirtualFileSystem::CreateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!AcceptableName(Data->GetName()))
	{
		LOG.Add("File name is not acceptable in function FEVirtualFileSystem::CreateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (!IsPathCorrect(Path))
	{
		LOG.Add("Path is not correct in function FEVirtualFileSystem::CreateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (IsPathToFile(Path))
		return false;

	if (DoesFileExistAnywhere(Data))
	{
		LOG.Add("File already exists elsewhere in function FEVirtualFileSystem::CreateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	FEVFSDirectory* Directory = PathToDirectory(Path);

	if (DirectoryHasFileWithName(Directory, Data->GetName()))
	{
		LOG.Add("File with this name already exists in target directory in function FEVirtualFileSystem::CreateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Directory->HasSubDirectory(Data->GetName()))
	{
		LOG.Add("Subdirectory with this name already exists in target directory in function FEVirtualFileSystem::CreateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_WARNING);
		return false;
	}

	if (Directory->IsReadOnly() || IsAnyAncestorReadOnly(Directory))
		return false;

	Directory->Files.push_back(FEVFSFile(Data->GetObjectID(), Directory));
	return true;
}

bool FEVirtualFileSystem::DirectoryHasFileWithName(FEVFSDirectory* Directory, const std::string Name)
{
	if (Directory == nullptr)
		return false;

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		FEObject* Existing = OBJECT_MANAGER.GetFEObject(Directory->Files[i].DataID);
		if (Existing != nullptr && Existing->GetName() == Name)
			return true;
	}

	return false;
}

bool FEVirtualFileSystem::IsAnyAncestorReadOnly(const FEVFSDirectory* Directory)
{
	if (Directory == nullptr)
		return false;

	for (FEVFSDirectory* Ancestor = Directory->Parent; Ancestor != nullptr; Ancestor = Ancestor->Parent)
	{
		if (Ancestor->IsReadOnly())
			return true;
	}

	return false;
}

std::vector<std::string> FEVirtualFileSystem::GetDirectoryContentIDs(const std::string Path)
{
	std::vector<std::string> Result;
	if (IsPathToFile(Path))
		return Result;

	const FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return Result;

	for (size_t i = 0; i < Directory->SubDirectories.size(); i++)
	{
		Result.push_back(Directory->SubDirectories[i]->GetObjectID());
	}

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		Result.push_back(Directory->Files[i].DataID);
	}

	return Result;
}

bool FEVirtualFileSystem::CreateDirectory(const std::string Name, const std::string Path)
{
	if (!AcceptableName(Name))
		return false;

	if (IsPathToFile(Path))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	if (Directory->IsReadOnly() || IsAnyAncestorReadOnly(Directory))
		return false;

	if (Directory->HasSubDirectory(Name))
		return false;

	if (DirectoryHasFileWithName(Directory, Name))
		return false;

	FEVFSDirectory* NewDirectory = new FEVFSDirectory();
	NewDirectory->SetName(Name);
	NewDirectory->Parent = Directory;
	Directory->SubDirectories.push_back(NewDirectory);

	return true;
}

std::string FEVirtualFileSystem::CreateDirectory(const std::string Path)
{
	if (IsPathToFile(Path))
		return "";

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return "";

	if (Directory->IsReadOnly() || IsAnyAncestorReadOnly(Directory))
		return "";

	int Count = 1;
	const std::string NewDirectoryNameBaseName = "new directory";
	std::string NewDirectoryName = NewDirectoryNameBaseName;
	while (Directory->HasSubDirectory(NewDirectoryName) || DirectoryHasFileWithName(Directory, NewDirectoryName))
	{
		NewDirectoryName = NewDirectoryNameBaseName + "_" + std::to_string(Count);
		Count++;
	}

	FEVFSDirectory* NewDirectory = new FEVFSDirectory();
	NewDirectory->SetName(NewDirectoryName);
	NewDirectory->Parent = Directory;
	Directory->SubDirectories.push_back(NewDirectory);

	return NewDirectoryName;
}

void FEVirtualFileSystem::Clear()
{
	Root->Clear();
	Root->SetReadOnly(false);
	CurrentPath = "/";
}

bool FEVirtualFileSystem::RenameDirectory(const std::string NewName, const std::string Path)
{
	if (!AcceptableName(NewName))
		return false;

	if (IsPathToFile(Path))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	if (Directory->Parent == nullptr)
		return false;

	if (NewName == Directory->GetName())
		return true;

	if (Directory->Parent->HasSubDirectory(NewName))
		return false;

	if (DirectoryHasFileWithName(Directory->Parent, NewName))
		return false;

	if (Directory->IsReadOnly())
		return false;

	if (Directory->Parent->IsReadOnly() || IsAnyAncestorReadOnly(Directory->Parent))
		return false;

	Directory->SetName(NewName);
	return true;
}

bool FEVirtualFileSystem::AcceptableName(const std::string Name)
{
	if (Name.empty() || Name.find('/') != std::string::npos)
		return false;

	if (Name == "." || Name == "..")
		return false;

	return true;
}

std::string FEVirtualFileSystem::GetCurrentPath()
{
	return CurrentPath;
}

bool FEVirtualFileSystem::IsPathToFile(const std::string Path)
{
	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	// PathToDirectory returns the parent directory when Path names a file, so we need to check additionaly.
	std::string Normalized = Path;
	if (Normalized.size() > 1 && Normalized.back() == '/')
		Normalized.pop_back();

	return DirectoryToPath(Directory) != Normalized;
}

bool FEVirtualFileSystem::SetCurrentPath(const std::string Path)
{
	if (!IsPathCorrect(Path))
		return false;

	if (IsPathToFile(Path))
		return false;

	CurrentPath = DirectoryToPath(PathToDirectory(Path));
	return true;
}

bool FEVirtualFileSystem::MoveFile(FEObject* Data, const std::string OldPath, const std::string NewPath)
{
	if (Data == nullptr)
	{
		LOG.Add("Data is nullptr in function FEVirtualFileSystem::MoveFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (IsPathToFile(OldPath) || IsPathToFile(NewPath))
		return false;

	FEVFSDirectory* OldDirectory = PathToDirectory(OldPath);
	if (OldDirectory == nullptr)
		return false;

	FEVFSDirectory* NewDirectory = PathToDirectory(NewPath);
	if (NewDirectory == nullptr)
		return false;

	if (NewDirectory->HasFile(Data))
		return false;

	if (DirectoryHasFileWithName(NewDirectory, Data->GetName()))
		return false;

	if (NewDirectory->HasSubDirectory(Data->GetName()))
		return false;

	if (NewDirectory->IsReadOnly() || IsAnyAncestorReadOnly(NewDirectory))
		return false;

	if (OldDirectory->IsReadOnly() || IsAnyAncestorReadOnly(OldDirectory))
		return false;

	if (!OldDirectory->DeleteFile(Data))
		return false;
	NewDirectory->AddFile(Data);

	return true;
}

bool FEVirtualFileSystem::MoveDirectory(const std::string DirectoryPath, const std::string NewPath)
{
	if (IsPathToFile(DirectoryPath) || IsPathToFile(NewPath))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(DirectoryPath);
	if (Directory == nullptr)
		return false;

	FEVFSDirectory* NewDirectory = PathToDirectory(NewPath);
	if (NewDirectory == nullptr)
		return false;

	FEVFSDirectory* Ancestor = NewDirectory;
	while (Ancestor != nullptr)
	{
		if (Ancestor == Directory)
			return false;

		Ancestor = Ancestor->Parent;
	}

	if (NewDirectory->HasSubDirectory(Directory->GetName()))
		return false;

	if (DirectoryHasFileWithName(NewDirectory, Directory->GetName()))
		return false;

	if (Directory->IsReadOnly() || IsAnyAncestorReadOnly(Directory))
		return false;

	if (NewDirectory->IsReadOnly() || IsAnyAncestorReadOnly(NewDirectory))
		return false;

	if (Directory->Parent->IsReadOnly())
		return false;

	for (size_t i = 0; i < Directory->Parent->SubDirectories.size(); i++)
	{
		if (Directory->Parent->SubDirectories[i]->GetObjectID() == Directory->GetObjectID())
		{
			Directory->Parent->SubDirectories.erase(Directory->Parent->SubDirectories.begin() + i, Directory->Parent->SubDirectories.begin() + i + 1);
			break;
		}
	}

	Directory->Parent = NewDirectory;
	NewDirectory->SubDirectories.push_back(Directory);

	return true;
}

int FEVirtualFileSystem::SubDirectoriesCount(const std::string Path)
{
	if (IsPathToFile(Path))
		return 0;

	const FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return 0;

	return static_cast<int>(Directory->SubDirectories.size());
}

void FEVirtualFileSystem::DeleteDirectory(FEVFSDirectory* Directory)
{
	if (Directory == nullptr)
	{
		LOG.Add("Directory is nullptr in function FEVirtualFileSystem::DeleteDirectory.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return;
	}

	if (Directory->IsReadOnly())
		return;

	if (Directory->Parent == nullptr)
		return;

	if (Directory->Parent->IsReadOnly())
		return;

	for (size_t i = 0; i < Directory->Parent->SubDirectories.size(); i++)
	{
		if (Directory->Parent->SubDirectories[i]->GetObjectID() == Directory->GetObjectID())
		{
			Directory->Parent->SubDirectories.erase(Directory->Parent->SubDirectories.begin() + i, Directory->Parent->SubDirectories.begin() + i + 1);
			delete Directory;
			return;
		}
	}
}

bool FEVirtualFileSystem::DeleteEmptyDirectory(const std::string Path)
{
	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	if (Directory->Parent == nullptr)
		return false;

	if (!Directory->SubDirectories.empty() || !Directory->Files.empty())
		return false;

	if (Directory->IsReadOnly())
		return false;

	if (Directory->Parent->IsReadOnly() || IsAnyAncestorReadOnly(Directory->Parent))
		return false;

	DeleteDirectory(Directory);
	return true;
}

std::string FEVirtualFileSystem::DirectoryToPath(FEVFSDirectory* Directory)
{
	if (Directory == nullptr)
		return "";

	std::string Result = Directory->GetName();
	FEVFSDirectory* CurrentDirectory = Directory;

	while (CurrentDirectory->Parent != nullptr)
	{
		CurrentDirectory = CurrentDirectory->Parent;
		if (CurrentDirectory != Root)
			Result.insert(0, "/");
		
		Result.insert(0, CurrentDirectory->GetName());
	}

	return Result;
}

std::string FEVirtualFileSystem::GetDirectoryParent(const std::string Path)
{
	if (IsPathToFile(Path))
		return "";

	const FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return "";

	if (Directory->Parent == nullptr)
		return "";

	return DirectoryToPath(Directory->Parent);
}

bool FEVirtualFileSystem::DeleteFile(const FEObject* Data, const std::string Path)
{
	if (Data == nullptr)
	{
		LOG.Add("Data is nullptr in function FEVirtualFileSystem::DeleteFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	if (IsPathToFile(Path))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	if (!Directory->HasFile(Data))
		return false;

	if (Directory->IsReadOnly() || IsAnyAncestorReadOnly(Directory))
		return false;

	return Directory->DeleteFile(Data);
}

std::string FEVirtualFileSystem::LocateFileRecursive(FEVFSDirectory* Directory, FEObject* File)
{
	std::string Path = "";

	if (Directory == nullptr)
	{
		LOG.Add("Directory is nullptr in function FEVirtualFileSystem::LocateFileRecursive.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return Path;
	}

	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVirtualFileSystem::LocateFileRecursive.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return Path;
	}

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		if (Directory->Files[i].DataID == File->GetObjectID())
		{
			Path = DirectoryToPath(Directory);
			return Path;
		}
	}

	for (size_t i = 0; i < Directory->SubDirectories.size(); i++)
	{
		Path = LocateFileRecursive(Directory->SubDirectories[i], File);
		if (!Path.empty())
			return Path;
	}

	return Path;
}

std::string FEVirtualFileSystem::LocateFile(FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVirtualFileSystem::LocateFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return "";
	}

	return LocateFileRecursive(Root, File);
}

void FEVirtualFileSystem::LocateAndDeleteFile(FEObject* File)
{
	if (File == nullptr)
	{
		LOG.Add("File is nullptr in function FEVirtualFileSystem::LocateAndDeleteFile.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return;
	}

	const std::string Path = LocateFile(File);
	if (Path.empty())
		return;

	if (IsReadOnly(File, Path))
		return;

	PathToDirectory(Path)->DeleteFile(File);
}

void FEVirtualFileSystem::SaveStateRecursive(Json::Value* LocalRoot, FEVFSDirectory* Directory)
{
	LocalRoot->operator[](Directory->GetObjectID())["Name"] = Directory->GetName();
	LocalRoot->operator[](Directory->GetObjectID())["ReadOnly"] = Directory->IsReadOnly();

	Json::Value Files;
	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		Files[Directory->Files[i].DataID]["ReadOnly"] = Directory->Files[i].IsReadOnly();
	}
	LocalRoot->operator[](Directory->GetObjectID())["Files"] = Files;

	Json::Value SubDirectories;
	for (size_t i = 0; i < Directory->SubDirectories.size(); i++)
	{
		SaveStateRecursive(&SubDirectories, Directory->SubDirectories[i]);
	}

	LocalRoot->operator[](Directory->GetObjectID())["SubDirectories"] = SubDirectories;
}

void FEVirtualFileSystem::SaveState(const std::string FileName)
{
	Json::Value JsonRoot;
	std::ofstream StateFile;
	StateFile.open(FileName);

	JsonRoot["version"] = VIRTUAL_FILE_SYSTEM_VERSION;
	SaveStateRecursive(&JsonRoot, Root);

	const Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, JsonRoot);

	StateFile << JsonFile;
	StateFile.close();
}

void FEVirtualFileSystem::LoadStateRecursive(Json::Value* LocalRoot, FEVFSDirectory* Parent, FEVFSDirectory* Directory, const std::string ForceObjectID)
{
	Directory->SetIDOfUnTyped(ForceObjectID);
	Directory->SetName(LocalRoot->operator[]("Name").asCString());
	Directory->SetReadOnly(LocalRoot->operator[]("ReadOnly").asBool());
	Directory->Parent = Parent;

	const std::vector<Json::String> Files = LocalRoot->operator[]("Files").getMemberNames();
	for (size_t j = 0; j < Files.size(); j++)
	{
		if (Directory->AddFile(OBJECT_MANAGER.GetFEObject(Files[j])))
			Directory->Files.back().SetReadOnly(LocalRoot->operator[]("Files")[Files[j]]["ReadOnly"].asBool());
	}

	const std::vector<Json::String> SubDirectories = LocalRoot->operator[]("SubDirectories").getMemberNames();
	for (size_t j = 0; j < SubDirectories.size(); j++)
	{
		Directory->AddSubDirectory(LocalRoot->operator[]("SubDirectories")[SubDirectories[j]]["Name"].asCString(), SubDirectories[j]);
		LoadStateRecursive(&LocalRoot->operator[]("SubDirectories")[SubDirectories[j]], Directory, Directory->SubDirectories.back(), SubDirectories[j]);
	}
}

void FEVirtualFileSystem::LoadState(std::string FileName)
{
	std::ifstream StateFile;
	StateFile.open(FileName);

	std::string FileData((std::istreambuf_iterator<char>(StateFile)), std::istreambuf_iterator<char>());

	Json::Value JsonRoot;
	JSONCPP_STRING Err;
	Json::CharReaderBuilder Builder;

	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	if (!Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &JsonRoot, &Err))
		return;

	// read state file version
	float FileVersion = JsonRoot["version"].asFloat();
	if (FileVersion != VIRTUAL_FILE_SYSTEM_VERSION)
	{
		LOG.Add("State file version " + std::to_string(FileVersion) + " does not match current version " + std::to_string(VIRTUAL_FILE_SYSTEM_VERSION) + " in function FEVirtualFileSystem::LoadState.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		StateFile.close();
		return;
	}

	Clear();

	std::vector<Json::String> Values = JsonRoot.getMemberNames();
	for (size_t i = 0; i < Values.size(); i++)
	{
		if (Values[i] != "version")
		{
			LoadStateRecursive(&JsonRoot[Values[i]], nullptr, Root, Values[i]);
		}
	}

	StateFile.close();
}

bool FEVirtualFileSystem::IsReadOnly(const FEObject* Data, const std::string Path)
{
	if (Data == nullptr)
		return true;

	FEVFSDirectory* Directory = PathToDirectory(Path);

	if (Directory == nullptr)
		return true;

	if (Directory->IsReadOnly())
		return true;

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		if (Directory->Files[i].DataID == Data->GetObjectID())
			return Directory->Files[i].IsReadOnly();
	}

	return false;
}

void FEVirtualFileSystem::SetDirectoryReadOnly(const bool NewValue, const std::string Path)
{
	if (IsPathToFile(Path))
		return;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return;

	Directory->SetReadOnly(NewValue);
}

bool FEVirtualFileSystem::IsDirectoryReadOnly(const std::string Path)
{
	if (IsPathToFile(Path))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return false;

	return Directory->IsReadOnly();
}

void FEVirtualFileSystem::SetFileReadOnly(const bool NewValue, const FEObject* Data, const std::string Path)
{
	if (Data == nullptr)
		return;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
		return;

	for (size_t i = 0; i < Directory->Files.size(); i++)
	{
		if (Directory->Files[i].DataID == Data->GetObjectID())
		{
			Directory->Files[i].SetReadOnly(NewValue);
			return;
		}
	}
}

bool FEVirtualFileSystem::DoesFileExist(FEObject* Data, std::string Path)
{
	if (IsPathToFile(Path))
		return false;

	FEVFSDirectory* Directory = PathToDirectory(Path);
	if (Directory == nullptr)
	{
		LOG.Add("Directory is nullptr in function FEVirtualFileSystem::DoesFileExist.", "FE_VIRTUAL_FILE_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return Directory->HasFile(Data);
}

bool FEVirtualFileSystem::DoesFileExistAnywhere(FEObject* Data)
{
	return !LocateFile(Data).empty();
}

std::string FEVirtualFileSystem::GetTreeAsString()
{
	std::string Result = Root->GetName() + "\n";
	BuildTreeStringRecursive(Root, "", Result);
	return Result;
}

void FEVirtualFileSystem::BuildTreeStringRecursive(FEVFSDirectory* Directory, std::string Prefix, std::string& Output)
{
	if (Directory == nullptr)
		return;

	const size_t SubDirectoryCount = Directory->SubDirectories.size();
	const size_t FileCount = Directory->Files.size();
	const size_t TotalCount = SubDirectoryCount + FileCount;

	for (size_t i = 0; i < SubDirectoryCount; i++)
	{
		const bool bIsLast = (i + 1 == TotalCount);
		FEVFSDirectory* SubDirectory = Directory->SubDirectories[i];
		const std::string ReadOnlyTag = SubDirectory->IsReadOnly() ? " [ro]" : "";
		Output += Prefix + (bIsLast ? "\\-- " : "+-- ") + SubDirectory->GetName() + "/" + ReadOnlyTag + "\n";
		BuildTreeStringRecursive(SubDirectory, Prefix + (bIsLast ? "    " : "|   "), Output);
	}

	for (size_t i = 0; i < FileCount; i++)
	{
		const bool bIsLast = (i + 1 == FileCount);
		std::string FileDisplayName = Directory->Files[i].DataID;
		FEObject* FileObject = OBJECT_MANAGER.GetFEObject(Directory->Files[i].DataID);
		if (FileObject != nullptr)
			FileDisplayName = FileObject->GetName();

		const std::string ReadOnlyTag = Directory->Files[i].IsReadOnly() ? " [ro]" : "";
		Output += Prefix + (bIsLast ? "\\-- " : "+-- ") + FileDisplayName + ReadOnlyTag + "\n";
	}
}