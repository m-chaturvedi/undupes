find ${TMP_DIR}/dir_3 -links 1 -type f -print0 | sort -z | ${UNDUPES} -d --dry-run ${TMP_DIR}/dry_run.out
