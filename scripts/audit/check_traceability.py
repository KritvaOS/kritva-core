#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : check_traceability.py
# Description : Audits requirement-to-header-to-test traceability
#
# Component   : Infrastructure
# Module      : Traceability Audit
# Layer       : Development Infrastructure
#
# Requirements: CORE-REQ-002
# API         : Command-line traceability audit
#
# Author      : KritvaOS
# Created     : 02-10-2026
#==============================================================================
"""Audit REQUIREMENTS.md against the repository.

Errors (exit status 1):
  * a CORE-<DOMAIN>-<NNN> ID referenced outside REQUIREMENTS.md is not defined
  * an ID is defined more than once, or a defined ID has no traceability row
    (IDs defined as "Reserved" are exempt)
  * a traceability row names an undefined ID or a file that does not exist
  * a public header under include/kritva/core is missing from the table
  * a header in a row carries none of the row's IDs, or an ID is carried by no header
    in its row (Requirements: tag)
  * a tests/ *_test.cpp file is not registered in CMakeLists.txt
  * CORE-GEN-003: external dependency mechanisms or forbidden includes found

Warnings (do not fail): no test in a row carries one of the row's IDs in its
Requirements: tag.

The audit only checks that the recorded chain is real. It does not create
requirements or implementation behavior.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ID_RE = re.compile(r"CORE-[A-Z]+-\d{3}")
RANGE_RE = re.compile(r"(CORE-[A-Z]+-)(\d{3})\.\.(\d{3})")
PUBLIC = ROOT / "include" / "kritva" / "core"

SCAN_DIRS = ["include", "src", "tests", "scripts", "docs", ".github", ".githooks", "config", "examples"]
SCAN_FILES = ["README.md", "API.md", "ARCHITECTURE.md", "TESTING.md", "CMakeLists.txt",
              "CMakePresets.json", "Makefile", "AGENTS.md", "CHANGELOG.md"]
FORBIDDEN_INCLUDE = re.compile(r'#\s*include\s*[<"][^>"]*(rclcpp|rcl/|ros/|dds/|fastdds|fastrtps|ecrt|ethercat|soem)', re.I)
FORBIDDEN_CMAKE = re.compile(r"\b(find_package|FetchContent_\w+|ExternalProject_\w+|add_subdirectory)\s*\(", re.I)

errors: list[str] = []
warnings: list[str] = []


def expand(cell: str) -> list[str]:
    """Expand 'CORE-A-001..003, CORE-B-001' into a list of IDs."""
    ids: list[str] = []
    for m in RANGE_RE.finditer(cell):
        ids += [f"{m.group(1)}{n:03d}" for n in range(int(m.group(2)), int(m.group(3)) + 1)]
    ids += ID_RE.findall(RANGE_RE.sub("", cell))
    return ids


def declared(path: Path) -> set[str]:
    """IDs in the file's header 'Requirements:' field (it may span lines up to
    the next 'API:' field)."""
    ids: set[str] = set()
    collecting = False
    for line in path.read_text(errors="replace").splitlines()[:40]:
        if re.match(r"\W*\s*Requirements\s*:", line):
            collecting = True
        elif collecting and re.match(r"\W*\s*API\s*:", line):
            break
        if collecting:
            ids |= set(ID_RE.findall(line))
    return ids


def parse_requirements():
    text = (ROOT / "REQUIREMENTS.md").read_text()
    defined: dict[str, int] = {}
    reserved: set[str] = set()
    for line in text.splitlines():
        m = re.match(r"^- (CORE-[A-Z]+-\d{3}) (.*)", line)
        if m:
            defined[m.group(1)] = defined.get(m.group(1), 0) + 1
            if m.group(2).startswith("Reserved"):
                reserved.add(m.group(1))
    api_rows, proc_rows = [], []
    section = None
    for line in text.splitlines():
        if line.startswith("## Traceability"):
            section = "api"
        elif line.startswith("## Process traceability"):
            section = "proc"
        elif line.startswith("## "):
            section = None
        if section and line.startswith("|") and not line.startswith("|---") and "CORE-" in line:
            cells = [c.strip() for c in line.strip().strip("|").split("|")]
            (api_rows if section == "api" else proc_rows).append(cells)
    return defined, reserved, api_rows, proc_rows


def paths_in(cell: str, base: Path | None = None) -> list[str]:
    cell = re.sub(r"\(.*?\)", "", cell)
    out = []
    for part in cell.split(","):
        part = part.strip()
        if part and part not in {"-", "header-only", "contract only"}:
            out.append(part)
    return out


def resolve_header(name: str) -> Path:
    for base in (ROOT, PUBLIC):
        if (base / name).exists():
            return base / name
    return PUBLIC / name


def resolve_test(name: str) -> Path:
    for base in (ROOT, ROOT / "tests" / "unit", ROOT / "tests" / "contract"):
        if (base / name).exists():
            return base / name
    return ROOT / name


def main() -> int:
    defined, reserved, api_rows, proc_rows = parse_requirements()

    for rid, n in defined.items():
        if n > 1:
            errors.append(f"{rid} is defined {n} times in REQUIREMENTS.md")

    # 1. Every referenced ID is defined.
    files: list[Path] = [ROOT / f for f in SCAN_FILES if (ROOT / f).exists()]
    for d in SCAN_DIRS:
        if (ROOT / d).is_dir():
            files += [p for p in (ROOT / d).rglob("*") if p.is_file() and ".git/" not in str(p)]
    for f in files:
        if f.name == "check_traceability.py" or f.suffix in {".csv", ".gcda", ".gcno"}:
            continue
        try:
            text = f.read_text()
        except (UnicodeDecodeError, OSError):
            continue
        for rid in sorted(set(ID_RE.findall(text))):
            if rid not in defined:
                errors.append(f"{f.relative_to(ROOT)}: references undefined requirement {rid}")

    # 2. Table rows: IDs defined, files exist, header/tag consistency.
    traced: set[str] = set()
    listed_headers: set[Path] = set()
    for cells in api_rows:
        if len(cells) < 4:
            errors.append(f"malformed traceability row: {cells}")
            continue
        ids = expand(cells[0])
        for rid in ids:
            traced.add(rid)
            if rid not in defined:
                errors.append(f"traceability row names undefined requirement {rid}")
        header_tags: set[str] = set()
        for h in paths_in(cells[1]):
            p = resolve_header(h)
            if not p.exists():
                errors.append(f"{ids}: header '{h}' does not exist")
                continue
            if p.is_file():
                listed_headers.add(p.resolve())
                tags = declared(p)
                header_tags |= tags
                if not tags & set(ids):
                    errors.append(f"{p.relative_to(ROOT)}: Requirements: tag has none of {ids} (named in table)")
        if header_tags:
            for rid in ids:
                if rid in defined and rid not in header_tags:
                    errors.append(f"{rid}: no header in its row carries it in a Requirements: tag")
        for impl in paths_in(cells[2]):
            if not (ROOT / impl).exists():
                errors.append(f"{ids}: implementation '{impl}' does not exist")
        test_tags: set[str] = set()
        for t in paths_in(cells[3]):
            p = resolve_test(t)
            if not p.exists():
                errors.append(f"{ids}: test '{t}' does not exist")
            else:
                test_tags |= declared(p)
        for rid in ids:
            if rid in defined and paths_in(cells[3]) and rid not in test_tags:
                warnings.append(f"{rid}: no test in its row carries it in a Requirements: tag")

    for cells in proc_rows:
        if len(cells) < 3:
            errors.append(f"malformed process row: {cells}")
            continue
        ids = expand(cells[0])
        for rid in ids:
            traced.add(rid)
            if rid not in defined:
                errors.append(f"process row names undefined requirement {rid}")
        for a in paths_in(cells[1]):
            if not (ROOT / a).exists():
                errors.append(f"{ids}: artifact '{a}' does not exist")
        for v in paths_in(cells[2]):
            if v == "inspection":
                continue
            if not resolve_test(v).exists() and not (ROOT / v).exists():
                errors.append(f"{ids}: verification '{v}' does not exist")

    # 3. Every defined (non-reserved) ID is traced.
    for rid in sorted(defined):
        if rid not in traced and rid not in reserved:
            errors.append(f"{rid} is defined but has no traceability row")

    # 4. Every public header is in the table.
    for h in sorted(PUBLIC.rglob("*.hpp")):
        if h.resolve() not in listed_headers:
            errors.append(f"{h.relative_to(ROOT)}: public header is not in the traceability table")

    # 5. Every test source is registered with CTest.
    cmake = (ROOT / "CMakeLists.txt").read_text()
    for t in sorted((ROOT / "tests").rglob("*_test.cpp")):
        if str(t.relative_to(ROOT)) not in cmake:
            errors.append(f"{t.relative_to(ROOT)}: not registered in CMakeLists.txt")

    # 6. CORE-GEN-003: no external dependency mechanism or forbidden include.
    for m in FORBIDDEN_CMAKE.finditer(cmake):
        errors.append(f"CORE-GEN-003: CMakeLists.txt uses {m.group(1)}()")
    for d in ("include", "src"):
        for f in (ROOT / d).rglob("*"):
            if f.suffix in {".hpp", ".cpp", ".h"}:
                for m in FORBIDDEN_INCLUDE.finditer(f.read_text()):
                    errors.append(f"CORE-GEN-003: {f.relative_to(ROOT)} includes forbidden dependency: {m.group(0)}")

    for w in warnings:
        print(f"[traceability] WARNING: {w}")
    for e in errors:
        print(f"[traceability] ERROR: {e}")
    print(f"[traceability] {len(defined)} requirements, {len(traced)} traced, "
          f"{len(errors)} error(s), {len(warnings)} warning(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
