#pragma once
#include <stddef.h>  // for size_t

#include <filesystem>  // for directory_entry
#include <fstream>
#include <set>
#include <ostream>  // for ostream
#include <string>   // for basic_string, string

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
  explicit File(const std::string &path)
      : dir_entry{path}, file_type{this->get_file_type()} {}
  FileType get_file_type() const;
  std::string get_path() const;

  // TODO: Remove this.
  fs::directory_entry get_resolved_dir_entry() const;

  bool check_file_or_log(const std::set<FileType> &accepted) const;

 private:
  FileType file_type;
  friend std::ostream &operator<<(std::ostream &os, const File &obj);
  friend bool operator==(const File &l, const File &r);

  size_t orig_path_length() const;

  size_t resolved_path_length() const;

  bool is_nonbroken_symlink() const;

  bool is_file() const;
  bool is_dir() const;

  std::string time_string() const;
};
