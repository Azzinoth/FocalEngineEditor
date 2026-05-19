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
