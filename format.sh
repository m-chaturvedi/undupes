#!/usr/bin/env bash
set -euo pipefail

# For CMake files.
find . \( -path ./build -prune -a -type f \) \
	-o -name 'CMakeLists.txt' -exec cmake-format -i '{}' \;

# For C++ files.
find ./src ./tests \( -name '*.h' -o -name '*.cpp' \) \
	-exec clang-format -i '{}' \;

# For Bash files
find . \( -path ./build -prune -a -type f \) -o -name '*.sh' \
	-exec shfmt -w '{}' \;

# For Python files
find . \( -path ./build -prune -a -type f \) -o -name '*.py' \
	-exec black -q '{}' \;

pushd doxygen
doxygen doxygen_config.conf
popd
