#pragma once
#include <ostream>  // for ostream
#include <stdint.h> // for uint64_t

#include <functional>    // for function
#include <iostream>      // for basic_ostream, operator<<, cout, endl
#include <list>          // for list, __list_iterator, operator!=
#include <memory>        // for shared_ptr
#include <string>        // for basic_string, string
#include <type_traits>   // for invoke_result
#include <unordered_map> // for unordered_map
#include <utility>       // for pair
#include <vector>        // for vector

#include "file.h" // for File

using FilePtr = std::shared_ptr<File>;
using FileVector = std::vector<FilePtr>;
using FileSets = std::vector<FileVector>;
using KeepFileSets = std::vector<std::vector<bool>>;

std::ostream &operator<<(std::ostream &os, const FilePtr &file);
std::ostream &operator<<(std::ostream &os,
                         const std::pair<uint64_t, uint64_t> p);

bool operator==(const FilePtr &l, const FilePtr &r);
bool operator==(const FileVector &l, const FileVector &r);
bool operator==(const FileSets &l, const FileSets &r);

class Filter {
public:
  FileSets file_sets;
  FileSets new_file_sets;
  // attr needs to have == and < defined.  We can reduce to just '=='.
  template <class Attr>
  Filter(const FileSets &file_sets, Attr attr) : file_sets{file_sets} {
    for (FileVector files : file_sets) {
      using ReturnType = typename std::invoke_result<Attr, FilePtr>::type;
      std::unordered_map<ReturnType, FileVector> M;
      for (auto &file : files) {
        M[attr(file)].push_back(file);
      }
      for (auto const &[key, val] : M) {
        if (val.size() > 1)
          new_file_sets.push_back(std::move(val));
      }
    }
  }

  // TODO: Consider making the the strings as FilePtr
  // TODO: Cleanup the hashable boolean by correcting the design.
  Filter(const FileSets &file_sets,
         std::function<bool(const std::string &, const std::string &)> attr,
         bool hashable)
      : file_sets{file_sets} {
    for (const FileVector &files : file_sets) {
      std::list<FilePtr> file_list(files.begin(), files.end());

      while (!file_list.empty()) {
        FileVector file_vector{file_list.front()};
        FilePtr top = file_vector.at(0);
        file_list.erase(file_list.begin());
        for (auto it = file_list.begin(); it != file_list.end();) {
          if (attr(top->get_path(), (*it)->get_path())) {
            file_vector.emplace_back(*it);
            it = file_list.erase(it);
          } else
            ++it;
        }
        if (file_vector.size() > 1)
          new_file_sets.emplace_back(file_vector);
      }
    }
  }

  template <class Attr>
  void print_with_filter(const FileSets &file_sets, Attr attr) {
    for (FileVector files : file_sets) {
      for (auto &file : files) {
        std::cout << file << " " << attr(file) << std::endl;
      }
    }
  }

private:
};
