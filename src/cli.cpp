#include "cli.h"

#include <iostream>
#include <string>

#include "cxxopts.hpp"

cxxopts::ParseResult cxxopts_results;

/**
 * @brief Add the options for the CLI.
 *
 * @param options The cxxopts options object to add options to.
 * @param argc The number of arguments passed to the executable.
 * @param argv[] The string arguments passed to the executable.
 */
void add_options(cxxopts::Options &options, int argc, char *argv[]) {
  // clang-format off
  options.add_options()
    ("d,delete", "Delete duplicate files.")

    ("m,summary", "Summary for the files found.")

    ("dry-run", "Do a dry run (do not delete files). Pass a file to write to.",
     cxxopts::value<std::string>())

    ("h,help", "Print usage")
    ;

  // clang-format on
  cxxopts_results = options.parse(argc, argv);
  check_options();
  if (isatty(fileno(stdin))) {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  if (cxxopts_results.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }
}

/**
 * @brief Check for incompatible options.
 */
void check_options() {
  // Summary needs to be passed alone.
  if (cxxopts_results.count("summary") &&
      (cxxopts_results.count("delete") || cxxopts_results.count("help")))
    throw std::runtime_error("Incompatible options.");

  // help needs to be passed alone.
  if (cxxopts_results.count("help") &&
      (cxxopts_results.count("delete") || cxxopts_results.count("summary") ||
       cxxopts_results.count("dry-run")))
    throw std::runtime_error("Incompatible options.");
}
