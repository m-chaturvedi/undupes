#include <stdio.h>  // for fclose, fopen, fread, fseek, FILE, SEEK_SET
#include <string.h> // for memcmp
                    //

#include <string> // for basic_string, string
#define CHUNK_SIZE 65536
/**
 * @brief Binary compare two files. TODO: Handle singint.
 *
 * @param filename_1 Filename of the first file.
 * @param filename_2 Filename of the second file.
 *
 * @return Boolean whether the files are equal or not.
 */
// Taken from fdupes/confirmmatch.c
bool compare_files_fdupes(const std::string &filename_1,
                          const std::string &filename_2) {
  FILE *file1 = fopen(filename_1.c_str(), "rb");
  FILE *file2 = fopen(filename_2.c_str(), "rb");
  unsigned char c1[CHUNK_SIZE];
  unsigned char c2[CHUNK_SIZE];
  size_t r1;
  size_t r2;

  fseek(file1, 0, SEEK_SET);
  fseek(file2, 0, SEEK_SET);
  do {
    r1 = fread(c1, sizeof(unsigned char), sizeof(c1), file1);
    r2 = fread(c2, sizeof(unsigned char), sizeof(c2), file2);

    if (r1 != r2) {
      fclose(file1);
      fclose(file2);
      return 0; /* file lengths are different */
    }
    if (memcmp(c1, c2, r1)) {
      fclose(file1);
      fclose(file2);
      return 0; /* file contents are different */
    }
  } while (r2);

  fclose(file1);
  fclose(file2);
  return 1;
}
