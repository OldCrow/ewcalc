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
  - #73 IMPLEMENTED on dev/v1.2.0 (2026-09-09), awaiting cross-platform
    verification before close. Data layer: ewpresenter::refdata
    (pages → sections → rows; Value and Formula row kinds) + ewp_ref_*
    bridge accessors + test_reference; all three Reference pages now
    render from it (per-frontend content tables deleted). RESOLVED
    design point: formulas render as typeset SVG snippets
    (assets/formulas/, render-diagrams.sh renders 2x PNGs, mid-gray
    for both themes) with Unicode plain-text equivalents from the data
    layer as accessibility label, tooltip, and copy text — chosen by
    the user over monospace text; all masters share font-size 15 and
    frontends display at exact 1x so glyph size matches across
    formulas. Proof row: FSPL standard (4πdf/c)² | log 32.44 form.
    MSVC gets /utf-8 on the UTF-8-literal TUs. VERIFIED macOS (build,
    14/14 ctest, SwiftLint, visual). VERIFIED Windows 2026-09-09: MSVC
    Release core build warning-free, 14/14 ctest (incl. new
    test_reference/test_bridge), full WinUI solution build warning-free,
    `dotnet format style --verify-no-changes` clean, and a runtime check
    of the packaged app — Reference page renders from refdata, both FSPL
    formula images display, all four spot-checked calculator pages still
    compute (no regression from the PageCodeBehinds.cs trim). The four
    WinUI inspection caveats from commit 6be6f6a's PR-side report are
    now all resolved:
      - UTF-8 across the CLI boundary: CORRECT. ToManaged(const char*)
        routes through the existing UTF-8 decoder and maps nullptr to
        managed null (preserving refdata's "field absent" convention).
        Verified live: ≈, π, ², ₁₀ all render and survive to the
        clipboard intact.
      - ImageOpened sizing: CORRECT and DPI-safe. Width is in DIPs, so
        PixelWidth/2 on a 2x master keeps the formula matched to
        surrounding text at any display scale.
      - C4679 pragma: still needed. CI builds on windows-2022 (VS 17.x)
        where it fires; confirmed VS 18 2026 (MSVC 19.51) no longer
        emits it. Comment in RefData.cpp now says to drop the pragma
        once CI moves off windows-2022.
      - Formula-row spacing: acceptable for the single proof row, but
        see the alignment item under Known Gaps before #74-#78 add
        multi-row formula sections.
    Two defects found and FIXED on this branch by running the app:
      - Formula copy text was WinUI-only-divergent — it copied just the
        standard form while macOS and Linux both copy
        `std   |   log`. Now matches verbatim.
      - Two CA1859 warnings (FrameworkElement return types) had broken
        the repo's warning-free-build standard; concrete Grid/StackPanel
        return types restored it.
    PENDING: Linux VM compile+visual (new formulas.qrc).
  - #74 OPEN — Propagation reference page (+ dB math unless split out).
  - #75 OPEN — Antenna types reference page (isotropic model, per-type
    specs, pattern thumbnails; band-letters table here or split).
  - #76 OPEN — Link budget reference page (canonical diagram + equation
    columns).
  - #77 OPEN — RCS reference page (simple-shape formulas + typical-
    target table, m² and dBsm).
  - #78 OPEN — Umbrella for remaining per-calculator reference pages;
    split as designs firm up.
  - #79 IMPLEMENTED on dev/v1.2.0 (2026-09-10, filed and built same
    day at user request, before #77): Glossary reference page — seven
    sections of one-line definitions incl. the ERP-vs-EIRP convention
    record (Adamy's EW usage: "ERP" with dBi gains, strictly EIRP,
    2.15 dB offset documented). Pure data-layer page, no assets.
  - PROGRESS 2026-09-10: #74 (Propagation + dB & Units pages, split per
    user), #75 (Antenna Types + Frequency Bands pages; antenna-diagram
    review round applied), #76 (Link Budget page + waterfall diagram)
    all IMPLEMENTED on dev/v1.2.0 and green through manual-dispatch CI;
    awaiting the user's batched cross-machine UI pass. Reference nav is
    data-driven on all three frontends (a new refdata page = one icon-
    map entry per platform). Later same day: #77 IMPLEMENTED (RCS page —
    scattering-regimes diagram, six optical-region shape maxima,
    insect→ship target table in m² and dBsm with dBsm copy values) and
    #79 IMPLEMENTED (see its entry). All verified in the user's Linux
    and Windows UI passes (fixes at 7fadf97/24693cc/0250405).
  - DECIDED 2026-09-10 — page order: Quick Values first (provisional),
    then general references (Glossary, dB & Units, Frequency Bands),
    then pages in calculator order (Propagation, Antenna Types, Link
    Budget, RCS, ...). New pages insert by calculator position.
  - DECIDED 2026-09-10 — Quick Values rule: QV stays a deliberately
    small "most-reached-for" front page; duplication with domain pages
    is free (shared static row arrays), so per table: when a domain
    page ships the richer version, QV drops the table (RCS did this at
    #77) or keeps a trimmed high-frequency subset. Still on QV:
    Antenna Gain + Sidelobes (quick-entry density), Noise Floor
    (cross-domain), Eb/N₀ (until a Digital page exists). Re-evaluate
    QV's residence at milestone close.
  - #78 CLOSED 2026-09-12: split (user-ratified: combined Radar &
    Detection page; all domains in v1.2.0) into #82 Receiver, #83
    Jamming, #84 Location, #85 Radar & Detection, #86 Doppler &
    Resolution, #87 Digital/DSSS — ALL IMPLEMENTED on dev/v1.2.0 the
    same day (through 7f2223d). Fourteen reference pages total; every
    calculator pane has its page. Highlights: #83 restructured after
    user review (the #72 diagrams depict RADAR jamming — radar SPJ/SOJ
    J/S forms added with the 71 dB constant and the R⁴-vs-R² note;
    comms J/S its own section; jamming calculator relabeled "Comms
    Jamming", #88/v1.3.0 filed for the radar calculator); #84 shows
    both Wegner EEP→CEP forms; #85 fixed a doc typo (radar.h +
    formulas.md said 20·log₁₀(R)=(…)/4; the implementation was always
    40·log₁₀ — prose corrected, code untouched); #86 carries the
    dilemma product; #87 completed the QV shrink (RCS and Eb/N₀ tables
    superseded by domain pages; QV = 3 cross-domain sections).
    PENDING PINS for the next book session: radar SPJ/SOJ J/S forms
    and the 71 constant (EW101 ch 9 / EW102 ch 5 family).
  - GATE (user, 2026-09-12): NO milestone PR until the user completes
    Linux and Windows UI passes over #82–#87. #82–#87 stay open on
    GitHub for the PR to close. BOTH PASSES ARE NOW DONE — Linux
    (close-out below) and Windows (2026-09-12, below) — so the gate is
    satisfied for #82–#87; what remains before the PR is the macOS and
    Linux half of the formula-column restructure (Known Gaps).
  - WINDOWS UI PASS 2026-09-12 (#82–#87 on dev/v1.2.0 at c4882db): core
    14/14, WinUI solution warning-free, dotnet format clean. All 13
    reference pages driven via UIA — 93/93 formula and diagram images
    load, every page reviewed visually, all 67 formula PNGs and 23
    diagrams staged into the package. Calculator outputs unchanged and
    the "Comms Jamming" relabel is live; formula copy still yields
    `std   |   log`. Spot-checked content renders right: #85's
    40·log₁₀ range form, the kT and Eb/N₀ notation, 1d44219's subscript
    glyphs.
    ONE defect found and fixed — the WinUI formula-pair wrap gap, now
    CLOSED. 1e983c1 gave WinUI MaxWidth + Stretch=Uniform intending
    shrink-to-fit, but the pair sits in a horizontal StackPanel, which
    measures children with unbounded width: MaxWidth caps upscale and
    never shrinks, so at the 860 px default the radar-range log form
    clipped mid-equation at the card edge. Wrapping the pair in a
    Viewbox (Uniform, StretchDirection=DownOnly) gives it a finite width
    and scales it down; natural size stays the ceiling. Verified clean on
    all 13 pages at 860 px and again at 700 px: no horizontal scrollbar,
    no image past the viewport edge. (The one Viewbox around the pair was
    superseded later the same day by one per form — see the column fix
    below — once the forms became column-bound.)
    THEN, at the user's direction, the flow-layout column alignment
    (Known Gaps) was closed on WinUI too: one Grid per section, standard
    forms in a shared Auto column and log forms in the star column, so
    every log form in a section shares a left edge. Propagation's six
    distinct log-form left edges collapsed to one per section; all 13
    pages re-verified clean at 860 px and 700 px, copy buttons intact on
    both row kinds. Accepted trade-off: a log form that must shrink no
    longer matches its standard form's glyph size — unavoidable once the
    two are column-bound, and only visible below ~900 px. macOS and Linux
    keep the flow layout until a session on those machines can verify the
    same restructure.
  - WINDOWS UI PASS 2026-09-10 (#74-#77, #79 on dev/v1.2.0 at 7fadf97):
    core 14/14, WinUI solution warning-free, dotnet format clean; all 8
    reference pages driven via UIA with every formula/diagram image
    verified loaded (46/46) and each page reviewed visually; calculator
    outputs unchanged. Four WinUI defects found and fixed:
      - Section headers: the WinUI-only uppercase mangled notation
        ("RATIO → DB", "ABSOLUTE DB UNITS", "(KT, 290 K)"). Replaced the
        one-off Eb/N₀ patch with a token list (dB family, Hz family, kT,
        Eb/N₀) restored at source positions — extend it when a title
        gains new notation.
      - Value rows at the 860 px default window: Glossary prose sized the
        Auto value column to its unwrapped width, crushing labels to
        nothing and clipping values and copy buttons. Label column Auto
        (capped 280), value column star + word-wrap — the Linux 7fadf97
        and macOS behaviour.
      - Nav grouping (user-reported): PROPAGATION / ANALYSIS / REFERENCE
        headers replaced by the "Calculators" / "Reference" pair macOS
        and Linux use.
      - Compact-mode group boundary (user-reported): the stock separator
        brush is near-invisible on the dark pane, and the reference items
        reuse calculator icons, so collapsed they read as repeats. The
        Reference separator now overrides NavigationViewItemSeparator-
        Foreground on that element only (the key also draws the pane
        border), with literal per-theme ControlStrongStroke colours — a
        StaticResource alias from the element-local theme dictionary
        resolved to nothing and hid the line entirely.
    Not fixed, noted: formula pairs don't wrap on WinUI — the widest
    (knife-edge) fits the 860 px default with ~60 px to spare but would
    clip in a narrower window (Linux solved its equivalent with
    WrapLongRows). The flow-layout column alignment (Known Gaps) is now
    visibly ragged on Propagation.

- v1.3.0 — Calculator Growth (open, #7), created 2026-09-12.
  - #88 OPEN — Radar jamming calculator (SPJ/SOJ): separate sidebar
    tab (ratified); the comms-only calculator was relabeled "Comms
    Jamming" on dev/v1.2.0 the same day. Radar J/S forms + 71 dB
    constant need physical-book pins (also owed on their v1.2.0
    reference-page copies).

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
- RELEASED 2026-09-06: after the user's review, a UX parity round landed
  before tagging — help tooltips on every input/result across all panes
  on all three platforms (WinUI first, then ported to macOS/Linux),
  Propagation regime InfoBar → result row, Location "About CEP" InfoBar →
  per-row tooltips, nav pane widened, and the error-bound tooltips'
  blank-square fix (converter falls back to help text via
  ConverterParameter). The user visually verified all three platform UIs.
  An initial v1.1.0 tag (pre-fixes) was deleted via a temporary
  "Protect release tags" ruleset adjustment by the user; the final tag
  sits on the docs commit with README/frontend-README coverage of the
  new panes. Tag CI fully green; Release published with all four
  artifacts (signed+notarized .dmg, .msix, .AppImage, .deb).
- #73 VERIFIED all three platforms 2026-09-09/10: Linux VM and Windows
  passes done by the user (their fixes pulled: Linux sidebar-icon/
  AppImage-plugin/monospace-font round, WinUI copy-text parity), then a
  manual-dispatch CI run on dev/v1.2.0 went fully green at 640457c —
  including the AppImage deploy, after adding libqt6svg6 +
  qt6-gtk-platformtheme to the Linux job's apt line (cc933f6 updated
  AGENTS.md prerequisites but not CI). Ready to close with the PR.
- Branch/PR model for v1.2.0 (user-ratified 2026-09-10): ONE PR for the
  whole milestone — the arc builds naturally off #73, and each PR costs
  a cross-machine UI verification round, so splitting inflates work.
  All issues land on dev/v1.2.0 (subagent worktree branches merge
  there); validate at sensible checkpoints with manual CI dispatch
  (`gh workflow run ci.yml --ref dev/v1.2.0` — CI does not trigger on
  dev pushes; dispatch also exercises tag-gated-otherwise packaging)
  plus Windows/Linux UI passes; the final PR should be a slam dunk.
- NEXT (session of 2026-09-12 (Windows) ended here): the formula-column
  restructure on Linux and macOS (Known Gaps) — the only known parity
  divergence left — each on a machine that can build and UI-pass it.
  Then the single dev/v1.2.0 → main milestone PR, validated by
  manual-dispatch CI plus the UI passes already recorded, closing
  #82–#87. Also carried: the PENDING PINS for the next book session
  (radar SPJ/SOJ J/S forms and the 71 constant). Full formula-fidelity
  sweep remains future assurance work.

## Session Close-out 2026-09-12 (Linux UI pass) [DERIVED]

Final Linux pass over the thirteen-page reference library. Every reference
page walked at both the 1180 px default and the 980 px window minimum; build
clean under `-Werror`, 14/14 tests.

Fixed this session (all pushed to `dev/v1.2.0`):
- `ac7b433` — formula images now shrink to fit, completing the 1e983c1
  sizing audit. The audit gave macOS `maxWidth + scaledToFit` and WinUI
  `MaxWidth + Stretch="Uniform"`, both genuinely responsive, but Qt got a
  fixed 560-logical cap, which is a different thing: a QLabel pixmap is a
  fixed size, so the widest masters still set the page's minimum width and a
  static cap only moves the overflow threshold. At 980 px Radar & Detection
  clipped its text behind a horizontal scrollbar with the copy buttons off
  the right edge. Replaced with a `FormulaImage` QLabel that rescales to the
  width it is given (aspect-preserving, never past natural size,
  `heightForWidth` + small `minimumSizeHint`). Linux now matches the other
  two rather than being the one frontend that cannot adapt.
- `1d44219` — proper subscript glyphs in the reference prose (shared
  refdata, so macOS and WinUI inherit it with no code change). See the
  entry below for what the Windows pass should look at.
- `.deb`/`.rpm` Qt SVG dependency — see the resolved Known Gaps entry.

Carried into the Windows session:
- **Font coverage for the new glyphs.** `Rⱼ` is U+2C7C (Latin Extended-C),
  rarer than the rest; `Gₛ Gₘ Rₜ Rₘₐₓ` are U+2090 block. All render in
  Yaru's UI and monospace fonts — unverified in Segoe UI / Consolas, where
  a gap shows as tofu. Affected rows: Jamming (`Gₛ vs Gₘ`, `Ranges`),
  Location (`Geometry`), Doppler & Resolution (`Adamy's form`,
  `Resolution cell`), Digital / DSSS (`BW vs R(bit)`), RCS
  (`Range scaling`, now `⁴√σ`).
- **Narrow-window check on Radar & Detection**, the page that broke here.
  WinUI's `MaxWidth`/`Uniform` should shrink correctly, but it is the one
  worth resizing by hand.
- **Uppercase-subscript caveat.** The typeset masters render G_S/G_M/R_T/R_J
  with uppercase subscripts; the prose now shows lowercase glyphs, because
  Unicode has no uppercase subscripts (nor b/z/θ — those three cases took
  the parenthetical form `σ(θ)`, `θ(az)`, `θ(el)`, `R(bit)` instead). A
  deliberate deviation from the images, not an oversight; revisit if the
  Windows pass finds it jarring.
- **Formula alt text keeps ASCII underscores** (`h_t`, `ERP_J`, `L_FSPL`,
  `d_FZ`, `T_D` ...) by decision, not omission: it is a plain-text
  transcription that also feeds the clipboard, and most of those subscripts
  have no glyph at all, so a partial conversion would read worse.

Pre-PR / release items, none of them Linux-specific:
- **Version is still 1.1.0** in five places — `CMakeLists.txt:2`,
  `frontend/linux/CMakeLists.txt:5`, `frontend/linux/src/main.cpp:13`,
  `frontend/macos/CMakeLists.txt:234` (`MACOSX_BUNDLE_SHORT_VERSION_STRING`)
  and `frontend/windows/.../Package.appxmanifest:19` (`1.1.0.0`). The built
  `.deb` is therefore `ewcalc_1.1.0_amd64.deb`. Bump with the changelog at
  release time, as in the v1.1.0 close-out.
- **No CI has run on `dev/v1.2.0` at any point.** `ci.yml` triggers on push
  only for `main` and `v*` tags, plus PRs targeting `main`, so every commit
  on this branch is verified locally only. Opening the PR is the first real
  CI signal, and packaging steps stay tag-gated even then —
  `workflow_dispatch` is the escape hatch if the AppImage/deb changes want
  exercising before a tag.
- Both lint scripts now run on the Linux box and pass clean
  (`lint-linux.sh`, `lint-cpp.sh`); cppcheck and clang-tidy are installed
  user-space there, not system-wide.

## Session Close-out 2026-09-12 (Windows UI pass) [DERIVED]

Final Windows pass over the thirteen-page reference library, closing the
UI-pass gate for #82–#87. Core build warning-free with 14/14 tests, WinUI
solution warning-free, `dotnet format style --verify-no-changes` clean. All
thirteen pages driven through UIA: 93/93 formula and diagram images load,
all 67 formula PNGs and 23 diagrams staged into the package, every page
reviewed visually, every page re-checked at 860 px and 700 px. Calculator
outputs unchanged, the "Comms Jamming" relabel is live, and formula copy
still yields `std   |   log`.

Fixed this session (all pushed to `dev/v1.2.0`):
- `0661674` — formula pairs shrink to fit, completing 1e983c1 on WinUI.
  The audit gave WinUI `MaxWidth` + `Stretch="Uniform"` believing it
  responsive, but the pair sat in a horizontal `StackPanel`, which measures
  children with unbounded width: `MaxWidth` only caps upscaling, so nothing
  ever shrank and at the 860 px default the radar-range log form clipped
  mid-equation. Same class of mistake as Qt's fixed 560-logical cap — a
  sizing rule that cannot actually respond to the width on offer.
- `848ad92` — formula columns aligned per section, closing the WinUI half
  of the flow-layout gap. See Known Gaps for the design and the
  glyph-size trade-off it accepts.

Carried items from the Linux close-out, all three resolved:
- **Font coverage — CLEAN.** `Rⱼ` (U+2C7C) and the U+2090 subscripts
  (`Gₛ Gₘ Rₜ Rₘₐₓ`) render correctly in both Segoe UI and the monospace
  face; no tofu. Checked by pixel-cropping the named rows: Jamming
  (`Gₛ vs Gₘ`, `Ranges`), RCS (`Range scaling`, now `⁴√σ`), Doppler &
  Resolution (`Resolution cell`), plus the Location/Digital prose seen in
  the page review.
- **Narrow-window check on Radar & Detection — this is where the clipping
  bug surfaced**, found and fixed as `0661674` above. The prediction that
  WinUI's `MaxWidth`/`Uniform` "should shrink correctly" was wrong, for the
  StackPanel reason above.
- **Uppercase-subscript caveat — not jarring in practice.** The typeset
  masters keep uppercase subscripts while the prose uses lowercase glyphs
  and the parenthetical forms; side by side in a row the difference reads
  as ordinary typographic variation, not as an error. Left as-is; reopen if
  the user disagrees on sight.

Carried into the next session:
- The formula-column restructure on **Linux and macOS** — the only known
  parity divergence left, and the last thing before the milestone PR.
  Details and the accepted trade-off are in the Known Gaps entry.

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
- Gaps are otherwise filed as GitHub issues on sight (see milestones
  above). The former entries — WinUI3 colour-coding (#62) and the macOS
  `EWCALC_BUILD_FRONTEND` no-op (#66) — are both closed.
- [RESOLVED 2026-09-12] Linux sidebar icons need Qt's SVG plugin at
  runtime. Found 2026-09-09 on Ubuntu 24.04; declared in AGENTS.md, fixed
  for the AppImage (cc933f6), and the `.deb`/`.rpm` path now closed —
  `CPACK_DEBIAN_PACKAGE_DEPENDS` gains `libqt6svg6` (verified to carry both
  `iconengines/libqsvgicon.so` and `imageformats/libqsvg.so`) and
  `CPACK_RPM_PACKAGE_REQUIRES` gains `qt6-qtsvg`. A built `.deb` was
  inspected to confirm the control file carries them. Also added
  `CPACK_DEBIAN_PACKAGE_RECOMMENDS=qt6-gtk-platformtheme`: without
  `platformthemes/libqgtk3.so` Qt never learns the desktop's icon theme and
  *no* theme icon resolves, SVG or not — but the app is fully usable
  without it, hence Recommends. Its RPM equivalent is deliberately not
  listed: the package name varies by distro and was not verified on a real
  Fedora/openSUSE box.
- [OPEN 2026-09-12 — macOS and Linux only] Formula rows used a flow
  layout, not columns, on all three frontends (WinUI `StackPanel`
  Horizontal, Linux `QHBoxLayout` + stretch, macOS HStack): each row's log
  form started wherever its own standard form ended, so a multi-row
  section had ragged log forms — measured at a ~330 px spread on
  Propagation, 287 on Link Budget. The v1.2.0 vision ("one equation per
  row") and #76 ("equation columns") both want columns.
  FIXED ON WINUI 2026-09-12 (user-sequenced: fix where it can be
  verified, leave the frontends this machine cannot build): one Grid per
  section, standard forms in a shared Auto column (capped 400 px) and log
  forms in the star column beside them, so the Auto column sizes to the
  section's widest standard form and every log form in that section lands
  on one left edge. Diagrams, value rows and formula headers span both
  columns, so interleaving order is unaffected. Each form sits in its own
  DownOnly Viewbox — a Grid cell hands its child a finite width, so a form
  scales down only when its column is too narrow and natural size stays
  the ceiling. Verified: Propagation's log edges collapsed from six to one
  per section, no clipping or horizontal scrollbar on any of the thirteen
  pages at 860 px or 700 px, copy buttons intact on both row kinds.
  Note the trade-off the column model accepts: at widths where a log form
  must shrink, it no longer matches its standard form's glyph size — the
  pair-scaled-as-a-unit behaviour only held while the forms were ragged.
  At any width that fits, both render at natural size and still match.
  REMAINING: the same restructure on Linux (`ReferencePage.cpp`,
  QFormLayout → per-section QGridLayout) and macOS (`ReferenceView.swift`,
  HStack → Grid/LazyVGrid) — neither builds on the Windows box, so they
  belong to a session on those machines, each with its own UI pass. Until
  then the three frontends differ on this detail.
- [RESOLVED 2026-09-09] The Windows box's VS 18 2026 install was damaged,
  not merely stale — a strictly worse case than the vswhere lag AGENTS.md
  records, and worth recognising if it recurs after an in-place upgrade.
  Symptoms: `vswhere` reported the instance `isComplete: False`,
  `isLaunchable: False` with **zero** workload packages and an empty
  `%ProgramData%\Microsoft\VisualStudio\Packages\_Instances`, so CMake
  refused to auto-select the generator ("the instance is not known to the
  Visual Studio Installer"); separately, no .NET SDK was present at all
  (runtime only, no `MSBuild\Sdks\Microsoft.NET.Sdk`), failing restore of
  both `ewcalc-winui.csproj` and `ewpresenter.net.vcxproj` with MSB4236.
  The MSVC C++ payload was intact throughout. Fix: VS Installer
  `setup.exe repair --installPath <path> --passive --norestart`, which
  restored the instance metadata and installed a .NET SDK; Windows
  rebooted during it. Afterwards `cmake --preset release` configures
  unpinned again, so the no-pin rule in AGENTS.md stands unchanged — the
  `CMAKE_GENERATOR_INSTANCE` pin used mid-diagnosis was a temporary
  crutch and is not in any committed file. Note the repair installed only
  .NET SDK 10.0.401; the net8.0 target still builds under it, but CI
  (windows-2022) uses the .NET 8 SDK, so this box and CI now differ. The
  one local-only symptom of that divergence is a benign NETSDK1198
  "publish profile 'win-x64' was not found" warning from SDK 10 — it does
  not appear on CI and is unrelated to #73.
  - `MainWindow.cpp`'s `addPage` sources nav icons from
    `QIcon::fromTheme`, and current GNOME themes (Adwaita 46+, Yaru) ship
    most of those names *only* as SVG — several only in their `-symbolic`
    form. Without Qt SVG (`libqsvgicon.so` iconengine + `libqsvg.so`
    imageformat) `fromTheme` fails silently and the sidebar renders just
    the handful of names the theme still carries as PNG. Nothing errors;
    the icons are simply absent.
  - DONE 2026-09-09: AGENTS.md's Linux prerequisite named only
    `qt6-base-dev`, which does **not** pull in Qt SVG on Debian/Ubuntu;
    now reads `qt6-base-dev libqt6svg6` and explains the silent failure.
    Same trap on a minimal aqtinstall setup, where `qtsvg` is a separate
    *archive* (`--archives qtsvg`; it is not a `-m` module) — also noted.
  - DONE 2026-09-09: the AppImage packaging gap was real, and worse than
    predicted. `linuxdeploy-plugin-qt` selects plugins from what the
    binary links, and `ewcalc` never links Qt6Svg (the dependency is a
    pure runtime theme lookup), so the packaged sidebar rendered with
    **no icons at all** — not merely the SVG ones. Two plugins were
    missing: `iconengines/libqsvgicon.so` (themed SVG entries go through
    the icon *engine*; the `libqsvg.so` imageformat the plugin does bundle
    is not sufficient) and `platformthemes/libqgtk3.so`, without which
    `QIcon::themeName()` is empty inside the AppImage and no theme icon
    resolves, PNG included. Fixed in `scripts/build-linux.sh` with
    `EXTRA_QT_MODULES="svg;gtk3"` + `DEPLOY_PLATFORM_THEMES=1`; verified
    by building an AppImage and launching it — all 11 nav icons render.
  - STILL OPEN: the `.deb`/`.rpm` path. CPack does not declare a Qt SVG
    dependency, so an installed package on a machine without `libqt6svg6`
    hits the same silent icon loss. Not yet reproduced or fixed.
- AGENTS.md context trim, complete.
  Raised by the 2026-09-07 fleet-wide AGENTS.md audit (durable vs
  on-demand context). AGENTS.md is imported eagerly by CLAUDE.md, so all
  of it is paid in every session in this repo; docs/ costs nothing until
  read. Sizes measured, line numbers current as of that date.
  - DONE 2026-09-07: the three per-frontend sections (5,265 B) moved to
    `docs/ARCHITECTURE.md`; AGENTS.md went 16,380 -> 11,410 bytes.
  - [DECLINED 2026-09-07] AGENTS.md:193-200, the GitHub-rulesets paragraph.
    Re-read: its last sentence is an operational rule — "Check a real CI run
    after pushing; the push's own success is not evidence" — earned by this
    repo being the only one of the six with required status checks and a
    bypass actor set to `always`. Behind a link it would not fire at the
    moment it matters. Proven the same day: three docs-only commits were
    pushed here with `Bypassed rule violations` reported, and the check was
    not made until afterwards.
  - [DERIVED 2026-09-07] That check, once made, was clean and explains itself:
    `ci.yml` and `codeql.yml` both carry
    `paths-ignore: ['**.md', 'docs/**', 'LICENSE']`, so a documentation-only
    push triggers no run at all. The three required checks could never have
    run for those commits, which is exactly the case the bypass actor exists
    for. Worth knowing before treating a missing run here as a failure.

## Build-Stack Standardization (2026-07-23) [DERIVED]
Cross-repo effort tracked in the fleet standards repo
([record](https://github.com/OldCrow/standards/blob/main/records/BUILD-STANDARDIZATION-PLAN.md)).
Commits: `25d1114` (`EWCALC_FRONTEND_CONFIG` fixed from a BOOL `option()` to
a `CACHE STRING`), `c8494fa` (CMakePresets.json schema 6, CMake minimum
bumped to 3.25, `frontend` extra preset). No Phase 3 work touched this repo
(app, not a library — see AGENTS.md CMake-standard deviations); its
AGENTS.md section checked post-Phase-3 and is still accurate.
