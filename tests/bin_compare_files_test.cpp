/*
Undupes: Find duplicate files.
Copyright (C) 2024 Mmanu Chaturvedi <mmanu.chaturvedi@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
