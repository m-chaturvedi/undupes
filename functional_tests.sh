#!/bin/bash
set -euo pipefail

TMP_DIR=$(mktemp -d /tmp/func_test-XXXXX)
export TMP_DIR

function cleanup() {
	declare -i ret_code=$?
	if [[ $ret_code -ne 0 ]]; then
		echo "Test failed."
	fi
	rm -rf ${TMP_DIR}
}

trap 'cleanup' EXIT

declare -r UNDUPES=${UNDUPES:-${PWD}/build/install/bin/undupes}
export UNDUPES
declare -r ORIG_DIR=${PWD}

function vanilla_test() {
	find tests/artifacts/dir_3 -type f -print0 | sort -z | ${UNDUPES} |
		jq '.[].file_list' >${TMP_DIR}/a.txt

	diff ${TMP_DIR}/a.txt tests/artifacts/functional_test/expected_dir_3_output.txt
}

function summary_test() {
	find tests/artifacts/dir_3 -type f -print0 | sort -z | ${UNDUPES} -m >${TMP_DIR}/b.txt

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

	# TODO: Read up.
	# https://stackoverflow.com/questions/20532195/socat-with-a-virtual-tty-link-and-fork-removes-my-pty-link
	cat tests/artifacts/functional_test/expected_dir_3_delete_1.in |
		socat stdio exec:'bash -x tests/artifacts/functional_test/delete_script.sh',pty,setsid,echo=0

	pushd ${TMP_DIR}/dir_3
	declare -a DOES_NOT_EXIST=(1KB_1.copy.1 1KB_1.copy.3 1KB_2.copy.3 3KB_1.copy.2
		4KB_1 4KB_1.copy.1 4KB_1.copy.2 4KB_1.copy.3 4KB_1.copy.4 4KB_1.copy.5)

	declare -a EXISTS=(1KB_1 1KB_1.copy.2 1KB_2 1KB_2.copy.1 1KB_2.copy.2 1KB_3 1KB_4 1KB_5 3KB_1 3KB_1.copy.1 3KB_1.copy.3 3KB_2 4KB_4)

	for f in ${DOES_NOT_EXIST[@]}; do
		if [[ -e $f ]]; then false; fi
	done

	for f in ${EXISTS[@]}; do
		if ! [[ -e $f ]]; then false; fi
	done

	popd

	cp -R tests/artifacts/dir_3 ${TMP_DIR}
	pushd ${TMP_DIR}
	find . -type f -print0 | ${UNDUPES} >${TMP_DIR}/dir_3_op.json
	popd
	diff ${TMP_DIR}/dir_3_op.json tests/artifacts/functional_test/expected_dir_3.json
	diff <(ls -1 tests/artifacts/dir_3) <(ls -1 ${TMP_DIR}/dir_3)
}

function dry_run_test() {
	cp -R tests/artifacts/dir_3 ${TMP_DIR}
	declare -a DOES_NOT_EXIST=(1KB_1.copy.1 1KB_1.copy.3 1KB_2.copy.3 3KB_1.copy.2
		4KB_1 4KB_1.copy.1 4KB_1.copy.2 4KB_1.copy.3 4KB_1.copy.4 4KB_1.copy.5)

	cat tests/artifacts/functional_test/expected_dir_3_delete_1.in |
		socat stdio exec:'bash -x tests/artifacts/functional_test/delete_script_dry_run.sh',pty,setsid,echo=0

	for f in ${DOES_NOT_EXIST[@]}; do
		echo "\"${TMP_DIR}/dir_3/$f\"" >>${TMP_DIR}/expected_dry_run.out
	done

	diff <(cat ${TMP_DIR}/dry_run.out | sort) <(cat ${TMP_DIR}/expected_dry_run.out | sort)
	diff <(ls -1 tests/artifacts/dir_3) <(ls -1 ${TMP_DIR}/dir_3)
}

function warnings_test() {
	find tests/artifacts -print0 |
		${UNDUPES} -m 2>${TMP_DIR}/warnings_test.out
	cat ${TMP_DIR}/warnings_test.out | cut -f16 -d' ' >${TMP_DIR}/warning_file_names.out

	find tests/artifacts -type d >${TMP_DIR}/artifacts_dir_names.txt
	echo "tests/artifacts/broken_symlink_1" >>${TMP_DIR}/artifacts_dir_names.txt
	echo "tests/artifacts/nested_broken_symlink_1" >>${TMP_DIR}/artifacts_dir_names.txt
	echo "tests/artifacts/symlink_3" >>${TMP_DIR}/artifacts_dir_names.txt
	diff <(cat ${TMP_DIR}/warning_file_names.out | sort) \
		<(cat ${TMP_DIR}/artifacts_dir_names.txt | sort)
}

function timeout_test() {
	sleep 10 | ${UNDUPES} >${TMP_DIR}/timeout.stdout 2>${TMP_DIR}/timeout.stderr
	if [[ $(cat ${TMP_DIR}/timeout.stdout) != "" ]]; then false; fi
	stderr_content=$(
		cat <<EOF
Timeout occurred.
terminate called without an active exception
EOF
	)

	if [[ $(cat ${TMP_DIR}/timeout.stderr) != "$stderr_content" ]]; then false; fi

}

function sorting_test() {
	find tests/artifacts/dir_3 -type f -print0 | sort -z |
		${UNDUPES} >${TMP_DIR}/sorted.out
	diff ${TMP_DIR}/sorted.out tests/io/sorted_file_io_test1.out
}

# timeout_test
vanilla_test
summary_test
test_with_j_f_dupes
delete_test
dry_run_test
warnings_test
sorting_test

echo "Tests passed."
