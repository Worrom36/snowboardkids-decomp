#!/usr/bin/env python3
"""Entry point: python tools/online_client.py --host HOST --port PORT"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.online.client import main

if __name__ == "__main__":
    main()
