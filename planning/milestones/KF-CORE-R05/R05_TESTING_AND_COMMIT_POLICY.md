# KF-CORE-R05 — Testing and Commit Policy

## 1. Unit Testing Rule

Every production contract introduced or changed by an R0.5 task requires focused unit/contract tests.

Tests must cover, as applicable:

- valid behavior;
- invalid arguments;
- invalid state;
- unavailable services;
- failure propagation;
- ownership/lifetime;
- boundary conditions;
- API self-containment.

## 2. Integration Testing Rule

Integration tests are mandatory when behavior crosses:

- RuntimeManager;
- PlatformContext;
- IPlatformAdapter;
- scheduler;
- clock;
- timer;
- watchdog;
- capability requirements.

Integration tests must:

- use public APIs;
- use test-only reference/fake platform services;
- avoid physical hardware;
- verify observable behavior;
- avoid private implementation hooks.

## 3. Regression Rule

Every task must run the complete existing regression suite.

Minimum:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

A focused test pass does not compensate for a regression failure.

## 4. Quality Rule

Run applicable:

```bash
make check
make traceability-check
```

And where configured:

- ASan/UBSan;
- TSan;
- `-Werror`;
- GCC `-fanalyzer`;
- coverage;
- install-consumer;
- prohibited dependency audit.

## 5. Commit Rule

One logical task = one primary implementation commit.

Exact messages are frozen in each acceptance document.

Do not:

- mix unrelated changes;
- amend accepted implementation commits;
- rewrite accepted history;
- silently change an earlier frozen API.

Review fixes after acceptance use focused follow-up commits.

## 6. Acceptance Rule

The independent reviewer decides:

- PASS;
- CHANGES REQUIRED;
- BLOCKED.

Implementation agents do not self-accept tasks.
