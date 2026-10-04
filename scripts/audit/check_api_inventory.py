#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : check_api_inventory.py
# Description : Mechanical audit of the Core public API inventory against the installed headers.
#
# Component   : Kritva Core
# Module      : Audit
# Layer       : Core Foundation
#
# Requirements: CORE-COMPAT-001
# API         : CORE-API-INVENTORY
#
# Author      : KritvaOS Core Team
# Created     : 05-10-2026
#==============================================================================
"""Audit docs/compatibility/API_INVENTORY.md against include/kritva/core/ (structure and references only).

Checks (deterministic, no network, standard library only):
  - the inventory exists and every installed public header (include/kritva/core/**/*.hpp) is listed exactly once;
  - every listed header exists, and the stated total matches the number of rows;
  - every stability class is one of stable, experimental, deprecated, internal, test-only;
  - every sensitivity flag is `Y` or `-`; the domain matches the header path;
  - every owning documentation reference is `none` or a page that exists and is listed in docs/api/API_INDEX.md,
    and the stated documentation status equals the status in the index (`none` pairs with `none`);
  - flag consistency with the header text: an Enum flag requires the word `enum`, a Virt flag requires `virtual`
    (and a header containing `virtual` must carry it), a Thr flag requires the word `thread` (case-insensitive).
Exit status 0 when clean, 1 when any error is found. `--self-test` proves the audit detects deliberate defects.
"""
import argparse
import os
import re
import shutil
import sys
import tempfile

STABILITY = {"stable", "experimental", "deprecated", "internal", "test-only"}
FLAGS = {"Y", "-"}
INVENTORY = os.path.join("docs", "compatibility", "API_INVENTORY.md")
ROW = re.compile(r"^\|\s*`([^`]+\.hpp)`\s*\|([^|]*)\|([^|]*)\|([^|]*)\|([^|]*)\|([^|]*)\|([^|]*)\|([^|]*)\|([^|]*)\|\s*$")


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def installed_headers(root):
    base = os.path.join(root, "include", "kritva", "core")
    found = []
    for d, _, files in os.walk(base):
        for name in files:
            if name.endswith(".hpp"):
                found.append(os.path.relpath(os.path.join(d, name), base).replace(os.sep, "/"))
    return sorted(found)


def index_status(root):
    status = {}
    path = os.path.join(root, "docs", "api", "API_INDEX.md")
    if os.path.isfile(path):
        for line in read(path).splitlines():
            m = re.match(r"\|[^|]*\|\s*`([^`]+\.md)`\s*\|\s*([a-z]+)\s*\|", line)
            if m:
                status[m.group(1)] = m.group(2)
    return status


def audit(root):
    errors = []
    path = os.path.join(root, INVENTORY)
    if not os.path.isfile(path):
        return [f"{INVENTORY.replace(os.sep, '/')} is missing"], 0
    text = read(path)
    index = index_status(root)
    seen = {}
    rows = 0
    for line in text.splitlines():
        m = ROW.match(line)
        if not m:
            continue
        rows += 1
        header = m.group(1)
        domain, stability, own, thr, enum, virt, doc, dstat = (c.strip() for c in m.groups()[1:])
        if header in seen:
            errors.append(f"{header} is listed more than once")
        seen[header] = True
        full = os.path.join(root, "include", "kritva", "core", header)
        body = read(full) if os.path.isfile(full) else None
        if body is None:
            errors.append(f"{header} is listed but does not exist under include/kritva/core/")
        if domain != (header.split("/")[0] if "/" in header else "core"):
            errors.append(f"{header} has domain '{domain}' that does not match its path")
        if stability not in STABILITY:
            errors.append(f"{header} has the invalid stability '{stability}'")
        for label, value in (("Own", own), ("Thr", thr), ("Enum", enum), ("Virt", virt)):
            if value not in FLAGS:
                errors.append(f"{header} has the invalid {label} flag '{value}' (expected Y or -)")
        if doc == "none":
            if dstat != "none":
                errors.append(f"{header} has no owning documentation but the documentation status '{dstat}'")
        else:
            page = doc.strip("`")
            if page not in index:
                errors.append(f"{header} names the owning documentation {page}, which is not listed in docs/api/API_INDEX.md")
            elif not os.path.isfile(os.path.join(root, "docs", "api", page)):
                errors.append(f"{header} names the owning documentation {page}, which does not exist")
            elif dstat != index[page]:
                errors.append(f"{header} states the documentation status '{dstat}' but API_INDEX.md says '{index[page]}'")
        if body is not None:
            if enum == "Y" and not re.search(r"\benum\b", body):
                errors.append(f"{header} is flagged Enum but declares no enumeration")
            if virt == "Y" and not re.search(r"\bvirtual\b", body):
                errors.append(f"{header} is flagged Virt but declares nothing virtual")
            if virt != "Y" and re.search(r"\bvirtual\b", body):
                errors.append(f"{header} declares virtual members but is not flagged Virt")
            if thr == "Y" and not re.search(r"thread", body, re.I):
                errors.append(f"{header} is flagged Thr but does not mention threads")
    actual = installed_headers(root)
    for h in actual:
        if h not in seen:
            errors.append(f"installed header {h} is not listed in the inventory")
    total = re.search(r"\*\*Total public headers:\s*(\d+)\*\*", text)
    if not total:
        errors.append("the inventory does not state 'Total public headers'")
    elif int(total.group(1)) != rows:
        errors.append(f"the stated total {total.group(1)} does not match the {rows} listed rows")
    return errors, rows


def _edit(tmp, rel, old, new):
    path = os.path.join(tmp, rel)
    text = read(path)
    if re.search(old, text, flags=re.M) is None:
        raise SystemExit(f"self-test setup error: {old!r} not found in {rel}")
    with open(path, "w", encoding="utf-8") as f:
        f.write(re.sub(old, new, text, count=1, flags=re.M))


def _drop_row(tmp, header):
    _edit(tmp, INVENTORY, r"^\| `" + re.escape(header) + r"` .*\n", "")


def self_test(root):
    """Copy the inventory, headers and index, inject one defect at a time and require the audit to report it."""
    inv = INVENTORY
    hdr = "capability/capability.hpp"
    cases = [
        ("a missing inventory", lambda t: os.remove(os.path.join(t, inv)), "is missing"),
        ("an installed header absent from the inventory", lambda t: (_drop_row(t, hdr), _edit(t, inv, r"Total public headers: 49", "Total public headers: 48")), "is not listed in the inventory"),
        ("a duplicated header", lambda t: _edit(t, inv, r"^(\| `capability/capability_id.hpp` .*)$", r"\1\n\1"), "more than once"),
        ("a listed header that does not exist", lambda t: _edit(t, inv, "`capability/capability_id.hpp`", "`capability/no_such.hpp`"), "does not exist"),
        ("an invalid stability class", lambda t: _edit(t, inv, r"(`capability/capability_id.hpp` \| capability \| )stable", r"\1mature"), "invalid stability"),
        ("an invalid flag", lambda t: _edit(t, inv, r"(`capability/capability_id.hpp` \| capability \| stable \| )-", r"\1yes"), "invalid Own flag"),
        ("a wrong domain", lambda t: _edit(t, inv, r"(`capability/capability_id.hpp` \| )capability", r"\1runtime"), "domain"),
        ("a broken documentation reference", lambda t: _edit(t, inv, r"`capability/CAPABILITY_SET.md` \| maintained", "`capability/NOPE.md` | maintained"), "not listed in docs/api/API_INDEX.md"),
        ("a documentation status that disagrees with the index", lambda t: _edit(t, inv, r"`capability/CAPABILITY.md` \| maintained", "`capability/CAPABILITY.md` | stub"), "API_INDEX.md says"),
        ("a stated page for a header with none", lambda t: _edit(t, inv, r"(`core.hpp` .*\| )none \| none", r"\1none | stub"), "has no owning documentation"),
        ("an Enum flag on a header without an enumeration", lambda t: _edit(t, inv, r"(`capability/capability_id.hpp` \| capability \| stable \| - \| - \| )-", r"\1Y"), "declares no enumeration"),
        ("a Virt flag on a header without virtual members", lambda t: _edit(t, inv, r"(`capability/capability_id.hpp` \| capability \| stable \| - \| - \| - \| )-", r"\1Y"), "declares nothing virtual"),
        ("a virtual interface not flagged", lambda t: _edit(t, inv, r"(`runtime/component.hpp` \| runtime \| stable \| [Y-] \| [Y-] \| [Y-] \| )Y", r"\1-"), "not flagged Virt"),
        ("a Thr flag on a header that never mentions threads", lambda t: _edit(t, inv, r"(`capability/capability_id.hpp` \| capability \| stable \| - \| )-", r"\1Y"), "does not mention threads"),
        ("a stated total that is wrong", lambda t: _edit(t, inv, r"Total public headers: 49", "Total public headers: 50"), "stated total"),
        ("a missing total", lambda t: _edit(t, inv, r"\*\*Total public headers: 49\*\*", "total"), "does not state"),
    ]
    failures = 0
    for label, mutate, expected in cases:
        tmp = tempfile.mkdtemp(prefix="apiinv_selftest_")
        try:
            os.makedirs(os.path.join(tmp, "docs"))
            shutil.copytree(os.path.join(root, "docs", "compatibility"), os.path.join(tmp, "docs", "compatibility"))
            shutil.copytree(os.path.join(root, "docs", "api"), os.path.join(tmp, "docs", "api"))
            shutil.copytree(os.path.join(root, "include"), os.path.join(tmp, "include"))
            mutate(tmp)
            errors, _ = audit(tmp)
            if not any(expected in e for e in errors):
                print(f"[api-inventory] SELF-TEST FAILED: the audit did not detect {label}")
                failures += 1
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    errors, _ = audit(root)
    if errors:
        print("[api-inventory] SELF-TEST FAILED: the real inventory has errors")
        failures += 1
    if failures == 0:
        print(f"[api-inventory] self-test passed: {len(cases)} deliberate defects detected")
    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    root = os.path.abspath(args.root)
    if args.self_test:
        return 1 if self_test(root) else 0
    errors, rows = audit(root)
    for e in errors:
        print(f"[api-inventory] ERROR: {e}")
    print(f"[api-inventory] {rows} headers inventoried, {len(installed_headers(root))} installed, {len(errors)} error(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
