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
#include "cli.h"

#include <unistd.h>

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

    ("m,summary", "Summary for the files found equal.")

    ("y,dry-run", "Do a dry run (i.e. do not delete files). Pass a file to write to.",
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
      (cxxopts_results.count("delete") || cxxopts_results.count("help") ||
       cxxopts_results.count("dry-run")))
    throw std::runtime_error("Incompatible options.");

  // help needs to be passed alone.
  if (cxxopts_results.count("help") &&
      (cxxopts_results.count("delete") || cxxopts_results.count("summary") ||
       cxxopts_results.count("dry-run")))
    throw std::runtime_error("Incompatible options.");

  if (cxxopts_results.count("dry-run") && !cxxopts_results.count("delete"))
    throw std::runtime_error("Incompatible options.");

  if (cxxopts_results.count("dry-run")) {
    if (cxxopts_results["dry-run"].as<std::string>().at(0) == '-') {
      throw std::runtime_error(
          "The dry-run option takes an input. Input file name should not start "
          "with a -.");
    }
  }
}
