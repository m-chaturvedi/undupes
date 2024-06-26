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

BUILD_DIR=${BUILD_DIR:-build}
INSIDE_DOCKER=${INSIDE_DOCKER:-0}

if ! [[ -d ${BUILD_DIR} ]]; then
	BUILD_TYPE="Debug" bash build.sh
else
	pushd "${BUILD_DIR}"
	if [[ ${INSIDE_DOCKER} -eq 1 ]]; then
		make -j
	else
		ninja install
	fi
	popd
fi

pushd "${BUILD_DIR}/tests"
ctest --output-on-failure
popd
bash functional_tests.sh
