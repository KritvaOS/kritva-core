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
# Requirements: CORE-COMPAT-002, CORE-COMPAT-003, CORE-COMPAT-004, CORE-COMPAT-005, CORE-COMPAT-006, CORE-COMPAT-007, CORE-COMPAT-008, CORE-COMPAT-010
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
    "DEPRECATION_POLICY.md": (
        ["purpose", "scope", "lifecycle", "deprecating an item", "compatibility window", "migration guidance", "register",
         "security", "relationship", "traceability", "exclusions"],
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


# (page, phrase) pairs that must appear: the cross-document rules the R1.0 review froze
REQUIRED_PHRASES = [
    ("COMPATIBILITY_POLICY.md", "the most severe class applies"),
    ("DEPRECATION_POLICY.md", "removed only after it has been published as deprecated"),
    ("DEPRECATION_POLICY.md", "applies only when a whole header"),
    ("API_INVENTORY.md", "whole header is deprecated"),
    ("API_INVENTORY.md", "recorded in `DEPRECATIONS.md`"),
    ("VERSIONING_POLICY.md", "`kritva-core-rMAJOR.MINOR.PATCH` for a patch release"),
]
# (page, text identifying a table row, class that row must carry)
REQUIRED_ROW_CLASSES = [
    ("COMPATIBILITY_POLICY.md", "Remove `noexcept` where the contract documents", "Incompatible"),
    ("COMPATIBILITY_POLICY.md", "Remove `noexcept` where the contract does not document", "Review-required"),
]


def tag_for(version):
    major, minor, patch = (int(x) for x in version.split("."))
    return f"kritva-core-r{major}.{minor}" if patch == 0 else f"kritva-core-r{major}.{minor}.{patch}"


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
    tags = 0
    for line in text.splitlines():
        m = re.match(r"^\|\s*(\d+\.\d+\.\d+)\s*\|\s*`(kritva-core-r[0-9.]+)`\s*\|\s*$", line)
        if m:
            tags += 1
            if tag_for(m.group(1)) != m.group(2):
                errors.append(f"docs/compatibility/{page}: the version {m.group(1)} is given the tag {m.group(2)}, which contradicts the tag naming rule")
    if tags == 0:
        errors.append(f"docs/compatibility/{page}: the tag naming section has no example row")


def audit_deprecations(root, errors):
    """The register and the code must agree: every [[deprecated]] in the public headers is registered, every registered
    item is deprecated in its header, every row is complete, and removal is a MAJOR release after the deprecating one."""
    register = os.path.join(root, "docs", "compatibility", "DEPRECATIONS.md")
    if not os.path.isfile(register):
        errors.append("docs/compatibility/DEPRECATIONS.md is missing")
        return
    rows, in_table, seen = [], False, set()
    for line in read(register).splitlines():
        if re.match(r"^\|\s*Header\s*\|\s*Item\s*\|", line):
            in_table = True
            continue
        if in_table and line.startswith("|") and not line.startswith("|---"):
            rows.append([c.strip() for c in line.strip().strip("|").split("|")])
    if not in_table:
        errors.append("docs/compatibility/DEPRECATIONS.md has no register table")
        return
    base = os.path.join(root, "include", "kritva", "core")
    registered = set()
    for cells in rows:
        if len(cells) != 6 or any(not c for c in cells):
            errors.append(f"DEPRECATIONS.md has an incomplete row: {'|'.join(cells)}")
            continue
        header, item, since, why, removal, migration = cells
        header = header.strip("`")
        if (header, item) in seen:
            errors.append(f"DEPRECATIONS.md lists {header} {item} more than once")
        seen.add((header, item))
        registered.add(header)
        path = os.path.join(base, header)
        if not os.path.isfile(path):
            errors.append(f"DEPRECATIONS.md names the header {header}, which does not exist")
        elif "[[deprecated" not in read(path):
            errors.append(f"DEPRECATIONS.md lists {header} {item} but the header contains no [[deprecated]] marker")
        if not re.fullmatch(r"\d+\.\d+(\.\d+)?", since):
            errors.append(f"DEPRECATIONS.md gives {header} {item} the invalid deprecated-since version '{since}'")
        elif not re.fullmatch(r"\d+\.0(\.0)?", removal) or int(removal.split(".")[0]) <= int(since.split(".")[0]):
            errors.append(f"DEPRECATIONS.md gives {header} {item} the earliest removal '{removal}', which is not a MAJOR release after {since}")
    for d, _, files in os.walk(base):
        for name in files:
            if name.endswith(".hpp"):
                rel = os.path.relpath(os.path.join(d, name), base).replace(os.sep, "/")
                if "[[deprecated" in read(os.path.join(d, name)) and rel not in registered:
                    errors.append(f"{rel} contains [[deprecated]] but has no row in DEPRECATIONS.md")


def audit_cross_policy(root, errors):
    """Cross-document rules frozen by the R1.0 API / Compatibility Review."""
    base = os.path.join(root, "docs", "compatibility")
    for page, phrase in REQUIRED_PHRASES:
        path = os.path.join(base, page)
        if os.path.isfile(path) and phrase not in read(path):
            errors.append(f"docs/compatibility/{page}: the cross-policy rule '{phrase}' is missing")
    for page, key, cls in REQUIRED_ROW_CLASSES:
        path = os.path.join(base, page)
        if not os.path.isfile(path):
            continue
        rows = [l for l in read(path).splitlines() if key in l and l.startswith("|")]
        if not rows:
            errors.append(f"docs/compatibility/{page}: the row '{key}' is missing")
        elif not all(re.search(r"\|\s*" + re.escape(cls) + r"\s*\|", r) for r in rows):
            errors.append(f"docs/compatibility/{page}: the row '{key}' must carry the class {cls}")
    # a header classed deprecated in the inventory (whole header) must have a register row
    inv, reg = os.path.join(base, "API_INVENTORY.md"), os.path.join(base, "DEPRECATIONS.md")
    if os.path.isfile(inv) and os.path.isfile(reg):
        registered = set(re.findall(r"^\|\s*`([^`]+\.hpp)`\s*\|", read(reg), flags=re.M))
        for line in read(inv).splitlines():
            m = re.match(r"^\|\s*`([^`]+\.hpp)`\s*\|[^|]*\|\s*deprecated\s*\|", line)
            if m and m.group(1) not in registered:
                errors.append(f"API_INVENTORY.md classes {m.group(1)} as deprecated but DEPRECATIONS.md has no row for it")


def audit_package_rule(root, errors):
    """The package build must implement the version-selection rule of VERSIONING_POLICY.md (same MAJOR, installed >= requested)."""
    cmake = os.path.join(root, "CMakeLists.txt")
    if not os.path.isfile(cmake):
        return
    m = re.search(r"write_basic_package_version_file\([^)]*COMPATIBILITY\s+(\w+)", read(cmake))
    if not m:
        errors.append("CMakeLists.txt has no write_basic_package_version_file COMPATIBILITY mode")
    elif m.group(1) != "SameMajorVersion":
        errors.append(f"CMakeLists.txt uses the package COMPATIBILITY mode {m.group(1)}, which does not implement the version-selection rule of docs/compatibility/VERSIONING_POLICY.md (SameMajorVersion)")
    for rel in ("tests/install/package_version_matrix.cmake", "tests/install/run_install_test.cmake"):
        if os.path.isdir(os.path.join(root, "tests")) and not os.path.isfile(os.path.join(root, rel)):
            errors.append(f"{rel} (package version-selection validation) is missing")


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
    audit_deprecations(root, errors)
    audit_cross_policy(root, errors)
    audit_package_rule(root, errors)
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


def _append(tmp, rel, text):
    with open(os.path.join(tmp, rel), "a", encoding="utf-8") as f:
        f.write(text)


def _row(tmp, header, item, since, why, removal, migration):
    _append(tmp, "docs/compatibility/DEPRECATIONS.md", f"| `{header}` | {item} | {since} | {why} | {removal} | {migration} |\n")


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
        ("a deprecated marker with no register row", lambda t: _append(t, "include/kritva/core/capability/capability_id.hpp", '[[deprecated("use x")]] inline void old_fn();\n'), "has no row in DEPRECATIONS.md"),
        ("a register row for an item that is not deprecated", lambda t: _row(t, "capability/capability_id.hpp", "old_fn", "1.2.0", "use new_fn", "2.0.0", "see guide"), "contains no [[deprecated]] marker"),
        ("a register row for a header that does not exist", lambda t: _row(t, "capability/nope.hpp", "old_fn", "1.2.0", "use new_fn", "2.0.0", "see guide"), "does not exist"),
        ("an incomplete register row", lambda t: _row(t, "capability/capability_id.hpp", "old_fn", "1.2.0", "", "2.0.0", "see guide"), "incomplete row"),
        ("a removal that is not a later MAJOR release", lambda t: (_append(t, "include/kritva/core/capability/capability_id.hpp", '[[deprecated("x")]] inline void old_fn();\n'), _row(t, "capability/capability_id.hpp", "old_fn", "1.2.0", "use new_fn", "1.3.0", "see guide"))[1], "not a MAJOR release after"),
        ("an invalid deprecated-since version", lambda t: (_append(t, "include/kritva/core/capability/capability_id.hpp", '[[deprecated("x")]] inline void old_fn();\n'), _row(t, "capability/capability_id.hpp", "old_fn", "soon", "use new_fn", "2.0.0", "see guide"))[1], "invalid deprecated-since"),
        ("a duplicated register row", lambda t: (_append(t, "include/kritva/core/capability/capability_id.hpp", '[[deprecated("x")]] inline void old_fn();\n'), _row(t, "capability/capability_id.hpp", "old_fn", "1.2.0", "use new_fn", "2.0.0", "g"), _row(t, "capability/capability_id.hpp", "old_fn", "1.2.0", "use new_fn", "2.0.0", "g"))[2], "more than once"),
        ("a missing register", lambda t: os.remove(os.path.join(t, "docs/compatibility/DEPRECATIONS.md")), "DEPRECATIONS.md is missing"),
        ("a missing lifecycle section", lambda t: _edit(t, "docs/compatibility/DEPRECATION_POLICY.md", r"^## 3\. Lifecycle$", "## 3. Notes"), "'lifecycle' is missing"),
        ("a missing most-severe-class rule", lambda t: _edit(t, "docs/compatibility/COMPATIBILITY_POLICY.md", "the most severe class applies", "the first class applies"), "most severe class applies"),
        ("a documented noexcept removal classed Review-required", lambda t: _edit(t, "docs/compatibility/COMPATIBILITY_POLICY.md", r"(Remove `noexcept` where the contract documents the no-throw guarantee \| )Incompatible", r"\1Review-required"), "must carry the class Incompatible"),
        ("a missing removal-after-deprecation rule", lambda t: _edit(t, "docs/compatibility/DEPRECATION_POLICY.md", "removed only after it has been published as deprecated", "removed at any time"), "removed only after"),
        ("a missing header-level deprecated rule in the inventory", lambda t: _edit(t, "docs/compatibility/API_INVENTORY.md", "whole header is deprecated", "header is old"), "whole header is deprecated"),
        ("a whole header classed deprecated without a register row", lambda t: _edit(t, "docs/compatibility/API_INVENTORY.md", r"(\| `capability/capability_id.hpp` \| capability \| )stable", r"\1deprecated"), "has no row for it"),
        ("a tag that contradicts the naming rule", lambda t: _edit(t, "docs/compatibility/VERSIONING_POLICY.md", r"(\| 1\.0\.1 \| )`kritva-core-r1\.0\.1`", r"\1`kritva-core-r1.0`"), "contradicts the tag naming rule"),
        ("a missing patch-tag rule", lambda t: _edit(t, "docs/compatibility/VERSIONING_POLICY.md", "for a patch release", "for a release"), "patch release"),
        ("a package mode that does not implement the rule", lambda t: _edit(t, "CMakeLists.txt", "COMPATIBILITY SameMajorVersion", "COMPATIBILITY SameMinorVersion"), "does not implement the version-selection rule"),
        ("an unlinked page", lambda t: _edit(t, "docs/api/API_GUIDELINES.md", "COMPATIBILITY_POLICY.md", "POLICY.md"), "does not link"),
    ]
    failures = 0
    for label, mutate, expected in cases:
        tmp = tempfile.mkdtemp(prefix="compatpol_selftest_")
        try:
            os.makedirs(os.path.join(tmp, "docs"))
            for d in ("compatibility", "api"):
                shutil.copytree(os.path.join(root, "docs", d), os.path.join(tmp, "docs", d))
            shutil.copytree(os.path.join(root, "include"), os.path.join(tmp, "include"))
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
