#define XXH_PRIVATE_API 0
#include "filters_list.h"

#include <stdint.h> // for uint64_t
#include <stdlib.h> // for free, malloc
#include <unordered_set>
#include <xxhash.h> // for XXH_INLINE_XXH3_128bits_digest

#include <exception>  // for exception
#include <filesystem> // for file_size, directory_entry
#include <iostream>   // for operator<<, basic_ostream, cout

#include "debug.h"  // for error, format, vformat_to, format...
#include "filter.h" // for FilePtr

namespace fs = std::filesystem;

size_t FiltersList::file_size(const FilePtr a) {
  size_t size_a = fs::file_size((a->get_resolved_dir_entry()).path());
  return size_a;
}

std::string FiltersList::xxhash_4KB(const FilePtr file) {
  XXH3_state_t state3;
  XXH128_hash_t hash;
  FILE *fptr = fopen(file->get_path().c_str(), "rb");

  size_t read_size, block_size = 4 * (1 << 10);
  void *const buffer = malloc(block_size);
  XXH3_128bits_reset(&state3);

  read_size = fread(buffer, 1, block_size, fptr);
  (void)XXH3_128bits_update(&state3, buffer, read_size);
  hash = XXH3_128bits_digest(&state3);
  fclose(fptr);
  free(buffer);

  uint64_t low{hash.low64}, high{hash.high64};
  return (std::to_string(low) + std::to_string(high));
}

std::string FiltersList::xxhash(const FilePtr file) {
  constexpr size_t block_size = 64 * (1 << 10);
  void *const buffer = malloc(block_size);
  try {
    XXH128_hash_t hash;
    // size_t total_size_bytes{0};
    std::string path_str = file->get_path();
    const char *path = path_str.c_str();
    FILE *fptr = fopen(path, "rb");
    if (fptr == nullptr) {
      spdlog::error("XXHash: Ignoring file: {}", path_str);
      free(buffer);
      return std::string{};
    }

    XXH3_state_t state3;
    XXH3_128bits_reset(&state3);

    size_t read_size;
    while ((read_size = fread(buffer, 1, block_size, fptr)) > 0) {
      (void)XXH3_128bits_update(&state3, buffer, read_size);
      // total_size_bytes += read_size;
    }

    hash = XXH3_128bits_digest(&state3);
    fclose(fptr);
    uint64_t low{hash.low64}, high{hash.high64};
    free(buffer);

    return (std::to_string(low) + std::to_string(high));
  } catch (const std::exception &exp) {
    std::cout << "Caught exception: '" << exp.what() << "'\n";
    free(buffer);
    throw exp;
  }
}

/**
 * @brief Checks whether directory p2 is a subdirectory of directory p1
 *
 * @param p1 Directory which is to be check for parent directory.
 * @param p2 Directory to be checked for subdirectory
 *
 * @return true if p2 is subdirectory of p1 and false otherwise.
 */
bool FiltersList::is_subdirectory(const std::filesystem::path &p1,
                                  const std::filesystem::path &p2) {
  using path = std::filesystem::path;
  if (!std::filesystem::is_directory(p1) || !std::filesystem::is_directory(p2))
    throw std::runtime_error("The paths should be directories.");
  // Needs to be canonicalized to take care of ., .., etc.
  path p1_abs = fs::canonical(fs::absolute(p1));
  path p2_abs = fs::canonical(fs::absolute(p2));
  path::const_iterator it1, it2;

  IC(p1_abs, p2_abs);
  for (it1 = p1_abs.begin(), it2 = p2_abs.begin();
       it1 != p1_abs.end() && it2 != p2_abs.end(); ++it1, ++it2) {
    if (*it1 != *it2)
      return false;
  }
  return it1 == p1_abs.end();
  ;
}

bool FiltersList::check_filters(
    const std::vector<std::string> &keep_filters,
    const std::vector<std::string> &remove_filters) {
  using path = std::filesystem::path;
  for (const auto &keep_filter : keep_filters) {
    for (const auto &remove_filter : remove_filters) {
      path keep_path{keep_filter}, remove_path{remove_filter};
      if (is_subdirectory(keep_path, remove_path) ||
          is_subdirectory(remove_path, keep_path)) {
        return false;
      }
    }
  }
  return true;
}

void FiltersList::keep_remove_filter(
    const FileSets &filtered_file_sets, KeepFileSets &keep_file_sets,
    const std::vector<std::string> &keep_filters,
    const std::vector<std::string> &remove_filters,
    std::vector<size_t> &matching_sets) {
  std::unordered_set<size_t> S;
  using path = std::filesystem::path;
  assert(keep_file_sets.size() == filtered_file_sets.size());
  for (size_t i = 0; i < keep_file_sets.size(); ++i) {
    assert(keep_file_sets.at(i).size() == filtered_file_sets.at(i).size());

    for (size_t j = 0; j < keep_file_sets.at(i).size(); ++j) {
      path keep_path, remove_path;

      FilePtr file = filtered_file_sets.at(i).at(j);
      for (const auto &keep_filter : keep_filters) {
        keep_path = path{keep_filter};
        if (is_subdirectory(keep_path, file->dir_entry.path())) {
          keep_file_sets.at(i).at(j) = true;
          S.insert(i);
        }
      }
      for (const auto &remove_filter : remove_filters) {
        remove_path = path{remove_filter};
        if (is_subdirectory(remove_path, file->dir_entry.path())) {
          keep_file_sets.at(i).at(j) = false;
          S.insert(i);
        }
      }
    }
  }

  matching_sets = std::vector<size_t>(S.begin(), S.end());
  std::sort(matching_sets.begin(), matching_sets.end());
}
