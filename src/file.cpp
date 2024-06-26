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
#include "file.h"

#include <ostream> // for operator<<, char_traits, basic_ostream
#include <set>
#include <string_view> // for operator==, basic_string_view, operator""sv

#include "debug.h"

using namespace std::literals::string_view_literals;
/**
 * @brief The output operator for FileType
 *
 * @param os: the output stream
 * @param obj: FileType enumerated objected
 *
 * @return ostream object.
 */
std::ostream &operator<<(std::ostream &os, const FileType &obj) {
  switch (obj) {
  case FileType::regular_file:
    os << "RegularFile";
    break;
  case FileType::symlinked_file:
    os << "SymlinkedFile";
    break;
  case FileType::symlinked_dir:
    os << "SymlinkedDirectory";
    break;
  case FileType::regular_dir:
    os << "RegularDirectory";
    break;
  case FileType::broken_symlink:
    os << "BrokenSymlink";
    break;
  case FileType::other:
    os << "Other";
    break;
  }
  return os;
}

/**
 * @brief The FileType for the file represented by the class
 *
 * @return The FileType object.
 */
FileType File::get_file_type() const {
  // Symlinks return 1 for is_regular_file when passed as paths.
  if (dir_entry.is_regular_file() && !dir_entry.is_symlink())
    return FileType::regular_file;
  else if (dir_entry.is_directory() && !dir_entry.is_symlink())
    return FileType::regular_dir;
  else if (dir_entry.is_symlink()) {
    fs::directory_entry resolved_dir_entry = get_resolved_dir_entry();
    if (!fs::exists(dir_entry.path()))
      return FileType::broken_symlink;
    if (resolved_dir_entry.is_regular_file())
      return FileType::symlinked_file;
    if (resolved_dir_entry.is_directory())
      return FileType::symlinked_dir;
  }
  return FileType::other;
}

/**
 * @brief The  output operator for File object
 *
 * @param os: the ostream to print the File object to.
 * @param obj: The object to print.
 *
 * @return The ostream.
 */
std::ostream &operator<<(std::ostream &os, const File &obj) {
  os << "Path: " << fs::absolute(obj.dir_entry.path()).string();
  return os;
}

/**
 * @brief Equality operator for File class
 *
 * @param l File on the left of the =
 * @param r File on the right of the =
 *
 * @return true if the files are equal and false otherwise.
 */
bool operator==(const File &l, const File &r) {
  return fs::absolute(l.dir_entry.path()) == fs::absolute(r.dir_entry.path());
}

/**
 * @brief Gets the resolved path for a symlink.
 *
 * @return A directory_entry after resolving the path and an exception
 * otherwise.
 */
fs::directory_entry File::get_resolved_dir_entry() const {
  fs::path resolved_path;
  try {
    resolved_path = fs::canonical(this->dir_entry);
  } catch (const fs::filesystem_error &exp) {
    if (exp.what() == "cannot make canonical path: No such file or directory"sv)
      resolved_path = "";
  }
  return fs::directory_entry(resolved_path);
}

/**
 * @brief Get the file path string.
 *
 * @return A string containing the path for the dir_entry
 */
std::string File::get_path() const { return this->dir_entry.path().string(); }

/**
 * @brief Check whether a file is a file or a symlink pointing to a file.
 *
 * @param accepted A set of FileTypes.
 *
 * @return true if the file is of the accepted type, false otherwise and create
 * an spdlog.
 */
bool File::check_file_or_log(const std::set<FileType> &accepted) const {
  if (accepted.find(this->get_file_type()) == accepted.end()) {
    spdlog::warn("Path not a file or a symlink to a file, skipping: {}",
                 this->get_path());
    return false;
  }

  std::ifstream ifs;
  ifs.open(this->get_path(), std::ios_base::in);
  if (!ifs.good()) {
    spdlog::warn("Could not open file, skipping: {}", this->get_path());
    return false;
  }

  ifs.close();
  return true;
}
