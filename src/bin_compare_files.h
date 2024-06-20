#pragma once
#include <iterator>
#include <string>

// TODO: Readup!
// Initial version taken from:
// https://stackoverflow.com/questions/15118661/whats-the-fastest-way-to-tell-whether-two-strings-or-binary-files-are-different

using II = std::istreambuf_iterator<char>;
bool compare_files_fdupes(const std::string &filename_1,
                          const std::string &filename2);
