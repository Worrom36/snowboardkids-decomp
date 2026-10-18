#!/usr/bin/env python3
"""Launch a local 2P Online Battle test stack (wrapper around test_np)."""

from __future__ import annotations

import sys

from tools.online import test_np


def main() -> None:
    sys.argv = [sys.argv[0], "--players", "2", *sys.argv[1:]]
    test_np.main()


if __name__ == "__main__":
    main()
