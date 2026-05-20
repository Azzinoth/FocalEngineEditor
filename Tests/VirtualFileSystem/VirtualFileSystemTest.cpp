#include "VirtualFileSystemTest.h"

TEST(VirtualFileSystem, AcceptableName_RejectsEmptyAndSlashes)
{
	EXPECT_TRUE(VIRTUAL_FILE_SYSTEM.AcceptableName("MyAsset"));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.AcceptableName(""));
	EXPECT_FALSE(VIRTUAL_FILE_SYSTEM.AcceptableName("Has/Slash"));
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