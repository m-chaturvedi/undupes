#!/bin/bash
set -euo pipefail

find tests/artifacts/dir_3 -type f -print0 | ./build/src/undupe | jq '.[].file_list'
