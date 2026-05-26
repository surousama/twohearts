#!/usr/bin/env python3
"""
开发者管理工具。
"""

from __future__ import annotations

import sys
from datetime import datetime
from pathlib import Path

from .paths import (
    DIR_TASKS,
    DIR_WORKFLOW,
    DIR_WORKSPACE,
    FILE_DEVELOPER,
    FILE_JOURNAL_PREFIX,
    check_developer,
    get_developer,
    get_repo_root,
)


def init_developer(name: str, repo_root: Path | None = None) -> bool:
    if not name:
        print("错误：必须提供开发者名称", file=sys.stderr)
        return False

    if repo_root is None:
        repo_root = get_repo_root()

    dev_file = repo_root / DIR_WORKFLOW / FILE_DEVELOPER
    workspace_dir = repo_root / DIR_WORKFLOW / DIR_WORKSPACE / name

    initialized_at = datetime.now().isoformat()
    try:
        dev_file.write_text(f"name={name}\ninitialized_at={initialized_at}\n", encoding="utf-8")
    except (OSError, IOError) as e:
        print(f"错误：创建 .developer 文件失败：{e}", file=sys.stderr)
        return False

    try:
        workspace_dir.mkdir(parents=True, exist_ok=True)
    except (OSError, IOError) as e:
        print(f"错误：创建工作区目录失败：{e}", file=sys.stderr)
        return False

    journal_file = workspace_dir / f"{FILE_JOURNAL_PREFIX}1.md"
    if not journal_file.exists():
        today = datetime.now().strftime("%Y-%m-%d")
        journal_content = f"""# Journal - {name} (Part 1)

> AI development session journal
> Started: {today}

---

"""
        try:
            journal_file.write_text(journal_content, encoding="utf-8")
        except (OSError, IOError) as e:
            print(f"错误：创建 journal 文件失败：{e}", file=sys.stderr)
            return False

    index_file = workspace_dir / "index.md"
    if not index_file.exists():
        index_content = f"""# Workspace Index - {name}

> Journal tracking for AI development sessions.

---

## Current Status

<!-- @@@auto:current-status -->
- **Active File**: `journal-1.md`
- **Total Sessions**: 0
- **Last Active**: -
<!-- @@@/auto:current-status -->

---

## Active Documents

<!-- @@@auto:active-documents -->
| File | Lines | Status |
|------|-------|--------|
| `journal-1.md` | ~0 | Active |
<!-- @@@/auto:active-documents -->

---

## Session History

<!-- @@@auto:session-history -->
| # | Date | Title | Commits | Branch |
|---|------|-------|---------|--------|
<!-- @@@/auto:session-history -->

---

## Notes

- Sessions are appended to journal files
- New journal file created when current exceeds 2000 lines
- Use `add_session.py` to record sessions
"""
        try:
            index_file.write_text(index_content, encoding="utf-8")
        except (OSError, IOError) as e:
            print(f"错误：创建 index.md 失败：{e}", file=sys.stderr)
            return False

    print(f"开发者初始化完成：{name}")
    print(f"  .developer 文件：{dev_file}")
    print(f"  工作区目录：{workspace_dir}")
    return True


def ensure_developer(repo_root: Path | None = None) -> None:
    if repo_root is None:
        repo_root = get_repo_root()

    if not check_developer(repo_root):
        print("错误：开发者尚未初始化。", file=sys.stderr)
        print(f"请运行：python ./{DIR_WORKFLOW}/scripts/init_developer.py <your-name>", file=sys.stderr)
        sys.exit(1)


def show_developer_info(repo_root: Path | None = None) -> None:
    if repo_root is None:
        repo_root = get_repo_root()

    developer = get_developer(repo_root)
    if not developer:
        print("开发者：未初始化")
    else:
        print(f"开发者：{developer}")
        print(f"工作区：{DIR_WORKFLOW}/{DIR_WORKSPACE}/{developer}/")
        print(f"Tasks：{DIR_WORKFLOW}/{DIR_TASKS}/")


if __name__ == "__main__":
    show_developer_info()
