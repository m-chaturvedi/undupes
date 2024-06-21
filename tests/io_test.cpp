#include "io.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <numeric>
#include <string>

#include "debug.h"
#include "filter.h"
#include "filters_list.h"
#include "io.h"

extern bool dry_run;

extern bool processing_done;
using namespace std;
class IOTest : public testing::Test {
protected:
  // Remember that SetUp() is run immediately before a test starts.
  void SetUp() override {
    iota(expected_file_list.begin(), expected_file_list.end(), 1);
    read_dir("artifacts/dir_2", file_sets_dir_2);
    read_dir("artifacts/dir_3", file_sets_dir_3);

    fs::copy("artifacts/dir_3", "/tmp/dir_3.copy",
             fs::copy_options::overwrite_existing |
                 fs::copy_options::recursive);
    auto ms = [](std::string file_name) {
      return std::make_shared<File>("artifacts/dir_3/" + file_name);
    };

    resulting_file_sets = {
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
  }

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override {
    fs::copy("/tmp/dir_3.copy", "artifacts/dir_3",
             fs::copy_options::overwrite_existing |
                 fs::copy_options::recursive);
  }
  // https://stackoverflow.com/a/39560347/873956
  vector<int> expected_file_list = std::vector<int>(10);
  vector<bool> expected_keep_file_list = std::vector<bool>(10, true);

  void read_dir(const std::string &dir, FileSets &_file_sets) {
    FileVector file_vector;
    for (const fs::directory_entry &dir_entry :
         fs::recursive_directory_iterator(dir)) {
      file_vector.emplace_back(new File{dir_entry.path()});
    }
    _file_sets.emplace_back(file_vector);
  }
  FileSets file_sets_dir_2, file_sets_dir_3, resulting_file_sets;
};

bool files_eq(const string &A, const string &B);

void delete_files(FileSets &resulting_file_sets,
                  std::vector<int> test_cases = {1, 2, 3}) {
  for (const int j : test_cases) {
    KeepFileSets kps(resulting_file_sets.size());
    for (size_t i = 0; i < resulting_file_sets.size(); ++i) {
      size_t N = resulting_file_sets.at(i).size();
      kps.at(i) = std::move(std::vector<bool>(N, true));
    }
    std::string in_file = "io/remove_file_io_test_" + std::to_string(j) + ".in";
    std::string out_file =
        "io/remove_file_io_test_" + std::to_string(j) + ".out";

    std::string op_name = "op_test_" + std::to_string(j) + ".out";
    IO::remove_file_io(resulting_file_sets, kps, in_file,
                       op_name);
    bool ret = files_eq(op_name, out_file);
    if (dry_run) {
      IC(out_file);
      EXPECT_TRUE(ret);
    }
  }
}
void check_exceptions_parse_file_test(const string &s) {
  vector<int> file_list;
  try {
    IO::parse_file_list(s, file_list, 10);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &exp) {
    // exp.what() returns const char pointer;
    EXPECT_EQ(string(exp.what()), "Unexpected input.");
    EXPECT_TRUE(file_list.empty());
  } catch (...) {
    FAIL() << "Different exception than expected caught";
  }
}

// https://stackoverflow.com/a/2602258/873956
bool files_eq(const string &A, const string &B) {
  std::ifstream is_A(A.c_str()), is_B(B.c_str());
  std::stringstream buffer_A, buffer_B;
  buffer_A << is_A.rdbuf();
  buffer_B << is_B.rdbuf();
  is_A.close();
  is_B.close();
  return buffer_A.str() == buffer_B.str();
}

// NOTE: check_exceptions_parse_file_test and this one can be combined into
// one using templates.
void check_exceptions_sanitize_test(const string &s, const size_t &set_size) {
  vector<bool> keep_file_list(set_size);
  try {
    IO::sanitize_and_check_input(s, keep_file_list);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &exp) {
    // exp.what() returns const char pointer;
    EXPECT_EQ(string(exp.what()), "Unexpected input.");
  } catch (...) {
    FAIL() << "Different exception than expected caught";
  }
}

TEST_F(IOTest, ParseFileTest) {
  vector<int> file_list;
  IO::parse_file_list("1,2,3,4", file_list, 4);
  EXPECT_EQ(file_list, vector<int>({1, 2, 3, 4}));

  IO::parse_file_list("1,2-4", file_list, 4);
  EXPECT_EQ(file_list, vector<int>({1, 2, 3, 4}));

  IO::parse_file_list("1-4", file_list, 4);
  EXPECT_EQ(file_list, vector<int>({1, 2, 3, 4}));

  IO::parse_file_list("1-1", file_list, 10);
  EXPECT_EQ(file_list, vector<int>({1}));

  IO::parse_file_list("4-4", file_list, 10);
  EXPECT_EQ(file_list, vector<int>({4}));

  IO::parse_file_list("4", file_list, 10);
  EXPECT_EQ(file_list, vector<int>({4}));

  IO::parse_file_list("1,3-3,4-4,1", file_list, 10);
  EXPECT_EQ(file_list, vector<int>({1, 3, 4}));

  // TODO: There may be a better solution with ranges?

  IO::parse_file_list("1-10", file_list, 10);
  EXPECT_EQ(file_list, expected_file_list);

  IO::parse_file_list("1 ,2 , 3, 4, 5,6,7,8,9, 10", file_list, 10);
  EXPECT_EQ(file_list, expected_file_list);

  IO::parse_file_list(" 10,9, 1 , 3- 5,2 , 6 - 8  ", file_list, 10);
  EXPECT_EQ(file_list, expected_file_list);
  IO::parse_file_list("1,2, 10,9, 1 , 3- 5,2 , 6 - 8  ", file_list, 10);
  EXPECT_EQ(file_list, expected_file_list);

  IO::parse_file_list("", file_list, 10);
  EXPECT_EQ(file_list, vector<int>{});

  IO::parse_file_list("1,2, 10,9, 1 , 3- 5,2 , 6 - 8 ,8-8, 6-6 ", file_list,
                      10);
  EXPECT_EQ(file_list, expected_file_list);

  check_exceptions_parse_file_test("-  10,9, 1 , 3- 5,2 , 6 - 8  ");
  check_exceptions_parse_file_test("r10-33");
  check_exceptions_parse_file_test("random");
  check_exceptions_parse_file_test("11");
  check_exceptions_parse_file_test("4-3");
  check_exceptions_parse_file_test("1,2,3,4,0");
  check_exceptions_parse_file_test("6-11");
  check_exceptions_parse_file_test("0-1");
  check_exceptions_parse_file_test("0-12");
  check_exceptions_parse_file_test("1,2,3,4,6-5");
  check_exceptions_parse_file_test(",");
}

TEST_F(IOTest, SanitizeAndCheckInputTest) {
  vector<bool> keep_file_list(10, true);
  vector<bool> L;

  IO::sanitize_and_check_input("all", keep_file_list);
  EXPECT_EQ(keep_file_list, expected_keep_file_list);
  IO::sanitize_and_check_input("a", keep_file_list);
  EXPECT_EQ(keep_file_list, expected_keep_file_list);

  IO::sanitize_and_check_input("none", keep_file_list);
  EXPECT_EQ(keep_file_list, vector<bool>(10, false));

  IO::sanitize_and_check_input("n", keep_file_list);
  EXPECT_EQ(keep_file_list, vector<bool>(10, false));

  IO::sanitize_and_check_input("1,2,3", keep_file_list);
  L = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
  EXPECT_EQ(keep_file_list, L);

  IO::sanitize_and_check_input("3, 8-9", keep_file_list);
  L = {0, 0, 1, 0, 0, 0, 0, 1, 1, 0};
  EXPECT_EQ(keep_file_list, L);

  check_exceptions_sanitize_test("-", 10);
  check_exceptions_sanitize_test("r10-33", 10);
  check_exceptions_sanitize_test("-  10,9, 1 , 3- 5,2 , 6 - 8  ", 10);
}

TEST_F(IOTest, RemoveFileIOTest) {
  string input;

  bool dry_run_before = dry_run;

  dry_run = true;
  delete_files(resulting_file_sets);

  dry_run = false;
  delete_files(resulting_file_sets, {1});
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_1.copy.1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_1.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_1.copy.3"));

  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2.copy.1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2.copy.3"));

  EXPECT_TRUE(fs::exists("artifacts/dir_3/3KB_1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/3KB_1.copy.1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/3KB_1.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/3KB_1.copy.3"));

  EXPECT_TRUE(fs::exists("artifacts/dir_3/4KB_1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.3"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.4"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.5"));
  dry_run = dry_run_before;
}

TEST_F(IOTest, RemoveFileIOTest2) {
  bool dry_run_before = dry_run;
  dry_run = false;
  delete_files(resulting_file_sets, {2});
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_1.copy.1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_1.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_1.copy.3"));

  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2.copy.1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_2.copy.2"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_2.copy.3"));

  EXPECT_TRUE(fs::exists("artifacts/dir_3/3KB_1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/3KB_1.copy.1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/3KB_1.copy.2"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/3KB_1.copy.3"));

  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.3"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.4"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.5"));
  dry_run = dry_run_before;
}

TEST_F(IOTest, RemoveFileIOTest3) {
  bool dry_run_before = dry_run;
  dry_run = false;
  delete_files(resulting_file_sets, {3});
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_1.copy.1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_1.copy.2"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_1.copy.3"));

  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2.copy.1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/1KB_2.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/1KB_2.copy.3"));

  EXPECT_TRUE(fs::exists("artifacts/dir_3/3KB_1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/3KB_1.copy.1"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/3KB_1.copy.2"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/3KB_1.copy.3"));

  EXPECT_TRUE(fs::exists("artifacts/dir_3/4KB_1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/4KB_1.copy.1"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/4KB_1.copy.2"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/4KB_1.copy.3"));
  EXPECT_TRUE(fs::exists("artifacts/dir_3/4KB_1.copy.4"));
  EXPECT_FALSE(fs::exists("artifacts/dir_3/4KB_1.copy.5"));
  dry_run = dry_run_before;
}

TEST_F(IOTest, PprintBytesTest) {
  // format is rounding off.
  EXPECT_EQ(IO::pprint_bytes(100), "100.00 B");
  EXPECT_EQ(IO::pprint_bytes(1025), "1.00 KiB");
  EXPECT_EQ(IO::pprint_bytes(1048577), "1.00 MiB");
  EXPECT_EQ(IO::pprint_bytes(1150976), "1.10 MiB");
  EXPECT_EQ(IO::pprint_bytes(1560576), "1.49 MiB");
  EXPECT_EQ(IO::pprint_bytes(2147483648), "2.00 GiB");
}

TEST_F(IOTest, ParseInputTest) {
  string input = "artifacts/dir_2\0"
                 "artifacts/dir_2/4KB_3\0"
                 "artifacts/dir_2/3KB_3\0"
                 "artifacts/dir_2/1KB_3\0"
                 "artifacts/dir_2/3KB_2\0"
                 "artifacts/dir_2/1KB_1\0"
                 "artifacts/dir_2/4KB_1\0"
                 "artifacts/dir_2/3KB_4\0"
                 "artifacts/dir_2/5KB_1\0"
                 "artifacts/dir_2/2KB_3\0"
                 "artifacts/dir_2/5KB_3\0"
                 "artifacts/dir_2/4KB_5\0"
                 "artifacts/dir_2/3KB_1\0"
                 "artifacts/dir_2/1KB_4\0"
                 "artifacts/dir_2/5KB_4\0"
                 "artifacts/dir_2/2KB_4\0"
                 "artifacts/dir_2/5KB_5\0"
                 "artifacts/dir_2/4KB_2\0"
                 "artifacts/dir_2/1KB_5\0"
                 "artifacts/dir_2/3KB_5\0"
                 "artifacts/dir_2/2KB_1\0"
                 "artifacts/dir_2/4KB_4\0"
                 "artifacts/dir_2/2KB_5\0"
                 "artifacts/dir_2/5KB_2\0"
                 "artifacts/dir_2/2KB_2\0"
                 "artifacts/dir_2/1KB_2\0";
}
