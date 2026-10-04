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
# Requirements: CORE-COMPAT-002, CORE-COMPAT-003, CORE-COMPAT-004, CORE-COMPAT-005, CORE-COMPAT-006, CORE-COMPAT-007
# API         : CORE-API-COMPATIBILITY-POLICY
#
# Author      : KritvaOS Core Team
# Created     : 05-10-2026
#==============================================================================
"""Audit docs/compatibility/ policy pages: structure and references only, never whether a classification is right.

Checks (deterministic, no network, standard library only), for every policy page listed in POLICY_PAGES (the release-impact table is checked for consistency with the compatibility classes, the package examples against the documented selection rule, and CMakeLists.txt for the ABI-machinery guard):
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
    "VERSIONING_POLICY.md": (
        ["purpose", "version identity", "release impact", "enumerations", "evolution review", "package version selection",
         "not security", "relationship", "traceability", "exclusions"],
        [],
    ),
    "ABI_POLICY.md": (
        ["purpose", "decision", "rationale", "not promised", "may rely", "future", "guard", "not security", "relationship",
         "traceability", "exclusions"],
        [],
    ),
}
# ABI machinery that ABI_POLICY.md says does not exist; its appearance in the build forces the policy to be revisited.
ABI_MACHINERY = re.compile(r"SOVERSION|VISIBILITY_PRESET|CXX_VISIBILITY|generate_export_header|GenerateExportHeader|add_library\s*\([^)]*\bSHARED\b", re.I)


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def sections(text):
    """Map lower-cased heading text -> body, for '## ' headings."""
    parts = re.split(r"^##\s+(.+)$", text, flags=re.M)
    return {parts[i].lower(): parts[i + 1] for i in range(1, len(parts) - 1, 2)}


IMPACTS = {"MAJOR", "MINOR", "PATCH"}
# compatibility class -> release impacts the policy may assign (an Incompatible change is never MINOR or PATCH, a
# Review-required change is never PATCH, a Compatible change is never MAJOR)
ALLOWED_IMPACT = {"Incompatible": {"MAJOR"}, "Review-required": {"MINOR", "MAJOR"}, "Compatible": {"MINOR", "PATCH"}}


def parse_version(text):
    return tuple(int(x) for x in text.split("."))


def package_accepts(installed, requested):
    """The documented selection rule: same MAJOR and installed >= requested (missing components are 0)."""
    i, r = parse_version(installed), parse_version(requested)
    r = r + (0,) * (3 - len(r))
    return i[0] == r[0] and i[:3] >= r[:3]


def audit_versioning(page, text, errors):
    secs = sections(text)
    impact_rows = 0
    seen_impacts = set()
    for line in secs.get(next((h for h in secs if "release impact" in h), ""), "").splitlines():
        m = re.match(r"^\|(.+)\|\s*([A-Za-z-]+)\s*\|\s*([A-Z]+)\s*\|(.+)\|\s*$", line)
        if not m or m.group(2) in ("Compatibility class",):
            continue
        cls, impact = m.group(2), m.group(3)
        impact_rows += 1
        if cls not in CLASSES:
            errors.append(f"docs/compatibility/{page}: a release-impact row has the invalid class '{cls}'")
        elif impact not in IMPACTS:
            errors.append(f"docs/compatibility/{page}: a release-impact row has the invalid impact '{impact}'")
        else:
            seen_impacts.add(impact)
            if impact not in ALLOWED_IMPACT[cls]:
                errors.append(f"docs/compatibility/{page}: a {cls} change is assigned the release impact {impact}, which the policy does not allow")
    if impact_rows == 0:
        errors.append(f"docs/compatibility/{page}: the release-impact table has no row")
    for impact in sorted(IMPACTS - seen_impacts):
        errors.append(f"docs/compatibility/{page}: the release-impact table has no {impact} row")
    examples = 0
    for line in secs.get(next((h for h in secs if "package version selection" in h), ""), "").splitlines():
        m = re.match(r"^\|\s*(\d+(?:\.\d+){0,2})\s*\|\s*(\d+(?:\.\d+){0,2})\s*\|\s*(Accept|Reject)\s*\|\s*$", line)
        if m:
            examples += 1
            if package_accepts(m.group(1), m.group(2)) != (m.group(3) == "Accept"):
                errors.append(f"docs/compatibility/{page}: the example installed {m.group(1)} / requested {m.group(2)} states {m.group(3)}, which contradicts the documented selection rule")
    if examples == 0:
        errors.append(f"docs/compatibility/{page}: the package version selection section has no example row")


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
        if page == "VERSIONING_POLICY.md":
            audit_versioning(page, text, errors)
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
    cmake = os.path.join(root, "CMakeLists.txt")
    if os.path.isfile(cmake) and ABI_MACHINERY.search(read(cmake)):
        errors.append("CMakeLists.txt introduces ABI machinery (SOVERSION, visibility, export header or a shared library) that docs/compatibility/ABI_POLICY.md says does not exist")
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
        ("a missing ABI decision section", lambda t: _edit(t, "docs/compatibility/ABI_POLICY.md", r"^## 2\. Decision$", "## 2. Notes"), "'decision' is missing"),
        ("a missing ABI guard section", lambda t: _edit(t, "docs/compatibility/ABI_POLICY.md", r"^## 7\. Guard$", "## 7. Notes"), "'guard' is missing"),
        ("ABI machinery added to the build", lambda t: _edit(t, "CMakeLists.txt", r"^add_library\(kritva_core$", "set_target_properties(kritva_core PROPERTIES SOVERSION 1)\nadd_library(kritva_core"), "introduces ABI machinery"),
        ("an undefined ABI requirement", lambda t: _edit(t, "docs/compatibility/ABI_POLICY.md", "`CORE-COMPAT-004`", "`CORE-COMPAT-" + "998`"), "not defined in REQUIREMENTS.md"),
        ("an Incompatible change released as MINOR", lambda t: _edit(t, "docs/compatibility/VERSIONING_POLICY.md", r"(\| Incompatible \| )MAJOR( \| Never permitted in MINOR or PATCH\. \|)", r"\1MINOR\2"), "does not allow"),
        ("a Review-required change released as PATCH", lambda t: _edit(t, "docs/compatibility/VERSIONING_POLICY.md", r"(Add an enumerator or `ErrorCode` value \| Review-required \| )MINOR", r"\1PATCH"), "does not allow"),
        ("a package example that contradicts the rule", lambda t: _edit(t, "docs/compatibility/VERSIONING_POLICY.md", r"(\| 1\.2\.3 \| 1\.3 \| )Reject", r"\1Accept"), "contradicts the documented selection rule"),
        ("an invalid release impact", lambda t: _edit(t, "docs/compatibility/VERSIONING_POLICY.md", r"(\| Compatible \| )PATCH( \| Changelog entry)", r"\1BUILD\2"), "invalid impact"),
        ("a missing version-selection section", lambda t: _edit(t, "docs/compatibility/VERSIONING_POLICY.md", r"^## 6\. Installed package version selection$", "## 6. Notes"), "'package version selection' is missing"),
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
            shutil.copy(os.path.join(root, "CMakeLists.txt"), os.path.join(tmp, "CMakeLists.txt"))
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
