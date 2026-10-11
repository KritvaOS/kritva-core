#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : test_gen_status.py
# Description : Regression tests for the Core status report generator
#
# Component   : Kritva Core
# Module      : Program Status Reporting
# Layer       : Development Infrastructure
#
# Requirements: kritva-program decisions D-009, D-011, D-012
# API         : python3 -m unittest scripts/status/test_gen_status.py
#
# Author      : KritvaOS
# Created     : 11-10-2026
#==============================================================================
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_status  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
IDS = [f"KF-CORE-R10-00{i}" for i in range(1, 10)]
MANIFEST = "| Task | Title |\n|---|---|\n" + "".join(f"| {t} | x |\n" for t in IDS) + "| R10 Release Gate | Release |\n"


def status_text(rows):
    return "| Item | Status |\n|---|---|\n" + "".join(f"| {t} | {s} |\n" for t, s in rows)


class RealFiles(unittest.TestCase):
    def test_manifest_has_nine_unique_tasks(self):
        ids = gen_status.parse_manifest(
            ROOT / "planning/milestones/KF-CORE-R10/TASK_MANIFEST.md")
        self.assertEqual(sorted(ids), IDS)

    def test_build_emits_nine_tasks_without_acceptance_fields(self):
        doc, _ = gen_status.build(ROOT, "2026-10-11T00:00:00Z")
        self.assertEqual([t["task_id"] for t in doc["tasks"]], IDS)
        for t in doc["tasks"]:
            self.assertEqual(set(t), {"task_id", "execution", "git_sync", "readiness", "evidence"})
            self.assertIsNone(t["evidence"][0]["commit"])
            self.assertTrue((ROOT / t["evidence"][0]["uri"]).is_file())

    def test_committed_status_facts_match_planning_records(self):
        import json
        doc, _ = gen_status.build(ROOT, "2026-10-11T00:00:00Z")
        old = json.loads((ROOT / "status/status.json").read_text())
        self.assertEqual(gen_status.facts(old), gen_status.facts(doc))


class Synthetic(unittest.TestCase):
    def parse(self, rows):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "S.md"
            p.write_text(status_text(rows), encoding="utf-8")
            return gen_status.parse_status(p, set(IDS))

    def test_accepted_maps_to_complete(self):
        out = self.parse([(t, "ACCEPTED (`abc1234`)") for t in IDS])
        self.assertEqual(gen_status.execution(out[IDS[0]]), "COMPLETE")

    def test_missing_status_row_fails(self):
        with self.assertRaises(ValueError):
            self.parse([(t, "ACCEPTED") for t in IDS[:-1]])

    def test_unknown_status_fails(self):
        with self.assertRaises(ValueError):
            self.parse([(t, "ACCEPTED") for t in IDS[:-1]] + [(IDS[-1], "SHIPPED?")])

    def test_duplicate_status_row_fails(self):
        with self.assertRaises(ValueError):
            self.parse([(t, "ACCEPTED") for t in IDS] + [(IDS[0], "ACCEPTED")])

    def test_other_milestone_rows_ignored(self):
        out = self.parse([(t, "ACCEPTED") for t in IDS] + [("KF-CORE-R09-001", "ACCEPTED")])
        self.assertEqual(sorted(out), IDS)

    def test_manifest_duplicate_fails(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "M.md"
            p.write_text(MANIFEST + f"| {IDS[0]} | again |\n", encoding="utf-8")
            with self.assertRaises(ValueError):
                gen_status.parse_manifest(p)

    def test_missing_evidence_target_fails(self):
        with tempfile.TemporaryDirectory() as d:
            r = Path(d)
            shutil.copytree(ROOT / "planning", r / "planning")
            (r / ".git" / "refs" / "heads").mkdir(parents=True)
            (r / ".git" / "refs" / "heads" / "main").write_text("a" * 40)
            (r / "planning/milestones/KF-CORE-R10/tasks/KF-CORE-R10-001/ACCEPTANCE_CRITERIA.md").unlink()
            with self.assertRaises(ValueError):
                gen_status.build(r, "2026-10-11T00:00:00Z")


if __name__ == "__main__":
    unittest.main()
