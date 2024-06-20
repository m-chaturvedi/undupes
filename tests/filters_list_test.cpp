#include "filters_list.h" // for xxhash, fs, file_size

#include <gtest/gtest.h> // for TestInfo (ptr only), EXPECT_EQ, TEST_F

#include <exception>
#include <filesystem> // for recursive_directory_iterator, begin
#include <ostream>    // for operator<<
#include <string>

#include "debug.h"

using namespace std;
class FiltersListTest : public testing::Test {
protected:
  // Remember that SetUp() is run immediately before a test starts.
  void SetUp() override {}

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override {}
  void read_dir(const std::string &dir, FileSets &_file_sets){};
};

TEST_F(FiltersListTest, IsSubdirectoryTest) {
  using path = std::filesystem::path;
  auto is_sub = FiltersList::is_subdirectory;
  path p1{"artifacts/dir_4/a"}, p2{"artifacts/dir_4/a/d"};
  EXPECT_TRUE(is_sub(p1, p2));

  p1 = path{"artifacts/dir_4/a/d"}, p2 = path{"artifacts/dir_4/a"};
  EXPECT_FALSE(is_sub(p1, p2));
  p1 = path{"artifacts/dir_4/a"}, p2 = path{"artifacts/dir_4/ab"};
  EXPECT_FALSE(is_sub(p1, p2));

  p1 = path{"artifacts/dir_4/a"}, p2 = path{"artifacts/dir_4/ab/c"};
  EXPECT_FALSE(is_sub(p1, p2));

  p1 = path{"./artifacts/dir_4/a"}, p2 = path{"artifacts/dir_4/a/d"};
  EXPECT_TRUE(is_sub(p1, p2));

  p1 = path{"./artifacts/dir_4/"}, p2 = path{"artifacts/dir_4/f1"};
  try {
    is_sub(p1, p2);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &exp) {
    // exp.what() returns const char pointer;
    EXPECT_EQ(string(exp.what()), "The paths should be directories.");
  } catch (...) {
    FAIL() << "Different exception than expected caught";
  }
}
