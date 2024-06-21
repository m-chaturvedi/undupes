cp -R tests/artifacts/dir_3 /tmp/
find /tmp/dir_3 -links 1 -type f -print0 | sort -z | ./tsan_build/src/undupe -d
