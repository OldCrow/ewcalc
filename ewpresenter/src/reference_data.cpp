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

// ── Quick-values page ────────────────────────────────────────────────────────
// Content migrated verbatim from the former per-frontend Reference pages
// (ReferenceView.swift / ReferencePage.cpp / ReferencePage.xaml).

constexpr Row kAntennaGain[] = {
    val("Isotropic (reference)",                 "0.0 dBi",            "0.0"),
    val("Short whip / rubber duck",              "0 dBi  (typical)",   "0"),
    val("Quarter-wave whip, ground-plane mount", "2 dBi  (typical)",   "2"),
    val("Half-wave dipole",                      "2.15 dBi  (0 dBd)",  "2.15"),
    val("2-element Yagi",                        "≈ 7 dBi",            "7"),
    val("3-element Yagi",                        "≈ 8.5 dBi",          "8.5"),
    val("5-element Yagi",                        "≈ 10.5 dBi",         "10.5"),
    val("10-element Yagi",                       "≈ 14 dBi",           "14"),
};

constexpr Row kSidelobes[] = {
    val("Uniform aperture — 1st SLL",   "−13 dBc",         "-13"),
    val("Taylor weighted — 1st SLL",    "−25 dBc",         "-25"),
    val("Low-sidelobe array — 1st SLL", "−35 dBc",         "-35"),
    val("Typical back lobe",            "−25 to −35 dBc"),
};

constexpr Row kNoiseFloor[] = {
    val("1 Hz bandwidth",    "−174.0 dBm", "-174.0"),
    val("1 kHz bandwidth",   "−144.0 dBm", "-144.0"),
    val("1 MHz bandwidth",   "−114.0 dBm", "-114.0"),
    val("10 MHz bandwidth",  "−104.0 dBm", "-104.0"),
    val("100 MHz bandwidth", "−94.0 dBm",  "-94.0"),
    val("1 GHz bandwidth",   "−84.0 dBm",  "-84.0"),
};

constexpr Row kRcs[] = {
    val("Large aircraft (broadside)",      "+15 dBsm  (rep.)",   "15"),
    val("Fighter (broadside)",             "+7 dBsm  (rep.)",    "7"),
    val("Fighter (nose-on, conventional)", "0 dBsm  (≈ 1 m²)",   "0"),
    val("LO fighter (nose-on)",            "−15 dBsm  (rep.)",   "-15"),
    val("Cruise missile",                  "−7 dBsm  (rep.)",    "-7"),
    val("Bird",                            "−15 dBsm  (rep.)",   "-15"),
    val("Ship (small, ∼1 000 t)",          "+25 dBsm  (rep.)",   "25"),
    val("Ship (large, >10 000 t)",         "+45 dBsm  (rep.)",   "45"),
};

constexpr Row kEbNo[] = {
    val("BPSK / QPSK,  BER 10⁻³", "6.8 dB",     "6.8"),
    val("BPSK / QPSK,  BER 10⁻⁵", "9.6 dB",     "9.6"),
    val("BPSK / QPSK,  BER 10⁻⁶", "10.5 dB",    "10.5"),
    val("Non-coh. FSK, BER 10⁻³", "≈ 13.5 dB",  "13.5"),
    val("Non-coh. FSK, BER 10⁻⁵", "≈ 17.0 dB",  "17.0"),
};

constexpr Section kQuickValueSections[] = {
    {"Antenna Gain",                        kAntennaGain, std::size(kAntennaGain)},
    {"Antenna Sidelobe Levels (re main lobe)", kSidelobes, std::size(kSidelobes)},
    {"Thermal Noise Floor  (kT, 290 K)",    kNoiseFloor,  std::size(kNoiseFloor)},
    {"Radar Cross Section (Typical)",       kRcs,         std::size(kRcs)},
    {"Eb/N₀ Requirements (AWGN)",           kEbNo,        std::size(kEbNo)},
};

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

constexpr Page kPages[] = {
    {"quick-values", "Quick Values", "Common EW values for quick entry",
     kQuickValueSections, std::size(kQuickValueSections)},
    {"ref-propagation", "Propagation", "Path-loss, diffraction, and earth geometry",
     kPropagationSections, std::size(kPropagationSections)},
    {"ref-db-units", "dB & Units", "Decibel arithmetic and absolute dB units",
     kDbSections, std::size(kDbSections)},
};

} // namespace

std::span<const Page> pages() noexcept {
    return {kPages, std::size(kPages)};
}

} // namespace ewpresenter::refdata
