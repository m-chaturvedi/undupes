#include "file.h"

#include <set>
#include <ostream>      // for operator<<, char_traits, basic_ostream
#include <string_view>  // for operator==, basic_string_view, operator""sv

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
    if (!fs::exists(dir_entry.path())) return FileType::broken_symlink;
    if (resolved_dir_entry.is_regular_file()) return FileType::symlinked_file;
    if (resolved_dir_entry.is_directory()) return FileType::symlinked_dir;
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

bool operator==(const File &l, const File &r) {
  return fs::absolute(l.dir_entry.path()) == fs::absolute(r.dir_entry.path());
}

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

std::string File::get_path() const { return this->dir_entry.path().string(); }

size_t File::orig_path_length() const { return this->get_path().size(); }

size_t File::resolved_path_length() const {
  return this->get_resolved_dir_entry().path().string().size();
}

bool File::is_file() const {
  return file_type == FileType::symlinked_file ||
         file_type == FileType::regular_file;
}

bool File::is_dir() const {
  return file_type == FileType::symlinked_dir ||
         file_type == FileType::regular_dir;
}

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
