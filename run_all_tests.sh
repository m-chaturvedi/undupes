#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${BUILD_DIR:-build}

if ! [[ -d ${BUILD_DIR} ]]; then
	BUILD_TYPE="Debug" bash build.sh
else
	pushd "${BUILD_DIR}"
	if [[ ${INSIDE_DOCKER} -eq 1 ]]; then
		make -j
	else
		ninja
	fi
	popd
fi

pushd "${BUILD_DIR}/tests"
ctest --output-on-failure
popd
bash functional_tests.sh
