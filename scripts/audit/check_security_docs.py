#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : check_security_docs.py
# Description : Mechanical audit of the security documentation and the per-task security-impact records.
#
# Component   : Kritva Core
# Module      : Audit
# Layer       : Core Foundation
#
# Requirements: CORE-SEC-001
# API         : CORE-API-SECURITY-DOCUMENTATION
#
# Author      : KritvaOS Core Team
# Created     : 05-10-2026
#==============================================================================
"""Audit docs/security/ and the R1.0 task evidence records: structure and consistency only.

Checks (deterministic, no network, standard library only):
  - the four security documents exist (SECURITY_ARCHITECTURE, SECURITY_DECISIONS, TRUST_BOUNDARIES, THREAT_MODEL);
  - the R1.0 decisions SD-R10-NN in SECURITY_DECISIONS.md are unique, contiguous from 01 and each referenced by THREAT_MODEL.md;
  - SECURITY_ARCHITECTURE.md records the R1.0 classification and the R1.0 security review record exists with exactly one
    classification (SECURITY IMPACT: NONE, DOCUMENTATION ONLY or ARCHITECTURE REVIEW REQUIRED);
  - every accepted KF-CORE-R10 task record carries exactly one valid `**SECURITY IMPACT: ...**` classification in its
    Implementor Evidence section.
The audit validates that the classification is recorded consistently, never whether the engineering judgment behind it is right.
Exit status 0 when clean, 1 when any error is found. `--self-test` proves the audit detects deliberate defects.
"""
import argparse
import glob
import os
import re
import shutil
import sys
import tempfile

DOCS = ("SECURITY_ARCHITECTURE.md", "SECURITY_DECISIONS.md", "TRUST_BOUNDARIES.md", "THREAT_MODEL.md")
CLASSES = ("NONE", "DOCUMENTATION ONLY", "ARCHITECTURE REVIEW REQUIRED")
CLASS_RE = re.compile(r"\*\*SECURITY IMPACT: ([A-Z ]+)\*\*")
REVIEW = os.path.join("planning", "milestones", "KF-CORE-R10", "R10_SECURITY_REVIEW.md")
TASKS = os.path.join("planning", "milestones", "KF-CORE-R10", "tasks")


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def audit(root):
    errors = []
    sec = os.path.join(root, "docs", "security")
    texts = {}
    for name in DOCS:
        path = os.path.join(sec, name)
        if os.path.isfile(path):
            texts[name] = read(path)
        else:
            errors.append(f"docs/security/{name} is missing")
    decisions = texts.get("SECURITY_DECISIONS.md", "")
    ids = re.findall(r"\*\*SD-R10-(\d+)\*\*", decisions)
    if not ids:
        errors.append("SECURITY_DECISIONS.md records no SD-R10 decision")
    else:
        for i in sorted(set(ids)):
            if ids.count(i) > 1:
                errors.append(f"SD-R10-{i} is defined more than once in SECURITY_DECISIONS.md")
        numbers = sorted(int(i) for i in set(ids))
        if numbers != list(range(1, len(numbers) + 1)):
            errors.append(f"the SD-R10 decisions are not contiguous from 01: {', '.join(f'{n:02d}' for n in numbers)}")
        threat = texts.get("THREAT_MODEL.md", "")
        for i in sorted(set(ids)):
            if f"SD-R10-{i}" not in threat:
                errors.append(f"SD-R10-{i} is not referenced by THREAT_MODEL.md")
    arch = texts.get("SECURITY_ARCHITECTURE.md", "")
    if arch and not re.search(r"R1\.0 security-impact classification", arch):
        errors.append("SECURITY_ARCHITECTURE.md does not record the R1.0 security-impact classification")
    review = os.path.join(root, REVIEW)
    if not os.path.isfile(review):
        errors.append(f"{REVIEW.replace(os.sep, '/')} is missing")
    else:
        found = CLASS_RE.findall(read(review))
        if len(found) != 1 or found[0] not in CLASSES:
            errors.append(f"the R1.0 security review must carry exactly one valid classification, found {found}")
    checked = 0
    for path in sorted(glob.glob(os.path.join(root, TASKS, "*", "ACCEPTANCE_CRITERIA.md"))):
        text = read(path)
        if not re.search(r"^Status: ACCEPTED\s*$", text, flags=re.M):
            continue
        checked += 1
        evidence = text.split("## Implementor Evidence", 1)[1] if "## Implementor Evidence" in text else ""
        evidence = evidence.split("## Reviewer Decision", 1)[0]
        found = CLASS_RE.findall(evidence)
        rel = os.path.relpath(path, root).replace(os.sep, "/")
        if len(found) != 1:
            errors.append(f"{rel} records {len(found)} security-impact classifications (exactly one is required)")
        elif found[0] not in CLASSES:
            errors.append(f"{rel} records the invalid security-impact classification '{found[0]}'")
    return errors, checked


def _edit(tmp, rel, old, new):
    path = os.path.join(tmp, rel)
    text = read(path)
    if re.search(old, text, flags=re.M) is None:
        raise SystemExit(f"self-test setup error: {old!r} not found in {rel}")
    with open(path, "w", encoding="utf-8") as f:
        f.write(re.sub(old, new, text, count=1, flags=re.M))


def self_test(root):
    sd, th, ar = "docs/security/SECURITY_DECISIONS.md", "docs/security/THREAT_MODEL.md", "docs/security/SECURITY_ARCHITECTURE.md"
    t1 = "planning/milestones/KF-CORE-R10/tasks/KF-CORE-R10-001/ACCEPTANCE_CRITERIA.md"
    t2 = "planning/milestones/KF-CORE-R10/tasks/KF-CORE-R10-002/ACCEPTANCE_CRITERIA.md"
    cases = [
        ("a missing security document", lambda t: os.remove(os.path.join(t, "docs/security/TRUST_BOUNDARIES.md")), "TRUST_BOUNDARIES.md is missing"),
        ("a duplicated decision id", lambda t: _edit(t, sd, r"\*\*SD-R10-02\*\*", "**SD-R10-01**"), "defined more than once"),
        ("a gap in the decision ids", lambda t: _edit(t, sd, r"\*\*SD-R10-07\*\*", "**SD-R10-09**"), "not contiguous"),
        ("a decision the threat model does not reference", lambda t: _edit(t, th, r"SD-R10-06", "SD-R10-0X"), "SD-R10-06 is not referenced"),
        ("a missing R1.0 architecture classification", lambda t: _edit(t, ar, "R1.0 security-impact classification", "R1.0 notes"), "R1.0 security-impact classification"),
        ("a missing security review record", lambda t: os.remove(os.path.join(t, "planning/milestones/KF-CORE-R10/R10_SECURITY_REVIEW.md")), "R10_SECURITY_REVIEW.md is missing"),
        ("a task record with no classification", lambda t: _edit(t, t2, r"\*\*SECURITY IMPACT: DOCUMENTATION ONLY\*\*", "no classification"), "records 0 security-impact"),
        ("a task record with two classifications", lambda t: _edit(t, t1, r"\*\*SECURITY IMPACT: NONE\*\*", "**SECURITY IMPACT: NONE** and **SECURITY IMPACT: DOCUMENTATION ONLY**"), "records 2 security-impact"),
        ("a task record with an invalid classification", lambda t: _edit(t, t1, r"\*\*SECURITY IMPACT: NONE\*\*", "**SECURITY IMPACT: PROBABLY FINE**"), "invalid security-impact"),
        ("a review record with two classifications", lambda t: _edit(t, "planning/milestones/KF-CORE-R10/R10_SECURITY_REVIEW.md", r"\*\*SECURITY IMPACT: DOCUMENTATION ONLY\*\*", "**SECURITY IMPACT: DOCUMENTATION ONLY** **SECURITY IMPACT: NONE**"), "exactly one valid classification"),
    ]
    failures = 0
    for label, mutate, expected in cases:
        tmp = tempfile.mkdtemp(prefix="secdocs_selftest_")
        try:
            os.makedirs(os.path.join(tmp, "docs"))
            shutil.copytree(os.path.join(root, "docs", "security"), os.path.join(tmp, "docs", "security"))
            shutil.copytree(os.path.join(root, "planning", "milestones", "KF-CORE-R10"), os.path.join(tmp, "planning", "milestones", "KF-CORE-R10"))
            mutate(tmp)
            errors, _ = audit(tmp)
            if not any(expected in e for e in errors):
                print(f"[security-docs] SELF-TEST FAILED: the audit did not detect {label}")
                failures += 1
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    errors, _ = audit(root)
    if errors:
        print("[security-docs] SELF-TEST FAILED: the real records have errors")
        failures += 1
    if failures == 0:
        print(f"[security-docs] self-test passed: {len(cases)} deliberate defects detected")
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
        print(f"[security-docs] ERROR: {e}")
    print(f"[security-docs] {checked} accepted R1.0 task record(s) checked, {len(errors)} error(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
