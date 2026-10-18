#!/usr/bin/env python3
"""Entry point: python tools/online_test_4p.py"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.online.test_np import main

if __name__ == "__main__":
    sys.argv = [sys.argv[0], "--players", "4", *sys.argv[1:]]
    main()
