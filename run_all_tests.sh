#!/usr/bin/env bash
set -euo pipefail

if ! [[ -d build ]]; then
	BUILD_TYPE="Debug" bash build.sh
else
	pushd build
	ninja
	popd
fi

pushd build/tests
ctest --output-on-failure
popd
bash functional_tests.sh
