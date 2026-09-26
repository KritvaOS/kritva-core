#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : check_source_headers.py
# Description : Validates standardized KritvaOS source headers
#
# Component   : Infrastructure
# Module      : Source Header Checker
# Layer       : Development Infrastructure
#
# Requirements: HEADER-001
# API         : Command-line source header validation
#
# Author      : KritvaOS
# Created     : 26-09-2026
#==============================================================================

from __future__ import annotations
import argparse
import re
import subprocess
from pathlib import Path

REQUIRED_FIELDS = [
    "File", "Description", "Component", "Module", "Layer",
    "Requirements", "API", "Author", "Created"
]

SOURCE_EXTENSIONS = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx",
    ".py", ".sh", ".bash", ".v", ".sv", ".svh", ".vh", ".dts", ".dtsi"
}

EXCLUDED_DIRS = {
    ".git", "build", "out", "third_party", "vendor", "generated", "external"
}

EXEMPT_NAMES = {"requirements.txt", "source_header_check.version"}

EXEMPT_DIRS = {
    ".github/agents",
    ".github/instructions",
    "docs/development/templates",
    "tests/source_header_check",
}


def classify(path: Path) -> str:
    s = path.as_posix()

    if any(part in EXCLUDED_DIRS for part in path.parts):
        return "skip"

    if path.name in EXEMPT_NAMES:
        return "skip"

    if any(s == d or s.startswith(d + "/") for d in EXEMPT_DIRS):
        return "skip"

    if s == ".githooks/pre-commit":
        return "source"

    if s.startswith(".github/workflows/") and path.suffix in {".yml", ".yaml"}:
        return "source"

    if path.suffix in SOURCE_EXTENSIONS:
        return "source"

    return "skip"


def _header_block(text: str):
    lines = text.splitlines()
    index = 1 if lines and lines[0].startswith("#!") else 0

    while index < len(lines) and not lines[index].strip():
        index += 1

    if index >= len(lines):
        return None

    first = lines[index].lstrip()
    if not (first.startswith("#") or first.startswith("//")):
        return None

    block = []
    while index < len(lines):
        line = lines[index]
        stripped = line.lstrip()
        if not stripped:
            break
        if not (stripped.startswith("#") or stripped.startswith("//")):
            break
        block.append(line)
        index += 1

    return "\n".join(block) if block else None


def validate_file(path: Path) -> list[str]:
    if classify(path) != "source":
        return []

    try:
        text = path.read_text(encoding="utf-8")
    except Exception:
        return [f"{path}: [HEADER-006] malformed/encoding"]

    block = _header_block(text)
    if not block:
        return [f"{path}: [HEADER-008] missing or malformed leading header block"]

    errors = []

    if "SPDX-License-Identifier: Apache-2.0" not in block:
        errors.append(
            f"{path}: [HEADER-001] missing SPDX-License-Identifier: Apache-2.0"
        )

    if "Copyright (c) 2026 KritvaOS" not in block:
        errors.append(
            f"{path}: [HEADER-003] missing Copyright (c) 2026 KritvaOS"
        )

    for field in REQUIRED_FIELDS:
        pattern = rf"^\s*(?:#|//)\s*{re.escape(field)}\s*:\s*(.*?)\s*$"
        match = re.search(pattern, block, re.MULTILINE)
        if not match or not match.group(1).strip():
            errors.append(
                f"{path}: [HEADER-004] missing or empty required field '{field}'"
            )

    created = re.search(
        r"^\s*(?:#|//)\s*Created\s*:\s*(\S+)\s*$",
        block,
        re.MULTILINE,
    )
    if not created or not re.fullmatch(r"\d{2}-\d{2}-\d{4}", created.group(1)):
        errors.append(
            f"{path}: [HEADER-002] missing 'Created : DD-MM-YYYY'"
        )

    return errors


def tracked_files() -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files"],
        text=True,
        capture_output=True,
        check=True,
    )
    return [Path(item) for item in result.stdout.splitlines() if item.strip()]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=["tracked", "files"], default="tracked")
    parser.add_argument("--files", nargs="*")
    parser.add_argument("--strict", action="store_true")
    args = parser.parse_args()

    files = (
        tracked_files()
        if args.mode == "tracked"
        else [Path(item) for item in (args.files or [])]
    )

    checked = 0
    skipped = 0
    errors = []

    for path in files:
        if classify(path) == "source":
            checked += 1
            errors.extend(validate_file(path))
        else:
            skipped += 1

    print(f"[header-check] scanned {len(files)} tracked/file(s)")
    print(f"[header-check] checked {checked} source/config file(s)")
    print(f"[header-check] skipped {skipped} file(s)")

    if errors:
        print("[header-check] FAILED")
        for error in errors:
            print(f"  {error}")
        return 1

    print("[header-check] PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
