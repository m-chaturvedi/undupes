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
#include "filter.h"
#include <gtest/gtest.h> // for TestInfo (ptr only), EXPECT_EQ, TEST_F

#include <filesystem> // for recursive_directory_iterator, begin
#include <ostream>    // for operator<<

#include "bin_compare_files.h" // for compare_files_fdupes
#include "file.h"              // for File
#include "filters_list.h"      // for xxhash, fs, file_size

class FilterTest : public testing::Test {
protected:
  // Remember that SetUp() is run immediately before a test starts.
  void SetUp() override {
    read_dir("artifacts/dir_2", file_sets_dir_2);
    read_dir("artifacts/dir_3", file_sets_dir_3);
  }

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override {}
  void read_dir(const std::string &dir, FileSets &_file_sets) {
    FileVector file_vector;
    for (const fs::directory_entry &dir_entry :
         fs::recursive_directory_iterator(dir)) {
      file_vector.emplace_back(new File{dir_entry.path()});
    }
    _file_sets.emplace_back(file_vector);
  }
  FileSets file_sets_dir_2, file_sets_dir_3;
};

TEST_F(FilterTest, ConstructorFileSizeFilter) {
  HashableFilter filter{file_sets_dir_2, FiltersList::file_size};
  FileSets &new_file_sets = filter.new_file_sets;
  EXPECT_EQ(new_file_sets.size(), 5);
  FileSets expected_file_set;
  for (int i = 1; i <= 5; ++i) {
    FileVector file_vector;
    for (int j = 1; j <= 5; ++j) {
      FilePtr f = std::make_shared<File>(
          "artifacts/dir_2/" + std::to_string(i) + "KB_" + std::to_string(j));
      file_vector.push_back(f);
    }
    expected_file_set.push_back(file_vector);
  }
  EXPECT_EQ(new_file_sets, expected_file_set);
}

TEST_F(FilterTest, ConstructorXxhashFilter) {
  HashableFilter filter{file_sets_dir_2, FiltersList::xxhash};
  const FileSets &new_file_sets = filter.new_file_sets;
  filter.print_with_filter(new_file_sets, FiltersList::xxhash);
  EXPECT_EQ(new_file_sets.size(), 0);
}

TEST_F(FilterTest, ConstructorBinComparison) {
  NonHashableFilter filter{file_sets_dir_3, compare_files_fdupes};
  FileSets &new_file_sets = filter.new_file_sets;
  auto ms = [](const std::string &file_name) {
    return std::make_shared<File>("artifacts/dir_3/" + file_name);
  };

  FileSets expected = {
      {
          ms("1KB_1"),
          ms("1KB_1.copy.1"),
          ms("1KB_1.copy.2"),
          ms("1KB_1.copy.3"),
      },
      {
          ms("1KB_2"),
          ms("1KB_2.copy.1"),
          ms("1KB_2.copy.2"),
          ms("1KB_2.copy.3"),
      },
      {
          ms("3KB_1"),
          ms("3KB_1.copy.1"),
          ms("3KB_1.copy.2"),
          ms("3KB_1.copy.3"),
      },
      {
          ms("4KB_1"),
          ms("4KB_1.copy.1"),
          ms("4KB_1.copy.2"),
          ms("4KB_1.copy.3"),
          ms("4KB_1.copy.4"),
          ms("4KB_1.copy.5"),
      },
  };

  // IC(new_file_sets);
  // filter.print_with_filter(new_file_sets, FiltersList::xxhash);
  EXPECT_EQ(new_file_sets, expected);
}
TEST_F(FilterTest, ConstructorRealLife1) {
  HashableFilter filter{file_sets_dir_3, FiltersList::xxhash};
  FileSets &new_file_sets = filter.new_file_sets;
  auto ms = [](const std::string &file_name) {
    return std::make_shared<File>("artifacts/dir_3/" + file_name);
  };

  FileSets expected = {
      {
          ms("1KB_1"),
          ms("1KB_1.copy.1"),
          ms("1KB_1.copy.2"),
          ms("1KB_1.copy.3"),
      },
      {
          ms("1KB_2"),
          ms("1KB_2.copy.1"),
          ms("1KB_2.copy.2"),
          ms("1KB_2.copy.3"),
      },
      {
          ms("3KB_1"),
          ms("3KB_1.copy.1"),
          ms("3KB_1.copy.2"),
          ms("3KB_1.copy.3"),
      },
      {
          ms("4KB_1"),
          ms("4KB_1.copy.1"),
          ms("4KB_1.copy.2"),
          ms("4KB_1.copy.3"),
          ms("4KB_1.copy.4"),
          ms("4KB_1.copy.5"),
      },
  };

  // IC(new_file_sets);
  // filter.print_with_filter(new_file_sets, FiltersList::xxhash);
  EXPECT_EQ(new_file_sets, expected);
}
