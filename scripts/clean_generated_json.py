#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable, List


GENERATED_PATTERNS = (
    "public/desktop_*.json",
    "public/map.json",
    "docs/desktop_*.json",
    "docs/map.json",
)


def collect_generated_files(repo_root: Path) -> List[Path]:
    files: List[Path] = []
    for pattern in GENERATED_PATTERNS:
        files.extend(repo_root.glob(pattern))
    unique_sorted = sorted({path.resolve() for path in files if path.is_file()})
    return [Path(item) for item in unique_sorted]


def print_file_list(repo_root: Path, files: Iterable[Path]) -> None:
    for file_path in files:
        try:
            rel = file_path.relative_to(repo_root)
            print(rel.as_posix())
        except ValueError:
            print(file_path.as_posix())


def remove_files(files: Iterable[Path]) -> int:
    deleted = 0
    for file_path in files:
        if file_path.exists() and file_path.is_file():
            file_path.unlink()
            deleted += 1
    return deleted


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="清理 generate.py 生成的 JSON 文件（不会删除 musics.json）。"
    )
    parser.add_argument(
        "--repo-root",
        default=".",
        help="仓库根目录，默认当前目录。",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="仅显示将删除的文件，不执行删除。",
    )
    parser.add_argument(
        "--yes",
        action="store_true",
        help="跳过交互确认，直接删除。",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()

    files = collect_generated_files(repo_root)
    if not files:
        print("未发现 generate.py 生成的 JSON 文件。")
        return 0

    print(f"发现 {len(files)} 个候选文件：")
    print_file_list(repo_root, files)

    if args.dry_run:
        print("\n--dry-run 模式：未执行删除。")
        return 0

    if not args.yes:
        answer = input("\n确认删除以上文件？输入 y 确认: ").strip().lower()
        if answer not in ("y", "yes"):
            print("已取消。")
            return 1

    deleted_count = remove_files(files)
    print(f"\n已删除 {deleted_count} 个文件。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
