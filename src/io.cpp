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
#include "io.h"

#include <unistd.h>

#include <algorithm>  // for sort
#include <cassert>    // for assert
#include <cctype>     // for isprint
#include <chrono>     // for milliseconds
#include <filesystem> // for directory_entry
#include <fstream>
#include <iostream>          // for operator<<, basic_ostream, basic_is...
#include <locale>            // for isspace, locale
#include <map>               // for operator!=, operator==
#include <memory>            // for shared_ptr, make_shared
#include <mutex>             // for mutex
#include <nlohmann/json.hpp> // for basic_json
#include <numeric>           // for iota
#include <regex>             // for regex_match, match_results, regex
#include <sstream>           // for istringstream
#include <sstream>           // for basic_istringstream
#include <stdexcept>         // for runtime_error
#include <string>            // for basic_string, char_traits, operator==
#include <thread>            // for thread, sleep_for
#include <utility>
#include <vector> // for vector

#include "cli.h"
#include "debug.h"
#include "file.h"   // for File
#include "filter.h" // for FileSets, FileVector
#include "fmt/core.h"
#include "nlohmann/json_fwd.hpp" // for json
#include "unistd.h"

std::mutex animation_mutex;
bool processing_done{false};
bool dry_run{false};
size_t num_files;

using json = nlohmann::json;
std::shared_ptr<std::thread> animation_thread{};
extern cxxopts::ParseResult cxxopts_results;

/**
 * @brief Function to call waiting animation.
 *
 * @param sleep_time_milliseconds Sleep time between the rotation of cursor. The
 * mutex is needed because this function is run in a thread.
 */
void IO::animation(size_t sleep_time_milliseconds = 75) {
  {
    const std::lock_guard<std::mutex> lock(animation_mutex);
    processing_done = false;
  }
  size_t &t = sleep_time_milliseconds;
  std::string anim = "|/-\\";
  while (true)
    for (const auto &ele : anim) {
      {
        const std::lock_guard<std::mutex> lock(animation_mutex);
        if (processing_done)
          goto done;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(t));
      std::cout << "\b" << ele << std::flush;
    }
done:
  std::cout << "\b " << std::flush;
}

/**
 * @brief This ends the cursor animation.
 */
void IO::end_animation() {
  {
    const std::lock_guard<std::mutex> lock(animation_mutex);
    processing_done = true;
  }
  if (animation_thread != nullptr && animation_thread->joinable()) {
    animation_thread->join();
  }
}

/**
 * @brief This function parses the initial file sets.
 *
 * @param initial_file_sets  The nested vector containing the sets of files
 * to be filtered.
 */
void IO::parse_input(FileSets &initial_file_sets,
                     const std::set<FileType> &accepted) {
  if (isatty(fileno(stdout)))
    animation_thread = std::make_shared<std::thread>([]() { IO::animation(); });

  std::string line;
  FileVector file_vector;
  size_t file_index = 1;
  while (std::getline(std::cin, line, '\0')) {
    FilePtr f = std::make_shared<File>(File{line});
    bool check_file = f->check_file_or_log(accepted);
    if (check_file) {
      f->index = file_index;
      ++file_index;
      file_vector.emplace_back(f);
    }
  }
  initial_file_sets.emplace_back(file_vector);
  num_files = initial_file_sets.at(0).size();
}

/**
 * @brief Returns bytes in human readable format
 *
 * @param bytes Bytes in size_t type
 *
 * @return A string with human readable size, rounded-off to two decimal places.
 * Like "1.28 MB".  Support till GB.
 */
std::string IO::pprint_bytes(size_t bytes) {
  using PSI = std::pair<std::string, size_t>;
  std::vector<PSI> units = {PSI{"B", 1}, PSI{"KiB", 1 << 10},
                            PSI{"MiB", 1 << 20}, PSI{"GiB", 1 << 30}};
  auto it = std::upper_bound(
      units.begin(), units.end(), PSI{"", bytes},
      [](const PSI &a, const PSI &b) { return a.second < b.second; });
  --it;
  return fmt::format("{:.2f} {}", static_cast<double>(bytes) / it->second,
                     it->first);
}

/**
 * @brief Prints the summary of the duplicates found.  Similar to f/j-dupes.
 *
 * @param resulting_file_sets The file sets after filtering.
 */
void IO::print_summary(const FileSets &resulting_file_sets) {
  size_t num_duplicate_files{}, num_sets{}, duplicates_size{};
  num_sets = resulting_file_sets.size();

  if (num_sets == 0) {
    std::cout << "No duplicates found." << std::endl;
    return;
  }

  for (const auto &files : resulting_file_sets) {
    num_duplicate_files += files.size() - 1;
    size_t file_size =
        std::filesystem::file_size(files.at(0)->dir_entry.path());
    duplicates_size += (files.size() - 1) * file_size;
  }

  std::cout << fmt::format("{} duplicate files (in {} sets), occupying {}",
                           num_duplicate_files, num_sets,
                           IO::pprint_bytes(duplicates_size))
            << std::endl;
}

/**
 * @brief Parses the string for file numbers which is fed by the user.
 *
 * @param orig_string The original string (except "none" and "all").
 * @param file_list The list of file number after parsing the string.
 */
void IO::parse_file_list(std::string orig_string, std::vector<int> &file_list,
                         const int max_file_number) {
  std::string string_without_space, token;
  std::smatch sm;
  file_list.clear();

  // Remove spaces.
  for (const char &c : orig_string) {
    if (!std::isspace(c, std::locale{}))
      string_without_space.push_back(c);
  }
  std::istringstream is(string_without_space);

  while (std::getline(is, token, ',')) {
    auto res = std::regex_match(token, sm, std::regex{R"((\d+)(-\d+)?)"});
    if (!res)
      throw std::runtime_error("Unexpected input.");

    if (sm[1].matched && !sm[2].matched) {
      assert(token == sm.str(1));
      int file_number = std::stoi(sm.str(1));
      if (file_number > max_file_number || file_number < 1) {
        file_list.clear();
        throw std::runtime_error("Unexpected input.");
      }
      file_list.push_back(file_number);
    } else {
      std::regex_match(token, sm, std::regex{R"((\d+)-(\d+))"});
      int file_start = std::stoi(sm.str(1)), file_end = std::stoi(sm.str(2));
      if (file_start > file_end || file_start < 1 ||
          file_end > max_file_number) {
        file_list.clear();
        throw std::runtime_error("Unexpected input.");
      }

      for (int i = file_start; i <= file_end; ++i)
        file_list.push_back(i);
    }
  }
  std::sort(file_list.begin(), file_list.end());
  file_list.erase(std::unique(file_list.begin(), file_list.end()),
                  file_list.end());
}

/**
 * @brief Parses the input by the user.  Parses "all" and "none" and delegates
 * for parsing the other options to the `parse_file_list` function.
 *
 * @param str The string input by the user
 * @param set_size The size of file set.
 * @param file_list The return file list numbers.
 */
void IO::sanitize_and_check_input(const std::string &str,
                                  std::vector<bool> &keep_file_list) {
  const size_t set_size = keep_file_list.size();
  for (const auto &s : str) {
    if (!std::isprint(s))
      throw std::runtime_error("Non-printable character detected");
  }
  if (str.empty())
    throw std::runtime_error("Unexpected input.");

  if (str == "all" || str == "a") {
    std::fill(keep_file_list.begin(), keep_file_list.end(), true);
  } else if (str == "none" || str == "n")
    std::fill(keep_file_list.begin(), keep_file_list.end(), false);
  else {
    std::vector<int> file_list;
    IO::parse_file_list(str, file_list, set_size);
    std::fill(keep_file_list.begin(), keep_file_list.end(), false);
    for (const auto &n : file_list) {
      keep_file_list.at(n - 1) = true;
    }
  }
}

/**
 * @brief  Prints the list of files indicating which files will be deleted and
 * which ones will be kept.
 *
 * @param files The vector of files to be printed.
 * @param keep_files The vector of files that will be kept.
 * @param indent The indentation to be printed before the information.
 */
void IO::show_file_list(const FileVector &orig_files,
                        const std::vector<bool> &keep_files,
                        std::string indent) {
  FileVector files = orig_files;
  std::sort(files.begin(), files.end(), [](const FilePtr &a, const FilePtr &b) {
    return a->index < b->index;
  });

  for (size_t i = 0; i < files.size(); ++i) {
    char file_status = keep_files.at(i) ? '+' : '-';
    std::cout << fmt::format("{}[{}] [{}] {}\n", indent, file_status, i + 1,
                             files.at(i)->get_path());
  }
}

/**
 * @brief Removes the files as indicated by the keep_files vector.
 *
 * @param files The vector of files which should be considered for deletion
 * depeneding on the keep_files vector
 * @param keep_files A vector of booleans which indicates whether a file should
 * be kept or deleted.  true for keeping and false for deleting.
 */
void remove_files(const FileVector &files, const std::vector<bool> &keep_files,
                  std::ofstream &of) {
  // dry_run but not run from cli.
  if (dry_run && cxxopts_results.count("dry-run") == 0)
    return;

  // dry_run and run from cli.
  if (dry_run && cxxopts_results.count("dry-run") > 0) {
    for (size_t i = 0; i < files.size(); ++i) {
      if (!keep_files.at(i))
        of << files.at(i)->dir_entry << std::endl;
    }
    return;
  }

  // Not dry-run and run from cli.
  for (size_t i = 0; i < files.size(); ++i) {
    if (!keep_files.at(i)) {
      assert(dry_run == false);
      if (!fs::remove(files.at(i)->dir_entry))
        spdlog::warn("Cannot remove file: {}", files.at(i)->get_path());
    }
  }
}

/**
 * @brief The prompt to keep or delete files.
 *
 * @param file_sets The sets of files to be prompted for.
 * @param keep_file_sets The sets of file booleans which store the information
 * gotten after the prompt is made.
 * @param input_dev The input device, default if tty.  Is useful for testing.
 * @param output_dev The output devices, default is tty.  This is useful for
 * testing.
 */
void IO::remove_file_io(const FileSets &file_sets, KeepFileSets &keep_file_sets,
                        std::string input_dev, std::string output_dev) {
  std::string files_to_keep_str;

  std::streambuf *cin_backup, *cout_backup;
  cin_backup = std::cin.rdbuf();
  cout_backup = std::cout.rdbuf();

  std::ifstream tty_in(input_dev);
  std::cin.rdbuf(tty_in.rdbuf());
  std::ofstream tty_out(output_dev);
  std::cout.rdbuf(tty_out.rdbuf());

  std::ofstream of;
  // TODO: Handle signals and close this.
  if (dry_run && cxxopts_results.count("dry-run") > 0) {
    std::string output_file = cxxopts_results["dry-run"].as<std::string>();
    of = std::ofstream{output_file, std::ios::out};
  }

  std::cout << "\n"; // Make sure that the animation doesn't interfere.
  for (size_t i = 0; i < file_sets.size(); ++i) {
    IO::show_file_list(file_sets.at(i), keep_file_sets.at(i));
    std::string options = "[n]one, [a]ll.";
    std::cout << fmt::format(
        "\nSpecify comma or dash separated values for files to "
        "keep (ex: 1,2,3-4). {}\n",
        options);
    do {
      try {
        std::cout << ">>> ";
        if (!std::getline(std::cin, files_to_keep_str, '\n'))
          std::abort();
        sanitize_and_check_input(files_to_keep_str, keep_file_sets.at(i));
        std::cout << "\n";
        IO::show_file_list(file_sets.at(i), keep_file_sets.at(i), "    ");
        remove_files(file_sets.at(i), keep_file_sets.at(i), of);
        std::cout << "\n\n";
        break;
      } catch (const std::runtime_error &exp) {
        if (std::string(exp.what()) == "Unexpected input.")
          std::cout << "Unexpected input.  Try again." << std::endl;
      }
    } while (1);
  }
  std::cin.rdbuf(cin_backup);
  std::cout.rdbuf(cout_backup);
  of.close();
}

/**
 * @brief Print the json containing the set of file sets. This needs a mutex
 * because processing_done variable is shared with the animation thread.
 *
 * @param file_sets The vector of vectors to be printed in JSON format.
 */
void IO::print_json(const FileSets &file_sets) {
  nlohmann::json json_output;
  for (const auto &file_vector : file_sets) {
    std::vector<std::string> file_paths;
    // TODO: See if the cppcheck is meaningful.
    for (const auto &file : file_vector)
      // cppcheck-suppress useStlAlgorithm
      file_paths.emplace_back(file->dir_entry.path().string());

    // We want to only take files which have duplicates.
    if (file_paths.size() >= 2) {
      json obj;
      obj["file_list"] = file_paths;
      json_output.emplace_back(obj);
    }
  }
  // Dump json with an indentation of 4.
  std::cout << json_output.dump(4) << std::endl;
}
