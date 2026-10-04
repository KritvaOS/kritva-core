#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : check_api_docs.py
# Description : Mechanical audit of the maintained Markdown API documentation (structure and references only).
#
# Component   : Kritva Core
# Module      : Audit
# Layer       : Core Foundation
#
# Requirements: CORE-CAP-011
# API         : CORE-API-DOCUMENTATION
#
# Author      : KritvaOS Core Team
# Created     : 05-10-2026
#==============================================================================
"""Audit docs/api/: structure and references only, never prose quality.

Checks (deterministic, no network, standard library only):
  - README.md, API_INDEX.md and API_GUIDELINES.md exist; README states that Markdown is authoritative;
  - every document listed in API_INDEX.md exists and has a Status of `maintained` or `stub`;
  - every .md file under docs/api/<domain>/ is listed in API_INDEX.md (no orphan page) and no generated
    HTML/PDF is stored there;
  - every `maintained` page contains each of the sections required by API_GUIDELINES.md (matched by heading keyword);
  - every header (`xxx/yyy.hpp`), test (`tests/...`), docs path (`docs/...`) and requirement id (`CORE-XXX-NNN`)
    a maintained page names in backticks exists (headers under include/kritva/core/, ids in REQUIREMENTS.md).
Exit status 0 when clean, 1 when any error is found. `--self-test` proves the audit detects deliberate defects.
"""
import argparse
import os
import re
import shutil
import sys
import tempfile

SECTIONS = [  # (label, heading keyword)
    ("Purpose", "purpose"), ("API surface", "api surface"), ("Semantics", "semantics"), ("Ownership and lifetime", "ownership"),
    ("Lifecycle interaction", "lifecycle interaction"), ("Error behavior", "error behavior"), ("Thread safety", "thread"),
    ("Allocation, blocking and real time", "allocation"), ("Compatibility", "compatib"), ("Security considerations", "security"),
    ("Examples", "example"), ("Requirements traceability", "requirement"), ("Related headers", "related headers"),
    ("Related tests", "related tests"), ("Explicit exclusions", "exclusion"),
]
STATUSES = {"maintained", "stub"}


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def audit(root):
    errors = []
    api = os.path.join(root, "docs", "api")
    for name in ("README.md", "API_INDEX.md", "API_GUIDELINES.md"):
        if not os.path.isfile(os.path.join(api, name)):
            errors.append(f"docs/api/{name} is missing")
    if errors:
        return errors, 0, 0
    if "Markdown is authoritative" not in read(os.path.join(api, "README.md")):
        errors.append("docs/api/README.md does not state that Markdown is authoritative")

    requirement_ids = set(re.findall(r"CORE-[A-Z]+-\d+", read(os.path.join(root, "REQUIREMENTS.md"))))
    listed = {}
    for line in read(os.path.join(api, "API_INDEX.md")).splitlines():
        m = re.match(r"\|\s*([A-Za-z ]+?)\s*\|\s*`([^`]+\.md)`\s*\|\s*([a-z]+)\s*\|", line)
        if not m:
            continue
        path, status = m.group(2), m.group(3)
        if path in listed:
            errors.append(f"API_INDEX.md lists {path} more than once")
        if status not in STATUSES:
            errors.append(f"API_INDEX.md gives {path} the status '{status}' (expected maintained or stub)")
        listed[path] = status
    if not listed:
        errors.append("API_INDEX.md lists no document")

    for path in sorted(listed):
        if not os.path.isfile(os.path.join(api, path)):
            errors.append(f"API_INDEX.md lists {path} but the file does not exist")

    on_disk = set()
    for dirpath, _dirs, files in os.walk(api):
        for f in files:
            rel = os.path.relpath(os.path.join(dirpath, f), api).replace(os.sep, "/")
            if f.lower().endswith((".html", ".htm", ".pdf")):
                errors.append(f"docs/api/{rel}: generated HTML/PDF must not be stored as API documentation")
            if f.endswith(".md") and "/" in rel:
                on_disk.add(rel)
    for rel in sorted(on_disk - set(listed)):
        errors.append(f"docs/api/{rel} exists but is not listed in API_INDEX.md")

    maintained = 0
    for path, status in sorted(listed.items()):
        full = os.path.join(api, path)
        if status != "maintained" or not os.path.isfile(full):
            continue
        maintained += 1
        text = read(full)
        headings = [h.lower() for h in re.findall(r"^#{2,3}\s+(.+)$", text, flags=re.M)]
        for label, key in SECTIONS:
            if not any(key in h for h in headings):
                errors.append(f"docs/api/{path}: required section '{label}' is missing")
        for ref in sorted(set(re.findall(r"`([^`\s]+)`", text))):
            if ref.endswith(".hpp") and "/" in ref and not ref.startswith(("docs/", "tests/")):
                if not os.path.isfile(os.path.join(root, "include", "kritva", "core", ref)):
                    errors.append(f"docs/api/{path}: header {ref} does not exist")
            elif ref.startswith("tests/") and ref.endswith(".cpp"):
                if not os.path.isfile(os.path.join(root, ref)):
                    errors.append(f"docs/api/{path}: test {ref} does not exist")
            elif ref.startswith("docs/") and ref.endswith(".md"):
                if not os.path.isfile(os.path.join(root, ref)):
                    errors.append(f"docs/api/{path}: document {ref} does not exist")
            elif re.fullmatch(r"CORE-[A-Z]+-\d+", ref) and ref not in requirement_ids:
                errors.append(f"docs/api/{path}: requirement {ref} is not defined in REQUIREMENTS.md")
    return errors, len(listed), maintained


def self_test(root):
    """Copy the documentation, inject one defect at a time and require the audit to report it."""
    cases = [
        ("an indexed page that does not exist", lambda t: os.remove(os.path.join(t, "docs/api/capability/CAPABILITY_SET.md")), "does not exist"),
        ("an orphan page", lambda t: open(os.path.join(t, "docs/api/runtime/ORPHAN.md"), "w").write("# x\n"), "not listed in API_INDEX.md"),
        ("a generated HTML page", lambda t: open(os.path.join(t, "docs/api/capability/CAPABILITY.html"), "w").write("<html/>"), "HTML/PDF"),
        ("a missing required section", lambda t: _edit(t, "docs/api/capability/CAPABILITY.md", r"^## 7\. Thread safety$", "## 7. Notes"), "Thread safety"),
        ("a header that does not exist", lambda t: _edit(t, "docs/api/capability/CAPABILITY.md", "`capability/capability_set.hpp`", "`capability/no_such_header.hpp`"), "does not exist"),
        ("a test that does not exist", lambda t: _edit(t, "docs/api/capability/CAPABILITY.md", "`tests/unit/capability_contract_test.cpp`", "`tests/unit/no_such_test.cpp`"), "does not exist"),
        ("an undefined requirement id", lambda t: _edit(t, "docs/api/capability/CAPABILITY.md", "`CORE-CAP-004`", "`CORE-CAP-" + "999`"), "not defined in REQUIREMENTS.md"),
        ("an invalid status", lambda t: _edit(t, "docs/api/API_INDEX.md", r"CAPABILITY\.md` \| maintained", "CAPABILITY.md` | draft"), "expected maintained or stub"),
        ("a missing README statement", lambda t: _edit(t, "docs/api/README.md", "Markdown is authoritative", "Markdown is nice"), "authoritative"),
        ("a missing index", lambda t: os.remove(os.path.join(t, "docs/api/API_INDEX.md")), "API_INDEX.md is missing"),
    ]
    failures = 0
    for label, mutate, expected in cases:
        tmp = tempfile.mkdtemp(prefix="apidocs_selftest_")
        try:
            os.makedirs(os.path.join(tmp, "docs"))
            shutil.copytree(os.path.join(root, "docs", "api"), os.path.join(tmp, "docs", "api"))
            for d in ("include", "tests"):                                   # references are resolved against the real tree
                os.symlink(os.path.join(root, d), os.path.join(tmp, d))
            shutil.copy(os.path.join(root, "REQUIREMENTS.md"), os.path.join(tmp, "REQUIREMENTS.md"))
            mutate(tmp)
            errors, _, _ = audit(tmp)
            if not any(expected in e for e in errors):
                print(f"[api-docs] SELF-TEST FAILED: the audit did not detect {label}")
                failures += 1
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    errors, _, _ = audit(root)
    if errors:
        print("[api-docs] SELF-TEST FAILED: the real documentation has errors")
        failures += 1
    if failures == 0:
        print(f"[api-docs] self-test passed: {len(cases)} deliberate defects detected")
    return failures


def _edit(tmp, rel, old, new):
    path = os.path.join(tmp, rel)
    text = read(path)
    if re.search(old, text, flags=re.M) is None:
        raise SystemExit(f"self-test setup error: {old!r} not found in {rel}")
    with open(path, "w", encoding="utf-8") as f:
        f.write(re.sub(old, new, text, count=1, flags=re.M))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    root = os.path.abspath(args.root)
    if args.self_test:
        return 1 if self_test(root) else 0
    errors, listed, maintained = audit(root)
    for e in errors:
        print(f"[api-docs] ERROR: {e}")
    print(f"[api-docs] {listed} documents indexed, {maintained} maintained, {listed - maintained} stub(s), {len(errors)} error(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
