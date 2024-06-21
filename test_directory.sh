#!/bin/bash
set -euo pipefail
TEST_WITH_SUMMARY=1

for i in ${1}/*; do
	if [[ -d $i ]]; then
		if [[ $TEST_WITH_SUMMARY -eq 1 ]]; then
			find $i -links 1 -type f -print0 | ./tsan_build/src/undupe -m
		elif ! python3 test_with_fdupes.py $i | grep 'Perfect Match!' >/dev/null; then
			echo $i
			python3 test_with_fdupes.py $i
		fi
	fi
done
