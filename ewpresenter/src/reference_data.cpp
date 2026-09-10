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

// Proof row for the v1.2.0 formula-rendering path (typeset SVG + Unicode
// alt text). Constant 32.44 matches libew::constants::fspl_constant_km_mhz.
constexpr Row kKeyFormulas[] = {
    formula("Free-space path loss", "fspl",
            "FSPL = (4πd/λ)²",
            "L = 32.44 + 20 log₁₀ f(MHz) + 20 log₁₀ d(km)  dB"),
};

constexpr Section kQuickValueSections[] = {
    {"Antenna Gain",                        kAntennaGain, std::size(kAntennaGain)},
    {"Antenna Sidelobe Levels (re main lobe)", kSidelobes, std::size(kSidelobes)},
    {"Thermal Noise Floor  (kT, 290 K)",    kNoiseFloor,  std::size(kNoiseFloor)},
    {"Radar Cross Section (Typical)",       kRcs,         std::size(kRcs)},
    {"Eb/N₀ Requirements (AWGN)",           kEbNo,        std::size(kEbNo)},
    {"Key Formulas",                        kKeyFormulas, std::size(kKeyFormulas)},
};

constexpr Page kPages[] = {
    {"quick-values", "Reference", "Common EW values for quick entry",
     kQuickValueSections, std::size(kQuickValueSections)},
};

} // namespace

std::span<const Page> pages() noexcept {
    return {kPages, std::size(kPages)};
}

} // namespace ewpresenter::refdata
