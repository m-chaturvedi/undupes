#!/usr/bin/env bash
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
set -euo pipefail

# For CMake files.
find . \( -path ./build -prune -a -type f \) \
	-o -name 'CMakeLists.txt' -exec cmake-format -i '{}' \;

# For C++ files.
find ./src ./tests \( -name '*.h' -o -name '*.cpp' \) \
	-exec clang-format -i '{}' \;

# For Bash files
find . \( -path ./build -prune -a -type f \) -o -name '*.sh' \
	-exec shfmt -w '{}' \;

# For Python files
find . \( -path ./build -prune -a -type f \) -o -name '*.py' \
	-exec black -q '{}' \;

pushd doxygen
doxygen doxygen_config.conf
popd
