#include "filter.h"

#include <algorithm> // for find
#include <ostream>   // IWYU pragma: export

std::ostream &operator<<(std::ostream &os, const FilePtr &file) {
  os << *file.get();
  return os;
}

bool operator==(const FilePtr &l, const FilePtr &r) {
  return *l.get() == *r.get();
}

// TODO: Look into complexity, currently being used only for testing.
bool operator==(const FileVector &l, const FileVector &r) {
  if (l.size() != r.size())
    return false;
  for (auto &element : l) {
    FileVector::const_iterator it = std::find(r.begin(), r.end(), element);
    if (it == r.end())
      return false;
  }
  return true;
}

// TODO: Look into complexity, currently being used only for testing.
bool operator==(const FileSets &l, const FileSets &r) {
  if (l.size() != r.size())
    return false;
  for (auto &element : l) {
    FileSets::const_iterator it = std::find(r.begin(), r.end(), element);
    if (it == r.end())
      return false;
  }
  return true;
}

std::ostream &operator<<(std::ostream &os,
                         const std::pair<uint64_t, uint64_t> p) {
  os << "( " << p.first << ", " << p.second << " )";
  return os;
}
