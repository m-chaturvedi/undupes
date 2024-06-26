#!/usr/bin/env bash
set -euo pipefail

function cleanup() {
	declare -r ret=$?
	if [[ $ret -ne 0 ]]; then
		exit $ret
	fi
}

trap cleanup EXIT

export BUILD_DIR="${BUILD_DIR:-${PWD}/build_docker}"

declare -a IMAGE_LIST=(
	ubuntu:24.04
	ubuntu:22.04
	ubuntu:20.04
	debian:bookworm
	debian:bullseye
)

for image in ${IMAGE_LIST[@]}; do
	undupes_image=${image}_undupes
	if [[ $(docker images -q ${undupes_image} | wc -l) -ne 1 ]]; then
		docker build --tag ${undupes_image} --build-arg="IMAGE_NAME=${image}" .
	fi

	docker run --rm --env BUILD_DIR -v $PWD:/undupes -it ${undupes_image} \
		bash -c 'rm -rf ${BUILD_DIR}; bash -x install.sh;'
done

echo "All tests passed. All packages built."
