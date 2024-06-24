#!/usr/bin/env bash
set -euo pipefail

if ! [[ -d build ]]; then
	BUILD_TYPE="Debug" bash build.sh
else
	pushd build
	if [[ -z "$(which ninja)" ]]; then
		make -j
	else
		ninja
	fi
	popd
fi

pushd build/tests
ctest --output-on-failure
popd
bash functional_tests.sh
