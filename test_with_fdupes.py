import subprocess
import tempfile
import json
import sys
import time
from pprint import pprint

from typing import collections

USE_FD = False
prog_name = "jdupes -z"
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
    if USE_FD:
        files_output = subprocess.check_output(
            f"fd -u . --type f --print0 {path}", shell=True
        )
    else:
        files_output = subprocess.check_output(
            f"find {path} -type f -print0", shell=True
        )

    undupes_output = subprocess.check_output("./build/src/undupe", input=files_output)
    json_output = json.loads(undupes_output)

    file_sets = []
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
