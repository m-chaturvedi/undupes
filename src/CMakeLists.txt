# Undupes: Find duplicate files.
# Copyright (C) 2024 Mmanu Chaturvedi <mmanu.chaturvedi@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
add_executable(undupes main.cpp file.h)

add_library(debug debug.h debug.cpp)
add_library(filter filter.h filter.cpp)
add_library(file file.h file.cpp debug.h)
add_library(filters_list filters_list.h filters_list.cpp)
add_library(io io.h io.cpp)
add_library(cli cli.h cli.cpp)
add_library(bin_compare_files bin_compare_files.h bin_compare_files.cpp)

target_link_libraries(
  undupes
  filters_list
  filter
  file
  io
  cli
  bin_compare_files
  nlohmann_json::nlohmann_json
  cxxopts
  pthread)

target_link_directories(undupes PRIVATE ${gflags_BINARY_DIR}
                        ${spdlog_BINARY_DIR})

install(TARGETS undupes RUNTIME DESTINATION bin)
