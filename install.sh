#!/bin/bash
set -euo pipefail

declare -r DISTRO_INFO="$(dpkg --print-architecture 2>/dev/null)_$(lsb_release -is 2>/dev/null)_$(lsb_release -cs 2>/dev/null)"

declare -r INSTALL_PREFIX=${INSTALL_PREFIX:-"/usr/local"}
declare -r UNDUPES="${INSTALL_PREFIX}/bin/undupes"
declare -r BUILD_DIR="${BUILD_DIR:-${PWD}/build}"

export UNDUPES

cmake \
	-DCMAKE_CXX_COMPILER=/usr/bin/g++ \
	-DCMAKE_BUILD_TYPE=Release \
	-S . \
	-B ${BUILD_DIR} \
	-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
	-DDISTRO_INFO:STRING=${DISTRO_INFO} \
	.

pushd ${BUILD_DIR}
make -j
make install
make undupes
popd

bash -x run_all_tests.sh
pushd ${BUILD_DIR}
cpack -G DEB undupes
popd
