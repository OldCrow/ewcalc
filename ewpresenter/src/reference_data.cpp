/// @file reference_data.cpp
/// @brief Static reference-library content (#73). Single source of truth:
/// the three frontends render exactly what is defined here.
///
/// This file is UTF-8 with non-ASCII literals (−, ≈, superscripts); MSVC
/// builds it with /utf-8 (set in ewpresenter/CMakeLists.txt).

#include "ewpresenter/reference_data.h"

#include <iterator>

namespace ewpresenter::refdata {

namespace {

constexpr Row val(const char* label, const char* value,
                  const char* copy = nullptr) {
    return Row{RowKind::Value, label, value, copy, nullptr, nullptr};
}

constexpr Row formula(const char* name, const char* svg_base,
                      const char* std_text, const char* log_text = nullptr) {
    return Row{RowKind::Formula, name, std_text, nullptr, log_text, svg_base};
}

// ── Propagation page (#74) ───────────────────────────────────────────────────
// Forms and constants follow libew exactly; provenance per docs/formulas.md
// (32.44 = fspl_constant_km_mhz; /24 000 Fresnel-crossover per Adamy EW103
// Sec 5.6; knife-edge anchors from libew's Lee (1982) fit, L(0) = 6.02 dB
// and the v >= 2.4 branch 20·log10(v/0.225) = 12.95 + 20·log10(v);
// /17.0 = 2·(4/3)·6371 km in metres; 4.122 = sqrt(2·(4/3)·6371) — Adamy
// rounds it to 4.11, EW103 Sec 6.5).

constexpr Row kPathLoss[] = {
    formula("Free-space path loss", "fspl",
            "FSPL = (4πdf/c)²",
            "L = 32.44 + 20 log₁₀ f(MHz) + 20 log₁₀ d(km)  dB"),
    formula("Two-ray ground reflection", "tworay",
            "L = d⁴ / (h_t² h_r²)",
            "L = 120 + 40 log₁₀ d(km) − 20 log₁₀ h_t(m) − 20 log₁₀ h_r(m)  dB"),
    formula("Fresnel-zone crossover", "fresnelxo",
            "d_FZ = 4π h_t h_r / λ",
            "d_FZ(km) = h_t(m) h_r(m) f(MHz) / 24 000"),
};

constexpr Row kDiffraction[] = {
    formula("Knife-edge diffraction", "knife",
            "ν = h √( 2(d₁+d₂) / (λ d₁d₂) )",
            "L(0) = 6.02 dB;  L ≈ 12.95 + 20 log₁₀ ν  (ν ≥ 2.4)"),
};

constexpr Row kEarthGeometry[] = {
    formula("Earth bulge", "bulge",
            "h = d₁ d₂ / (2 k Rₑ)",
            "h(m) = d₁(km) d₂(km) / 17.0  (k = 4/3)"),
    formula("Radio horizon", "horizon",
            "R = √(2 k Rₑ) (√h_t + √h_r)",
            "R(km) = 4.122 (√h_t(m) + √h_r(m))"),
};

constexpr Section kPropagationSections[] = {
    {"Path Loss",      kPathLoss,      std::size(kPathLoss),      "prop-two-ray"},
    {"Diffraction",    kDiffraction,   std::size(kDiffraction),   "prop-knife-edge"},
    {"Earth Geometry", kEarthGeometry, std::size(kEarthGeometry), "prop-horizon-bulge"},
};

// ── Glossary page (#79) ──────────────────────────────────────────────────────
// One-line clean-room definitions of terms the calculators and reference
// pages use without explanation. Unit definitions (dBm, dBi, …) stay on the
// dB & Units page. The ERP/EIRP pair records the app's convention: Adamy's
// EW usage pairs "ERP" with dBi gains (strictly EIRP); libew follows the
// primary source's term (EW103 Sec 5.2).

constexpr Row kGlossPower[] = {
    val("ERP",  "Tx power × antenna gain, referenced to a half-wave dipole (dBd)"),
    val("EIRP", "Tx power × antenna gain, referenced to isotropic (dBi); EIRP = ERP + 2.15 dB"),
    val("Convention here", "\"ERP\" with dBi gains, following Adamy's EW usage (strictly EIRP)"),
    val("Antenna gain", "directivity × efficiency, relative to isotropic (dBi)"),
    val("Effective aperture", "equivalent collecting area of an antenna; Aₑ = Gλ²/4π"),
};

constexpr Row kGlossAntenna[] = {
    val("3-dB beamwidth", "angular width where the pattern stays within 3 dB (half power) of the peak"),
    val("Boresight",      "direction of the main-lobe maximum"),
    val("Sidelobe",       "secondary pattern maximum outside the main lobe (quoted in dBc below peak)"),
    val("Back lobe",      "sidelobe opposite boresight"),
    val("Polarization",   "E-field orientation: linear (V/H/slant) or circular (RH/LH)"),
    val("Cross-pol mismatch", "orthogonal polarizations couple poorly: ≈20 dB linear–linear, ≈3 dB linear–circular"),
};

constexpr Row kGlossFrequency[] = {
    val("Octave",  "2:1 frequency span — the upper frequency is twice the lower (e.g. 2–4 GHz)"),
    val("Decade",  "10:1 frequency span — the upper frequency is ten times the lower (e.g. 2–20 GHz)"),
    val("Percent bandwidth", "bandwidth ÷ center frequency × 100"),
    val("Instantaneous bandwidth", "band processed at one moment, vs the wider tunable range"),
};

constexpr Row kGlossReceiver[] = {
    val("SNR",  "signal-to-noise power ratio (dB)"),
    val("Eb/N₀", "energy per bit over noise density — SNR normalized by data rate"),
    val("Noise figure", "SNR degradation added by a receiver, referenced to 290 K (dB)"),
    val("Noise temperature", "noise figure expressed as an equivalent source temperature (K)"),
    val("Sensitivity (MDS)", "minimum detectable signal: kTB + NF + required SNR"),
    val("Dynamic range", "span between sensitivity and overload/compression"),
    val("Processing gain", "SNR improvement from spreading or integration (e.g. chip rate ÷ data rate)"),
};

constexpr Row kGlossRadar[] = {
    val("RCS (σ)", "target's equivalent reflecting area (m² or dBsm)"),
    val("Swerling cases", "statistical RCS fluctuation models 0–4 (steady; slow/fast scan-to-scan or pulse-to-pulse)"),
    val("Pd",  "probability a real target crosses the detection threshold"),
    val("Pfa", "probability noise alone crosses the threshold"),
    val("Unambiguous range", "max range before echoes fold into the next pulse interval; c/(2·PRF)"),
    val("Blind speed", "radial speed whose Doppler aliases to zero; multiples of λ·PRF/2"),
    val("Duty cycle", "pulse width ÷ PRI; average power = peak power × duty cycle"),
};

constexpr Row kGlossJamming[] = {
    val("J/S", "jammer-to-signal power ratio at the victim receiver (dB)"),
    val("Burnthrough range", "range where the signal overcomes jamming (J/S drops below the effective threshold)"),
    val("Self-protection jamming", "jammer on the protected platform itself"),
    val("Stand-off jamming", "jammer on a separate platform outside the threat's range"),
    val("Partial-band jamming", "jammer power concentrated on part of a hopped or spread band"),
};

constexpr Row kGlossLocation[] = {
    val("AOA",  "angle of arrival — bearing-based direction finding"),
    val("TDOA", "time difference of arrival — hyperbolic location from a receiver baseline"),
    val("FDOA", "frequency difference of arrival — location from differential Doppler"),
    val("CEP",  "circular error probable — circle around the estimate holding 50 % of fixes"),
    val("EEP",  "elliptical error probable — the 50 % error ellipse; CEP ≈ 0.59·(a+b) (both forms on the Location page)"),
};

constexpr Section kGlossarySections[] = {
    {"Power & Gain",          kGlossPower,     std::size(kGlossPower)},
    {"Antenna Terms",         kGlossAntenna,   std::size(kGlossAntenna)},
    {"Frequency & Bandwidth", kGlossFrequency, std::size(kGlossFrequency)},
    {"Receiver & Signal",     kGlossReceiver,  std::size(kGlossReceiver)},
    {"Radar & Detection",     kGlossRadar,     std::size(kGlossRadar)},
    {"Jamming & EW",          kGlossJamming,   std::size(kGlossJamming)},
    {"Emitter Location",      kGlossLocation,  std::size(kGlossLocation)},
};

// ── dB & Units page (#74, split out per user decision 2026-09-10) ────────────

constexpr Row kDbRatios[] = {
    val("×2",     "+3.01 dB",  "3.01"),
    val("×4",     "+6.02 dB",  "6.02"),
    val("×5",     "+6.99 dB",  "6.99"),
    val("×8",     "+9.03 dB",  "9.03"),
    val("×10",    "+10.0 dB",  "10"),
    val("×100",   "+20.0 dB",  "20"),
    val("×1000",  "+30.0 dB",  "30"),
    val("Reciprocal (1/x)", "negate the dB value"),
};

constexpr Row kDbUnits[] = {
    val("dBm",  "power re 1 mW"),
    val("dBW",  "power re 1 W  (0 dBW = +30 dBm)", "30"),
    val("dBi",  "gain re isotropic antenna"),
    val("dBd",  "gain re half-wave dipole  (0 dBd = +2.15 dBi)", "2.15"),
    val("dBsm", "RCS re 1 m²"),
    val("dBc",  "level re carrier"),
};

constexpr Section kDbSections[] = {
    {"Ratio → dB",        kDbRatios, std::size(kDbRatios)},
    {"Absolute dB Units", kDbUnits,  std::size(kDbUnits)},
};

// ── Antenna Types page (#75) ─────────────────────────────────────────────────
// One section per antenna type: the section diagram is its pattern thumbnail
// (assets/diagrams/ant-*.svg) and value rows carry the specs. Values are
// clean-room engineering typicals compiled from standard antenna references
// (Balanis, "Antenna Theory"; ARRL Antenna Book; IEEE Std 145 terminology);
// gain copy values feed the Antenna calculator directly. Frequency ranges
// use IEEE Std 521 band letters (see the Frequency Bands page).

constexpr Row kIsotropic[] = {
    formula("Effective aperture", "aperture",
            "Aₑ = G λ² / (4π)",
            "Aₑ(dBsm) = G(dBi) + 20 log₁₀ λ(m) − 11.0"),
    val("Directivity", "0 dBi  (G = 1)",          "0"),
    val("Pattern",     "uniform over a sphere"),
    val("Purpose",     "gain reference for dBi"),
};

constexpr Row kDipole[] = {
    val("Polarization",    "linear"),
    val("Typical gain",    "2.15 dBi  (0 dBd)", "2.15"),
    val("3-dB beamwidth",  "≈ 78° (E-plane), omni (H-plane)"),
    val("Bandwidth",       "≈ 10 %"),
    val("Frequency range", "HF – UHF"),
};

constexpr Row kMonopole[] = {
    val("Polarization",    "linear (vertical)"),
    val("Typical gain",    "5.15 dBi over ideal ground plane", "5.15"),
    val("Ground-plane mount (typical)", "2 dBi",              "2"),
    val("Short whip / rubber duck",     "≈ 0 dBi  (typical)", "0"),
    val("3-dB beamwidth",  "omni (azimuth)"),
    val("Bandwidth",       "≈ 10 %"),
    val("Frequency range", "HF – UHF"),
};

constexpr Row kPatch[] = {
    val("Polarization",    "linear (circular with dual feed)"),
    val("Typical gain",    "6–9 dBi",           "7"),
    val("3-dB beamwidth",  "60–90°"),
    val("Bandwidth",       "1–5 %"),
    val("Frequency range", "UHF – Ka"),
};

constexpr Row kHorn[] = {
    val("Polarization",    "linear"),
    val("Typical gain",    "10–20 dBi",         "15"),
    val("3-dB beamwidth",  "10–60°"),
    val("Bandwidth",       "full waveguide band (≈ 40 %)"),
    val("Frequency range", "L – W"),
};

constexpr Row kHelix[] = {
    val("Polarization",    "circular"),
    val("Typical gain",    "10–15 dBi",         "12"),
    val("3-dB beamwidth",  "30–50°"),
    val("Bandwidth",       "≈ 70 %"),
    val("Frequency range", "VHF – S"),
};

constexpr Row kYagi[] = {
    val("Polarization",    "linear"),
    val("Typical gain",    "7–15 dBi (grows with boom length)", "10"),
    val("Gain, 2-element",  "≈ 7 dBi",    "7"),
    val("Gain, 3-element",  "≈ 8.5 dBi",  "8.5"),
    val("Gain, 5-element",  "≈ 10.5 dBi", "10.5"),
    val("Gain, 10-element", "≈ 14 dBi",   "14"),
    val("3-dB beamwidth",  "30–60°"),
    val("Bandwidth",       "≈ 5 %"),
    val("Frequency range", "HF – UHF"),
};

constexpr Row kLogPeriodic[] = {
    val("Polarization",    "linear"),
    val("Typical gain",    "6–8 dBi",           "7"),
    val("3-dB beamwidth",  "60–80°"),
    val("Bandwidth",       "10:1 (multi-decade)"),
    val("Frequency range", "HF – X"),
};

constexpr Row kSpiral[] = {
    val("Polarization",    "circular"),
    val("Typical gain",    "−5 to +3 dBi",      "0"),
    val("3-dB beamwidth",  "70–90°"),
    val("Bandwidth",       "9:1 (e.g. 2–18 GHz RWR)"),
    val("Frequency range", "S – Ka"),
};

constexpr Row kParabolic[] = {
    val("Polarization",    "set by feed (linear or circular)"),
    val("Typical gain",    "20–50 dBi (aperture-set)", "30"),
    val("3-dB beamwidth",  "0.5–5°"),
    val("Bandwidth",       "feed-limited"),
    val("Frequency range", "L – W"),
};

constexpr Row kArray[] = {
    val("Polarization",    "set by elements"),
    val("Typical gain",    "20–40 dBi",         "30"),
    val("3-dB beamwidth",  "1–10° (electronically steered)"),
    val("Bandwidth",       "module-limited (≈ octave)"),
    val("Frequency range", "L – Ka"),
};

constexpr Row kSidelobeLevels[] = {
    val("Uniform aperture — 1st SLL",   "−13 dBc",         "-13"),
    val("Taylor weighted — 1st SLL",    "−25 dBc",         "-25"),
    val("Low-sidelobe array — 1st SLL", "−35 dBc",         "-35"),
    val("Typical back lobe",            "−25 to −35 dBc"),
};

constexpr Section kAntennaSections[] = {
    {"Isotropic Reference",       kIsotropic,   std::size(kIsotropic),   "ant-isotropic"},
    {"Half-Wave Dipole",          kDipole,      std::size(kDipole),      "ant-dipole"},
    {"Quarter-Wave Monopole",     kMonopole,    std::size(kMonopole),    "ant-monopole"},
    {"Microstrip Patch",          kPatch,       std::size(kPatch),       "ant-patch"},
    {"Pyramidal Horn",            kHorn,        std::size(kHorn),        "ant-horn"},
    {"Axial-Mode Helix",          kHelix,       std::size(kHelix),       "ant-helix"},
    {"Yagi–Uda",                  kYagi,        std::size(kYagi),        "ant-yagi"},
    {"Log-Periodic Dipole Array", kLogPeriodic, std::size(kLogPeriodic), "ant-logperiodic"},
    {"Cavity-Backed Spiral",      kSpiral,      std::size(kSpiral),      "ant-spiral"},
    {"Parabolic Reflector",       kParabolic,   std::size(kParabolic),   "ant-parabolic"},
    {"Phased Array",              kArray,       std::size(kArray),       "ant-array"},
    {"Sidelobe Levels (re main lobe)", kSidelobeLevels, std::size(kSidelobeLevels)},
};

// ── Link Budget page (#76) ───────────────────────────────────────────────────
// Forms follow libew/docs/formulas.md: one-way budget and the LOS
// effective-range inversion per Adamy JED Feb 2011 p. 51 (our 32.44 where
// Adamy rounds to 32); sensitivity kTB + NF + SNR per Adamy EW103 Sec 4.4
// p. 97 (−114 dBm = kTB at 290 K in 1 MHz); ERP per EW103 Sec 5.2 p. 120.
// The FSPL row reuses the Propagation page's asset pair — one master, two
// pages.

constexpr Row kOneWayLink[] = {
    formula("Received power (Friis)", "friis",
            "P_r = P_t G_t G_r (λ/4πd)²",
            "P_rx = P_tx + G_tx + G_rx − L_FSPL  dBm"),
    formula("Link margin", "margin",
            "M = P_rx − S  dB"),
    formula("Effective range (LOS)", "linkrange",
            "R = 10^((ERP + G_r − S − 32.44 − 20 log₁₀ f(MHz))/20)  km"),
};

constexpr Row kLinkComponents[] = {
    formula("ERP", "erp",
            "ERP = P_t G_t",
            "ERP = P_tx(dBm) + G_tx(dB)"),
    formula("Receiver sensitivity", "sensitivity",
            "S = k T B F · SNRᵣ",
            "S = −114 + 10 log₁₀ B(MHz) + NF + SNRᵣ  dBm"),
    formula("Free-space path loss", "fspl",
            "FSPL = (4πdf/c)²",
            "L = 32.44 + 20 log₁₀ f(MHz) + 20 log₁₀ d(km)  dB"),
};

constexpr Section kLinkSections[] = {
    {"One-Way Link",        kOneWayLink,     std::size(kOneWayLink), "link-budget"},
    {"Component Equations", kLinkComponents, std::size(kLinkComponents)},
};

// ── Receiver page (#82) ──────────────────────────────────────────────────────
// Pins per docs/formulas.md: sensitivity per Adamy EW103 Sec 4.4 p. 97
// (shares the Link Budget page's asset pair); cascade per Friis (1944,
// sole source — Adamy's coverage is component-diagram based); DR vs SQNR
// per the resolved fidelity docket (Adamy EW103 4.5.3 for DR; Walden for
// SQNR, +1.76 dB = 10·log10(1.5) sine crest factor); SFDR per the
// standard IP2/IP3 slope definitions (Razavi, Pozar — Adamy is
// graphical); noise temperature per IEEE Std 686.

constexpr Row kRxSensitivity[] = {
    formula("Receiver sensitivity", "sensitivity",
            "S = k T B F · SNRᵣ",
            "S = −114 + 10 log₁₀ B(MHz) + NF + SNRᵣ  dBm"),
    formula("Noise temperature ↔ NF", "noisetemp",
            "Tₑ = (F − 1) · 290 K",
            "NF = 10 log₁₀(1 + Tₑ/290)  dB"),
    val("Passive loss", "a lossy line at temperature T adds Tₑ = (L − 1)·T"),
};

constexpr Row kRxCascade[] = {
    formula("Cascaded noise figure (Friis)", "cascade",
            "F = F₁ + (F₂−1)/G₁ + (F₃−1)/(G₁G₂) + ⋯"),
    val("Domains", "cascade in linear F and G, then convert back to dB"),
    val("Rule of thumb", "a low-noise, high-gain first stage sets the system NF"),
};

constexpr Row kRxDynamicRange[] = {
    formula("Digital dynamic range", "ddr",
            "DR = 20 log₁₀ 2ᴺ ≈ 6.02 N  dB"),
    formula("SQNR (full-scale sine)", "sqnr",
            "SQNR = 6.02 N + 1.76  dB"),
    val("DR vs SQNR", "the 1.76 dB is 10 log₁₀(1.5), the sine's crest-factor term"),
    formula("SFDR (2nd-order)", "sfdr2",
            "SFDR₂ = (IIP₂ − S) / 2"),
    formula("SFDR (3rd-order)", "sfdr3",
            "SFDR₃ = 2 (IIP₃ − S) / 3"),
};

constexpr Row kNoiseFloor[] = {
    val("1 Hz bandwidth",    "−174.0 dBm", "-174.0"),
    val("1 kHz bandwidth",   "−144.0 dBm", "-144.0"),
    val("1 MHz bandwidth",   "−114.0 dBm", "-114.0"),
    val("10 MHz bandwidth",  "−104.0 dBm", "-104.0"),
    val("100 MHz bandwidth", "−94.0 dBm",  "-94.0"),
    val("1 GHz bandwidth",   "−84.0 dBm",  "-84.0"),
};

constexpr Section kReceiverSections[] = {
    {"Sensitivity & Noise", kRxSensitivity,  std::size(kRxSensitivity)},
    {"Thermal Noise Floor  (kT, 290 K)", kNoiseFloor, std::size(kNoiseFloor)},
    {"Cascaded Stages",     kRxCascade,      std::size(kRxCascade), "receiver-cascade"},
    {"Dynamic Range",       kRxDynamicRange, std::size(kRxDynamicRange)},
};

// ── Jamming page (#83) ───────────────────────────────────────────────────────
// Pins per docs/formulas.md: comms J/S per Adamy EW102 Sec 5.8.1 p. 138 /
// EW103 Sec 9.1 p. 252; burnthrough inversions per Adamy EW101 Sec 9.3
// pp. 187-191 (our 32.44 where Adamy rounds); partial-band optimum per
// Adamy EW102 Sec 5.9.1.2 / EW103 Sec 9.3.1 (the corrected v0.7.0
// surplus-power behavior). Radar J/S forms (self-protection and
// stand-off; the 71 dB constant = 11 dB for 4π + 60 dB for the km/m²
// unit choices) are the standard Adamy radar-jamming equations
// (EW101 ch 9 / EW102 ch 5 family — exact page pin pending a book
// check), matching the geometries the two #72 diagrams depict; the
// comms form keeps its own section with the R⁴-vs-R² contrast noted.

constexpr Row kSpjRadar[] = {
    formula("Self-protection J/S", "jsspj",
            "J/S = ERP_J − ERP_S + 71 + 20 log₁₀ R(km) − 10 log₁₀ σ(m²)  dB"),
    val("R⁴ vs R²", "the skin return falls off as R⁴ (two-way) but jamming only as R² (one-way) — J/S improves 20 dB per range decade, so burnthrough happens close-in"),
    val("Constant 71", "collects 4π (11 dB) and the km / m² unit choices (60 dB)"),
};

constexpr Row kSojRadar[] = {
    formula("Stand-off J/S", "jssoj",
            "J/S = ERP_J − ERP_S + G_S − G_M + 71 + 40 log₁₀ R_T(km) − 20 log₁₀ R_J(km) − 10 log₁₀ σ(m²)  dB"),
    val("Gₛ vs Gₘ", "stand-off jamming usually enters the radar's sidelobes (Gₛ) while the target sits in the mainbeam (Gₘ)"),
    val("Ranges", "Rₜ is radar–target (two-way, 40 log); Rⱼ is radar–jammer (one-way, 20 log)"),
};

constexpr Row kJsComms[] = {
    formula("Communications J/S", "js",
            "J/S = (ERP_J + G_rj − L_j) − (ERP_S + G_rs − L_s)  dB"),
    val("One-way symmetry", "both paths are one-way (R²), so the terms mirror — unlike radar jamming's R⁴ vs R²"),
    val("Sign convention", "positive J/S favors the jammer"),
    val("Path regimes", "signal and jammer paths may use different LOS / two-ray models"),
};

constexpr Row kBurnthrough[] = {
    formula("Burnthrough range (LOS)", "burnlos",
            "d = 10^((M − 32.44 − 20 log₁₀ f(MHz))/20)  km"),
    formula("Burnthrough range (two-ray)", "burn2ray",
            "d = 10^((M − 120 + 20 log₁₀ h_t(m) + 20 log₁₀ h_r(m))/40)  km"),
    val("Margin M", "collects the link terms against the J/S threshold — see the Jamming calculator"),
};

constexpr Row kPartialBand[] = {
    formula("Optimum jamming bandwidth", "pbopt",
            "BW_opt = BW_sig · 10^(J/S₁/10)"),
    formula("Duty cycle", "pbduty",
            "δ = min(BW_opt, BW_hop) / BW_hop"),
    val("Rationale", "surplus jammer power widens coverage across more hops rather than over-jamming one channel"),
};

constexpr Section kJammingSections[] = {
    {"Radar Jamming — Self-Protection", kSpjRadar,    std::size(kSpjRadar),    "jamming-self-protection"},
    {"Radar Jamming — Stand-Off",       kSojRadar,    std::size(kSojRadar),    "jamming-stand-off"},
    {"Comms Jamming",                   kJsComms,     std::size(kJsComms)},
    {"Burnthrough (one-way link)",      kBurnthrough, std::size(kBurnthrough)},
    {"Partial-Band Jamming",            kPartialBand, std::size(kPartialBand)},
};

// ── Location page (#84) ──────────────────────────────────────────────────────
// Pins per docs/formulas.md: all three CEP forms trace to Wegner R-722-PR
// (1971), obtained and verified 2026-09-06. CEP-from-EEP is Wegner Eq (24a)
// 0.59(a+b), ≤1 % for a/b ≤ 2; Adamy's 0.75·√(a²+b²) (EW102 Sec 6.6.2) is
// Wegner Eq (30), the rotation-free ≤10 % shortcut — both shown, per the
// citation rule. AOA/TDOA closed forms are standard rules of thumb; Wegner
// treats both rigorously by Cramér-Rao covariance and isocontour.

constexpr Row kCepAoa[] = {
    formula("CEP from AOA", "cepaoa",
            "CEP ≈ 1.2 R tan σ_θ"),
    val("Basis", "CEP = 1.1774·σ for circular 2-D Gaussian error, rounded to 1.2"),
    val("Geometry", "two receivers, ideal 90° crossing; R·tan σ(θ) is RMS cross-range error"),
};

constexpr Row kCepTdoa[] = {
    formula("CEP from TDOA", "ceptdoa",
            "CEP ≈ c σ_t R / (2B)"),
    val("Geometry", "ideal baseline perpendicular to the emitter; Wegner Sec IV maps real geometries by isocontour"),
};

constexpr Row kCepEep[] = {
    formula("CEP from EEP (Wegner Eq 24a)", "cepeep",
            "CEP ≈ 0.59 (a + b)"),
    formula("Adamy's shortcut (Wegner Eq 30)", "cepadamy",
            "CEP ≈ 0.75 √(a² + b²)"),
    val("Which to use", "Eq 24a is ≤1 % for a/b ≤ 2 (libew's choice); Eq 30 is rotation-free but ≤10 %"),
    val("Axes", "a, b are 1-σ semi-major/semi-minor axes of the error ellipse"),
};

constexpr Section kLocationSections[] = {
    {"CEP from Angle of Arrival", kCepAoa,  std::size(kCepAoa),  "loc-aoa-cep"},
    {"CEP from TDOA",             kCepTdoa, std::size(kCepTdoa)},
    {"Error Ellipse → CEP",       kCepEep,  std::size(kCepEep),  "loc-eep-cep"},
};

// ── Radar & Detection page (#85, combined per user ratification) ─────────────
// Pins per docs/formulas.md: range equation per Adamy EW102 Sec 3.2/3.2.2
// (the 40·log10 form matching libew's implementation — the former
// 20·log10(R)=(…)/4 doc line was a typo, fixed with this page); pulse
// compression per EW102 Sec 3.5.2; LPI advantage per EW102 Sec 3.9.5;
// coherent integration per Richards (no EW-series anchor); Albersheim/
// Shnidman are numeric methods anchored to the Detection calculator
// (oracle-validated, worst 0.30 dB); Swerling summaries per Richards ch 6.

constexpr Row kRangeEquation[] = {
    formula("Radar range equation", "radarrange",
            "R_max⁴ = P_t G² λ² σ / ((4π)³ · kTBF · SNRᵣ · L)",
            "40 log₁₀ R(m) = P_t + 2G + 20 log₁₀ λ(m) + σ − 30 log₁₀ 4π − N − SNRᵣ − L"),
    formula("Pulse compression gain", "pulsecomp",
            "G_pc = 10 log₁₀ (T·B)"),
    formula("Coherent integration gain", "cohint",
            "G_int = 10 log₁₀ N"),
    formula("LPI advantage", "lpi",
            "LPI advantage = 10 log₁₀ (T·B) / 4"),
    val("LPI caveat", "the advantage collapses to 0 dB if the intercept receiver also matches the waveform"),
};

constexpr Row kDetectionStats[] = {
    val("Albersheim", "closed-form required SNR (nonfluctuating, noncoherent N pulses; <0.2 dB in envelope) — computed by the Detection calculator"),
    val("Shnidman", "required SNR for Swerling 0–4; oracle-validated to 0.30 dB — computed by the Detection calculator"),
    formula("Fluctuation loss", "flucloss",
            "L_f = SNRᵣ(Sw n) − SNRᵣ(Sw 0)"),
    formula("Dwell time and hits", "dwell",
            "T_D = θ_az / ω;  hits = T_D · PRF"),
    formula("False-alarm rate", "far",
            "FAR = P_fa · B"),
};

constexpr Row kSwerlingCases[] = {
    val("Swerling 0 (V)", "steady target — no fluctuation (Marcum)"),
    val("Swerling 1", "many equal scatterers, scan-to-scan (slow) fluctuation"),
    val("Swerling 2", "many equal scatterers, pulse-to-pulse (fast) fluctuation"),
    val("Swerling 3", "one dominant + small scatterers, scan-to-scan"),
    val("Swerling 4", "one dominant + small scatterers, pulse-to-pulse"),
    val("Rule of thumb", "fluctuation costs most near high Pd — see Fluctuation loss above"),
};

constexpr Section kRadarDetSections[] = {
    {"Range Equation",       kRangeEquation,  std::size(kRangeEquation)},
    {"Detection Statistics", kDetectionStats, std::size(kDetectionStats)},
    {"Swerling Cases",       kSwerlingCases,  std::size(kSwerlingCases)},
};

// ── Doppler & Resolution page (#86) ──────────────────────────────────────────
// Pins per docs/formulas.md: Doppler shift per Adamy EW102 Sec 3.6.1 p. 55
// (6.67 = 2/0.29979 in the GHz engineering form); unambiguous range per
// EW102's PRI statement (algebraically c/(2·PRF)); blind speed and
// unambiguous velocity per Richards/Skolnik (absent from Adamy, verified
// 2026-09-06); the dilemma product R_u·v_u = c·λ/8 is test-guarded in
// test_radar.cpp; range resolution c/(2B) with Adamy's SAR c·PW/2 as the
// uncompressed variant; cross-range is real-beam R·θ with Adamy's SAR
// λR/(2L) noted as a method delta.

constexpr Row kDopplerRows[] = {
    formula("Doppler shift (two-way)", "doppler",
            "f_d = 2 v_r f / c",
            "f_d ≈ 6.67 v_r(m/s) f(GHz)  Hz"),
    formula("Unambiguous range", "unambrange",
            "R_u = c / (2 PRF)"),
    val("Adamy's form", "stated as Rₘₐₓ < 0.5·PRI·c — algebraically the same"),
    formula("First blind speed", "blindspeed",
            "v_b = λ PRF / 2  (multiples n·v_b)"),
    formula("Unambiguous velocity", "unambvel",
            "v_u = λ PRF / 4"),
    formula("Doppler dilemma", "dilemma",
            "R_u · v_u = c λ / 8"),
    val("Dilemma", "the product is PRF-invariant — raising PRF buys velocity coverage at the cost of range coverage"),
};

constexpr Row kResolutionRows[] = {
    formula("Range resolution", "rangeres",
            "ΔR = c / (2B)",
            "ΔR = c τ / 2  (uncompressed pulse, B ≈ 1/τ)"),
    formula("Cross-range resolution", "crossres",
            "ΔX = R θ_3dB  (θ in radians)"),
    val("SAR azimuth (method delta)", "Adamy EW102 3.8.2 gives d = λR/(2L) — SAR's two-way phase history halves the real-beam λR/L"),
    val("Resolution cell", "ΔR × R·θ(az) × R·θ(el) bounds the cell — see the diagram"),
};

constexpr Section kDopplerSections[] = {
    {"Doppler",    kDopplerRows,    std::size(kDopplerRows)},
    {"Resolution", kResolutionRows, std::size(kResolutionRows), "resolution-cell"},
};

// ── Digital / DSSS page (#87) ────────────────────────────────────────────────
// Pins per docs/formulas.md: Eb/N0 ↔ SNR per Adamy EW102 Sec 5.6.6/5.6.7
// p. 128; process gain per EW102 Sec 5.7.3 p. 136; jamming margin and
// required J/S per EW102 Sec 5.9.3 pp. 147-149. The modulation table
// supersedes Quick Values' Eb/N0 table per the ratified QV rule — same
// audited five rows plus the Shannon-limit anchor (−1.6 dB, ln 2).

constexpr Row kEbnoRows[] = {
    formula("Eb/N₀ ↔ SNR", "ebno",
            "Eb/N₀ = SNR + 10 log₁₀(BW/R_b)",
            "SNR = Eb/N₀ − 10 log₁₀(BW/R_b)"),
    val("BW vs R(bit)", "BW is the receiver noise bandwidth; R(bit) the bit rate — the ratio normalizes SNR per bit"),
};

constexpr Row kModulation[] = {
    val("BPSK / QPSK,  BER 10⁻³", "6.8 dB",     "6.8"),
    val("BPSK / QPSK,  BER 10⁻⁵", "9.6 dB",     "9.6"),
    val("BPSK / QPSK,  BER 10⁻⁶", "10.5 dB",    "10.5"),
    val("Non-coh. FSK, BER 10⁻³", "≈ 13.5 dB",  "13.5"),
    val("Non-coh. FSK, BER 10⁻⁵", "≈ 17.0 dB",  "17.0"),
    val("Shannon limit",          "−1.6 dB — the ultimate coding bound (ln 2)", "-1.6"),
};

constexpr Row kSpreadSpectrum[] = {
    formula("DSSS process gain", "procgain",
            "PG = 10 log₁₀(R_c/R_b)"),
    formula("Jamming margin", "jammargin",
            "JM = PG − (Eb/N₀)ᵣ − L_impl;  (J/S)ᵣ = −JM"),
    val("Sign conventions", "positive JM: spreading gain exceeds the jammer's advantage; positive (J/S)ᵣ: the jammer must overpower the signal at the receiver"),
};

constexpr Section kDigitalSections[] = {
    {"Eb/N₀ & SNR",                      kEbnoRows,       std::size(kEbnoRows)},
    {"Eb/N₀ Requirements (AWGN, uncoded)", kModulation,   std::size(kModulation)},
    {"Spread Spectrum",                  kSpreadSpectrum, std::size(kSpreadSpectrum)},
};

// ── RCS page (#77) ───────────────────────────────────────────────────────────
// Simple-shape maxima are optical-region (dimension ≫ λ) closed forms per
// the standard radar literature (Skolnik, Knott); the regimes diagram is the
// classic sphere-RCS sketch. Typical targets are clean-room representative
// values in both m² and dBsm; copy values are dBsm, feeding the Radar
// calculator's σ input. Supersedes the former Quick Values RCS table.

constexpr Row kRcsShapes[] = {
    formula("Sphere (optical)",            "rcs-sphere",    "σ = π r²"),
    formula("Flat plate (normal, a×b)",    "rcs-plate",     "σ = 4π a² b² / λ²"),
    formula("Cylinder (broadside)",        "rcs-cyl",       "σ = 2π r h² / λ"),
    formula("Dihedral corner (max)",       "rcs-dihedral",  "σ = 8π a² b² / λ²"),
    formula("Trihedral corner (max)",      "rcs-trihedral", "σ = 4π a⁴ / (3λ²)"),
    formula("m² → dBsm",                   "rcs-dbsm",      "σ(dBsm) = 10 log₁₀ σ(m²)"),
    val("Validity", "closed forms hold in the optical region (dimensions ≫ λ)"),
    val("Range scaling", "detection range ∝ ⁴√σ — every 12 dB of RCS reduction halves it"),
};

constexpr Row kRcsTargets[] = {
    val("Insect",                          "≈ 0.0001 m²  (−40 dBsm)", "-40"),
    val("Bird",                            "≈ 0.03 m²  (−15 dBsm)",   "-15"),
    val("Person",                          "≈ 1 m²  (0 dBsm)",        "0"),
    val("Automobile",                      "≈ 100 m²  (+20 dBsm)",    "20"),
    val("Cruise missile",                  "≈ 0.2 m²  (−7 dBsm)",     "-7"),
    val("LO fighter (nose-on)",            "≈ 0.03 m²  (−15 dBsm)",   "-15"),
    val("Fighter (nose-on, conventional)", "≈ 1 m²  (0 dBsm)",        "0"),
    val("Fighter (broadside)",             "≈ 5 m²  (+7 dBsm)",       "7"),
    val("Airliner / large aircraft",       "≈ 30 m²  (+15 dBsm)",     "15"),
    val("Ship (small, ∼1 000 t)",          "≈ 300 m²  (+25 dBsm)",    "25"),
    val("Ship (large, >10 000 t)",         "≈ 30 000 m²  (+45 dBsm)", "45"),
};

constexpr Section kRcsSections[] = {
    {"Simple Shapes",   kRcsShapes,  std::size(kRcsShapes),  "rcs-regions"},
    {"Typical Targets", kRcsTargets, std::size(kRcsTargets)},
};

// ── Frequency Bands page (#75, split out — same call as dB & Units) ─────────
// IEEE Std 521 radar-band letters; NATO/EU EW band letters; ITU radio bands.

constexpr Row kIeeeBands[] = {
    val("HF",  "3–30 MHz"),
    val("VHF", "30–300 MHz"),
    val("UHF", "300 MHz–1 GHz"),
    val("L",   "1–2 GHz"),
    val("S",   "2–4 GHz"),
    val("C",   "4–8 GHz"),
    val("X",   "8–12 GHz"),
    val("Ku",  "12–18 GHz"),
    val("K",   "18–27 GHz"),
    val("Ka",  "27–40 GHz"),
    val("V",   "40–75 GHz"),
    val("W",   "75–110 GHz"),
    val("mm (G)", "110–300 GHz"),
};

constexpr Row kNatoBands[] = {
    val("A", "< 250 MHz"),
    val("B", "250–500 MHz"),
    val("C", "500 MHz–1 GHz"),
    val("D", "1–2 GHz"),
    val("E", "2–3 GHz"),
    val("F", "3–4 GHz"),
    val("G", "4–6 GHz"),
    val("H", "6–8 GHz"),
    val("I", "8–10 GHz"),
    val("J", "10–20 GHz"),
    val("K", "20–40 GHz"),
    val("L", "40–60 GHz"),
    val("M", "60–100 GHz"),
};

constexpr Row kItuBands[] = {
    val("VLF (band 4)",  "3–30 kHz"),
    val("LF (band 5)",   "30–300 kHz"),
    val("MF (band 6)",   "300 kHz–3 MHz"),
    val("HF (band 7)",   "3–30 MHz"),
    val("VHF (band 8)",  "30–300 MHz"),
    val("UHF (band 9)",  "300 MHz–3 GHz"),
    val("SHF (band 10)", "3–30 GHz"),
    val("EHF (band 11)", "30–300 GHz"),
};

constexpr Section kBandSections[] = {
    {"IEEE Radar Bands (IEEE Std 521)", kIeeeBands, std::size(kIeeeBands)},
    {"NATO / EU EW Bands",              kNatoBands, std::size(kNatoBands)},
    {"ITU Radio Bands",                 kItuBands,  std::size(kItuBands)},
};

constexpr Page kPages[] = {
    {"ref-glossary", "Glossary", "Definitions of common EW terms",
     kGlossarySections, std::size(kGlossarySections)},
    {"ref-db-units", "dB & Units", "Decibel arithmetic and absolute dB units",
     kDbSections, std::size(kDbSections)},
    {"ref-bands", "Frequency Bands", "IEEE, NATO/EU, and ITU band letters",
     kBandSections, std::size(kBandSections)},
    {"ref-propagation", "Propagation", "Path-loss, diffraction, and earth geometry",
     kPropagationSections, std::size(kPropagationSections)},
    {"ref-antennas", "Antenna Types", "Per-type patterns, gain, and specs",
     kAntennaSections, std::size(kAntennaSections)},
    {"ref-link", "Link Budget", "One-way budget, margin, and components",
     kLinkSections, std::size(kLinkSections)},
    {"ref-receiver", "Receiver", "Sensitivity, cascade, and dynamic range",
     kReceiverSections, std::size(kReceiverSections)},
    {"ref-jamming", "Jamming", "J/S, burnthrough, and partial-band forms",
     kJammingSections, std::size(kJammingSections)},
    {"ref-location", "Location", "CEP from AOA, TDOA, and the error ellipse",
     kLocationSections, std::size(kLocationSections)},
    {"ref-rcs", "RCS", "Simple-shape formulas and typical targets",
     kRcsSections, std::size(kRcsSections)},
    {"ref-radar-det", "Radar & Detection", "Range equation and detection statistics",
     kRadarDetSections, std::size(kRadarDetSections)},
    {"ref-doppler", "Doppler & Resolution", "Doppler relations and the resolution cell",
     kDopplerSections, std::size(kDopplerSections)},
    {"ref-digital", "Digital / DSSS", "Eb/N₀ relations and spread-spectrum forms",
     kDigitalSections, std::size(kDigitalSections)},
};

} // namespace

std::span<const Page> pages() noexcept {
    return {kPages, std::size(kPages)};
}

} // namespace ewpresenter::refdata
