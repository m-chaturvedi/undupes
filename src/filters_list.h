#pragma once
#include <stdio.h>  // for size_t

#include <filesystem>  // for filesystem
#include <string>      // for string

#include "filter.h"  // for FilePtr

namespace fs = std::filesystem;

namespace FiltersList {
size_t file_size(const FilePtr a);
std::string xxhash(const FilePtr file);
std::string xxhash_4KB(const FilePtr file);

bool is_subdirectory(const std::filesystem::path &p1,
                     const std::filesystem::path &p2);

void keep_remove_filter(const FileSets &input_file_sets,
                        KeepFileSets &keep_file_sets,
                        const std::vector<std::string> &keep_filters,
                        const std::vector<std::string> &remove_filters,
                        std::vector<size_t> &matching_sets);

bool check_filters(const std::vector<std::string> &keep_filters,
                   const std::vector<std::string> &remove_filters);
}  // namespace FiltersList
