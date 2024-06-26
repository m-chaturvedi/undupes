#!/bin/bash
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

declare -r DISTRO_INFO="$(dpkg --print-architecture 2>/dev/null)_$(lsb_release -is 2>/dev/null)_$(lsb_release -cs 2>/dev/null)"

declare -r BUILD_DIR="${BUILD_DIR:-${PWD}/build}"
declare -r INSTALL_PREFIX=${INSTALL_PREFIX:-"${BUILD_DIR}/install"}
declare -r PACKAGES_DIR="${PACKAGES_DIR:-${PWD}/_packages}"

export UNDUPES="${INSTALL_PREFIX}/bin/undupes"

cmake \
	-DCMAKE_CXX_COMPILER=/usr/bin/g++ \
	-DCMAKE_BUILD_TYPE=Release \
	-S . \
	-B ${BUILD_DIR} \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
	-DDISTRO_INFO:STRING=${DISTRO_INFO} \
	.

pushd ${BUILD_DIR}
make -j
make install
make undupes
popd

bash -x run_all_tests.sh
pushd ${BUILD_DIR}
cpack -G DEB undupes
popd

dpkg -i ${PACKAGES_DIR}/*${DISTRO_INFO}.deb
UNDUPES=$(find /opt -type f -name undupes) bash -x run_all_tests.sh
