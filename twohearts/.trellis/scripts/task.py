#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Task 管理脚本。

用法：
    python task.py create "<title>" [--slug <name>] [--assignee <dev>] [--priority P0|P1|P2|P3] [--parent <dir>] [--package <pkg>]
    python task.py add-context <dir> <file> <path> [reason]  # 添加 JSONL 上下文条目
    python task.py validate <dir>                            # 校验 JSONL 文件
    python task.py list-context <dir>                        # 列出 JSONL 条目
    python task.py start <dir>                               # 设置当前活跃 task
    python task.py current [--source]                        # 显示当前活跃 task
    python task.py finish                                    # 清除当前活跃 task
    python task.py set-branch <dir> <branch>                 # 设置 git 分支
    python task.py set-base-branch <dir> <branch>            # 设置 PR 目标分支
    python task.py set-scope <dir> <scope>                   # 设置 PR scope
    python task.py archive <task-dir>                        # 归档已完成 task
    python task.py list                                      # 列出未归档 task
    python task.py list-archive [month]                      # 列出归档 task
    python task.py add-subtask <parent-dir> <child-dir>      # 关联父子 task
    python task.py remove-subtask <parent-dir> <child-dir>   # 解除父子 task 关联
"""

from __future__ import annotations

import argparse
import sys

from common.active_task import (
    clear_active_task,
    resolve_active_task,
    resolve_context_key,
    set_active_task,
)
from common.io import read_json, write_json
from common.log import Colors, colored
from common.paths import (
    DIR_TASKS,
    DIR_WORKFLOW,
    FILE_TASK_JSON,
    get_current_task,
    get_developer,
    get_repo_root,
    get_tasks_dir,
)
from common.task_context import cmd_add_context, cmd_list_context, cmd_validate
from common.task_store import (
    cmd_add_subtask,
    cmd_archive,
    cmd_create,
    cmd_remove_subtask,
    cmd_set_base_branch,
    cmd_set_branch,
    cmd_set_scope,
)
from common.tasks import children_progress, iter_active_tasks
from common.task_utils import resolve_task_dir, run_task_hooks


def cmd_start(args: argparse.Namespace) -> int:
    """设置当前活跃 task。"""
    repo_root = get_repo_root()
    task_input = args.dir

    if not task_input:
        print(colored("错误：必须提供 task 目录或 task 名称", Colors.RED))
        return 1

    full_path = resolve_task_dir(task_input, repo_root)
    if not full_path.is_dir():
        print(colored(f"错误：未找到 task：{task_input}", Colors.RED))
        print("提示：可使用 task 名称，或传入完整路径，例如 `.trellis/tasks/05-26-示例任务`")
        return 1

    try:
        task_dir = full_path.relative_to(repo_root).as_posix()
    except ValueError:
        task_dir = str(full_path)

    task_json_path = full_path / FILE_TASK_JSON

    if not resolve_context_key():
        print(
            colored(
                "当前会话没有可用的 session identity；本次不会持久化 active-task 指针，"
                "AI 将仅基于当前对话上下文继续工作。",
                Colors.YELLOW,
            )
        )
        print(
            colored(
                "提示：请在能暴露 session identity 的 AI IDE/会话中运行，"
                "或先设置 `TRELLIS_CONTEXT_ID` 再执行 `task.py start`。",
                Colors.YELLOW,
            )
        )

        if task_json_path.is_file():
            data = read_json(task_json_path)
            if data and data.get("status") == "planning":
                data["status"] = "in_progress"
                if write_json(task_json_path, data):
                    print(colored("已更新状态：planning -> in_progress（降级模式）", Colors.GREEN))
            run_task_hooks("after_start", task_json_path, repo_root)
        return 0

    active = set_active_task(task_dir, repo_root)
    if not active:
        print(colored("错误：设置当前 task 失败", Colors.RED))
        return 1

    print(colored(f"已设置当前 task：{task_dir}", Colors.GREEN))
    print(f"来源：{active.source}")

    if task_json_path.is_file():
        data = read_json(task_json_path)
        if data and data.get("status") == "planning":
            data["status"] = "in_progress"
            if write_json(task_json_path, data):
                print(colored("已更新状态：planning -> in_progress", Colors.GREEN))

    print()
    print(colored("后续 hook 会从该 task 的 JSONL 文件注入上下文。", Colors.BLUE))
    run_task_hooks("after_start", task_json_path, repo_root)
    return 0


def cmd_finish(args: argparse.Namespace) -> int:
    """清除当前活跃 task。"""
    repo_root = get_repo_root()
    active = clear_active_task(repo_root)
    current = active.task_path

    if not current:
        print(colored("当前没有已设置的 task", Colors.YELLOW))
        return 0

    task_json_path = repo_root / current / FILE_TASK_JSON
    print(colored(f"已清除当前 task（原为：{current}）", Colors.GREEN))
    print(f"来源：{active.source}")

    if task_json_path.is_file():
        run_task_hooks("after_finish", task_json_path, repo_root)
    return 0


def cmd_current(args: argparse.Namespace) -> int:
    """显示当前活跃 task。"""
    repo_root = get_repo_root()
    active = resolve_active_task(repo_root)

    if args.source:
        print(f"当前 task：{active.task_path or '(none)'}")
        print(f"来源：{active.source}")
        if active.stale:
            print("状态：stale")
        return 0 if active.task_path else 1

    if active.task_path:
        print(active.task_path)
        return 0

    return 1


def cmd_list(args: argparse.Namespace) -> int:
    """列出未归档 task。"""
    repo_root = get_repo_root()
    tasks_dir = get_tasks_dir(repo_root)
    current_task = get_current_task(repo_root)
    developer = get_developer(repo_root)
    filter_mine = args.mine
    filter_status = args.status

    if filter_mine:
        if not developer:
            print(colored("错误：尚未设置开发者，请先运行 init_developer.py", Colors.RED), file=sys.stderr)
            return 1
        print(colored(f"我的 task（负责人：{developer}）：", Colors.BLUE))
    else:
        print(colored("全部未归档 task：", Colors.BLUE))
    print()

    all_tasks = {t.dir_name: t for t in iter_active_tasks(tasks_dir)}
    all_statuses = {name: t.status for name, t in all_tasks.items()}
    count = 0

    def _print_task(dir_name: str, indent: int = 0) -> None:
        nonlocal count
        t = all_tasks[dir_name]

        if filter_mine and (t.assignee or "-") != developer:
            return
        if filter_status and t.status != filter_status:
            return

        relative_path = f"{DIR_WORKFLOW}/{DIR_TASKS}/{dir_name}"
        marker = ""
        if relative_path == current_task:
            marker = f" {colored('<- current', Colors.GREEN)}"

        progress = children_progress(t.children, all_statuses)
        pkg_tag = f" @{t.package}" if t.package else ""
        prefix = "  " * indent + "  - "

        if filter_mine:
            print(f"{prefix}{dir_name}/ ({t.status}){pkg_tag}{progress}{marker}")
        else:
            print(f"{prefix}{dir_name}/ ({t.status}){pkg_tag}{progress} [{colored(t.assignee or '-', Colors.CYAN)}]{marker}")
        count += 1

        for child_name in t.children:
            if child_name in all_tasks:
                _print_task(child_name, indent + 1)

    for dir_name in sorted(all_tasks.keys()):
        if not all_tasks[dir_name].parent:
            _print_task(dir_name)

    if count == 0:
        if filter_mine:
            print("  （当前没有分配给你的 task）")
        else:
            print("  （当前没有未归档 task）")

    print()
    print(f"合计：{count} 个 task")
    return 0


def cmd_list_archive(args: argparse.Namespace) -> int:
    """列出归档 task。"""
    repo_root = get_repo_root()
    tasks_dir = get_tasks_dir(repo_root)
    archive_dir = tasks_dir / "archive"
    month = args.month

    print(colored("已归档 task：", Colors.BLUE))
    print()

    if month:
        month_dir = archive_dir / month
        if month_dir.is_dir():
            print(f"[{month}]")
            for d in sorted(month_dir.iterdir()):
                if d.is_dir():
                    print(f"  - {d.name}/")
        else:
            print(f"  {month} 没有归档 task")
    else:
        if archive_dir.is_dir():
            for month_dir in sorted(archive_dir.iterdir()):
                if month_dir.is_dir():
                    month_name = month_dir.name
                    count = sum(1 for d in month_dir.iterdir() if d.is_dir())
                    print(f"[{month_name}] - {count} 个 task")

    return 0


def show_usage() -> None:
    """显示帮助。"""
    print(
        """Task 管理脚本

用法：
  python task.py create <title>                     创建新 task 目录
  python task.py create <title> --package <pkg>     为指定 package 创建 task
  python task.py create <title> --parent <dir>      作为父 task 的子 task 创建
  python task.py add-context <dir> <jsonl> <path> [reason]  向 jsonl 追加条目
  python task.py validate <dir>                     校验上下文文件
  python task.py list-context <dir>                 列出上下文条目
  python task.py start <dir>                        设置当前活跃 task
  python task.py current [--source]                 查看当前活跃 task
  python task.py finish                             清除当前活跃 task
  python task.py set-branch <dir> <branch>          设置 git 分支
  python task.py set-base-branch <dir> <branch>     设置 PR 目标分支
  python task.py set-scope <dir> <scope>            设置 PR scope
  python task.py archive <task-dir>                 归档已完成 task
  python task.py add-subtask <parent> <child>       关联父子 task
  python task.py remove-subtask <parent> <child>    解除父子 task 关联
  python task.py list [--mine] [--status <status>]  列出 task
  python task.py list-archive [YYYY-MM]             列出归档 task

多仓选项：
  --package <pkg>      package 名称（会对照 config.yaml 校验）

列表选项：
  --mine, -m           只显示当前开发者负责的 task
  --status, -s <s>     按状态过滤（planning、in_progress、review、completed）

示例：
  python task.py create "登录功能落地" --slug 登录功能落地
  python task.py create "登录功能落地" --slug 登录功能落地 --package cli
  python task.py create "子任务示例" --slug 子任务示例 --parent .trellis/tasks/01-21-父任务
  python task.py add-context <dir> implement .trellis/spec/cli/backend/auth.md "认证相关规范"
  python task.py set-branch <dir> task/login-feature
  python task.py start .trellis/tasks/05-26-登录功能落地
  python task.py current --source
  python task.py finish
  python task.py archive 登录功能落地
  python task.py add-subtask parent-task child-task  # 关联现有 task
  python task.py remove-subtask parent-task child-task
  python task.py list                               # 列出全部未归档 task
  python task.py list --mine                        # 只列出我的 task
  python task.py list --mine --status in_progress   # 只列出我的进行中 task
"""
    )


def main() -> int:
    """CLI 入口。"""
    if len(sys.argv) >= 2 and sys.argv[1] == "init-context":
        print(
            colored("错误：`task.py init-context` 已在 v0.5.0-beta.12 移除。", Colors.RED),
            file=sys.stderr,
        )
        print(
            "`implement.jsonl` / `check.jsonl` 现在会在 `task.py create` 时自动生成，适用于",
            file=sys.stderr,
        )
        print("支持子代理的平台，并由 AI 在 Phase 1.3 负责整理。", file=sys.stderr)
        print("详见 `.trellis/workflow.md` Phase 1.3，或直接运行：", file=sys.stderr)
        print("  python ./.trellis/scripts/get_context.py --mode phase --step 1.3", file=sys.stderr)
        print(
            "也可以使用 `task.py add-context <dir> implement|check <path> <reason>` 追加条目。",
            file=sys.stderr,
        )
        return 2

    parser = argparse.ArgumentParser(
        description="Task 管理脚本",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    subparsers = parser.add_subparsers(dest="command", help="命令")

    p_create = subparsers.add_parser("create", help="创建新 task")
    p_create.add_argument("title", help="task 标题")
    p_create.add_argument("--slug", "-s", help="task slug；未提供时默认由中文标题生成")
    p_create.add_argument("--assignee", "-a", help="负责人开发者")
    p_create.add_argument("--priority", "-p", default="P2", help="优先级（P0-P3）")
    p_create.add_argument("--description", "-d", help="task 描述")
    p_create.add_argument("--parent", help="父 task 目录（会建立父子关联）")
    p_create.add_argument("--package", help="单仓或多仓中的 package 名称")

    p_add = subparsers.add_parser("add-context", help="追加上下文条目")
    p_add.add_argument("dir", help="task 目录")
    p_add.add_argument("file", help="JSONL 文件（implement|check）")
    p_add.add_argument("path", help="要加入的文件路径")
    p_add.add_argument("reason", nargs="?", help="加入原因")

    p_validate = subparsers.add_parser("validate", help="校验上下文文件")
    p_validate.add_argument("dir", help="task 目录")

    p_listctx = subparsers.add_parser("list-context", help="列出上下文条目")
    p_listctx.add_argument("dir", help="task 目录")

    p_start = subparsers.add_parser("start", help="设置当前活跃 task")
    p_start.add_argument("dir", help="task 目录")

    p_current = subparsers.add_parser("current", help="查看当前 task")
    p_current.add_argument("--source", action="store_true", help="同时显示当前 task 来源")

    subparsers.add_parser("finish", help="清除当前活跃 task")

    p_branch = subparsers.add_parser("set-branch", help="设置 git 分支")
    p_branch.add_argument("dir", help="task 目录")
    p_branch.add_argument("branch", help="分支名")

    p_base = subparsers.add_parser("set-base-branch", help="设置 PR 目标分支")
    p_base.add_argument("dir", help="task 目录")
    p_base.add_argument("base_branch", help="目标分支名（PR target）")

    p_scope = subparsers.add_parser("set-scope", help="设置 scope")
    p_scope.add_argument("dir", help="task 目录")
    p_scope.add_argument("scope", help="scope 名称")

    p_archive = subparsers.add_parser("archive", help="归档 task")
    p_archive.add_argument("name", help="task 目录或 task 名称")
    p_archive.add_argument("--no-commit", action="store_true", help="归档后跳过自动 git commit")

    p_list = subparsers.add_parser("list", help="列出 task")
    p_list.add_argument("--mine", "-m", action="store_true", help="只看我的 task")
    p_list.add_argument("--status", "-s", help="按状态过滤")

    p_addsub = subparsers.add_parser("add-subtask", help="关联父子 task")
    p_addsub.add_argument("parent_dir", help="父 task 目录")
    p_addsub.add_argument("child_dir", help="子 task 目录")

    p_rmsub = subparsers.add_parser("remove-subtask", help="解除父子 task 关联")
    p_rmsub.add_argument("parent_dir", help="父 task 目录")
    p_rmsub.add_argument("child_dir", help="子 task 目录")

    p_listarch = subparsers.add_parser("list-archive", help="列出归档 task")
    p_listarch.add_argument("month", nargs="?", help="月份（YYYY-MM）")

    args = parser.parse_args()
    if not args.command:
        show_usage()
        return 1

    commands = {
        "create": cmd_create,
        "add-context": cmd_add_context,
        "validate": cmd_validate,
        "list-context": cmd_list_context,
        "start": cmd_start,
        "current": cmd_current,
        "finish": cmd_finish,
        "set-branch": cmd_set_branch,
        "set-base-branch": cmd_set_base_branch,
        "set-scope": cmd_set_scope,
        "archive": cmd_archive,
        "add-subtask": cmd_add_subtask,
        "remove-subtask": cmd_remove_subtask,
        "list": cmd_list,
        "list-archive": cmd_list_archive,
    }
    return commands[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
