#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : check_contract_pin.py
# Description : Check that the engineering contract matches its pin
#
# Component   : Kritva Core
# Module      : Engineering Contract Pin
# Layer       : Development Infrastructure
#
# Requirements: kritva-program decisions D-023, PD-015
# API         : python3 scripts/contract/check_contract_pin.py [--verify-git]
#
# Author      : KritvaOS
# Created     : 07-11-2026
#==============================================================================
"""Check that the engineering contract matches its pin.

Exit codes: 0 pass, 1 mismatch or invalid pin, 2 pin file missing.
"""
import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

HEX40 = re.compile(r"^[0-9a-f]{40}$")
HEX64 = re.compile(r"^[0-9a-f]{64}$")
DEFAULT_PIN = "docs/development/ENGINEERING_CONTRACT.pin.json"


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def check(root, pin_rel=DEFAULT_PIN, verify_git=False):
    """Return (exit_code, messages)."""
    root = Path(root)
    pin_path = root / pin_rel
    if not pin_path.is_file():
        return 2, [f"PIN_MISSING: {pin_rel} does not exist"]
    try:
        pin = json.loads(pin_path.read_text(encoding="utf-8"))
    except ValueError as exc:
        return 1, [f"PIN_INVALID: {pin_rel} is not valid JSON ({exc})"]
    errs = []
    for key in ("contract_path", "sha256", "commit_sha"):
        if not isinstance(pin.get(key), str) or not pin.get(key):
            errs.append(f"PIN_INVALID: missing field {key}")
    if errs:
        return 1, errs
    if not HEX64.match(pin["sha256"]):
        errs.append("PIN_INVALID: sha256 must be 64 lower-case hex characters")
    if not HEX40.match(pin["commit_sha"]):
        errs.append("PIN_INVALID: commit_sha must be 40 lower-case hex characters")
    if errs:
        return 1, errs
    contract = root / pin["contract_path"]
    if not contract.is_file():
        return 1, [f"CONTRACT_MISSING: {pin['contract_path']} does not exist"]
    actual = sha256_bytes(contract.read_bytes())
    if actual != pin["sha256"]:
        errs.append(f"CONTRACT_CHANGED: file sha256 {actual} differs from pin {pin['sha256']}")
    if verify_git:
        res = subprocess.run(
            ["git", "-C", str(root), "show", f"{pin['commit_sha']}:{pin['contract_path']}"],
            capture_output=True,
        )
        if res.returncode != 0:
            errs.append(f"PIN_COMMIT_UNREADABLE: cannot read {pin['contract_path']} at {pin['commit_sha']}")
        elif sha256_bytes(res.stdout) != pin["sha256"]:
            errs.append("PIN_COMMIT_MISMATCH: file at commit_sha does not match the pinned sha256")
    if errs:
        return 1, errs
    return 0, ["PASS: contract matches its pin"]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--root", default=".")
    ap.add_argument("--pin", default=DEFAULT_PIN)
    ap.add_argument("--verify-git", action="store_true")
    ap.add_argument("--print-sha256", metavar="FILE", help="print the sha256 of FILE and exit")
    args = ap.parse_args(argv)
    if args.print_sha256:
        print(sha256_bytes(Path(args.print_sha256).read_bytes()))
        return 0
    code, msgs = check(args.root, args.pin, args.verify_git)
    for m in msgs:
        print(m)
    return code


if __name__ == "__main__":
    sys.exit(main())
