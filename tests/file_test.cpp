#include "file.h"

#include <gtest/gtest.h> // for EXPECT_EQ, TestInfo (ptr only), TEST_F, Test

#include <filesystem> // for current_path, operator/, path, directory_entry
#include <fstream>
#include <regex>

#include "debug.h"
#include "filter.h"

using namespace std;
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
  File sudo_file{"/etc/security/opasswd"};
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

TEST_F(FileTest, CheckFileOrLog) {
  std::set<FileType> accepted = {FileType::symlinked_file,
                                 FileType::regular_file};
  std::string spdlog_test_file = "spdlog_file_test.txt";
  std::shared_ptr<spdlog::logger> bt_spdlog =
      spdlog::basic_logger_mt("basic_logger", spdlog_test_file);

  std::shared_ptr<spdlog::logger> prev_logger = spdlog::default_logger();
  spdlog::flush_on(spdlog::level::warn);
  spdlog::set_default_logger(bt_spdlog);
  // redirecting stdout to a file doesn't work.
  EXPECT_FALSE(broken_symlink_object_1.check_file_or_log(accepted));
  EXPECT_FALSE(dir_object_1.check_file_or_log(accepted));

  ifstream spdlog_file{spdlog_test_file, std::ios::in};
  EXPECT_TRUE(spdlog_file.is_open());
  std::smatch sm;
  std::string line;
  FileVector logged_files;

  for (int i = 0; i < 2; ++i) {
    std::getline(spdlog_file, line, '\n');
    auto res = std::regex_search(
        line, sm,
        std::regex{
            R"(\[\S+ \S+\] \[basic_logger\] \[warning\] Path not a file or a symlink to a file, skipping: (\S+))"});
    EXPECT_TRUE(res && sm[1].matched);
    logged_files.emplace_back(std::make_shared<File>(sm.str(1)));
  }
  FileVector expected_files = {make_shared<File>(broken_symlink_object_1),
                               make_shared<File>(dir_object_1)};
  EXPECT_EQ(logged_files, expected_files);
  const char *inside_docker = std::getenv("INSIDE_DOCKER");

  if (inside_docker == nullptr || std::strcmp(inside_docker, "1")) {
    EXPECT_FALSE(sudo_file.check_file_or_log(accepted));
  }
  std::getline(spdlog_file, line, '\n');
  auto res = std::regex_search(
      line, sm,
      std::regex{
          R"(\[\S+ \S+\] \[basic_logger\] \[warning\] Could not open file, skipping: /etc/security/opasswd)"});

  if (inside_docker == nullptr || std::strcmp(inside_docker, "1")) {
    EXPECT_TRUE(res);
  }
  spdlog::set_default_logger(prev_logger);
  std::filesystem::remove(spdlog_test_file);
  spdlog_file.close();
}
