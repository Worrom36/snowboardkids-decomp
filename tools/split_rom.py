#!/usr/bin/env python3
"""Run Splat with GLOBAL_ASM detection for regional assembly paths."""

import re

from splat.__main__ import splat_main
from splat.segtypes.common import c


if __name__ == "__main__":
    # Splat 0.39's directory pattern excludes the dot in asm.jp.
    c.C_GLOBAL_ASM_IDO_RE = re.compile(
        r'GLOBAL_ASM\("([^"\n]*/)?(\w+)\.s"\)', re.MULTILINE
    )
    splat_main()
