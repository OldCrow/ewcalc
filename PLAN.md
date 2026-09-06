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
Last reconciled against live GitHub state: 2026-09-06 UTC (v1.1.0 leg:
#62/#69/#70/#71/#72 closed, milestone #5 closed, tag v1.1.0 pushed).
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
- v1.1.0 — Coverage & Illustration (closed 2026-09-06 UTC, #5), created
  2026-09-05. Released as tag v1.1.0.
  - #62 CLOSED 2026-09-06 — resolved as already-implemented: the
    converter-based colour-coding (`FieldErrorToBrushConverter` on
    `BorderBrush`) shipped in v0.8.0 on every input of all pages; verified
    at runtime on the Windows machine (clean startup, red border + dashed
    sub-section on an invalid Location EEP input). The issue's premise
    ("ships without colour-coding") was stale — the crashing
    dependency-property attempt had already been superseded by the
    classic-binding approach the issue itself proposed as option 2.
  - #66 CLOSED 2026-09-05 — ewcalc_frontend target wired on macOS
    (ad-hoc signing; scripts/build-macos.sh stays the signed path);
    stale Phase-N naming purged repo-wide.
  - #67 CLOSED 2026-09-05 — header/formulas.md were already correct;
    stale DISCREPANCY note retired from test_radar.cpp.
  - #68 DONE 2026-09-06 — citation sweep completed against the physical
    books under the precedence rule; all seven headers now carry source
    attribution. Yield: firm pins (ERP, sensitivity, Fresnel, bulge,
    jamming set, digital set), five fidelity-docket deltas (DR 1.76 dB,
    beamwidth 30000/29000, horizon 4.122/4.11, both location CEP
    formulations), several verified-absent anchors (dBi/dBd, Friis
    closed form, SFDR, noise temp, coherent integration, dwell/FAR,
    all five Doppler/Resolution formulas — Adamy teaches those by
    nomograph/context), and the convention that Adamy never numbers
    equations. Wegner R-722-PR itself was then obtained from rand.org
    and checked: all three location primaries resolved, zero [OPEN]
    markers remain repo-wide. Fidelity docket subsequently cleared —
    see Provenance Framing.
  - #69 CLOSED 2026-09-06 — Radar detection statistics calculator on all
    three frontends (Albersheim + Shnidman with Swerling 0–4, fluctuation
    loss, dwell/hits/FAR; Shnidman constants validated against the
    exact-theory oracle in scripts/detection_oracle.py, worst error
    0.30 dB — the oracle needs scipy, not part of the test suite;
    reference values are baked into test_radar.cpp). WinUI3 pane shipped
    and runtime-verified (outputs match the presenter defaults).
  - #70/#71 CLOSED 2026-09-06 — one "Doppler & Resolution" pane
    (DopplerPresenter): Doppler shift, unambiguous range/velocity, first
    blind speed (dilemma product R_u·v_u = c·λ/8 test-guarded), range and
    az/el cross-range resolution. All three frontends; WinUI3 pane
    runtime-verified.
  - #72 CLOSED 2026-09-06 — Static geometry diagrams: ten SVG masters +
    PNG pipeline (7126d8c); style ratified by user. Wired into six panes
    on all three platforms — macOS/Linux (32f7d83), WinUI3 as collapsed
    Expander sections with the PNGs MSBuild-linked from the shared
    assets/diagrams/png set (not duplicated). Tier-3 panes deliberately
    have no diagram (Link/Radar/Digital/Detection are log-domain
    arithmetic; a Pd/SNR curve family would cross into the fenced-off
    plotting scope).
  - Reference material: R&S "Radar and electronic warfare" eGuide
    (Christian Wolff) used 2026-09-05 as the coverage checklist for #69–#72.
- v1.2.0 — Reference Library (open, #6), created 2026-09-06. Vision: a
  reference page per calculator where it makes sense — formulas in
  standard AND log form side by side (one equation per row), curated
  tables, thumbnails in the #72 style. Content compiled clean-room from
  multiple public sources under the citation precedence rule, never
  transcribed from the R&S eGuide that inspired it.
  - #73 OPEN — Reference data layer in ewpresenter (single source of
    truth; frontends render, not own). PREREQUISITE for all pages.
    Carries the open design point: formula rendering (monospace text
    vs. typeset SVG snippets).
  - #74 OPEN — Propagation reference page (+ dB math unless split out).
  - #75 OPEN — Antenna types reference page (isotropic model, per-type
    specs, pattern thumbnails; band-letters table here or split).
  - #76 OPEN — Link budget reference page (canonical diagram + equation
    columns).
  - #77 OPEN — RCS reference page (simple-shape formulas + typical-
    target table, m² and dBsm).
  - #78 OPEN — Umbrella for remaining per-calculator reference pages;
    split as designs firm up.

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

## Session Close-out 2026-09-05/06 (Windows) [DERIVED]
- v1.1.0 is COMPLETE and tagged. The Windows session delivered: WinUI3
  Detection and Doppler & Resolution panes (DetectionAdapter /
  DopplerAdapter in ewpresenter.net + XAML pages + settings
  persistence), the Receiver SQNR result row, #72 Geometry Expander
  sections on all six panes, and #62 closed as already-implemented
  (see milestone notes). Verified on the Windows machine: MSVC core
  build with all 13 tests passing, solution build warning-free,
  `dotnet format style --verify-no-changes` clean, and a runtime smoke
  of the packaged app (outputs cross-checked: SQNR−DR = 1.76 dB,
  Doppler 20.01 kHz @ 10 GHz/300 m/s, diagrams render, red-border
  validation live).
- Windows dev-run quirk recorded in AGENTS.md: the raw exe cannot run
  unpackaged (no WindowsPackageType=None) — register the loose AppX
  layout instead. The stale packaged v1.0.2 install was removed from
  this machine in the process; installing the v1.1.0 release MSIX
  restores it.
- OPTIONAL follow-up (Linux VM): visual smoke of the Qt6 panes
  (Detection, Doppler, Geometry sections, SQNR row) — CI compiles and
  ctests them but nobody has SEEN the Linux UI. [OPEN]
- NEXT: v1.2.0 Reference Library opens with #73 (data layer);
  full formula-fidelity sweep remains as future assurance work.

## Provenance Framing & Formula Fidelity [OPEN]
- Found 2026-09-06 while pinning #68's first citation: Adamy EW103
  Sec 4.5.3 p. 110 gives digital dynamic range as DR = 20·log10(2^n)
  = 6.02·n, while libew's digital_dynamic_range() returns the full-scale
  sinusoid SQNR 6.02·n + 1.76 (Walden). Both are standard; they differ by
  a constant 1.76 dB and answer slightly different questions. Left as-is
  for now — no code or doc change made.
- DR delta RESOLVED 2026-09-06 by offering both quantities:
  digital_dynamic_range() now returns the 20·log10(2^N) level ratio
  (Adamy EW103 4.5.3) and a new digital_sqnr() returns 6.02N + 1.76
  (Walden); the Receiver pane shows both rows. Semantic-identity issue,
  not precision — the name promised the level ratio.
- FIDELITY DOCKET CLEAR (2026-09-06): all five #68 deltas resolved —
  DR (both offered), beamwidth (29000 adopted), horizon (4.122 kept),
  both location CEP items (closed via Wegner: ours is Eq. 24a). The
  full formula-fidelity sweep remains as future assurance work, but no
  known deltas are pending decisions.
- DECIDED 2026-09-06 (side chat): framing (b) — ewcalc implements the
  standard formulas, inspired by and audited against Adamy's EW 101
  series and other authoritative resources. Expressed as a citation
  precedence rule:
  1. PRIMARY: the authoritative source the implementation actually
     follows (Friis, Walden, Shnidman, IEEE Std, ITU-R, ...).
  2. EW-SERIES ANCHOR: EW102 or EW103.
  3. EW101 only as fallback, and always annotated when its
     rule-of-thumb form differs from the implemented one.
  Book-vs-code deltas are documented differences at the definition
  site, not bugs (unless the implemented form is itself wrong).
- Fresnel-crossover finding (side chat 2026-09-06; book location
  corrected during the sweep): EW101's /75000 vs the /24000 form is NOT
  a unit change — two breakpoint definitions a factor of π apart:
  4·h₁·h₂/λ (EW101 rule of thumb) vs 4π·h₁·h₂/λ (the FSPL/two-ray
  intersection, ≈/23900, conventionally /24000 — found in EW103
  Sec 5.6 p.135, not EW102 as first guessed; Adamy explicitly chooses
  it because it equates LOS and two-ray attenuation). ewcalc implements
  this definition, correct for its regime-selection use. Do NOT
  "correct" the constant to EW101's. Recorded on #68.
- Horizon constant RESOLVED 2026-09-06: keep the exact k=4/3 value
  4.122 — both parties cite the same model and 4.11 is Adamy's
  rounding; fidelity to the shared model outranks the book's
  arithmetic. Documented difference stands, no code change.
- Nomograph method note (2026-09-06): Adamy EW103 Sec 5.7 (knife-edge)
  and nearly all of Sec 6.5 (bulge/horizon) work by nomograph, not
  closed form; libew implements Lee (1982) and the standard 4/3-earth
  algebra. Method difference documented at the definition sites.
- Beamwidth constant RESOLVED 2026-09-06: 29000 adopted (Adamy EW103
  Sec 3.7 p.70 — his stated choice among rule-of-thumb constants; no
  precision at stake, fidelity to the anchor text wins). 0.15 dB output
  shift, CHANGELOG'd. Also verified: Adamy EW101–EW103 never define
  dBi↔dBd — that conversion cites IEEE Std 145 alone.
- Location primaries RESOLVED 2026-09-06 against Wegner, RAND R-722-PR
  (1971) itself (free PDF at rand.org/pubs/reports/R0722.html; OCR'd
  locally): the implemented 0.59·(σs+σl) IS Wegner Eq. (24a) p.14 (max
  error 1% for σs/σl ≥ 0.5), and Adamy's 0.75·√(a²+b²) is Wegner
  Eq. (30) p.15 — the rotation-free 10%-error shortcut. Same primary,
  ours the tighter form → that fidelity item is CLOSED (keep 0.59).
  The 1.2·R·tan(σθ) and c·σt·R/(2B) rules have no closed-form twin in
  Wegner (he treats DF/TOA rigorously via Cramér-Rao covariance and
  isocontour charts, Secs III/IV) — both now cite their standard-
  rule-of-thumb status with Wegner as the rigorous treatment. Zero
  [OPEN] markers remain repo-wide.
- Follow-up: full formula-fidelity sweep of all libew functions against
  the books (superset of #68's pins; sequence: fidelity sweep → pins).

## Known Gaps [OPEN]
- None currently tracked here; gaps are filed as GitHub issues on sight
  (see milestones above). The former entries — WinUI3 colour-coding (#62)
  and the macOS `EWCALC_BUILD_FRONTEND` no-op (#66) — are both closed.

## Build-Stack Standardization (2026-07-23) [DERIVED]
Cross-repo effort tracked in the fleet standards repo
([record](https://github.com/OldCrow/standards/blob/main/records/BUILD-STANDARDIZATION-PLAN.md)).
Commits: `25d1114` (`EWCALC_FRONTEND_CONFIG` fixed from a BOOL `option()` to
a `CACHE STRING`), `c8494fa` (CMakePresets.json schema 6, CMake minimum
bumped to 3.25, `frontend` extra preset). No Phase 3 work touched this repo
(app, not a library — see AGENTS.md CMake-standard deviations); its
AGENTS.md section checked post-Phase-3 and is still accurate.
