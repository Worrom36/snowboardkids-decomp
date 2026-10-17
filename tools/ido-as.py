#!/usr/bin/env python3
"""Windows entrypoint for ido-as.sh (avoids cmd file-association dialogs)."""
from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path


def find_bash() -> str:
    candidates = [
        os.environ.get("BASH"),
        shutil.which("bash"),
        r"C:\Program Files\Git\usr\bin\bash.exe",
        r"C:\Program Files\Git\bin\bash.exe",
        str(Path(__file__).resolve().parents[1] / "tools" / "msys64" / "usr" / "bin" / "bash.exe"),
    ]
    for candidate in candidates:
        if candidate and Path(candidate).is_file():
            return candidate
    raise FileNotFoundError("bash not found (need Git Bash or tools/msys64)")


def main() -> int:
    if len(sys.argv) != 5:
        print(f"usage: {sys.argv[0]} <input.s> <output.o> <mips2|mips3> <-Oflag>", file=sys.stderr)
        return 2

    root = Path(__file__).resolve().parents[1]
    script = Path(__file__).resolve().parent / "ido-as.sh"
    bash = find_bash()

    tmp = root / "tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    ido = Path(os.environ.get("IDO_DIR", root / "tools" / "ido-recomp" / "windows"))

    env = os.environ.copy()
    env["MSYS_NO_PATHCONV"] = "1"
    env["MSYS2_ARG_CONV_EXCL"] = "*"
    env["IDO_DIR"] = str(ido.resolve()).replace("\\", "/")
    env["USR_LIB"] = env["IDO_DIR"]
    env["TEMP"] = str(tmp)
    env["TMP"] = str(tmp)
    env["IDO_HOST_TMP"] = str(tmp).replace("\\", "/")
    env["TMPDIR"] = "/tmp"
    # Keep Windows cmd.exe first — never put msys64/usr/bin ahead of System32
    # (that shadows cmd and can pop "Open with" dialogs).
    env["PATH"] = os.pathsep.join(
        [
            str(ido.resolve()),
            str(root / "tools" / "binutils" / "bin"),
            r"C:\Windows\System32",
            r"C:\Windows",
            str(Path(bash).parent),
            env.get("PATH", ""),
        ]
    )

    # Prefer POSIX-ish paths for the shell script.
    def sh_path(value: str) -> str:
        path = Path(value).resolve().as_posix()
        if len(path) >= 2 and path[1] == ":":
            return f"/{path[0].lower()}{path[2:]}"
        return path

    cmd = [
        bash,
        sh_path(str(script)),
        sh_path(sys.argv[1]),
        sh_path(sys.argv[2]),
        sys.argv[3],
        sys.argv[4],
    ]
    Path(sys.argv[2]).resolve().parent.mkdir(parents=True, exist_ok=True)
    return subprocess.call(cmd, env=env, cwd=str(root))


if __name__ == "__main__":
    raise SystemExit(main())
