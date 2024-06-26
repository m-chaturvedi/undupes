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
#pragma once
#include <stddef.h> // for size_t

#include <filesystem> // for directory_entry
#include <fstream>
#include <ostream> // for ostream
#include <set>
#include <string> // for basic_string, string

#include "debug.h"

namespace fs = std::filesystem;

/**
 * @brief The FileType object used to enumerate various file types.
 */
enum class FileType {
  regular_file,
  symlinked_file,
  symlinked_dir,
  regular_dir,
  broken_symlink,
  other
};

std::ostream &operator<<(std::ostream &os, const FileType &obj);

/**
 * @brief The File class.  This is used for representing file objects.  They
 * could be a file/directory/symlink etc. like mentioned in the FileType class.
 */
class File {
public:
  fs::directory_entry dir_entry;
  size_t index;
  explicit File(const std::string &path)
      : dir_entry{path}, index{}, file_type{this->get_file_type()} {}
  FileType get_file_type() const;
  std::string get_path() const;

  // TODO: Remove this.
  fs::directory_entry get_resolved_dir_entry() const;

  bool check_file_or_log(const std::set<FileType> &accepted) const;

private:
  FileType file_type;
  friend std::ostream &operator<<(std::ostream &os, const File &obj);
  friend bool operator==(const File &l, const File &r);

  bool is_nonbroken_symlink() const;
  std::string time_string() const;
};
