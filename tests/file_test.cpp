#include "file.h"

#include <gtest/gtest.h> // for EXPECT_EQ, TestInfo (ptr only), TEST_F, Test

#include <filesystem> // for current_path, operator/, path, directory_entry

class FileTest : public testing::Test {
protected:
  // Remember that SetUp() is run immediately before a test starts.
  void SetUp() override {}

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override {}
  File file_object_1{"artifacts/dir_1/file_1"};
  File dir_object_1{"artifacts/dir_1"};
  File symlink_object_1{"artifacts/symlink_1"};
  File symlink_object_3{"artifacts/symlink_3"};
  File broken_symlink_object_1{"artifacts/broken_symlink_1"};
  File broken_symlink_object_2{"artifacts/nested_broken_symlink_1"};
  File non_existent_object_1{"non_existent_file"};
  File nested_symlink_object_1{"artifacts/symlink_5"};
};

TEST_F(FileTest, GetFileTypeTest) {
  EXPECT_EQ(file_object_1.get_file_type(), FileType::regular_file);
  EXPECT_EQ(dir_object_1.get_file_type(), FileType::regular_dir);
  EXPECT_EQ(symlink_object_1.get_file_type(), FileType::symlinked_file);
  EXPECT_EQ(symlink_object_3.get_file_type(), FileType::symlinked_dir);
  EXPECT_EQ(broken_symlink_object_1.get_file_type(), FileType::broken_symlink);
  EXPECT_EQ(broken_symlink_object_2.get_file_type(), FileType::broken_symlink);
  EXPECT_EQ(non_existent_object_1.get_file_type(), FileType::other);
}

TEST_F(FileTest, GetResolvedDirEntry) {
  EXPECT_EQ(nested_symlink_object_1.get_resolved_dir_entry(),
            std::filesystem::current_path() / "artifacts/dir_1/file_1");
  EXPECT_EQ(symlink_object_1.get_resolved_dir_entry(),
            std::filesystem::current_path() / "artifacts/dir_1/file_1");
  EXPECT_EQ(symlink_object_3.get_resolved_dir_entry(),
            std::filesystem::current_path() / "artifacts/dir_1");
}

TEST_F(FileTest, GetOriginalDirEntry) {
  EXPECT_EQ(file_object_1.dir_entry,
            std::filesystem::directory_entry{"artifacts/dir_1/file_1"});
  EXPECT_EQ(nested_symlink_object_1.dir_entry,
            std::filesystem::directory_entry{"artifacts/symlink_5"});
}
