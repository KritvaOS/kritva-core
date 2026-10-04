#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : check_compat_policy.py
# Description : Mechanical audit of the Core compatibility policy pages (structure and references only).
#
# Component   : Kritva Core
# Module      : Audit
# Layer       : Core Foundation
#
# Requirements: CORE-COMPAT-002, CORE-COMPAT-003
# API         : CORE-API-COMPATIBILITY-POLICY
#
# Author      : KritvaOS Core Team
# Created     : 05-10-2026
#==============================================================================
"""Audit docs/compatibility/ policy pages: structure and references only, never whether a classification is right.

Checks (deterministic, no network, standard library only), for every policy page listed in POLICY_PAGES:
  - the page exists and contains every required section (matched by heading keyword);
  - every header (`xxx/yyy.hpp`), document (`docs/...md`, or a page in docs/compatibility) and requirement id
    (`CORE-XXX-NNN`) it names in backticks exists (ids in REQUIREMENTS.md);
  - every change-classification table row carries a valid class (Compatible, Review-required or Incompatible), and the
    source and semantic sections each contain at least one row of every class;
  - the page is linked from docs/api/README.md and docs/api/API_GUIDELINES.md.
Exit status 0 when clean, 1 when any error is found. `--self-test` proves the audit detects deliberate defects.
"""
import argparse
import os
import re
import shutil
import sys
import tempfile

CLASSES = {"Compatible", "Review-required", "Incompatible"}
# page -> (required heading keywords, sections that must hold a classification table with every class)
POLICY_PAGES = {
    "COMPATIBILITY_POLICY.md": (
        ["purpose", "scope", "dimensions", "contract sources", "source compatibility", "semantic compatibility",
         "not security", "relationship", "traceability", "exclusions"],
        ["source compatibility", "semantic compatibility"],
    ),
}


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def sections(text):
    """Map lower-cased heading text -> body, for '## ' headings."""
    parts = re.split(r"^##\s+(.+)$", text, flags=re.M)
    return {parts[i].lower(): parts[i + 1] for i in range(1, len(parts) - 1, 2)}


def audit(root):
    errors = []
    base = os.path.join(root, "docs", "compatibility")
    req_text = read(os.path.join(root, "REQUIREMENTS.md")) if os.path.isfile(os.path.join(root, "REQUIREMENTS.md")) else ""
    requirement_ids = set(re.findall(r"CORE-[A-Z]+-\d+", req_text))
    checked = 0
    for page, (keywords, table_sections) in sorted(POLICY_PAGES.items()):
        path = os.path.join(base, page)
        if not os.path.isfile(path):
            errors.append(f"docs/compatibility/{page} is missing")
            continue
        checked += 1
        text = read(path)
        secs = sections(text)
        for key in keywords:
            if not any(key in h for h in secs):
                errors.append(f"docs/compatibility/{page}: required section '{key}' is missing")
        for ref in sorted(set(re.findall(r"`([^`\s]+)`", text))):
            if ref.endswith(".hpp") and "/" in ref:
                if not os.path.isfile(os.path.join(root, "include", "kritva", "core", ref)):
                    errors.append(f"docs/compatibility/{page}: header {ref} does not exist")
            elif ref.startswith("docs/") and ref.endswith(".md"):
                if not os.path.isfile(os.path.join(root, ref)):
                    errors.append(f"docs/compatibility/{page}: document {ref} does not exist")
            elif re.fullmatch(r"[A-Z_]+\.md", ref) and not (os.path.isfile(os.path.join(base, ref)) or os.path.isfile(os.path.join(root, ref))):
                errors.append(f"docs/compatibility/{page}: document {ref} does not exist in docs/compatibility")
            elif re.fullmatch(r"CORE-[A-Z]+-\d+", ref) and ref not in requirement_ids:
                errors.append(f"docs/compatibility/{page}: requirement {ref} is not defined in REQUIREMENTS.md")
        for key in table_sections:
            body = next((b for h, b in secs.items() if key in h), "")
            found = set()
            for line in body.splitlines():
                m = re.match(r"^\|(.+)\|\s*([A-Za-z-]+)\s*\|(.+)\|\s*$", line)
                if m and not line.startswith("|---") and m.group(2) not in ("Class",):
                    if m.group(2) in CLASSES:
                        found.add(m.group(2))
                    elif m.group(2) not in ("Meaning",):
                        errors.append(f"docs/compatibility/{page}: '{key}' has a row with the invalid class '{m.group(2)}'")
            for c in sorted(CLASSES - found):
                errors.append(f"docs/compatibility/{page}: '{key}' has no row of class {c}")
        for entry in ("README.md", "API_GUIDELINES.md"):
            p = os.path.join(root, "docs", "api", entry)
            if not os.path.isfile(p) or page not in read(p):
                errors.append(f"docs/api/{entry} does not link {page}")
    return errors, checked


def _edit(tmp, rel, old, new):
    path = os.path.join(tmp, rel)
    text = read(path)
    if re.search(old, text, flags=re.M) is None:
        raise SystemExit(f"self-test setup error: {old!r} not found in {rel}")
    with open(path, "w", encoding="utf-8") as f:
        f.write(re.sub(old, new, text, count=1, flags=re.M))


def _no_incompatible_in_source(tmp):
    path = os.path.join(tmp, "docs/compatibility/COMPATIBILITY_POLICY.md")
    text = read(path)
    start = text.index("## 5. Source compatibility")
    end = text.index("## 6. Semantic compatibility")
    section = text[start:end].replace("| Incompatible |", "| Compatible |")
    with open(path, "w", encoding="utf-8") as f:
        f.write(text[:start] + section + text[end:])


def self_test(root):
    pol = "docs/compatibility/COMPATIBILITY_POLICY.md"
    cases = [
        ("a missing policy page", lambda t: os.remove(os.path.join(t, pol)), "is missing"),
        ("a missing required section", lambda t: _edit(t, pol, r"^## 7\. Compatibility is not security$", "## 7. Notes"), "'not security' is missing"),
        ("a header that does not exist", lambda t: _edit(t, pol, "`runtime/component.hpp`", "`runtime/no_such.hpp`"), "does not exist"),
        ("a document that does not exist", lambda t: _edit(t, pol, "`API_INVENTORY.md`", "`NO_SUCH.md`"), "does not exist in docs/compatibility"),
        ("an undefined requirement id", lambda t: _edit(t, pol, "`CORE-COMPAT-002`", "`CORE-COMPAT-" + "999`"), "not defined in REQUIREMENTS.md"),
        ("an invalid class", lambda t: _edit(t, pol, r"\| Compatible \| Existing clients do not name it\.", "| Fine | Existing clients do not name it."), "invalid class"),
        ("a source section lacking the Incompatible class", _no_incompatible_in_source, "no row of class Incompatible"),
        ("an unlinked page", lambda t: _edit(t, "docs/api/API_GUIDELINES.md", "COMPATIBILITY_POLICY.md", "POLICY.md"), "does not link"),
    ]
    failures = 0
    for label, mutate, expected in cases:
        tmp = tempfile.mkdtemp(prefix="compatpol_selftest_")
        try:
            os.makedirs(os.path.join(tmp, "docs"))
            for d in ("compatibility", "api"):
                shutil.copytree(os.path.join(root, "docs", d), os.path.join(tmp, "docs", d))
            os.symlink(os.path.join(root, "include"), os.path.join(tmp, "include"))
            shutil.copy(os.path.join(root, "REQUIREMENTS.md"), os.path.join(tmp, "REQUIREMENTS.md"))
            mutate(tmp)
            errors, _ = audit(tmp)
            if not any(expected in e for e in errors):
                print(f"[compat-policy] SELF-TEST FAILED: the audit did not detect {label}")
                failures += 1
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    errors, _ = audit(root)
    if errors:
        print("[compat-policy] SELF-TEST FAILED: the real policy has errors")
        failures += 1
    if failures == 0:
        print(f"[compat-policy] self-test passed: {len(cases)} deliberate defects detected")
    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    root = os.path.abspath(args.root)
    if args.self_test:
        return 1 if self_test(root) else 0
    errors, checked = audit(root)
    for e in errors:
        print(f"[compat-policy] ERROR: {e}")
    print(f"[compat-policy] {checked} policy page(s) checked, {len(errors)} error(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
