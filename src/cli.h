#pragma once
#include "cxxopts.hpp"

extern cxxopts::ParseResult cxxopts_results;

void check_options();
void add_options(cxxopts::Options &options, int argc, char *argv[]);

