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
# SPDX-License-Identifier: AGPL-3.0
# SPDX-FileCopyrightText: 2024 Mmanu Chaturvedi <mmanu.chaturvedi@gmail.com>
set -euo pipefail

SOURCE_DIR=$PWD
BUILD_DIR=${BUILD_DIR:-$PWD/build}
INSTALL_DIR=$PWD/build/install
CUSTOM_TYPE="${CUSTOM_TYPE:-}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

export CC=/usr/bin/gcc-12
export CXX=/usr/bin/g++-12

declare -a ADDITIONAL_CMAKE_FLAGS=()

if [[ $CUSTOM_TYPE = "gprof" ]]; then
	BUILD_TYPE=""
	ADDITIONAL_CMAKE_FLAGS=(
		-DCMAKE_CXX_FLAGS=-pg
		-DCMAKE_EXE_LINKER_FLAGS=-pg
		-DCMAKE_SHARED_LINKER_FLAGS=-pg
	)
elif [[ $CUSTOM_TYPE = "tsan" ]]; then
	BUILD_TYPE="DEBUG"
	ADDITIONAL_CMAKE_FLAGS=(
		-DCMAKE_CXX_FLAGS="-fsanitize=thread"
		-DCMAKE_C_FLAGS="-fsanitize=thread"
		-DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
		-DCMAKE_MODULE_LINKER_FLAGS="-fsanitize=thread"
	)
elif [[ $CUSTOM_TYPE = "asan" ]]; then
	BUILD_TYPE="DEBUG"
	ADDITIONAL_CMAKE_FLAGS=(
		-DCMAKE_CXX_FLAGS="-fsanitize=address"
		-DCMAKE_C_FLAGS="-fsanitize=address"
		-DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
		-DCMAKE_MODULE_LINKER_FLAGS="-fsanitize=address"
	)
fi

cmake \
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	-S "$SOURCE_DIR" \
	-B "${BUILD_DIR}" \
	-GNinja \
	-DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	${ADDITIONAL_CMAKE_FLAGS[@]} \
	.

cmake --build $PWD/build

cmake --install $PWD/build

ln -f "$BUILD_DIR/compile_commands.json" "${SOURCE_DIR}"
