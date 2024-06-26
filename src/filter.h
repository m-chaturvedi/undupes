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
#pragma once
#include <stdint.h> // for uint64_t

#include <functional>    // for function
#include <iostream>      // for basic_ostream, operator<<, cout, endl
#include <list>          // for list, __list_iterator, operator!=
#include <memory>        // for shared_ptr
#include <ostream>       // for ostream
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

template <class Attr> class Filter {
public:
  Filter(const FileSets &_file_sets, Attr _attr)
      : file_sets{_file_sets}, attr{_attr} {};
  FileSets file_sets;
  FileSets new_file_sets;
  Attr attr;
};

template <class Attr> class HashableFilter : public Filter<Attr> {
public:
  // attr needs to have == and < defined.  We can reduce to just '=='.
  HashableFilter(const FileSets &_file_sets, Attr _attr)
      : Filter<Attr>{_file_sets, _attr} {
    for (FileVector files : _file_sets) {
      using ReturnType = typename std::invoke_result<Attr, FilePtr>::type;
      std::unordered_map<ReturnType, FileVector> M;
      for (auto &file : files) {
        M[_attr(file)].push_back(file);
      }
      for (auto const &[key, val] : M) {
        if (val.size() > 1)
          (Filter<Attr>::new_file_sets).push_back(std::move(val));
      }
    }
  }

  void print_with_filter(const FileSets &file_sets, Attr attr) const {
    for (FileVector files : file_sets) {
      for (auto &file : files) {
        std::cout << file << " " << attr(file) << std::endl;
      }
    }
  }
};

template <class Attr> class NonHashableFilter : public Filter<Attr> {
public:
  // TODO: Consider making the the strings as FilePtr
  NonHashableFilter(const FileSets &_file_sets, Attr _attr)
      : Filter<Attr>{_file_sets, _attr} {
    for (const FileVector &files : _file_sets) {
      std::list<FilePtr> file_list(files.begin(), files.end());

      while (!file_list.empty()) {
        FileVector file_vector{file_list.front()};
        FilePtr top = file_vector.at(0);
        file_list.erase(file_list.begin());
        for (auto it = file_list.begin(); it != file_list.end();) {
          if (_attr(top->get_path(), (*it)->get_path())) {
            file_vector.emplace_back(*it);
            it = file_list.erase(it);
          } else
            ++it;
        }
        if (file_vector.size() > 1)
          Filter<Attr>::new_file_sets.emplace_back(file_vector);
      }
    }
  }

private:
};
