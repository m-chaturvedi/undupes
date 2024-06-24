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

void convert_xxh128_hash_to_string(XXH128_hash_t hash, std::string &str) {
  std::ostringstream os;
  XXH128_canonical_t cano;
  XXH128_canonicalFromHash(&cano, hash);
  for (size_t i = 0; i < sizeof(cano.digest); ++i) {
    os << std::setw(2) << std::setfill('0') << std::hex << int(cano.digest[i]);
  }
  str = os.str();
}

size_t FiltersList::file_size(const FilePtr a) {
  size_t size_a = fs::file_size((a->get_resolved_dir_entry()).path());
  return size_a;
}

// https://github.com/Cyan4973/xxHash/blob/805c00b68fa754200ada0c207ffeaa7a4409377c/cli/xxhsum.c#L243
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
