#!/usr/bin/env python3
"""Check a sha1sum-format file (tolerant of Windows CRLF)."""
from __future__ import annotations

import hashlib
import pathlib
import sys


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <file.sha1>", file=sys.stderr)
        return 2
    ok = True
    for raw in pathlib.Path(sys.argv[1]).read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        expect, path = line.split()
        digest = hashlib.sha1(pathlib.Path(path).read_bytes()).hexdigest()
        match = digest == expect.lower()
        ok = ok and match
        print(f"{path}: {'OK' if match else 'FAILED'}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
