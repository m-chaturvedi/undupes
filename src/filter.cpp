#include "filter.h"

#include <algorithm> // for find
#include <ostream>   // IWYU pragma: export

/**
 * @brief Output stream operator for FilePtr class
 *
 * @param os The ostream object.
 * @param file The file to write to the ostream
 *
 * @return A reference to the stream object to which the file has been written.
 */
std::ostream &operator<<(std::ostream &os, const FilePtr &file) {
  os << *file.get();
  return os;
}

/**
 * @brief Equality operator for FilePtr
 *
 * @param l The FilePtr object on the left of the equality sign.
 * @param r The FilePtr object on the right of the equality sign.
 *
 * @return true if the object on the left of the equality is equal to the right
 * one.
 */
bool operator==(const FilePtr &l, const FilePtr &r) {
  return *l.get() == *r.get();
}

// TODO: Look into complexity, currently being used only for testing.
/**
 * @brief The equality operator for FileVector. Doesn't consider the order of
 * elements.
 *
 * @param l The FileVector on the left of the ==.
 * @param r The FileVector on the right of the ==.
 *
 * @return true if l == r and false otherwise.
 */
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
/**
 * @brief The equality operatore for FileSets.  Doesn't care about the order of
 * the container FileVector.
 *
 * @param l The FileSets on the left of the ==.
 * @param r The FileSets on the right of the ==.
 *
 * @return trye if l == r and false otherwise.
 */
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

/**
 * @brief The output stream operator for the uint64_t pair.  Maybe needed for
 * xxhash.
 *
 * @param os The ostream object to write the uint64_t pair to.
 * @param p The uint64_t pair to write.
 *
 * @return Reference to the ostream object to which the pair has been written.
 */
std::ostream &operator<<(std::ostream &os,
                         const std::pair<uint64_t, uint64_t> p) {
  os << "( " << p.first << ", " << p.second << " )";
  return os;
}
