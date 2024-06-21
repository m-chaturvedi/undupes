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

declare -r UNDUPE=${PWD}/build/src/undupe
export UNDUPE

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

function delete_test() {
	# remove 1KB_1.copy.1 1_KB_1.copy.3 1KB_2.copy.3 3KB_1.copy.2 all 4KB_1
	cp -R tests/artifacts/dir_3 ${TMP_DIR}

	export TMP_DIR

	cat tests/artifacts/functional_test/expected_dir_3_delete_1.in |
		socat stdio exec:'bash -x tests/artifacts/functional_test/delete_script.sh',pty,setsid,echo=0

	pushd ${TMP_DIR}/dir_3
	declare -a DOES_NOT_EXIST=(1KB_1.copy.1 1_KB_1.copy.3 1KB_2.copy.3 3KB_1.copy.2
		4KB_1 4KB_1.copy.1 4KB_1.copy.2 4KB_1.copy.3 4KB_1.copy.4 4KB_1.copy.5)

	declare -a EXISTS=(1KB_1 1KB_1.copy.2 1KB_2 1KB_2.copy.1 1KB_2.copy.2 1KB_3 1KB_4 1KB_5 3KB_1 3KB_1.copy.1 3KB_1.copy.3 3KB_2 4KB_4)

	for f in ${DOES_NOT_EXIST[@]}; do
		if [[ -e $f ]]; then false; fi
	done

	for f in ${EXISTS[@]}; do
		if ! [[ -e $f ]]; then false; fi
	done

	popd

}

vanilla_test
summary_test
test_with_j_f_dupes
delete_test

echo "Tests passed."
