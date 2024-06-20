#!/usr/bin/env bash
set -euo pipefail

# g++ -g main_threaded.cpp -lxxhash -lpthread -DTHREADED=0 -o seq
#
# g++ -g main_threaded.cpp -lxxhash -lpthread -DTHREADED=1 -o parallel

SOURCE_DIR=$PWD
BUILD_DIR=$PWD/build
INSTALL_DIR=$PWD/build/install
CUSTOM_TYPE=""
BUILD_TYPE="${BUILD_TYPE:-Release}"

export CC=/usr/bin/gcc-12
export CXX=/usr/bin/g++-12

declare -a ADDITIONAL_CMAKE_FLAGS=()

if [[ $CUSTOM_TYPE = "gprof" ]]; then
	BUILD_TYPE=""
	ADDITIONAL_CMAKE_FLAGS=(
		-DCMAKE_CXX_FLAGS=-pg
		-DCMAKE_EXE_LINKER_FLAGS=-pg
		-DCMAKE_SHARED_LINKER_FLAGS=-pg
	)
fi

cmake \
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	-S "$SOURCE_DIR" \
	-B "${BUILD_DIR}" \
	-GNinja \
	-DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	${ADDITIONAL_CMAKE_FLAGS[@]} \
	.

cmake --build $PWD/build

cmake --install $PWD/build

ln -f "$BUILD_DIR/compile_commands.json" "${SOURCE_DIR}"
