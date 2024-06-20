#include "bin_compare_files.h"

#include <gtest/gtest.h>

class BinCompareFilesTest : public testing::Test {
protected:
  // Remember that SetUp() is run immediately before a test starts.
  void SetUp() override {}

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override {}
};

TEST_F(BinCompareFilesTest, SimpleTestTrue) {
  std::string file_1 = "artifacts/sample_1.pdf";
  std::string file_2 = "artifacts/sample_1.pdf.copy";

  EXPECT_TRUE(compare_files_fdupes(file_1, file_2));
}

TEST_F(BinCompareFilesTest, SimpleTestFalse) {
  std::string file_1 = "artifacts/dir_2/1KB_1";
  std::string file_2 = "artifacts/dir_2/2KB_1";

  EXPECT_FALSE(compare_files_fdupes(file_1, file_2));
}
