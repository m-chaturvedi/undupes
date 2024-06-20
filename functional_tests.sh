#!/bin/bash
set -euo pipefail

TMP_DIR=$(mktemp -d /tmp/func_test-XXXXX)

function cleanup() {
	declare -i ret_code=$?
	if [[ $ret_code -ne 0 ]]; then
		echo "Test failed."
	fi
	rm -rf ${TMP_DIR}
}

trap 'cleanup' EXIT

function vanilla_test() {
	find tests/artifacts/dir_3 -type f -print0 | sort -z | ./build/src/undupe |
		jq '.[].file_list' >${TMP_DIR}/a.txt

	diff ${TMP_DIR}/a.txt tests/artifacts/functional_test/expected_dir_3_output.txt
}

function summary_test() {
	find tests/artifacts/dir_3 -type f -print0 | sort -z | ./build/src/undupe -m >${TMP_DIR}/b.txt

	diff ${TMP_DIR}/b.txt tests/artifacts/functional_test/expected_dir_3_summary.txt
}

function test_with_j_f_dupes() {
	declare -a DIRS_TO_CHECK=(
		${PWD}
		$HOME/Downloads
		$HOME/Music
		$HOME/Desktop
		$HOME/bin
	)

	for dir_name in ${DIRS_TO_CHECK[@]}; do
		if [[ -d $dir_name ]]; then
			# Checks with jdupes
			python3 test_with_fdupes.py $dir_name | grep 'Perfect Match!'
			# Checkes with fdupes
			PROG_NAME="fdupes" python3 test_with_fdupes.py $dir_name | grep 'Perfect Match!'
		fi

	done
}

vanilla_test
summary_test
test_with_j_f_dupes

echo "Tests passed."
