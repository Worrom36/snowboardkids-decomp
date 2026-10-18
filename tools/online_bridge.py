#!/usr/bin/env python3
"""Entry point: python tools/online_bridge.py [--mailbox PATH]"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.online.mupen_bridge import main

if __name__ == "__main__":
    main()
