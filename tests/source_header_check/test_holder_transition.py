#!/usr/bin/env python3
"""Tests for the copyright holder line in scripts/lint/check_source_headers.py (kritva-standard decision D-1)."""
import importlib.util
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("check_source_headers", ROOT / "scripts/lint/check_source_headers.py")
H = importlib.util.module_from_spec(spec)
spec.loader.exec_module(H)

FIELDS = """# File        : x.py
# Description : test
# Component   : A
# Module      : B
# Layer       : C
# Requirements: R
# API         : API
# Author      : A
# Created     : 16-11-2026
"""


def header(copyright_line, spdx="# SPDX-License-Identifier: Apache-2.0"):
    return f"#!/usr/bin/env python3\n#====\n# {copyright_line}\n{spdx}\n#\n{FIELDS}#====\n\nprint(1)\n"


class HolderLine(unittest.TestCase):
    def check(self, text):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "x.py"
            p.write_text(text, encoding="utf-8")
            return H.validate_file(p)

    def test_new_holder_is_accepted(self):
        self.assertEqual(self.check(header("Copyright (c) 2026 Kritva Robotics Private Limited.")), [])

    def test_old_holder_is_rejected(self):
        errs = self.check(header("Copyright (c) 2026 KritvaOS"))
        self.assertTrue(any("HEADER-003" in e for e in errs))

    def test_other_holder_is_rejected(self):
        errs = self.check(header("Copyright (c) 2026 Somebody Else"))
        self.assertTrue(any("HEADER-003" in e for e in errs))

    def test_missing_trailing_period_is_rejected(self):
        errs = self.check(header("Copyright (c) 2026 Kritva Robotics Private Limited"))
        self.assertTrue(any("HEADER-003" in e for e in errs))

    def test_missing_spdx_is_still_rejected(self):
        errs = self.check(header("Copyright (c) 2026 Kritva Robotics Private Limited.", spdx="# SPDX-License-Identifier: MIT"))
        self.assertTrue(any("HEADER-001" in e for e in errs))


if __name__ == "__main__":
    unittest.main()
