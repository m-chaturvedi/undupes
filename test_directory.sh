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
TEST_WITH_SUMMARY=1

for i in ${1}/*; do
	if [[ -d $i ]]; then
		if [[ $TEST_WITH_SUMMARY -eq 1 ]]; then
			find $i -links 1 -type f -print0 | ./build_tsan/src/undupes -m
		elif ! python3 test_with_fdupes.py $i | grep 'Perfect Match!' >/dev/null; then
			echo $i
			python3 test_with_fdupes.py $i
		fi
	fi
done
