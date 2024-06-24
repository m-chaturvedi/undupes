#!/usr/bin/env bash
set -euo pipefail

function cleanup() {
	declare -r ret=$?
	if [[ $ret -ne 0 ]]; then
		exit $ret
	fi
}

trap cleanup EXIT

declare -a IMAGE_LIST=(
	ubuntu:24.04
	ubuntu:22.04
	ubuntu:20.04
	debian:bookworm
	debian:bullseye
)

for image in ${IMAGE_LIST[@]}; do
	undupe_image=${image}_undupe
	if [[ $(docker images -q ${undupe_image} | wc -l) -ne 1 ]]; then
		docker build --tag ${undupe_image} --build-arg="IMAGE_NAME=${image}" .
	fi

	docker run --rm -it -v $PWD:/undupe ${undupe_image} bash -c \
		'rm -rf build; bash -x install.sh; bash -x run_all_tests.sh;'
done

echo "All deb tests passed."
