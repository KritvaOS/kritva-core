#!/usr/bin/env python3
#==============================================================================
# Copyright (c) 2026 KritvaOS
# SPDX-License-Identifier: Apache-2.0
#
# File        : gen_status.py
# Description : Generate status/status.json (kritva-program schema 2.0) for the Core R1.0 tasks
#
# Component   : Kritva Core
# Module      : Program Status Reporting
# Layer       : Development Infrastructure
#
# Requirements: kritva-program decisions D-009, D-011, D-012
# API         : Command-line status report generator
#
# Author      : KritvaOS
# Created     : 11-10-2026
#==============================================================================
"""Generate status/status.json (kritva-program schema 2.0) for kritva-core.

Scope: the nine KF-CORE-R10 implementation tasks only (Core R1.0). Earlier
milestones (R02..R09) are deliberately not reported here; whether they enter
the program baseline is a central decision.

Reports FACTS only, derived from this repo's own planning records:
  planning/milestones/KF-CORE-R10/TASK_MANIFEST.md        the task roster
  planning/milestones/KF-CORE-R10/tasks/<id>/ACCEPTANCE_CRITERIA.md  evidence pointer
  planning/MILESTONE_STATUS.md                            per-task status text
  git (or .git/refs when git is unavailable)              source commit and sync state
It never reports review or acceptance: those are decided centrally in
kritva-program (v2/tracking/approvals.json). Core's own planning records use
the word ACCEPTED; here that only means "planning records the task as done"
and is reported as execution=COMPLETE. The ACCEPTANCE_CRITERIA entry carries no `commit`,
`ci_run` or `report_sha256` because the planning records hold only short SHAs. Commit-bound CI
evidence comes from status/evidence.json (owner-curated, validated by load_ci_evidence) and is
emitted as a second evidence entry per task. Without that file, or for a task not listed in it, the
central consolidator grants no progress credit.

Field semantics (kept deliberately conservative):
  * execution: COMPLETE when the planning status starts with ACCEPTED or
    RELEASED. It does NOT mean central acceptance or any safety clearance.
  * git_sync: SYNCED only when git confirms a clean tree whose HEAD equals
    origin/main. Without git, cleanliness is unverified, so COMMITTED is used.
  * readiness: always IMPLEMENTABLE (a fixed default meaning only "software
    work may proceed, subject to central gates"; not safety clearance).
  * evidence: the ACCEPTANCE_CRITERIA.md path is a CRITERIA REFERENCE, not proof
    of completion. It has no commit or CI binding and can never support a PASS.
  * source.commit: the inspected source revision (HEAD at run time). The
    generated file is committed later, so it describes its parent state.

Usage: python3 scripts/status/gen_status.py [--root .] [--out status/status.json]
       [--updated-at 2026-10-11T00:00:00Z] [--check | --check-facts]
  --check        local: full comparison (also source commit and git_sync)
  --check-facts  CI: compare only the planning-derived facts (task IDs,
                 execution, evidence pointers); ignores source, git_sync and
                 updated_at, which legitimately differ in CI's checkout.
Exit: 0 ok | 1 parse/consistency error | 2 check drift
"""
import argparse
import datetime as dt
import json
import re
import subprocess
import sys
from pathlib import Path

WORKSTREAM_ID = "CORE"
REPO = "KritvaOS/kritva-core"
MILESTONE = "KF-CORE-R10"
MANIFEST_ROW = re.compile(r"^\|\s*(KF-CORE-R10-\d{3})\s*\|")
STATUS_ROW = re.compile(r"^\|\s*(KF-CORE-R10-\d{3})\s*\|\s*([^|]+?)\s*\|")
STATUS = re.compile(r"^(accepted|released|in progress|not started|blocked)", re.I)
SHA = re.compile(r"^[0-9a-f]{40}$")
RUN_URL = re.compile(r"^https://github\.com/KritvaOS/kritva-core/actions/runs/(\d+)$")
KINDS = {"OWN": "run on the task's own commit",
         "RERUN": "re-run of a run on the task's own commit",
         "MILESTONE": "run on a later commit; accepted as of that commit"}


def parse_manifest(path):
    """Return the ordered, unique list of task IDs in TASK_MANIFEST.md."""
    ids = []
    for line in path.read_text(encoding="utf-8").splitlines():
        m = MANIFEST_ROW.match(line)
        if m:
            if m.group(1) in ids:
                raise ValueError(f"{path}: duplicate task {m.group(1)}")
            ids.append(m.group(1))
    if not ids:
        raise ValueError(f"{path}: no task rows found")
    return ids


def parse_status(path, ids):
    """Return {task_id: status_text} from MILESTONE_STATUS.md for the given IDs.

    Fails instead of guessing: unrecognised status text, duplicate rows, or a
    manifest task with no status row are all errors.
    """
    out = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        m = STATUS_ROW.match(line)
        if not m or m.group(1) not in ids:
            continue
        tid, text = m.group(1), m.group(2).strip()
        if not STATUS.match(text):
            raise ValueError(f"{path}: unrecognised status for {tid}: {text!r}")
        if tid in out:
            raise ValueError(f"{path}: duplicate status row for {tid}")
        out[tid] = text
    missing = sorted(set(ids) - set(out))
    if missing:
        raise ValueError(f"{path}: no status row for {missing}")
    return out


def execution(status_text):
    t = status_text.lower()
    if t.startswith(("accepted", "released")):
        return "COMPLETE"
    if t.startswith("in progress"):
        return "IN_PROGRESS"
    if t.startswith("not started"):
        return "NOT_STARTED"
    if t.startswith("blocked"):
        return "BLOCKED"
    raise ValueError(f"unrecognised task status {status_text!r}")


def git(root, *args):
    try:
        r = subprocess.run(["git", "-C", str(root), *args], capture_output=True, text=True)
        return r.stdout.strip() if r.returncode == 0 else None
    except OSError:
        return None


def ref(root, rel):
    p = root / ".git" / rel
    return p.read_text().strip() if p.exists() else None


def source_info(root):
    head = git(root, "rev-parse", "HEAD") or ref(root, "refs/heads/main")
    if not head or not SHA.match(head):
        raise ValueError("cannot determine 40-hex HEAD commit")
    origin = git(root, "rev-parse", "origin/main") or ref(root, "refs/remotes/origin/main")
    dirty = git(root, "status", "--porcelain")
    if origin is None:
        sync = "NOT_SYNCED"
    elif dirty is None:  # git unavailable: cleanliness unverified, so never claim SYNCED
        sync = "COMMITTED"
    elif dirty:
        sync = "NOT_COMMITTED"
    elif head == origin:
        sync = "SYNCED"
    else:
        sync = "NOT_SYNCED"
    return {"branch": "main", "commit": head, "tag": None}, sync, dirty is None


def load_ci_evidence(root, known_tasks, executions):
    """Validate status/evidence.json and return {task_id: evidence entry}. Missing file means no CI evidence."""
    path = root / "status" / "evidence.json"
    if not path.is_file():
        return {}
    try:
        entries = json.loads(path.read_text(encoding="utf-8"))["entries"]
    except (ValueError, KeyError, TypeError) as e:
        raise ValueError(f"{path}: unreadable ({e})")
    out = {}
    for tid, e in entries.items():
        where = f"{path}: {tid}"
        if tid not in known_tasks:
            raise ValueError(f"{where}: unknown task")
        if executions[tid] != "COMPLETE":
            raise ValueError(f"{where}: has CI evidence but the task is not complete in planning")
        if not SHA.match(str(e.get("commit", ""))) or not SHA.match(str(e.get("task_commit", ""))):
            raise ValueError(f"{where}: commit and task_commit must be full 40-hex SHAs")
        m = RUN_URL.match(str(e.get("url", "")))
        if not m or str(e.get("ci_run")) != m.group(1):
            raise ValueError(f"{where}: url must be this repo's Actions run URL ending in ci_run")
        if e.get("result") != "PASS":
            raise ValueError(f"{where}: result must be PASS (record only successful runs)")
        if e.get("kind") not in KINDS:
            raise ValueError(f"{where}: kind must be one of {sorted(KINDS)}")
        if e["kind"] in ("OWN", "RERUN") and e["commit"] != e["task_commit"]:
            raise ValueError(f"{where}: {e['kind']} evidence must be on the task's own commit")
        if not e.get("workflow"):
            raise ValueError(f"{where}: workflow name required")
        out[tid] = {
            "type": "software", "uri": e["url"],
            "description": f"{e['workflow']} run {e['ci_run']} PASS on {e['commit'][:7]} ({KINDS[e['kind']]})",
            "commit": e["commit"], "ci_run": str(e["ci_run"]), "report_sha256": None, "result": "PASS"}
    return out


def build(root, updated_at):
    source, sync, dirty_unchecked = source_info(root)
    base = root / "planning" / "milestones" / MILESTONE
    ids = parse_manifest(base / "TASK_MANIFEST.md")
    statuses = parse_status(root / "planning" / "MILESTONE_STATUS.md", set(ids))
    tasks = []
    for tid in sorted(ids):
        acc = f"planning/milestones/{MILESTONE}/tasks/{tid}/ACCEPTANCE_CRITERIA.md"
        if not (root / acc).is_file():
            raise ValueError(f"evidence target missing for {tid}: {acc}")
        tasks.append({
            "task_id": tid,
            "execution": execution(statuses[tid]),
            "git_sync": sync,
            "readiness": "IMPLEMENTABLE",
            "evidence": [{
                "type": "software",
                "uri": acc,
                "description": f"Criteria reference only (not completion proof); planning status text: {statuses[tid]!r}",
                "commit": None, "ci_run": None, "report_sha256": None, "result": None,
            }],
        })
    ci = load_ci_evidence(root, set(ids), {t["task_id"]: t["execution"] for t in tasks})
    for t in tasks:
        if t["task_id"] in ci:
            t["evidence"].append(ci[t["task_id"]])
    doc = {"schema_version": "2.0", "workstream_id": WORKSTREAM_ID, "repo": REPO,
           "source": source, "tasks": tasks, "updated_at": updated_at}
    return doc, dirty_unchecked


def facts(d):
    return {**{k: v for k, v in d.items() if k not in ("source", "updated_at")},
            "tasks": [{k: v for k, v in t.items() if k != "git_sync"} for t in d["tasks"]]}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=".")
    ap.add_argument("--out", default="status/status.json")
    ap.add_argument("--updated-at", help="ISO-8601 UTC (default: now)")
    ap.add_argument("--check", action="store_true", help="fail if output file differs from regeneration")
    ap.add_argument("--check-facts", action="store_true",
                    help="fail if planning-derived facts differ (ignores source, git_sync, updated_at)")
    a = ap.parse_args()
    root = Path(a.root)
    ts = a.updated_at or dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    try:
        doc, dirty_unchecked = build(root, ts)
    except (ValueError, OSError) as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1
    if dirty_unchecked:
        print("WARN: git unavailable; uncommitted changes were not checked", file=sys.stderr)
    out = root / a.out
    if a.check_facts or a.check:
        if not out.exists():
            print(f"ERROR: {out} not found", file=sys.stderr)
            return 2
        old = json.loads(out.read_text())
        if a.check_facts:
            same = facts(old) == facts(doc)
        else:
            old["updated_at"] = doc["updated_at"]
            same = old == doc
        if not same:
            print("status.json differs from planning records; regenerate with scripts/status/gen_status.py", file=sys.stderr)
            return 2
        return 0
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(doc, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
