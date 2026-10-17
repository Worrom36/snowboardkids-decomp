#!/usr/bin/env bash
# Wrap Windows IDO cc.exe so Git Bash/MSYS path rewriting cannot crash it.
set -euo pipefail
export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL='*'

root=$(cd "$(dirname "$0")/.." && pwd)
if [ -n "${IDO_DIR:-}" ]; then
    ido="$IDO_DIR"
else
    ido="$root/tools/ido-recomp/windows"
fi
cc="$ido/cc"
[ -f "${cc}.exe" ] && cc="${cc}.exe"

winpath() {
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -m "$1"
    else
        printf '%s\n' "$1"
    fi
}

cc_w=$(winpath "$cc")
args=()
pending=
for arg in "$@"; do
    if [ -n "$pending" ]; then
        args+=("$(winpath "$arg")")
        pending=
        continue
    fi
    case "$arg" in
        -o|-I|-L|-include|-isystem)
            args+=("$arg")
            pending=1
            ;;
        -I*|-L*|-o*)
            opt=${arg:0:2}
            val=${arg:2}
            if [ -n "$val" ]; then
                args+=("${opt}$(winpath "$val")")
            else
                args+=("$arg")
                pending=1
            fi
            ;;
        -*)
            args+=("$arg")
            ;;
        *)
            args+=("$(winpath "$arg")")
            ;;
    esac
done

exec "$cc_w" "${args[@]}"
