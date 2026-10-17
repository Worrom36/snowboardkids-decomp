#!/usr/bin/env python3
"""Invoke Windows IDO cc.exe with Win32-friendly paths."""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
import hashlib
from pathlib import Path

# Flags whose following argument is a filesystem path.
PATH_FLAGS = {"-o", "-I", "-L", "-include", "-isystem"}
# Flags whose following argument is a non-path token (must not be shortened).
VALUE_FLAGS = {"-G", "-g", "-mips", "-O", "-woff", "-Wab", "-D", "-U", "-A"}


def winpath(value: str) -> str:
    """Normalize separators without forcing absolute paths.

    Absolute Windows paths inflate IDO #line directives and can push audio
    TUs over the Windows cfe ~32KiB limit.
    """
    if not value:
        return value
    normalized = value.replace("\\", "/")
    path = Path(value)
    try:
        if path.is_absolute():
            return path.resolve().as_posix()
    except OSError:
        pass
    return normalized


def project_tmp(root: Path) -> Path:
    """Keep build temps on the project drive (D:), not the often-full C: TEMP."""
    tmp = root / "tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    # IDO opens unix /tmp as <cwd-drive>:\tmp\...
    drive_tmp = Path(root.drive + "/tmp") if root.drive else Path("D:/tmp")
    try:
        drive_tmp.mkdir(parents=True, exist_ok=True)
    except OSError:
        pass
    return tmp


def ensure_ido_runtime(ido: Path) -> None:
    """Fix packaged Windows IDO quirks that break large translation units."""
    err = ido / "err.english.cc"
    if err.is_file():
        data = err.read_bytes()
        if b"\r\n" in data:
            err.write_bytes(data.replace(b"\r\n", b"\n"))


def shorten_source(root: Path, tmp: Path, source: str) -> str:
    """Copy absolute/long sources to a short relative path under tmp/cc/.

    asm-processor feeds absolute D:/tmp/asm_processor.../preprocessed_*.c paths;
    those strings land in #line directives and blow the Windows cfe ~32KiB budget.
    """
    path = Path(source)
    try:
        if not path.is_absolute():
            return source.replace("\\", "/")
        resolved = path.resolve()
    except OSError:
        return source.replace("\\", "/")

    # Already short enough relative-style names are fine.
    as_posix = resolved.as_posix()
    if len(as_posix) < 40 and "asm_processor" not in as_posix:
        return as_posix

    cc_dir = tmp / "cc"
    cc_dir.mkdir(parents=True, exist_ok=True)
    digest = hashlib.md5(as_posix.encode("utf-8")).hexdigest()[:16]
    dest = cc_dir / f"{digest}{resolved.suffix or '.c'}"
    try:
        if not dest.exists() or dest.stat().st_mtime < resolved.stat().st_mtime:
            shutil.copy2(resolved, dest)
    except OSError:
        return as_posix
    # Relative to project root so #line stays short.
    try:
        return dest.relative_to(root).as_posix()
    except ValueError:
        return dest.as_posix()


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    ido = Path(os.environ.get("IDO_DIR", root / "tools" / "ido-recomp" / "windows"))
    ensure_ido_runtime(ido)
    tmp = project_tmp(root)

    cc = ido / "cc.exe"
    if not cc.is_file():
        cc = ido / "cc"
    # -S emits basename.s beside cwd; shortening the source renames that .s and
    # breaks Makefile rules that assemble $(notdir $*).s (course_select BSS).
    emit_asm = "-S" in sys.argv[1:]

    args: list[str] = [str(cc)]
    pending_path = False
    pending_value = False
    for arg in sys.argv[1:]:
        if pending_path:
            args.append(winpath(arg))
            pending_path = False
            continue
        if pending_value:
            args.append(arg)
            pending_value = False
            continue
        if arg in PATH_FLAGS:
            args.append(arg)
            pending_path = True
        elif arg in VALUE_FLAGS:
            args.append(arg)
            pending_value = True
        elif len(arg) > 2 and arg.startswith(("-I", "-L")):
            args.append(arg[:2] + winpath(arg[2:]))
        elif len(arg) > 2 and arg.startswith("-o") and not arg[2:].isdigit():
            args.append("-o" + winpath(arg[2:]))
        elif arg.startswith("-"):
            args.append(arg)
        else:
            # Source / input file
            if emit_asm:
                args.append(winpath(arg))
            else:
                args.append(shorten_source(root, tmp, arg))
    env = os.environ.copy()
    env["MSYS_NO_PATHCONV"] = "1"
    env["MSYS2_ARG_CONV_EXCL"] = "*"
    env["USR_LIB"] = str(ido.resolve()).replace("\\", "/")
    # Python tempfile (asm-processor) on project D: tmp — do not set TMPDIR=/tmp
    # for the Python process that spawns us; only for IDO itself.
    env["TEMP"] = str(tmp)
    env["TMP"] = str(tmp)
    env["TMPDIR"] = "/tmp"
    # Patched Windows IDO maps guest /tmp -> this host directory (see libc_impl).
    env["IDO_HOST_TMP"] = str(tmp).replace("\\", "/")
    # Prefer the IDO dir so bundled msys-2.0.dll resolves without a global MSYS install.
    env["PATH"] = str(ido.resolve()) + os.pathsep + env.get("PATH", "")
    # Normal compiles use root-relative shortened sources (tmp/cc/...), so cwd must
    # be the project root. -S recipes `cd` into the object dir and expect basename.s
    # there — inherit the caller's cwd for those.
    cwd = None if emit_asm else str(root)
    return subprocess.call(args, env=env, cwd=cwd)


if __name__ == "__main__":
    raise SystemExit(main())
