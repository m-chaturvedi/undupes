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
import subprocess
import os
import tempfile
import json
import sys
import time
from pprint import pprint

from typing import collections

USE_FD = False
prog_name = os.getenv("PROG_NAME", "jdupes -z")
UNDUPES = os.getenv("UNDUPES", "./build/install/bin/undupes")
FIND_OPTIONS = os.getenv("FIND_OPTIONS", "")
#  prog_name = "fdupes"


def get_fdupes_output(path="."):
    global prog_name
    fdupes_output = subprocess.check_output(
        f"{prog_name} -H -r {path}", shell=True, universal_newlines=True
    )
    file_sets_split = fdupes_output.split("\n\n")
    file_sets_linewise = [x for x in file_sets_split if not x.isspace() and len(x) != 0]

    file_sets = []
    for file_set in file_sets_linewise:
        file_set_linewise_strip = file_set.strip()
        files_list = file_set_linewise_strip.split("\n")
        if files_list:
            files_list.sort()
            file_sets.append(files_list)

    return file_sets


def get_undupes_output(path="."):
    global USE_FD
    global UNDUPES
    global FIND_OPTIONS
    if USE_FD:
        files_output = subprocess.check_output(
            f"fd -u . --type f --print0 {path}", shell=True
        )
    else:
        files_output = subprocess.check_output(
            f"find {path} -type f {FIND_OPTIONS}-print0", shell=True
        )

    undupes_output = subprocess.check_output(UNDUPES, input=files_output)
    json_output = json.loads(undupes_output)

    file_sets = []

    if not json_output:
        return file_sets

    for ele in json_output:
        if ele["file_list"]:
            ele_list = ele["file_list"]
            ele_list.sort()
            file_sets.append(ele_list)

    file_sets.sort()
    return file_sets


def compare_file_sets(A, B):
    if len(A) != len(B):
        return False
    return collections.Counter(A) == collections.Counter(B)


class CustomTimer:
    def __init__(self, name=""):
        self.name = name
        pass

    def __enter__(self):
        self.start_time = time.time()

    def __exit__(self, *args):
        self.end_time = time.time()
        print(f"{self.name}: Elapsed time: {(self.end_time - self.start_time):.6f} s")


def main():
    global prog_name
    if len(sys.argv) == 1:
        path = "."
    else:
        path = " ".join(sys.argv[1:])

    with CustomTimer(prog_name) as ct:
        fdupes_sets = get_fdupes_output(path)

    with CustomTimer("undupes") as ct:
        undupes_sets = get_undupes_output(path)

    print(f"{prog_name} set size: {len(fdupes_sets)}")
    print(f"undupes set size: {len(undupes_sets)}")
    not_in_fdupes, not_in_undupes = [], []

    for a in undupes_sets:
        if a not in fdupes_sets:
            not_in_fdupes.append(a)

    for a in fdupes_sets:
        if a not in undupes_sets:
            not_in_undupes.append(a)

    if not_in_fdupes:
        print(f"Not in {prog_name}:")
        print(not_in_fdupes)
    if not_in_undupes:
        print(f"Not in undupes:")
        print(not_in_undupes)
    if len(not_in_fdupes) == 0 and len(not_in_undupes) == 0:
        print("Perfect Match!")


if __name__ == "__main__":
    main()
