#!/usr/bin/env bash
set -euo pipefail

trap 'echo "BUILD HAS FAILED. Treat this as a build failure; warnings or a non-matching checksum still count as failures."' ERR

build_make="${MAKE:-make}"
mode=matching
version="${VERSION:-us}"
while (($#)); do
    case "$1" in
        --version)
            [[ $# -ge 2 ]] || { echo "--version requires us or jp" >&2; exit 2; }
            version="$2"; shift 2 ;;
        --mode)
            [[ $# -ge 2 ]] || { echo "--mode requires matching or shiftable" >&2; exit 2; }
            mode="$2"; shift 2 ;;
        *) echo "Unknown argument: $1" >&2; exit 2 ;;
    esac
done
[[ "$mode" == matching || "$mode" == shiftable ]] || { echo "Invalid build mode: $mode" >&2; exit 2; }
[[ "$version" == us || "$version" == jp ]] || { echo "Invalid version: $version" >&2; exit 2; }
[[ "$version" == us || "$mode" == matching ]] || { echo "Shiftable builds currently support US only" >&2; exit 2; }

if [[ "$mode" == matching ]]; then
    QUIET=1 "$build_make" VERSION="$version" clean
else
    rm -rf build/shiftable
fi
QUIET=1 "$build_make" VERSION="$version" extract >/dev/null
BUILD_AND_VERIFY=1 QUIET=1 "$build_make" -j8 VERSION="$version" BUILD_MODE="$mode" all
if [[ "$mode" == matching ]]; then
    suffix=""
    [[ "$version" != jp ]] || suffix=".jp"
    printf "\033[0;32mBUILD SUCCEEDED\033[0m. The rebuilt ROM matches snowboardkids%s.sha1.\n" "$suffix"
else
    printf "\033[0;32mBUILD SUCCEEDED\033[0m. Shiftable ROM: build/shiftable/snowboardkids.z64 (N64 CRC applied; original SHA1 not required).\n"
fi
