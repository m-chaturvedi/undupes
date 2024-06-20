#pragma once
#include <stddef.h> // for size_t

#include <regex> // for regex
#include <set>
#include <string> // for string
#include <vector> // for vector

#include "filter.h" // for FileSets

extern bool dry_run;
namespace IO {
void animation(size_t sleep_time_milliseconds);
void end_animation();
void parse_input(FileSets &initial_file_sets,
                 std::set<FileType> accepted = {FileType::symlinked_file,
                                                FileType::regular_file});

void remove_file_io(const FileSets &file_sets, KeepFileSets &keep_file_sets,
                    std::string input_dev = "/dev/tty",
                    std::string output_dev = "/dev/tty");

void get_matches(const std::regex &reg, const std::string &S,
                 std::vector<std::string> &result);

void print_summary(const FileSets &resulting_file_sets);
void parse_file_list(std::string orig_string, std::vector<int> &file_list,
                     const int max_file_number);

void sanitize_and_check_input(const std::string &str,
                              std::vector<bool> &keep_file_list);
void print_json(const FileSets &file_sets);
std::string pprint_bytes(size_t bytes);

} // namespace IO
