# EWCalc — Plan / Status

## Decided [DERIVED]
- Three-layer architecture (libew → ewpresenter → frontend), `bridge` only
  for the macOS Swift frontend.
- Core coding conventions: C++20, strict warnings-as-errors, no external
  deps in libew/ewpresenter, `libew::units` for all RF quantities.
- Core static analysis is clang-tidy (`.clang-tidy`, repo root) + cppcheck,
  matching the existing CI `static-analysis` job — not lizard/CCN, which
  isn't used anywhere in this repo. `scripts/lint-cpp.sh` mirrors CI exactly
  and is a standalone local convenience wrapper (CI's inline commands in
  `ci.yml` are intentionally left as-is, not refactored to call the script).
- cppcheck applies cleanly to the Linux Qt6 frontend with no suppressions
  or Qt-aware ruleset needed — verified against `frontend/linux/src`.
- SwiftLint on the Ventura (macOS 13) machine must come from the official
  GitHub release's portable binary (`portable_swiftlint.zip`), not
  `brew install swiftlint` — Homebrew has no bottle for that OS/formula
  combination and falls back to building the entire Swift toolchain from
  source, which is impractically slow and already failed once (exit 132).
  This is Ventura-specific: on current macOS (e.g. the Tahoe M1 box) the
  formula is bottled and `brew install swiftlint` is fine.
  See `fix-homebrew-source-build` skill and `.swiftlint.yml`/
  `scripts/lint-macos.sh` (added 2026-07-15, closing #43).
- Windows toolchain detection (`scripts/build-windows.ps1`,
  `CMakeLists.txt`'s `EWCALC_BUILD_FRONTEND`) shares one `scripts/
  find-msbuild.ps1` helper: vswhere first, falling back to a
  version-sorted scan of the standard VS install roots. Needed because
  vswhere's cached instance-state reader can lag the actual installer
  version after an in-place VS upgrade (hit on this machine going from
  VS2022 to VS 18 2026) and silently report no installations even though
  VS is fully usable; the fallback is generic across VS version/edition,
  not specific to this machine. See AGENTS.md's Windows toolchain setup.
- Windows C# static analysis (`.editorconfig`, `EnableNETAnalyzers` in
  `ewcalc-winui.csproj`, added 2026-07-16 closing #44) runs only
  `dotnet format style <sln> --no-restore --verify-no-changes` in CI/
  locally — never bare `dotnet format`/`dotnet format whitespace`
  (collapses this codebase's deliberate vertical-alignment style) or
  `dotnet format analyzers` (can't reliably resolve `ewpresenter.net`'s
  C++/CLI types outside a full solution build). Roslyn analyzer
  diagnostics are gated by the normal build instead. See AGENTS.md's
  Windows Frontend conventions.

## GitHub Synchronization [DERIVED]
Last reconciled against live GitHub state: 2026-08-23.
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
- Dependabot PRs #60 (codeql-action group) and #61 (actionlint 1.73.2)
  merged 2026-08-23 (ac33d41, daa89c5); no Dependabot PRs outstanding.

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
- Open issues without milestone: 0 as of 2026-08-22.
- Closed issues without milestone: 9 (#5, #6, #37, #38, #39, #41, #43, #44,
  #45) as of 2026-08-22.
  - #43 CLOSED 2026-07-15 — SwiftLint set up as a standalone script
    (`scripts/lint-macos.sh`, matching the `lint-cpp.sh`/`lint-linux.sh`
    pattern) with a baseline `.swiftlint.yml`; codebase runs `--strict`
    clean.
  - #44 CLOSED 2026-07-16 — Roslyn analyzers / `dotnet format` set up for
    the Windows frontend, verified on an actual Windows/MSBuild toolchain
    (VS 18 2026); baseline CA1805/CA1001 findings fixed; CI step added.
    See Decided above for the exact `dotnet format` scoping.
  - #45 CLOSED 2026-07-15 — All four cppcheck baseline findings fixed;
    `scripts/lint-linux.sh` gated with `--error-exitcode=1`.

## Known Gaps [OPEN]
- WinUI3 colour-coding feature deferred due to x:Bind type-checking crash —
  not reattempted, root cause not fully resolved. [OPEN, file issue] — no
  GitHub issue yet; file one so the backlog has one source of truth.
- `EWCALC_BUILD_FRONTEND=ON` is a no-op on macOS: the APPLE branch of the
  top-level CMakeLists only prints "macOS frontend (Phase 4) — not yet
  implemented", although `frontend/macos/CMakeLists.txt` exists and
  `scripts/build-macos.sh` builds it (Xcode generator + signing vars).
  Either wire the option to the real frontend or say in AGENTS.md that
  the script is the only supported path. [OPEN, found 2026-08-23 during
  an M1 verification pass; core+tests 13/13 green on macOS Tahoe]

## Build-Stack Standardization (2026-07-23) [DERIVED]
Cross-repo effort tracked in the fleet standards repo
([record](https://github.com/OldCrow/standards/blob/main/records/BUILD-STANDARDIZATION-PLAN.md)).
Commits: `25d1114` (`EWCALC_FRONTEND_CONFIG` fixed from a BOOL `option()` to
a `CACHE STRING`), `c8494fa` (CMakePresets.json schema 6, CMake minimum
bumped to 3.25, `frontend` extra preset). No Phase 3 work touched this repo
(app, not a library — see AGENTS.md CMake-standard deviations); its
AGENTS.md section checked post-Phase-3 and is still accurate.
