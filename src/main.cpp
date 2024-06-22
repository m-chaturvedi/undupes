#include <chrono>
#include <fstream>
#include <functional>

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

void apply_three_common_filters(const FileSets &file_sets, FileSets &result,
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
  if (print) IO::print_json(filter_4.new_file_sets);
#else
  if (print) IO::print_json(filter_2.new_file_sets);
#endif
  result = std::move(filter_3.new_file_sets);
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::warn);
  cxxopts::Options options("undupe", "Remove duplicate files.");
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

  if (cxxopts_results.count("dry-run")) dry_run = true;

  if (cxxopts_results.count("delete")) {
    apply_three_common_filters(input_file_sets, resulting_file_sets, false);
    if (!resulting_file_sets.empty()) {
      KeepFileSets kps(resulting_file_sets.size());
      for (size_t i = 0; i < resulting_file_sets.size(); ++i) {
        size_t N = resulting_file_sets.at(i).size();
        kps.at(i) = std::move(std::vector<bool>(N, true));
      }
      IO::remove_file_io(resulting_file_sets, kps);
    }
  } else if (cxxopts_results.count("summary")) {
    apply_three_common_filters(input_file_sets, resulting_file_sets, false);
    IO::print_summary(resulting_file_sets);
  } else {
    apply_three_common_filters(input_file_sets, resulting_file_sets, true);
  }
  return 0;
}
