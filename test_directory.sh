#!/bin/bash
set -euo pipefail

for i in ${1}/*; do
	if [[ -d $i ]]; then
		if ! python3 test_with_fdupes.py $i | grep 'Perfect Match!' >/dev/null; then
			echo $i
			python3 test_with_fdupes.py $i
		fi
	fi
done
