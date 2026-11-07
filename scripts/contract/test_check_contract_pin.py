#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : test_check_contract_pin.py
# Description : Unit tests for the engineering contract pin checker
#
# Component   : Kritva Core
# Module      : Engineering Contract Pin
# Layer       : Development Infrastructure
#
# Requirements: kritva-program decisions D-023, PD-015
# API         : python3 -m unittest scripts/contract/test_check_contract_pin.py
#
# Author      : KritvaOS
# Created     : 07-11-2026
#==============================================================================
import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import check_contract_pin as C  # noqa: E402

CONTRACT = "docs/development/ENGINEERING_CONTRACT.md"


class PinCheck(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        (self.root / "docs/development").mkdir(parents=True)
        self.contract = self.root / CONTRACT
        self.contract.write_text("contract v1\n", encoding="utf-8")

    def tearDown(self):
        self.tmp.cleanup()

    def write_pin(self, **over):
        pin = {
            "contract_path": CONTRACT,
            "sha256": hashlib.sha256(self.contract.read_bytes()).hexdigest(),
            "commit_sha": "a" * 40,
        }
        pin.update(over)
        (self.root / C.DEFAULT_PIN).write_text(json.dumps(pin), encoding="utf-8")

    def test_missing_pin_is_not_a_pass(self):
        code, msgs = C.check(self.root)
        self.assertEqual(code, 2)
        self.assertIn("PIN_MISSING", msgs[0])

    def test_matching_pin_passes(self):
        self.write_pin()
        self.assertEqual(C.check(self.root)[0], 0)

    def test_changed_contract_fails(self):
        self.write_pin()
        self.contract.write_text("contract v2\n", encoding="utf-8")
        code, msgs = C.check(self.root)
        self.assertEqual(code, 1)
        self.assertIn("CONTRACT_CHANGED", msgs[0])

    def test_bad_sha_format_fails(self):
        self.write_pin(sha256="xyz")
        self.assertEqual(C.check(self.root)[0], 1)

    def test_bad_commit_format_fails(self):
        self.write_pin(commit_sha="abc")
        self.assertEqual(C.check(self.root)[0], 1)

    def test_missing_field_fails(self):
        (self.root / C.DEFAULT_PIN).write_text(json.dumps({"sha256": "0" * 64}), encoding="utf-8")
        self.assertEqual(C.check(self.root)[0], 1)

    def test_invalid_json_fails(self):
        (self.root / C.DEFAULT_PIN).write_text("{", encoding="utf-8")
        self.assertEqual(C.check(self.root)[0], 1)

    def test_missing_contract_fails(self):
        self.write_pin()
        self.contract.unlink()
        code, msgs = C.check(self.root)
        self.assertEqual(code, 1)
        self.assertIn("CONTRACT_MISSING", msgs[0])

    def test_verify_git_unreadable_commit_fails(self):
        self.write_pin()
        code, msgs = C.check(self.root, verify_git=True)
        self.assertEqual(code, 1)
        self.assertIn("PIN_COMMIT_UNREADABLE", msgs[-1])


if __name__ == "__main__":
    unittest.main()
