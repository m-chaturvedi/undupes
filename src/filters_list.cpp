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
#define XXH_PRIVATE_API 0
#include "filters_list.h"

#include <fmt/format.h>
#include <stdint.h> // for uint64_t
#include <stdlib.h> // for free, malloc
#include <xxhash.h> // for XXH_INLINE_XXH3_128bits_digest

#include <exception>  // for exception
#include <filesystem> // for file_size, directory_entry
#include <fstream>
#include <iostream> // for operator<<, basic_ostream, cout
#include <unordered_set>

#include "debug.h"  // for error, format, vformat_to, format...
#include "filter.h" // for FilePtr

namespace fs = std::filesystem;

/**
 * @brief Convert the hash returned by xxhash to a string same as what is
 * returned by the command `xxh128sum a.txt`. This is what is recommended by
 * the xxhash repository.
 * //
 * https://github.com/Cyan4973/xxHash/blob/805c00b68fa754200ada0c207ffeaa7a4409377c/cli/xxhsum.c#L243
 *
 * @param hash The XXH128_hash_t object returned by xxhash algorithm.
 * @param str The string to write the hash to.
 */
void convert_xxh128_hash_to_string(XXH128_hash_t hash, std::string &str) {
  std::ostringstream os;
  XXH128_canonical_t cano;
  XXH128_canonicalFromHash(&cano, hash);
  for (size_t i = 0; i < sizeof(cano.digest); ++i) {
    os << std::setw(2) << std::setfill('0') << std::hex << int(cano.digest[i]);
  }
  str = os.str();
}

/**
 * @brief Get the size of the file pointed to by the FilePtr.
 *
 * @param a The FilePtr object to calculate hash
 *
 * @return The size of the file.
 */
size_t FiltersList::file_size(const FilePtr a) {
  size_t size_a = fs::file_size((a->get_resolved_dir_entry()).path());
  return size_a;
}

/**
 * @brief Calculate the hash of the first 4KB of file.  This helps us speed up.
 * JDupes does something similar.  Instead of calcualting the hash of the whole
 * we just calculate the hash of the first 4KB.
 *
 * @param file The FilePtr object
 *
 * @return The xxhash calculated for the first 4KB of the file.
 */
std::string FiltersList::xxhash_4KB(const FilePtr file) {
  XXH3_state_t state3;
  XXH128_hash_t hash;
  FILE *fptr = fopen(file->get_path().c_str(), "rb");

  if (fptr == NULL)
    throw std::runtime_error(
        fmt::format("Cannot open file: {}", file->get_path()));

  size_t read_size, block_size = 4 * (1 << 10);
  void *const buffer = malloc(block_size);
  XXH3_128bits_reset(&state3);

  read_size = fread(buffer, 1, block_size, fptr);
  if (read_size != block_size && ferror(fptr)) {
    free(buffer);
    fclose(fptr);
    throw std::runtime_error(
        fmt::format("Error reading file: {}", file->get_path()));
  }

  (void)XXH3_128bits_update(&state3, buffer, read_size);
  hash = XXH3_128bits_digest(&state3);
  fclose(fptr);
  free(buffer);

  // https://github.com/Cyan4973/xxHash/blob/805c00b68fa754200ada0c207ffeaa7a4409377c/xxhash.h#L215
  std::string hash_str;
  convert_xxh128_hash_to_string(hash, hash_str);
  return hash_str;
}

/**
 * @brief Calculate the xxhash for the file.
 *
 * @param file The FilePtr object to get the hash for.
 *
 * @return The 128 byte XXHash for the file.
 */
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
    free(buffer);

    // https://github.com/Cyan4973/xxHash/blob/805c00b68fa754200ada0c207ffeaa7a4409377c/xxhash.h#L215
    std::string hash_str;
    convert_xxh128_hash_to_string(hash, hash_str);
    return hash_str;
  } catch (const std::exception &exp) {
    std::cout << "Caught exception: '" << exp.what() << "'\n";
    free(buffer);
    throw exp;
  }
}
