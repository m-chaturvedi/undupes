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
#include <stdio.h> // for size_t

#include <filesystem> // for filesystem
#include <string>     // for string

#include "filter.h" // for FilePtr

namespace fs = std::filesystem;

namespace FiltersList {
size_t file_size(const FilePtr a);
std::string xxhash(const FilePtr file);
std::string xxhash_4KB(const FilePtr file);

bool is_subdirectory(const std::filesystem::path &p1,
                     const std::filesystem::path &p2);

} // namespace FiltersList
