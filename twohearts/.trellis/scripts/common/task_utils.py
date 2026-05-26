#!/usr/bin/env python3
"""
Task 工具函数。
"""

from __future__ import annotations

import shutil
import sys
from datetime import datetime
from pathlib import Path

from .paths import get_repo_root, get_tasks_dir


def is_safe_task_path(task_path: str, repo_root: Path | None = None) -> bool:
    if repo_root is None:
        repo_root = get_repo_root()

    normalized = task_path.replace("\\", "/")
    if not normalized or normalized == "null":
        print("错误：task 路径为空", file=sys.stderr)
        return False
    if Path(task_path).is_absolute():
        print(f"错误：不允许使用绝对路径：{task_path}", file=sys.stderr)
        return False
    if normalized in (".", "..") or normalized.startswith("./") or normalized.startswith("../") or ".." in normalized:
        print(f"错误：不允许使用越界路径：{task_path}", file=sys.stderr)
        return False

    abs_path = repo_root / Path(normalized)
    if abs_path.exists():
        try:
            if abs_path.resolve() == repo_root.resolve():
                print(f"错误：路径解析后落在仓库根目录：{task_path}", file=sys.stderr)
                return False
        except (OSError, IOError):
            pass
    return True


def find_task_by_name(task_name: str, tasks_dir: Path) -> Path | None:
    if not task_name or not tasks_dir or not tasks_dir.is_dir():
        return None

    exact_match = tasks_dir / task_name
    if exact_match.is_dir():
        return exact_match

    for d in tasks_dir.iterdir():
        if d.is_dir() and d.name.endswith(f"-{task_name}"):
            return d
    return None


def archive_task_dir(task_dir_abs: Path, repo_root: Path | None = None) -> Path | None:
    if not task_dir_abs.is_dir():
        print(f"错误：未找到 task 目录：{task_dir_abs}", file=sys.stderr)
        return None

    tasks_dir = task_dir_abs.parent
    archive_dir = tasks_dir / "archive"
    year_month = datetime.now().strftime("%Y-%m")
    month_dir = archive_dir / year_month

    try:
        month_dir.mkdir(parents=True, exist_ok=True)
    except (OSError, IOError) as e:
        print(f"错误：创建归档目录失败：{e}", file=sys.stderr)
        return None

    dest = month_dir / task_dir_abs.name
    try:
        shutil.move(str(task_dir_abs), str(dest))
    except (OSError, IOError, shutil.Error) as e:
        print(f"错误：归档 task 失败：{e}", file=sys.stderr)
        return None

    return dest


def archive_task_complete(task_dir_abs: Path, repo_root: Path | None = None) -> dict[str, str]:
    if not task_dir_abs.is_dir():
        print(f"错误：未找到 task 目录：{task_dir_abs}", file=sys.stderr)
        return {}

    archive_dest = archive_task_dir(task_dir_abs, repo_root)
    if archive_dest:
        return {"archived_to": str(archive_dest)}
    return {}


def resolve_task_dir(target_dir: str, repo_root: Path) -> Path:
    if not target_dir:
        return Path()

    normalized = target_dir.replace("\\", "/")
    while normalized.startswith("./"):
        normalized = normalized[2:]

    if Path(target_dir).is_absolute():
        return Path(target_dir)
    if "/" in normalized or normalized.startswith(".trellis"):
        return repo_root / Path(normalized)

    tasks_dir = get_tasks_dir(repo_root)
    found = find_task_by_name(target_dir, tasks_dir)
    if found:
        return found

    return repo_root / Path(normalized)


def run_task_hooks(event: str, task_json_path: Path, repo_root: Path) -> None:
    import os
    import subprocess

    from .config import get_hooks
    from .log import Colors, colored

    commands = get_hooks(event, repo_root)
    if not commands:
        return

    env = {**os.environ, "TASK_JSON_PATH": str(task_json_path)}
    for cmd in commands:
        try:
            result = subprocess.run(
                cmd,
                shell=True,
                cwd=repo_root,
                env=env,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
            )
            if result.returncode != 0:
                print(colored(f"[WARN] Hook 执行失败（{event}）：{cmd}", Colors.YELLOW), file=sys.stderr)
                if result.stderr.strip():
                    print(f"  {result.stderr.strip()}", file=sys.stderr)
        except Exception as e:
            print(colored(f"[WARN] Hook 执行异常（{event}）：{cmd} - {e}", Colors.YELLOW), file=sys.stderr)


if __name__ == "__main__":
    repo = get_repo_root()
    tasks = get_tasks_dir(repo)
    print(f"Tasks dir: {tasks}")
    print(f"is_safe_task_path('.trellis/tasks/test'): {is_safe_task_path('.trellis/tasks/test', repo)}")
    print(f"is_safe_task_path('../test'): {is_safe_task_path('../test', repo)}")
