#!/bin/bash
set -euo pipefail

cmake \
	-DCMAKE_CXX_COMPILER=/usr/bin/g++ \
	-DCMAKE_BUILD_TYPE=Release \
	-S . \
	-B build \
	-DCMAKE_INSTALL_PREFIX=/usr/local/bin \
	.

pushd build
make -j
make install
popd
