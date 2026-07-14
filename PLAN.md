# EWCalc — Plan / Status

## Decided [DERIVED]
- Three-layer architecture (libew → ewpresenter → frontend), `bridge` only
  for the macOS Swift frontend.
- Core coding conventions: C++20, strict warnings-as-errors, no external
  deps in libew/ewpresenter, `libew::units` for all RF quantities.
- Core static analysis is clang-tidy (`.clang-tidy`, repo root) + cppcheck,
  matching the existing CI `static-analysis` job — not cppcheck+lizard.
  Lizard is not used anywhere in this repo; `scripts/lint-cpp.sh` mirrors
  CI exactly and is a standalone local convenience wrapper (CI's inline
  commands in `ci.yml` are intentionally left as-is, not refactored to
  call the script).
- cppcheck applies cleanly to the Linux Qt6 frontend with no suppressions
  or Qt-aware ruleset needed — verified against `frontend/linux/src`.

## GitHub Synchronization [DERIVED]
Last reconciled against live GitHub state: 2026-07-14.
- GitHub is the collaborator-facing source for issues and milestones; this
  PLAN.md is the agent-facing durable project state. Keep both in sync.
- When creating, closing, reopening, retitling, or moving a GitHub issue or
  milestone, update this section in the same change set or note why it could
  not be updated.
- Reconcile this section against live GitHub state when either is true:
  (a) the task at hand involves reading the backlog to decide what to work
  on next, or creating/closing/retitling/moving an issue or milestone, or
  (b) more than 7 days have passed since the "Last reconciled" date above.
  Skip the check for tasks that don't touch the backlog or this file at
  all — a per-session or per-task refresh regardless of relevance is
  wasted effort in one direction and a rubber stamp in the other. Update
  the "Last reconciled" date whenever this section is actually re-checked,
  whether or not anything had drifted.

## GitHub Milestones [DERIVED]
Closed milestones are summarized only (title + counts) since their issue
lists are immutable history — fetch details on demand via
`gh issue list --state all --search 'milestone:"<title>"'` if ever needed.
Open milestones are fully itemized here since they reflect actionable state.
- v0.7.0 — Correctness (closed, #1): 0 open / 8 closed.
- v0.8.0 — UX Parity & Accessibility (closed, #2): 0 open / 7 closed.
- v0.9.0 — Infrastructure & Hardening (closed, #3): 0 open / 4 closed.
- v1.0.0 — Release Readiness (closed, #4): 0 open / 4 closed.
  - #26 CLOSED — Add CHANGELOG.md (backfilled from README) and CONTRIBUTING.md.
  - #27 CLOSED — Add Doxyfile and publish API docs (comments are already Doxygen-style).
  - #28 CLOSED — Create user-facing "Formulas & references" document (output ↔ EW101 equation map).
  - #29 CLOSED — Final domain audit: re-verify every formula against EW101/102/103 with citations recorded in tests.
  - Milestone closed on GitHub 2026-07-14; no further v1.0.0 work planned.

## GitHub Issues Without Milestone [DERIVED]
Same leaner convention as milestones above: closed items are a count only
(fetch via `gh issue list --state closed --json number,title,milestone -q
'.[] | select(.milestone == null)'` if ever needed); open items are fully
itemized since they're actionable.
- Open issues without milestone: 3 (#43, #44, #45) as of 2026-07-14.
- Closed issues without milestone: 6 (#5, #6, #37, #38, #39, #41) as of 2026-07-14.

## In Progress [OPEN]
- SwiftLint setup for the macOS frontend (#43, noted 2026-07-14): not
  installed on this machine, no `.swiftlint.yml`. Deferred — needs a
  decision on whether it runs via a script or an Xcode Run Script build
  phase, then installation (`brew install swiftlint`) and a baseline config.
- Windows Roslyn analyzers / `dotnet format` (#44, noted 2026-07-14): no
  `.editorconfig`, no analyzers enabled in `ewcalc-winui.csproj`. Deferred
  — this needs to be set up and verified on an actual Windows/MSBuild
  toolchain; changes made blind from macOS (no dotnet/MSBuild available
  here) can't be compile-verified and risk breaking the Windows build.
- `scripts/lint-linux.sh` is report-only (no `--error-exitcode`) pending
  cleanup of pre-existing style findings (#45, see below). Add the flag
  once those are cleared so it gates the same way `scripts/lint-cpp.sh` does.

## Known Gaps [OPEN]
- `frontend/linux` cppcheck baseline (#45, 2026-07-14): a shadowed
  parameter name in `Settings.cpp` (`setValue`'s `value` shadows the
  `value()` getter), two `useStlAlgorithm` style suggestions
  (`PageUtils.h`, `ReceiverPage.cpp`), and one `uninitMemberVarNoCtor`
  note (`ReferencePage.cpp`'s `RefSection::title`). All are stylistic,
  not correctness bugs; not fixed as part of this cleanup pass since it
  was scoped to tooling setup, not frontend code changes.
- WinUI3 colour-coding feature deferred due to x:Bind type-checking crash —
  not reattempted, root cause not fully resolved.

## Next Steps
- #43 — Set up SwiftLint for the macOS frontend.
- #44 — Set up Roslyn analyzers / `dotnet format` for the Windows frontend;
  schedule the actual Windows-side verification.
- #45 — Clear the `frontend/linux` cppcheck baseline findings, then add
  `--error-exitcode=1` to `scripts/lint-linux.sh`.
