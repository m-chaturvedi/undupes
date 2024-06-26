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

#include <stdio.h>  // for fclose, fopen, fread, fseek, FILE, SEEK_SET
#include <string.h> // for memcmp

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

  fseek(file1, 0, SEEK_SET);
  fseek(file2, 0, SEEK_SET);
  size_t r2;
  do {
    size_t r1;
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
