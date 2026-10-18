#!/usr/bin/env python3
"""Shared Mupen64Plus win64 bundle download / extract / copy helpers."""
from __future__ import annotations

import hashlib
import shutil
import urllib.request
import zipfile
from pathlib import Path

MUPEN_VERSION = "2.6.0"
MUPEN_ZIP = f"mupen64plus-bundle-win64-{MUPEN_VERSION}.zip"
MUPEN_URL = (
    f"https://github.com/mupen64plus/mupen64plus-core/releases/download/"
    f"{MUPEN_VERSION}/{MUPEN_ZIP}"
)
MUPEN_SHA256 = "8292c68d6ffc3428d4181ac09b1368ef2adb2cc568325545aac78cb6c1e66e21"

# Keep the playable folder small: drop sample ROM and cheat DB.
SKIP_NAMES = {
    "m64p_test_rom.v64",
    "mupencheat.txt",
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def ensure_mupen(root: Path) -> Path:
    """Return tools/mupen64plus containing the console UI + plugins."""
    dest = root / "tools" / "mupen64plus"
    if (dest / "mupen64plus-ui-console.exe").is_file():
        return dest

    cache = root / "tmp" / MUPEN_ZIP
    cache.parent.mkdir(parents=True, exist_ok=True)
    if not cache.is_file():
        print(f"Downloading {MUPEN_URL} ...", flush=True)
        urllib.request.urlretrieve(MUPEN_URL, cache)

    digest = sha256_file(cache)
    if digest.lower() != MUPEN_SHA256:
        cache.unlink(missing_ok=True)
        raise SystemExit(
            f"mupen_bundle: Mupen64Plus zip hash mismatch\n"
            f"  expected {MUPEN_SHA256}\n"
            f"  got      {digest}"
        )

    extract_root = root / "tmp" / "mupen-extract"
    if extract_root.exists():
        shutil.rmtree(extract_root)
    extract_root.mkdir(parents=True)
    with zipfile.ZipFile(cache) as archive:
        archive.extractall(extract_root)

    release = extract_root / "Release"
    if not release.is_dir():
        release = extract_root

    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir(parents=True)

    for path in release.rglob("*"):
        if not path.is_file() or path.name in SKIP_NAMES:
            continue
        out = dest / path.relative_to(release)
        out.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, out)

    shutil.rmtree(extract_root, ignore_errors=True)
    if not (dest / "mupen64plus-ui-console.exe").is_file():
        raise SystemExit(f"mupen_bundle: mupen64plus-ui-console.exe missing under {dest}")
    return dest


def copy_mupen_tree(mupen_dir: Path, out_dir: Path) -> None:
    """Copy vendored Mupen files into out_dir (creates parents as needed)."""
    out_dir.mkdir(parents=True, exist_ok=True)
    for path in mupen_dir.rglob("*"):
        if not path.is_file():
            continue
        target = out_dir / path.relative_to(mupen_dir)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)
