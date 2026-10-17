#!/usr/bin/env bash
set -euo pipefail

# Git Bash/MSYS rewrites /d/... args for Win32 exes and can crash IDO; keep paths literal.
export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL='*'
# Patched Windows IDO maps guest /tmp creat/open onto IDO_HOST_TMP (or TEMP).
if [ -z "${IDO_HOST_TMP:-}" ] && [ -n "${TEMP:-}" ]; then
    export IDO_HOST_TMP="$TEMP"
fi

if [ "$#" -ne 4 ]; then
    echo "usage: $0 <input.s> <output.o> <mips2|mips3> <-Oflag>" >&2
    exit 2
fi

input=$1
output=$2
isa=$3
opt=$4

case "$isa" in
    mips2)
        mips_isa=2
        mips_flag=-mips2
        dwopcode=
        ;;
    mips3)
        mips_isa=3
        mips_flag=-mips3
        dwopcode=-dwopcode
        ;;
    *)
        echo "unsupported ISA: $isa" >&2
        exit 2
        ;;
esac

case "$opt" in
    -O0|-O1|-O2|-O3) ;;
    *)
        echo "unsupported optimization flag: $opt" >&2
        exit 2
        ;;
esac

root=$(cd "$(dirname "$0")/.." && pwd)
uname_s=$(uname -s 2>/dev/null || echo unknown)
case "$uname_s" in
    MINGW*|MSYS*|CYGWIN*) windows_host=1 ;;
    *) windows_host=0 ;;
esac

if [ -n "${IDO_DIR:-}" ]; then
    ido="$IDO_DIR"
else
    case "$uname_s" in
        Darwin) ido_platform=macos ;;
        Linux) ido_platform=linux ;;
        MINGW*|MSYS*|CYGWIN*|Windows_NT) ido_platform=windows ;;
        *)
            echo "unsupported host for IDO: $uname_s (set IDO_DIR)" >&2
            exit 2
            ;;
    esac
    ido="$root/tools/ido-recomp/$ido_platform"
fi

cfe="$ido/cfe"
as0="$ido/as0"
as1="$ido/as1"
if [ -f "${cfe}.exe" ]; then cfe="${cfe}.exe"; fi
if [ -f "${as0}.exe" ]; then as0="${as0}.exe"; fi
if [ -f "${as1}.exe" ]; then as1="${as1}.exe"; fi
export USR_LIB="$ido"
# Prefer bundled msys-2.0.dll next to the IDO tools.
export PATH="$ido:$PATH"
# Guest TMPDIR must be unix-style so creat("/tmp/...") hits our host redirect.
export TMPDIR=/tmp

# Mixed Windows paths (D:/...) are accepted by IDO and safe under MSYS_NO_PATHCONV.
winpath() {
    if [ "$windows_host" -eq 1 ] && command -v cygpath >/dev/null 2>&1; then
        cygpath -m "$1"
    else
        printf '%s\n' "$1"
    fi
}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

preprocessed="$tmpdir/source.i"
binasm="$tmpdir/source.B"
symtab="$tmpdir/source.T"
lf_input="$tmpdir/source.s"
tr -d '\r' < "$input" > "$lf_input"

root_w=$(winpath "$root")
input_w=$(winpath "$lf_input")
preprocessed_w=$(winpath "$preprocessed")
binasm_w=$(winpath "$binasm")
symtab_w=$(winpath "$symtab")
output_w=$(winpath "$output")
cfe_w=$(winpath "$cfe")
as0_w=$(winpath "$as0")
as1_w=$(winpath "$as1")
# Local #include "exceptasm.h" etc. live beside the original .s
input_dir_w=$(winpath "$(cd "$(dirname "$input")" && pwd)")

c_defs=(
    -D_MIPS_FPSET=16
    -D_MIPS_ISA="$mips_isa"
    -D_ABIO32=1
    -D_MIPS_SIM=_ABIO32
    -D_MIPS_SZINT=32
    -D_MIPS_SZLONG=32
    -D_MIPS_SZPTR=32
    -nostdinc
    -DLANGUAGE_ASSEMBLY
    -D_LANGUAGE_ASSEMBLY
    -D__INLINE_INTRINSICS
    -Dsgi
    -D__sgi
    -Dunix
    -Dmips
    -Dhost_mips
    -D__unix
    -D__host_mips
    -D_SVR4_SOURCE
    -D_MODERN_C
    -D_SGI_SOURCE
    -D__DSO__
    -DSYSTYPE_SVR4
    -D_SYSTYPE_SVR4
    -D_LONGLONG
    -D__mips="$mips_isa"
    -I"$input_dir_w"
    -I"$root_w/include"
    -I"$root_w/include/compiler/ido"
    -I"$root_w/include/PR"
    -D_MIPSEB
    -DMIPSEB
    -D__STDC__=1
    -D_MIPS_SZLONG=32
    -DCOMPILING_LIBULTRA
    -DBUILD_VERSION=VERSION_I
    -DBUILD_VERSION_STRING=\"2.0I\"
    -D_FINALROM
    -DNDEBUG
)

"$cfe_w" "${c_defs[@]}" "$input_w" -E -std0 \
    -DLANGUAGE_ASSEMBLY -D_LANGUAGE_ASSEMBLY -D__unix -D__host_mips -D__DSO__ \
    -I"$root_w/include" -I"$root_w/include/compiler/ido" -I"$root_w/include/PR" \
    -D_MIPSEB -DMIPSEB -D__STDC__=1 -D_MIPS_SZLONG=32 -DCOMPILING_LIBULTRA \
    -DBUILD_VERSION=VERSION_I -DBUILD_VERSION_STRING=\"2.0I\" -D_FINALROM -DNDEBUG \
    | tr -d '\r' > "$preprocessed"

"$as0_w" -G 0 -r4300_mul -r4300_mul "$mips_flag" -EB -g0 ${dwopcode:+"$dwopcode"} \
    "$opt" "$preprocessed_w" -o "$binasm_w" -t "$symtab_w"

as1_flags=(-pic0 -elf -G 0 -p0 -r4300_mul -r4300_mul "$mips_flag" -EB -g0)
if [ -n "$dwopcode" ]; then
    as1_flags+=("$dwopcode")
fi
as1_flags+=("$opt")
if [ "$opt" = "-O3" ]; then
    as1_flags+=(-noglobal -Olimit 5000)
fi

"$as1_w" "${as1_flags[@]}" "$binasm_w" -o "$output_w" -t "$symtab_w"
