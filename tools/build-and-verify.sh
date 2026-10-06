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
extra_targets=()
if ((shift_tests)); then
    extra_targets+=(runtime-index-objects)
fi
BUILD_AND_VERIFY=1 QUIET=1 make -j8 BUILD_MODE="$mode" all "${extra_targets[@]}"
if ((shift_tests)); then
    python3 -m unittest discover -s tools -p 'test_shift*.py'
    PYTHONPATH="tools:.:${PYTHONPATH:-}" python3 -m unittest \
        tools.test_readable_assets tools.test_asset_bundles \
        tools.test_course_graphics tools.test_course_sprite_table \
        tools.test_course_surface_data tools.test_course_display_list \
        tools.test_huffman_asset tools.test_textconv \
        tools.test_course_definitions tools.test_sk1_to_sk2_course
    QUIET=1 make shift-test BUILD_MODE="$mode"
    python3 tools/run_shift_tests.py --build-dir "$( [[ "$mode" == matching ]] && echo build || echo build/shiftable )"
    inventory_dir="$( [[ "$mode" == matching ]] && echo build || echo build/shiftable )"
    python3 tools/shift_inventory.py --build "$inventory_dir" --out "$inventory_dir/shift-inventory.json"
fi
if [[ "$mode" == matching ]]; then
    printf "\033[0;32mBUILD SUCCEEDED\033[0m. The rebuilt ROM matches snowboardkids.sha1.\n"
else
    printf "\033[0;32mBUILD SUCCEEDED\033[0m. Shiftable ROM: build/shiftable/snowboardkids.z64 (N64 CRC applied; original SHA1 not required).\n"
fi
