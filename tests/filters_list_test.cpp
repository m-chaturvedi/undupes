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
#include "filters_list.h" // for xxhash, fs, file_size

#include <gtest/gtest.h> // for TestInfo (ptr only), EXPECT_EQ, TEST_F

#include <exception>
#include <filesystem> // for recursive_directory_iterator, begin
#include <memory>
#include <ostream> // for operator<<
#include <string>

#include "debug.h"

using namespace std;
class FiltersListTest : public testing::Test {
protected:
  // Remember that SetUp() is run immediately before a test starts.
  void SetUp() override {}

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override {}
};

TEST_F(FiltersListTest, xxhashT4KBTest) {
  FilePtr f = make_shared<File>("artifacts/dir_3/4KB_1");
  std::string hash = FiltersList::xxhash_4KB(f);
  IC(hash);
  // xxh128sum artifacts/dir_3/4KB_1
  EXPECT_EQ(hash, "9095db0480326fe09475cd87df7cdb81");

  f = make_shared<File>("artifacts/sample_1.pdf");
  hash = FiltersList::xxhash_4KB(f);
  // head -c 4096 ./artifacts/sample_1.pdf
  EXPECT_EQ(hash, "1cdfeb1e989503ea743248b5a8122fc3");
}

TEST_F(FiltersListTest, xxhashTest) {
  FilePtr f = make_shared<File>("artifacts/sample_1.pdf");
  string hash = FiltersList::xxhash(f);
  // xxh128sum artifacts/sample_1.pdf
  EXPECT_EQ(hash, "a9e96523afa48867198c85f09dff5983");
}
