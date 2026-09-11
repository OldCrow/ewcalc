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
    val("3-dB beamwidth",  "30–60°"),
    val("Bandwidth",       "≈ 5 %"),
    val("Frequency range", "HF – UHF"),
};

constexpr Row kLogPeriodic[] = {
    val("Polarization",    "linear"),
    val("Typical gain",    "6–8 dBi",           "7"),
    val("3-dB beamwidth",  "60–80°"),
    val("Bandwidth",       "10:1 (multi-decade)"),
    val("Frequency range", "HF – SHF"),
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
    {"quick-values", "Quick Values", "Common EW values for quick entry",
     kQuickValueSections, std::size(kQuickValueSections)},
    {"ref-propagation", "Propagation", "Path-loss, diffraction, and earth geometry",
     kPropagationSections, std::size(kPropagationSections)},
    {"ref-antennas", "Antenna Types", "Per-type patterns, gain, and specs",
     kAntennaSections, std::size(kAntennaSections)},
    {"ref-bands", "Frequency Bands", "IEEE, NATO/EU, and ITU band letters",
     kBandSections, std::size(kBandSections)},
    {"ref-db-units", "dB & Units", "Decibel arithmetic and absolute dB units",
     kDbSections, std::size(kDbSections)},
};

} // namespace

std::span<const Page> pages() noexcept {
    return {kPages, std::size(kPages)};
}

} // namespace ewpresenter::refdata
