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
// SPDX-License-Identifier: AGPL-3.0
// SPDX-FileCopyrightText: 2024 Mmanu Chaturvedi <mmanu.chaturvedi@gmail.com>
#include <chrono>
#include <fstream>
#include <functional>
#include <memory>

#include "bin_compare_files.h"
#include "cli.h"
#include "debug.h"
#include "filter.h"
#include "filters_list.h"
#include "io.h"
#include "unistd.h"
#define WITH_BIN_COMPARISON 1

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

extern bool dry_run;
extern cxxopts::ParseResult cxxopts_results;

/**
 * @brief Calculates the four common filters for the files.  size, xxash for
 * first 4KB, and the xxhash of the whole file. Then it goes ahead and compares
 * files byte by byte
 *
 * @param file_sets The FileSets to apply filters to.
 * @param result The resulting FileSets object after applyint the filter.
 * @param print Whether or not to print the FileSets object in json format.
 */
void apply_four_common_filters(const FileSets &file_sets, FileSets &result,
                               bool print = true) {
  HashableFilter filter_1{file_sets, FiltersList::file_size};
  HashableFilter filter_2{filter_1.new_file_sets, FiltersList::xxhash_4KB};
  HashableFilter filter_3{filter_2.new_file_sets, FiltersList::xxhash};

#if WITH_BIN_COMPARISON == 1
  auto t1 = high_resolution_clock::now();
  NonHashableFilter filter_4{filter_3.new_file_sets, compare_files_fdupes};
  IO::end_animation();
  auto t2 = high_resolution_clock::now();
  spdlog::info("Bin comparison time: {}",
               (duration<double, std::milli>{t2 - t1}).count());
  if (print)
    IO::print_json(filter_4.new_file_sets);
#else
  if (print)
    IO::print_json(filter_2.new_file_sets);
#endif
  result = std::move(filter_3.new_file_sets);
}

int main(int argc, char *argv[]) {
  std::shared_ptr<spdlog::logger> stderr_sink =
      spdlog::stderr_color_mt("stderr");
  spdlog::set_default_logger(stderr_sink);
  spdlog::set_level(spdlog::level::warn);
  cxxopts::Options options("undupes", "Remove duplicate files.");
  try {
    add_options(options, argc, argv);
  } catch (std::runtime_error &exp) {
    std::string exp_string = std::string(exp.what());
    if (exp_string == "Incompatible options." ||
        exp_string.starts_with("The dry-run option takes an input.")) {
      std::cout << exp.what() << std::endl;
      exit(1);
    } else
      throw exp;
  } catch (cxxopts::exceptions::missing_argument &exp) {
    std::cout << exp.what() << std::endl;
    exit(1);
  }

  FileSets input_file_sets, resulting_file_sets;
  IO::parse_input(input_file_sets);

  if (cxxopts_results.count("dry-run"))
    dry_run = true;

  if (cxxopts_results.count("delete")) {
    apply_four_common_filters(input_file_sets, resulting_file_sets, false);
    if (!resulting_file_sets.empty()) {
      KeepFileSets kps(resulting_file_sets.size());
      for (size_t i = 0; i < resulting_file_sets.size(); ++i) {
        size_t N = resulting_file_sets.at(i).size();
        kps.at(i) = std::move(std::vector<bool>(N, true));
      }
      IO::remove_file_io(resulting_file_sets, kps);
    }
  } else if (cxxopts_results.count("summary")) {
    apply_four_common_filters(input_file_sets, resulting_file_sets, false);
    IO::print_summary(resulting_file_sets);
  } else {
    apply_four_common_filters(input_file_sets, resulting_file_sets, true);
  }
  return 0;
}
