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

function cleanup() {
	declare -r ret=$?
	if [[ $ret -ne 0 ]]; then
		exit $ret
	fi
}

trap cleanup EXIT

export BUILD_DIR="${BUILD_DIR:-${PWD}/build_docker}"

declare -a IMAGE_LIST=(
	ubuntu:24.04
	ubuntu:22.04
	ubuntu:20.04
	debian:bookworm
	debian:bullseye
)

for image in ${IMAGE_LIST[@]}; do
	undupes_image=${image}_undupes
	if [[ $(docker images -q ${undupes_image} | wc -l) -ne 1 ]]; then
		docker build --tag ${undupes_image} --build-arg="IMAGE_NAME=${image}" .
	fi

	docker run --rm --env BUILD_DIR -v $PWD:/undupes -it ${undupes_image} \
		bash -c 'rm -rf ${BUILD_DIR}; bash -x install.sh;'
done

echo "All tests passed. All packages built."
