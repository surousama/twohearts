#!/usr/bin/env python3
"""
Task JSONL 上下文管理。
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from .log import Colors, colored
from .paths import get_repo_root
from .task_utils import resolve_task_dir


def cmd_add_context(args: argparse.Namespace) -> int:
    repo_root = get_repo_root()
    target_dir = resolve_task_dir(args.dir, repo_root)

    jsonl_name = args.file
    path = args.path
    reason = args.reason or "手动补充"

    if not target_dir.is_dir():
        print(colored(f"错误：未找到目录：{target_dir}", Colors.RED))
        return 1

    if not jsonl_name.endswith(".jsonl"):
        jsonl_name = f"{jsonl_name}.jsonl"

    jsonl_file = target_dir / jsonl_name
    full_path = repo_root / path

    entry_type = "file"
    if full_path.is_dir():
        entry_type = "directory"
        if not path.endswith("/"):
            path = f"{path}/"
    elif not full_path.is_file():
        print(colored(f"错误：未找到路径：{path}", Colors.RED))
        return 1

    if jsonl_file.is_file():
        content = jsonl_file.read_text(encoding="utf-8")
        if f'"{path}"' in content:
            print(colored(f"提示：{path} 已存在于上下文中", Colors.YELLOW))
            return 0

    entry = {"file": path, "reason": reason}
    if entry_type == "directory":
        entry["type"] = "directory"

    with jsonl_file.open("a", encoding="utf-8") as f:
        f.write(json.dumps(entry, ensure_ascii=False) + "\n")

    kind = "目录" if entry_type == "directory" else "文件"
    print(colored(f"已添加{kind}上下文：{path}", Colors.GREEN))
    return 0


def cmd_validate(args: argparse.Namespace) -> int:
    repo_root = get_repo_root()
    target_dir = resolve_task_dir(args.dir, repo_root)

    if not target_dir.is_dir():
        print(colored("错误：必须提供有效的 task 目录", Colors.RED))
        return 1

    print(colored("=== 校验上下文文件 ===", Colors.BLUE))
    print(f"目标目录：{target_dir}")
    print()

    total_errors = 0
    for jsonl_name in ["implement.jsonl", "check.jsonl"]:
        total_errors += _validate_jsonl(target_dir / jsonl_name, repo_root)

    print()
    if total_errors == 0:
        print(colored("✓ 全部校验通过", Colors.GREEN))
        return 0

    print(colored(f"✗ 校验失败（共 {total_errors} 处错误）", Colors.RED))
    return 1


def _validate_jsonl(jsonl_file: Path, repo_root: Path) -> int:
    file_name = jsonl_file.name
    errors = 0

    if not jsonl_file.is_file():
        print(f"  {colored(f'{file_name}: 文件不存在（已跳过）', Colors.YELLOW)}")
        return 0

    line_num = 0
    real_entries = 0
    for line in jsonl_file.read_text(encoding="utf-8").splitlines():
        line_num += 1
        if not line.strip():
            continue

        try:
            data = json.loads(line)
        except json.JSONDecodeError:
            print(f"  {colored(f'{file_name}:{line_num}: JSON 无效', Colors.RED)}")
            errors += 1
            continue

        file_path = data.get("file")
        entry_type = data.get("type", "file")
        if not file_path:
            continue

        real_entries += 1
        full_path = repo_root / file_path
        if entry_type == "directory":
            if not full_path.is_dir():
                print(f"  {colored(f'{file_name}:{line_num}: 目录不存在：{file_path}', Colors.RED)}")
                errors += 1
        else:
            if not full_path.is_file():
                print(f"  {colored(f'{file_name}:{line_num}: 文件不存在：{file_path}', Colors.RED)}")
                errors += 1

    if errors == 0:
        print(f"  {colored(f'{file_name}: ✓（{real_entries} 条）', Colors.GREEN)}")
    else:
        print(f"  {colored(f'{file_name}: ✗（{errors} 处错误）', Colors.RED)}")

    return errors


def cmd_list_context(args: argparse.Namespace) -> int:
    target_dir = resolve_task_dir(args.dir, get_repo_root())

    if not target_dir.is_dir():
        print(colored("错误：必须提供有效的 task 目录", Colors.RED))
        return 1

    print(colored("=== 上下文文件 ===", Colors.BLUE))
    print()

    for jsonl_name in ["implement.jsonl", "check.jsonl"]:
        jsonl_file = target_dir / jsonl_name
        if not jsonl_file.is_file():
            continue

        print(colored(f"[{jsonl_name}]", Colors.CYAN))

        count = 0
        seed_only = True
        for line in jsonl_file.read_text(encoding="utf-8").splitlines():
            if not line.strip():
                continue
            try:
                data = json.loads(line)
            except json.JSONDecodeError:
                continue

            file_path = data.get("file")
            if not file_path:
                continue
            seed_only = False

            count += 1
            entry_type = data.get("type", "file")
            reason = data.get("reason", "-")

            if entry_type == "directory":
                print(f"  {colored(f'{count}.', Colors.GREEN)} [DIR] {file_path}")
            else:
                print(f"  {colored(f'{count}.', Colors.GREEN)} {file_path}")
            print(f"     {colored('->', Colors.YELLOW)} {reason}")

        if seed_only:
            print(f"  {colored('（当前还没有整理出的真实条目，只有种子行）', Colors.YELLOW)}")
        print()

    return 0
