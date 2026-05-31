#include "VirtualFileSystemTest.h"

TEST(VirtualFileSystem, AcceptableName_RejectsEmptyAndSlashesAndDots)
{
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.AcceptableName("MyAsset"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.AcceptableName(""));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.AcceptableName("Has/Slash"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.AcceptableName("."));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.AcceptableName(".."));
}

TEST(VirtualFileSystem, RootPathIsValidAndIsTheDefault)
{
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(""));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), "/");
}

TEST(VirtualFileSystem, CreateDirectory_AddsRemovableSubdirectoryAtRoot)
{
	const std::string TestName = "vfs_probe_create_delete";
	const std::string TestPath = "/" + TestName;
	const int InitialCount = VIRTUAL_FILE_SYSTEM.SubDirectoriesCount("/");

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(TestName, "/"));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount("/"), InitialCount + 1);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TestPath));

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateDirectory(TestName, "/"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateDirectory("", "/"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateDirectory(TestName, "/no_such_parent"));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TestPath));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount("/"), InitialCount);
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TestPath));
}

TEST(VirtualFileSystem, SetCurrentPath_RejectsInvalidAndAcceptsCreatedSubdirectory)
{
	const std::string OriginalPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
	const std::string TestName = "vfs_probe_navigation";
	const std::string TestPath = "/" + TestName;

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(""));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), OriginalPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.SetCurrentPath("/no_such_directory"));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), OriginalPath);

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(TestName, "/"));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(TestPath));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), TestPath);

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(OriginalPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TestPath));
}

TEST(VirtualFileSystem, RenameDirectory_RejectsSiblingNameCollision)
{
	const std::string FirstName = "vfs_rename_first";
	const std::string SecondName = "vfs_rename_second";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(FirstName, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(SecondName, "/"));

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.RenameDirectory(FirstName, "/" + SecondName));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory("/" + FirstName));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory("/" + SecondName));
}

TEST(VirtualFileSystem, MoveDirectory_RejectsMovingIntoOwnDescendant)
{
	const std::string Outer = "vfs_move_outer";
	const std::string Inner = "vfs_move_inner";
	const std::string OuterPath = "/" + Outer;
	const std::string InnerPath = OuterPath + "/" + Inner;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Outer, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Inner, OuterPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(InnerPath));

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveDirectory(OuterPath, InnerPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(OuterPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(InnerPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(InnerPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(OuterPath));
}

TEST(VirtualFileSystem, Root_HasNoParent)
{
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetDirectoryParent("/"), "");
}

TEST(VirtualFileSystem, RenameDirectory_RejectsRenamingRoot)
{
	const std::string NewName = "vfs_renamed_root";
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.RenameDirectory(NewName, "/"));
}

TEST(VirtualFileSystem, MoveDirectory_CompletesLegitimateMove)
{
	const std::string Source = "vfs_move_src";
	const std::string Target = "vfs_move_tgt";
	const std::string SourcePath = "/" + Source;
	const std::string TargetPath = "/" + Target;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Source, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.MoveDirectory(SourcePath, TargetPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath + "/" + Source));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, LoadState_ReplacesExistingState)
{
	const std::string SavedDir = "vfs_load_saved";
	const std::string ExtraDir = "vfs_load_extra";
	const std::string SaveFile = "vfs_load_state_clear_test.txt";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(SavedDir, "/"));
	VIRTUAL_FILE_SYSTEM.SaveState(SaveFile);

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(ExtraDir, "/"));

	VIRTUAL_FILE_SYSTEM.LoadState(SaveFile);

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/" + SavedDir));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/" + ExtraDir));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory("/" + SavedDir));
	std::remove(SaveFile.c_str());
}

TEST(VirtualFileSystem, CreateDirectory_RejectsReadOnlyParent)
{
	const std::string Parent = "vfs_read_only_create_parent";
	const std::string Child = "vfs_read_only_create_child";
	const std::string ParentPath = "/" + Parent;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Parent, "/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, ParentPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, ParentPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ParentPath + "/" + Child));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, ParentPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ParentPath));
}

TEST(VirtualFileSystem, MoveDirectory_RejectsReadOnlyTarget)
{
	const std::string Source = "vfs_read_only_move_src";
	const std::string Target = "vfs_read_only_move_tgt";
	const std::string SourcePath = "/" + Source;
	const std::string TargetPath = "/" + Target;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Source, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, TargetPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveDirectory(SourcePath, TargetPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(SourcePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TargetPath + "/" + Source));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, TargetPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(SourcePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, SaveState_PersistsFileReadOnlyFlag)
{
	const std::string Directory = "vfs_save_file_read_only_dir";
	const std::string DirPath = "/" + Directory;
	const std::string SaveFile = "vfs_save_file_read_only_test.txt";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_test_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirPath));
	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(true, TestFile, DirPath);

	VIRTUAL_FILE_SYSTEM.SaveState(SaveFile);

	std::ifstream In(SaveFile);
	const std::string Data((std::istreambuf_iterator<char>(In)), std::istreambuf_iterator<char>());
	In.close();

	Json::Value Saved;
	JSONCPP_STRING Error;
	Json::CharReaderBuilder Builder;
	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	ASSERT_TRUE(Reader->parse(Data.c_str(), Data.c_str() + Data.size(), &Saved, &Error));

	const std::string FileID = TestFile->GetObjectID();
	std::function<const Json::Value*(const Json::Value&)> Find = [&](const Json::Value& Node) -> const Json::Value* {
		if (Node.isObject())
		{
			for (const auto& Key : Node.getMemberNames())
			{
				if (Key == FileID)
					return &Node[Key];

				if (const Json::Value* Found = Find(Node[Key]))
					return Found;
			}
		}

		return nullptr;
	};

	const Json::Value* FileEntry = Find(Saved);
	ASSERT_NE(FileEntry, nullptr);
	EXPECT_TRUE(FileEntry->isObject());
	EXPECT_TRUE((*FileEntry)["ReadOnly"].asBool());

	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(false, TestFile, DirPath);
	VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirPath);
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirPath));
	std::remove(SaveFile.c_str());
}

TEST(VirtualFileSystem, RenameDirectory_RejectsReadOnlyParent)
{
	const std::string Parent = "vfs_read_only_rename_parent";
	const std::string Child = "vfs_read_only_rename_child";
	const std::string NewChildName = "vfs_read_only_rename_renamed";
	const std::string ParentPath = "/" + Parent;
	const std::string ChildPath = ParentPath + "/" + Child;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Parent, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, ParentPath));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, ParentPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.RenameDirectory(NewChildName, ChildPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ChildPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ParentPath + "/" + NewChildName));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, ParentPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ChildPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ParentPath));
}

TEST(VirtualFileSystem, DoesFileExist_RejectsFilePath)
{
	const std::string Directory = "vfs_doesfileexist_dir";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TargetFile = new FEObject(FE_TEXTURE, "vfs_doesfileexist_target");
	FEObject* DecorativeFile = new FEObject(FE_TEXTURE, "vfs_doesfileexist_decorative");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TargetFile, DirectoryPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(DecorativeFile, DirectoryPath));

	const std::string MismatchedPath = DirectoryPath + "/" + DecorativeFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(TargetFile, MismatchedPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TargetFile, DirectoryPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(DecorativeFile, DirectoryPath));
	delete TargetFile;
	delete DecorativeFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, GetDirectoryContentIDs_RejectsFilePath)
{
	const std::string Holder = "vfs_contentids_holder";
	const std::string Child = "vfs_contentids_child";
	const std::string HolderPath = "/" + Holder;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, HolderPath));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_contentids_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, HolderPath));

	const std::string FilePath = HolderPath + "/" + TestFile->GetName();
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(FilePath).empty());

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, HolderPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath + "/" + Child));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, SubDirectoriesCount_RejectsFilePath)
{
	const std::string Holder = "vfs_subdircount_holder";
	const std::string Child = "vfs_subdircount_child";
	const std::string HolderPath = "/" + Holder;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, HolderPath));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_subdircount_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, HolderPath));

	const std::string FilePath = HolderPath + "/" + TestFile->GetName();
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(FilePath), 0);

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, HolderPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath + "/" + Child));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, GetDirectoryParent_RejectsFilePath)
{
	const std::string Outer = "vfs_directory_parent_outer";
	const std::string Inner = "vfs_directory_parent_inner";
	const std::string OuterPath = "/" + Outer;
	const std::string InnerPath = OuterPath + "/" + Inner;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Outer, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Inner, OuterPath));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_directory_parent_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, InnerPath));

	const std::string FilePath = InnerPath + "/" + TestFile->GetName();
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetDirectoryParent(FilePath), "");

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, InnerPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(InnerPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(OuterPath));
}

TEST(VirtualFileSystem, IsDirectoryReadOnly_RejectsFilePath)
{
	const std::string Directory = "vfs_isdirreadonly_file_path_dir";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_isdirreadonly_file_path_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirectoryPath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, DirectoryPath);

	const std::string FilePath = DirectoryPath + "/" + TestFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(FilePath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, DirectoryPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirectoryPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, DeleteFile_RejectsFilePath)
{
	const std::string Directory = "vfs_delete_file_path_dir";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TargetFile = new FEObject(FE_TEXTURE, "vfs_delete_target_file");
	FEObject* DecorativeFile = new FEObject(FE_TEXTURE, "vfs_delete_decorative_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TargetFile, DirectoryPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(DecorativeFile, DirectoryPath));

	const std::string MismatchedPath = DirectoryPath + "/" + DecorativeFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DeleteFile(TargetFile, MismatchedPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(TargetFile, DirectoryPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TargetFile, DirectoryPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(DecorativeFile, DirectoryPath));
	delete TargetFile;
	delete DecorativeFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, CreateFile_RejectsSubdirectoryNameCollision)
{
	const std::string Holder = "vfs_create_file_subdirname_holder";
	const std::string HolderPath = "/" + Holder;
	const std::string SharedName = "vfs_create_file_subdirname_shared";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(SharedName, HolderPath));

	FEObject* CollidingFile = new FEObject(FE_TEXTURE, SharedName);
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateFile(CollidingFile, HolderPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(CollidingFile, HolderPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(HolderPath + "/" + SharedName));

	delete CollidingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath + "/" + SharedName));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, MoveFile_RejectsSubdirectoryNameCollisionInTarget)
{
	const std::string Source = "vfs_move_file_subdirname_src";
	const std::string Target = "vfs_move_file_subdirname_tgt";
	const std::string SourcePath = "/" + Source;
	const std::string TargetPath = "/" + Target;
	const std::string SharedName = "vfs_move_file_subdirname_shared";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Source, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(SharedName, TargetPath));

	FEObject* MovingFile = new FEObject(FE_TEXTURE, SharedName);
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(MovingFile, SourcePath));

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveFile(MovingFile, SourcePath, TargetPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(MovingFile, SourcePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(MovingFile, TargetPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TargetPath + "/" + SharedName));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(MovingFile, SourcePath));
	delete MovingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath + "/" + SharedName));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(SourcePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, RenameDirectory_RejectsFileNameCollisionInParent)
{
	const std::string Holder = "vfs_rename_dir_filename_holder";
	const std::string SubDir = "vfs_rename_dir_filename_sub";
	const std::string NewName = "vfs_rename_dir_filename_target";
	const std::string HolderPath = "/" + Holder;
	const std::string SubDirPath = HolderPath + "/" + SubDir;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(SubDir, HolderPath));

	FEObject* CollidingFile = new FEObject(FE_TEXTURE, NewName);
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(CollidingFile, HolderPath));

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.RenameDirectory(NewName, SubDirPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(SubDirPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(CollidingFile, HolderPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(CollidingFile, HolderPath));
	delete CollidingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(SubDirPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, MoveDirectory_RejectsFileNameCollisionInTarget)
{
	const std::string Source = "vfs_move_dir_filename_src";
	const std::string Target = "vfs_move_dir_filename_tgt";
	const std::string SourcePath = "/" + Source;
	const std::string TargetPath = "/" + Target;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Source, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));

	FEObject* CollidingFile = new FEObject(FE_TEXTURE, Source);
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(CollidingFile, TargetPath));

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveDirectory(SourcePath, TargetPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(SourcePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(CollidingFile, TargetPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(CollidingFile, TargetPath));
	delete CollidingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(SourcePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, CreateDirectory_RejectsFileNameCollision)
{
	const std::string Holder = "vfs_create_dir_filename_holder";
	const std::string HolderPath = "/" + Holder;
	const std::string SharedName = "vfs_create_dir_filename_shared";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));

	FEObject* ExistingFile = new FEObject(FE_TEXTURE, SharedName);
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(ExistingFile, HolderPath));

	const int BeforeCount = VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(HolderPath);
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateDirectory(SharedName, HolderPath));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(HolderPath), BeforeCount);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(ExistingFile, HolderPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(ExistingFile, HolderPath));
	delete ExistingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, CreateDirectoryAutoName_SkipsFileNameCollision)
{
	const std::string Holder = "vfs_create_dir_autoname_holder";
	const std::string HolderPath = "/" + Holder;
	const std::string DefaultName = "new directory";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));

	FEObject* ExistingFile = new FEObject(FE_TEXTURE, DefaultName);
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(ExistingFile, HolderPath));

	const std::string Created = VIRTUAL_FILE_SYSTEM.CreateDirectory(HolderPath);
	EXPECT_FALSE(Created.empty());
	EXPECT_NE(Created, DefaultName);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(ExistingFile, HolderPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath + "/" + Created));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(ExistingFile, HolderPath));
	delete ExistingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, CreateFile_RejectsDuplicateNameInSameDirectory)
{
	const std::string Directory = "vfs_dup_name_directory";
	const std::string DirectoryPath = "/" + Directory;
	const std::string SharedName = "vfs_duplicate_name_shared";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* FirstFile = new FEObject(FE_TEXTURE, SharedName);
	FEObject* SecondFile = new FEObject(FE_TEXTURE, SharedName);

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(FirstFile, DirectoryPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateFile(SecondFile, DirectoryPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(FirstFile, DirectoryPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(SecondFile, DirectoryPath));

	VIRTUAL_FILE_SYSTEM.DeleteFile(SecondFile, DirectoryPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(FirstFile, DirectoryPath));
	delete FirstFile;
	delete SecondFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, CreateFile_RejectsDuplicateInDifferentDirectory)
{
	const std::string DirectoryA = "vfs_dup_file_dir_a";
	const std::string DirectoryB = "vfs_dup_file_dir_b";
	const std::string DirectoryAPath = "/" + DirectoryA;
	const std::string DirectoryBPath = "/" + DirectoryB;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(DirectoryA, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(DirectoryB, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_dup_file_subject");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirectoryAPath));

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirectoryBPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(TestFile, DirectoryBPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirectoryAPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryAPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryBPath));
}

TEST(VirtualFileSystem, IsReadOnly_DefensiveOnInvalidPath)
{
	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_isreadonly_root_leak_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, "/"));

	// File is writable. Invalid path should still report read-only (defensive).
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsReadOnly(TestFile, "/no_such_path_for_root_leak"));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, "/"));
	delete TestFile;
}

TEST(VirtualFileSystem, MoveFile_RejectsFilePath)
{
	const std::string Source = "vfs_move_file_dest_src";
	const std::string Target = "vfs_move_file_dest_target_holder";
	const std::string SourcePath = "/" + Source;
	const std::string TargetPath = "/" + Target;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Source, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));

	FEObject* MovingFile = new FEObject(FE_TEXTURE, "vfs_move_file_dest_moving");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(MovingFile, SourcePath));

	FEObject* TargetFile = new FEObject(FE_TEXTURE, "vfs_move_file_dest_target_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TargetFile, TargetPath));

	const std::string FilePath = TargetPath + "/" + TargetFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveFile(MovingFile, SourcePath, FilePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(MovingFile, SourcePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(MovingFile, TargetPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(MovingFile, SourcePath));
	delete MovingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TargetFile, TargetPath));
	delete TargetFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(SourcePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, SetFileReadOnly_IgnoresNullData)
{
	const std::string Directory = "vfs_setfilereadonly_null_dir";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_setfilereadonly_null_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirectoryPath));

	// Should no-op (no file matches a null Data), not crash.
	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(true, nullptr, DirectoryPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsReadOnly(TestFile, DirectoryPath));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirectoryPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, IsReadOnly_HandlesNullDataAcrossPaths)
{
	FEObject* RootFile = new FEObject(FE_TEXTURE, "vfs_isreadonly_root_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(RootFile, "/"));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsReadOnly(nullptr, "/no_such_path_for_isreadonly"));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsReadOnly(nullptr, "/"));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(RootFile, "/"));
	delete RootFile;
}

TEST(VirtualFileSystem, RenameDirectory_RejectsFilePath)
{
	const std::string Holder = "vfs_rename_dir_file_path_holder";
	const std::string NewName = "vfs_rename_dir_file_path_renamed";
	const std::string HolderPath = "/" + Holder;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_rename_dir_file_path_target");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, HolderPath));

	const std::string FilePath = HolderPath + "/" + TestFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.RenameDirectory(NewName, FilePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(HolderPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/" + NewName));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, HolderPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, CreateFile_RejectsFilePathAsTarget)
{
	const std::string Holder = "vfs_create_file_file_path_holder";
	const std::string HolderPath = "/" + Holder;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));

	FEObject* ExistingFile = new FEObject(FE_TEXTURE, "vfs_create_file_file_path_existing");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(ExistingFile, HolderPath));

	const std::string FilePath = HolderPath + "/" + ExistingFile->GetName();
	FEObject* NewFile = new FEObject(FE_TEXTURE, "vfs_create_file_file_path_new");

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateFile(NewFile, FilePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(NewFile, HolderPath));

	delete NewFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(ExistingFile, HolderPath));
	delete ExistingFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, CreateDirectory_RejectsFilePathAsParent)
{
	const std::string Holder = "vfs_create_dir_file_path_holder";
	const std::string NewChild = "vfs_create_dir_file_path_child";
	const std::string HolderPath = "/" + Holder;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Holder, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_create_dir_file_path_target");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, HolderPath));

	const std::string FilePath = HolderPath + "/" + TestFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateDirectory(NewChild, FilePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(HolderPath + "/" + NewChild));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, HolderPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(HolderPath));
}

TEST(VirtualFileSystem, SetDirectoryReadOnly_RejectsFilePath)
{
	const std::string Directory = "vfs_set_read_only_file_path_holder";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_set_read_only_file_path_target");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirectoryPath));

	const std::string FilePath = DirectoryPath + "/" + TestFile->GetName();
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, FilePath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(DirectoryPath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, DirectoryPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirectoryPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, MoveDirectory_RejectsFilePathAsDestination)
{
	const std::string Source = "vfs_move_dest_src";
	const std::string Target = "vfs_move_dest_target_holder";
	const std::string SourcePath = "/" + Source;
	const std::string TargetPath = "/" + Target;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Source, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));

	FEObject* TargetFile = new FEObject(FE_TEXTURE, "vfs_move_dest_target_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TargetFile, TargetPath));

	const std::string FilePath = TargetPath + "/" + TargetFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveDirectory(SourcePath, FilePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(SourcePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TargetPath + "/" + Source));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TargetFile, TargetPath));
	delete TargetFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(SourcePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, Clear_ResetsCurrentPathToRoot)
{
	const std::string Directory = "vfs_clear_current_path_holder";
	const std::string DirectoryPath = "/" + Directory;
	const std::string OriginalPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(DirectoryPath));

	VIRTUAL_FILE_SYSTEM.Clear();

	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), "/");
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));

	VIRTUAL_FILE_SYSTEM.SetCurrentPath(OriginalPath);
}

TEST(VirtualFileSystem, SetCurrentPath_RejectsFilePath)
{
	const std::string Directory = "vfs_current_path_file_holder";
	const std::string DirectoryPath = "/" + Directory;
	const std::string OriginalPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_current_path_file_target");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirectoryPath));

	const std::string FilePath = DirectoryPath + "/" + TestFile->GetName();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(FilePath));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), OriginalPath);

	VIRTUAL_FILE_SYSTEM.SetCurrentPath(OriginalPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirectoryPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, CreateFile_RejectsReadOnlyDirectory)
{
	const std::string Directory = "vfs_create_file_read_only_dir";
	const std::string DirPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, DirPath);

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_create_file_read_only_test");
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(TestFile, DirPath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, DirPath);
	VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirPath);
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirPath));
}

TEST(VirtualFileSystem, MoveFile_RejectsReadOnlyTarget)
{
	const std::string Source = "vfs_move_file_src";
	const std::string Target = "vfs_move_file_tgt";
	const std::string SourcePath = "/" + Source;
	const std::string TargetPath = "/" + Target;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Source, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_move_file_test");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, SourcePath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, TargetPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveFile(TestFile, SourcePath, TargetPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(TestFile, SourcePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DoesFileExist(TestFile, TargetPath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, TargetPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, SourcePath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(SourcePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, DeleteFile_ReportsFailureOnReadOnlyFile)
{
	const std::string Directory = "vfs_delete_read_only_dir";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* TestFile = new FEObject(FE_TEXTURE, "vfs_delete_read_only_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(TestFile, DirectoryPath));
	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(true, TestFile, DirectoryPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirectoryPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(TestFile, DirectoryPath));

	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(false, TestFile, DirectoryPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(TestFile, DirectoryPath));
	delete TestFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, SaveState_PersistsReadOnlyFlag)
{
	const std::string Directory = "vfs_save_read_only_dir";
	const std::string DirPath = "/" + Directory;
	const std::string SaveFile = "vfs_save_read_only_test.txt";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, DirPath);
	VIRTUAL_FILE_SYSTEM.SaveState(SaveFile);

	VIRTUAL_FILE_SYSTEM.LoadState(SaveFile);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.CreateDirectory("child", DirPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(DirPath + "/child"));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, DirPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirPath));
	std::remove(SaveFile.c_str());
}

TEST(VirtualFileSystem, DeleteEmptyDirectory_RejectsReadOnlyParent)
{
	const std::string Parent = "vfs_read_only_del_parent";
	const std::string Child = "vfs_read_only_del_child";
	const std::string ParentPath = "/" + Parent;
	const std::string ChildPath = ParentPath + "/" + Child;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Parent, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, ParentPath));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, ParentPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ChildPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ChildPath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, ParentPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ChildPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ParentPath));
}

TEST(VirtualFileSystem, MoveDirectory_RejectsReadOnlySourceParent)
{
	const std::string Parent = "vfs_read_only_src_parent";
	const std::string Child = "vfs_read_only_src_child";
	const std::string Target = "vfs_read_only_src_tgt";
	const std::string ParentPath = "/" + Parent;
	const std::string ChildPath = ParentPath + "/" + Child;
	const std::string TargetPath = "/" + Target;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Parent, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, ParentPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Target, "/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, ParentPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveDirectory(ChildPath, TargetPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ChildPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TargetPath + "/" + Child));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, ParentPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ChildPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ParentPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TargetPath));
}

TEST(VirtualFileSystem, SaveLoad_FidelityRoundTripWithMixedTree)
{
	const std::string TreeRoot = "vfs_fidelity_root";
	const std::string TreeRootPath = "/" + TreeRoot;
	const std::string PathA = TreeRootPath + "/a";
	const std::string PathAA = PathA + "/aa";
	const std::string PathB = TreeRootPath + "/b";
	const std::string PathBA = PathB + "/ba";
	const std::string SaveFile = "vfs_fidelity_test.txt";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(TreeRoot, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory("a", TreeRootPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory("aa", PathA));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory("b", TreeRootPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory("ba", PathB));

	FEObject* FileAA1 = new FEObject(FE_TEXTURE, "vfs_fidelity_file_aa1");
	FEObject* FileA1 = new FEObject(FE_TEXTURE, "vfs_fidelity_file_a1");
	FEObject* FileBA1 = new FEObject(FE_TEXTURE, "vfs_fidelity_file_ba1");
	FEObject* FileRoot1 = new FEObject(FE_TEXTURE, "vfs_fidelity_file_root1");
	FEObject* FileRoot2 = new FEObject(FE_TEXTURE, "vfs_fidelity_file_root2");

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(FileAA1, PathAA));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(FileA1, PathA));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(FileBA1, PathBA));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(FileRoot1, TreeRootPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(FileRoot2, TreeRootPath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, PathA);
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, PathBA);
	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(true, FileBA1, PathBA);
	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(true, FileRoot2, TreeRootPath);

	VIRTUAL_FILE_SYSTEM.SaveState(SaveFile);

	// Mutate live state to confirm load wipes it.
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory("vfs_fidelity_extra", "/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, PathA);

	VIRTUAL_FILE_SYSTEM.LoadState(SaveFile);

	// Structure.
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TreeRootPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(PathA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(PathAA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(PathB));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(PathBA));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/vfs_fidelity_extra"));

	// File membership.
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(FileAA1, PathAA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(FileA1, PathA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(FileBA1, PathBA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(FileRoot1, TreeRootPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DoesFileExist(FileRoot2, TreeRootPath));

	// Directory read-only flags.
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(PathA));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(PathAA));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(PathB));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly(PathBA));

	// File read-only flags.
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsReadOnly(FileBA1, PathBA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsReadOnly(FileRoot2, TreeRootPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsReadOnly(FileRoot1, TreeRootPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsReadOnly(FileAA1, PathAA));

	// Cleanup: unset every read-only flag, then delete bottom-up.
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, PathA);
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, PathBA);
	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(false, FileBA1, PathBA);
	VIRTUAL_FILE_SYSTEM.SetFileReadOnly(false, FileRoot2, TreeRootPath);

	// Now that /a is writable again we can probe FileA1's own flag in isolation.
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsReadOnly(FileA1, PathA));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(FileAA1, PathAA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(FileA1, PathA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(FileBA1, PathBA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(FileRoot1, TreeRootPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(FileRoot2, TreeRootPath));
	delete FileAA1;
	delete FileA1;
	delete FileBA1;
	delete FileRoot1;
	delete FileRoot2;

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(PathAA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(PathA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(PathBA));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(PathB));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(TreeRootPath));
	std::remove(SaveFile.c_str());
}

TEST(VirtualFileSystem, PathFormEquivalence_AcceptsCanonicalRejectsMalformed)
{
	const std::string TestName = "vfs_path_form_test";
	const std::string Canonical = "/" + TestName;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(TestName, "/"));

	// Accepted forms, all naming /TestName.
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(Canonical));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(Canonical + "/"));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/"));

	// Rejected forms,none of these should resolve.
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(""));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("//"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("//" + TestName));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(Canonical + "//"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/" + TestName + "//child"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TestName));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(TestName + "/"));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(Canonical));
}

TEST(VirtualFileSystem, ManySiblings_FlatLargeDirectoryRoundTrips)
{
	const int Count = 1000;
	const std::string Parent = "vfs_many_siblings_parent";
	const std::string ParentPath = "/" + Parent;
	const std::string Prefix = "child_";
	const std::string SaveFile = "vfs_many_siblings_test.txt";

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Parent, "/"));

	for (int i = 0; i < Count; i++)
		ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Prefix + std::to_string(i), ParentPath));

	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(ParentPath), Count);
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs(ParentPath).size(), static_cast<size_t>(Count));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ParentPath + "/" + Prefix + "0"));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ParentPath + "/" + Prefix + std::to_string(Count / 2)));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ParentPath + "/" + Prefix + std::to_string(Count - 1)));

	VIRTUAL_FILE_SYSTEM.SaveState(SaveFile);
	VIRTUAL_FILE_SYSTEM.LoadState(SaveFile);

	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(ParentPath), Count);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ParentPath + "/" + Prefix + std::to_string(Count - 1)));

	for (int i = 0; i < Count; i++)
		EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ParentPath + "/" + Prefix + std::to_string(i)));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ParentPath));
	std::remove(SaveFile.c_str());
}

TEST(VirtualFileSystem, DeepNesting_PathOperationsHandleLongChains)
{
	const int Depth = 100;
	const std::string Prefix = "vfs_deep_";

	std::vector<std::string> Paths;
	std::string ParentPath = "/";
	for (int i = 0; i < Depth; i++)
	{
		const std::string Name = Prefix + std::to_string(i);
		ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Name, ParentPath));

		const std::string FullPath = (ParentPath == "/" ? "" : ParentPath) + "/" + Name;
		Paths.push_back(FullPath);
		ParentPath = FullPath;
	}

	const std::string DeepestPath = Paths.back();
	const std::string SecondDeepest = Paths[Depth - 2];

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(DeepestPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(DeepestPath));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), DeepestPath);
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetDirectoryParent(DeepestPath), SecondDeepest);

	const std::string SaveFile = "vfs_deep_nesting_test.txt";
	VIRTUAL_FILE_SYSTEM.SaveState(SaveFile);
	VIRTUAL_FILE_SYSTEM.LoadState(SaveFile);

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(DeepestPath));
	EXPECT_EQ(VIRTUAL_FILE_SYSTEM.GetDirectoryParent(DeepestPath), SecondDeepest);

	VIRTUAL_FILE_SYSTEM.SetCurrentPath("/");
	for (int i = Depth - 1; i >= 0; i--)
		EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(Paths[i]));
	std::remove(SaveFile.c_str());
}

TEST(VirtualFileSystem, DeleteEmptyDirectory_RejectsRoot)
{
	ASSERT_FALSE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly("/"));
	const std::vector<std::string> InitialContents = VIRTUAL_FILE_SYSTEM.GetDirectoryContentIDs("/");

	// If root is not empty, skip.
	if (!InitialContents.empty())
		return;

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory("/"));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect("/"));
}

TEST(VirtualFileSystem, Clear_ResetsRootReadOnlyFlag)
{
	ASSERT_FALSE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly("/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, "/");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly("/"));
	VIRTUAL_FILE_SYSTEM.Clear();
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsDirectoryReadOnly("/"));
}

TEST(VirtualFileSystem, CreateFile_RejectsNameContainingSlash)
{
	const std::string Directory = "vfs_create_slash_dir";
	const std::string DirectoryPath = "/" + Directory;
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* Bad = new FEObject(FE_TEXTURE, "has/slash");
	const bool Accepted = VIRTUAL_FILE_SYSTEM.CreateFile(Bad, DirectoryPath);
	if (Accepted)
		VIRTUAL_FILE_SYSTEM.DeleteFile(Bad, DirectoryPath);
	delete Bad;
	EXPECT_FALSE(Accepted);

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, CreateFile_RejectsEmptyName)
{
	const std::string Directory = "vfs_create_empty_dir";
	const std::string DirectoryPath = "/" + Directory;
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* Bad = new FEObject(FE_TEXTURE, "");
	const bool Accepted = VIRTUAL_FILE_SYSTEM.CreateFile(Bad, DirectoryPath);
	if (Accepted)
		VIRTUAL_FILE_SYSTEM.DeleteFile(Bad, DirectoryPath);
	delete Bad;
	EXPECT_FALSE(Accepted);

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, ReadOnly_AncestorBlocksCreateInDescendant)
{
	const std::string Parent = "vfs_ro_parent";
	const std::string Child = "vfs_ro_child";
	const std::string ParentPath = "/" + Parent;
	const std::string ChildPath = ParentPath + "/" + Child;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Parent, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, ParentPath));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, ParentPath);

	FEObject* CurrentFile = new FEObject(FE_TEXTURE, "vfs_ro_smuggled");
	const bool Created = VIRTUAL_FILE_SYSTEM.CreateFile(CurrentFile, ChildPath);
	EXPECT_FALSE(Created);

	if (Created)
		VIRTUAL_FILE_SYSTEM.DeleteFile(CurrentFile, ChildPath);
	delete CurrentFile;
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, ParentPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ChildPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ParentPath));
}

TEST(VirtualFileSystem, ReadOnly_AncestorBlocksMoveOutOfLockedSubtree)
{
	const std::string Locked = "vfs_ro_locked";
	const std::string Inner = "vfs_ro_inner";
	const std::string Leaf = "vfs_ro_leaf";
	const std::string Elsewhere = "vfs_ro_elsewhere";
	const std::string LockedPath = "/" + Locked;
	const std::string InnerPath = LockedPath + "/" + Inner;
	const std::string LeafPath = InnerPath + "/" + Leaf;
	const std::string ElsewherePath = "/" + Elsewhere;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Locked, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Inner, LockedPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Leaf, InnerPath));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Elsewhere, "/"));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, LockedPath);

	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.MoveDirectory(LeafPath, ElsewherePath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(LeafPath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(ElsewherePath + "/" + Leaf));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(false, LockedPath);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(LeafPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(InnerPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(LockedPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(ElsewherePath));
}

TEST(VirtualFileSystem, SetCurrentPath_CanonicalizesTrailingSlash)
{
	const std::string Directory = "vfs_cp_canon_dir";
	const std::string DirectoryPath = "/" + Directory;
	const std::string Original = VIRTUAL_FILE_SYSTEM.GetCurrentPath();

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(DirectoryPath + "/"));
	const std::string Stored = VIRTUAL_FILE_SYSTEM.GetCurrentPath();

	VIRTUAL_FILE_SYSTEM.SetCurrentPath(Original);
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));

	EXPECT_EQ(Stored, DirectoryPath);
}

TEST(VirtualFileSystem, IsPathCorrect_RejectsTrailingSlashOnFilePath)
{
	const std::string Directory = "vfs_path_trail_dir";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));

	FEObject* CurrentFile = new FEObject(FE_TEXTURE, "vfs_trail_file");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(CurrentFile, DirectoryPath));
	const std::string FilePath = DirectoryPath + "/" + CurrentFile->GetName();

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(FilePath));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(FilePath + "/"));

	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteFile(CurrentFile, DirectoryPath));
	delete CurrentFile;
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
}

TEST(VirtualFileSystem, LocateAndDeleteFile_HonorsReadOnlyAncestor)
{
	VIRTUAL_FILE_SYSTEM.Clear();

	const std::string Ancestor = "Ancestor";
	const std::string Child = "Child";
	const std::string AncestorPath = "/" + Ancestor;
	const std::string ChildPath = AncestorPath + "/" + Child;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Ancestor, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Child, AncestorPath));

	FEObject* File = new FEObject(FE_TEXTURE, "LockedAsset");
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateFile(File, ChildPath));

	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, AncestorPath);
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.LocateAndDeleteFile(File));

	delete File;
	VIRTUAL_FILE_SYSTEM.Clear();
}

TEST(VirtualFileSystem, CurrentPath_StaysValidAfterDeletingCurrentDirectory)
{
	VIRTUAL_FILE_SYSTEM.Clear();

	const std::string Original = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
	const std::string Directory = "SomeDirectory";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));
	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.SetCurrentPath(DirectoryPath));
	ASSERT_EQ(VIRTUAL_FILE_SYSTEM.GetCurrentPath(), DirectoryPath);

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.DeleteEmptyDirectory(DirectoryPath));
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.IsPathCorrect(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));

	VIRTUAL_FILE_SYSTEM.Clear();
}

TEST(VirtualFileSystem, RenameDirectory_OnReadOnlyDirectory_ReportsFailure)
{
	VIRTUAL_FILE_SYSTEM.Clear();

	const std::string Directory = "ReadOnlyDirectory";
	const std::string DirectoryPath = "/" + Directory;

	ASSERT_TRUE(VIRTUAL_FILE_SYSTEM.CreateDirectory(Directory, "/"));
	VIRTUAL_FILE_SYSTEM.SetDirectoryReadOnly(true, DirectoryPath);
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.RenameDirectory(Directory, DirectoryPath));

	VIRTUAL_FILE_SYSTEM.Clear();
}