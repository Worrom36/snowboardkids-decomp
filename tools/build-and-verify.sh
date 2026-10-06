#!/usr/bin/env bash
set -euo pipefail

trap 'echo "BUILD HAS FAILED. Treat this as a build failure; warnings or a non-matching checksum still count as failures."' ERR

mode=matching
shift_tests=0
while (($#)); do
    case "$1" in
        --mode)
            [[ $# -ge 2 ]] || { echo "--mode requires matching or shiftable" >&2; exit 2; }
            mode="$2"; shift 2 ;;
        --shift-tests) shift_tests=1; shift ;;
        *) echo "Unknown argument: $1" >&2; exit 2 ;;
    esac
done
[[ "$mode" == matching || "$mode" == shiftable ]] || { echo "Invalid build mode: $mode" >&2; exit 2; }

if [[ "$mode" == matching ]]; then
    QUIET=1 make clean
else
    rm -rf build/shiftable
fi
QUIET=1 make extract >/dev/null
BUILD_AND_VERIFY=1 QUIET=1 make -j8 BUILD_MODE="$mode"
if ((shift_tests)); then
    python3 -m unittest discover -s tools -p 'test_shift*.py'
    QUIET=1 make shift-test BUILD_MODE="$mode"
fi
if [[ "$mode" == matching ]]; then
    printf "\033[0;32mBUILD SUCCEEDED\033[0m. The rebuilt ROM matches snowboardkids.sha1.\n"
else
    printf "\033[0;32mBUILD SUCCEEDED\033[0m. Shiftable ROM: build/shiftable/snowboardkids.z64 (N64 CRC applied; original SHA1 not required).\n"
fi
