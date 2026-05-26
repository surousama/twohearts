#!/usr/bin/env python3
"""
初始化工作流开发者身份。
"""

from __future__ import annotations

import sys

from common.developer import init_developer
from common.paths import DIR_WORKFLOW, FILE_DEVELOPER, get_developer


def main() -> None:
    if len(sys.argv) < 2:
        print(f"用法：{sys.argv[0]} <developer-name>")
        print()
        print("示例：")
        print(f"  {sys.argv[0]} john")
        sys.exit(1)

    name = sys.argv[1]
    existing = get_developer()
    if existing:
        print(f"开发者已初始化：{existing}")
        print()
        print(f"如需重新初始化，请先删除 {DIR_WORKFLOW}/{FILE_DEVELOPER}")
        sys.exit(0)

    sys.exit(0 if init_developer(name) else 1)


if __name__ == "__main__":
    main()
