#include "test_main.h"
#include "libew/antenna/antenna.h"
#include <cmath>

using namespace libew::antenna;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// ERP — Effective Radiated Power
// Source: Adamy EW101 [OPEN: page/eq TBD]. ERP = Ptx + Gtx (log domain).
// ---------------------------------------------------------------------------

void test_erp_dbm() {
    // 20 dBm + 10 dB = 30 dBm
    const Dbm result = erp(20.0_dBm, 10.0_dB);
    ASSERT_NEAR(result.value, 30.0, 1e-9);
}

void test_erp_dbm_zero_gain() {
    // 0 dB gain: ERP == Tx power
    ASSERT_NEAR(erp(15.0_dBm, 0.0_dB).value, 15.0, 1e-9);
}

void test_erp_dbw() {
    // 0 dBW + 10 dB = 10 dBW
    const Dbw result = erp(0.0_dBw, 10.0_dB);
    ASSERT_NEAR(result.value, 10.0, 1e-9);
}

// ---------------------------------------------------------------------------
// Gain reference conversions: dBi ↔ dBd
// 2.15 dB offset: isotropic antenna is 2.15 dB above half-wave dipole.
// Source: IEEE Std 145 antenna terms; Adamy EW101 [OPEN: page/eq TBD].
// (2.15 dB = 10*log10(1.64), the half-wave dipole's directivity over isotropic.)
// ---------------------------------------------------------------------------

void test_dbi_to_dbd_known_value() {
    // 2.15 dBi = 0 dBd
    ASSERT_NEAR(dbi_to_dbd(2.15_dB).value, 0.0, 1e-9);
    // 10 dBi = 7.85 dBd
    ASSERT_NEAR(dbi_to_dbd(10.0_dB).value, 7.85, 1e-9);
}

void test_dbd_to_dbi_known_value() {
    // 0 dBd = 2.15 dBi
    ASSERT_NEAR(dbd_to_dbi(0.0_dB).value, 2.15, 1e-9);
}

void test_dbi_dbd_roundtrip() {
    const Db original{15.7};
    const Db roundtrip = dbd_to_dbi(dbi_to_dbd(original));
    ASSERT_NEAR(roundtrip.value, original.value, 1e-9);
}

// ---------------------------------------------------------------------------
// Beamwidth from gain
// θ_3dB ≈ 10^((11.1 - G_dBi) / 20)  degrees
// Source: Adamy EW101 [OPEN: page/eq TBD]. Dave Adamy's "EW 101 -- ES vs.
// SIGINT -- Part 2" (JED, Feb 2011, p. 52, Figure 3) plots gain vs. 3 dB
// beamwidth for a 55%-efficient parabolic dish, confirming the qualitative
// gain-beamwidth trade-off, but is a graph (not a closed-form equation), so
// it cannot confirm this specific "11.1" constant.
// NOTE (audit finding, not fixed here — see discrepancy report): the classic
// Tai & Pereira (1976) directivity approximation is a two-plane formula using
// separate azimuth/elevation half-power beamwidths (D0 ~ K/(theta_az*theta_el),
// the same family as gain_from_beamwidth() below), not a single-beamwidth
// relation. The "11.1" constant here does not reproduce that formula, or any
// other standard single-beamwidth gain relation, when theta is in degrees as
// documented; it is numerically close to the equivalent constant only if
// theta were expressed in radians. Attribution to "Tai & Pereira" for this
// specific single-variable form is therefore unverified.
// ---------------------------------------------------------------------------

void test_beamwidth_from_gain_0dbi() {
    // At 0 dBi: θ = 10^(11.1/20) = 10^0.555 = 3.589 degrees
    const double expected = std::pow(10.0, 11.1 / 20.0);
    ASSERT_NEAR(beamwidth_from_gain(0.0_dB).value, expected, 0.001);
}

void test_beamwidth_from_gain_increases_with_lower_gain() {
    // Lower gain → wider beam
    ASSERT_TRUE(beamwidth_from_gain(0.0_dB).value > beamwidth_from_gain(10.0_dB).value);
    ASSERT_TRUE(beamwidth_from_gain(10.0_dB).value > beamwidth_from_gain(30.0_dB).value);
}

// ---------------------------------------------------------------------------
// Gain from beamwidth
// G ≈ 10*log10(30000 / (θ_az * θ_el))
// Source: Adamy EW101 [OPEN: page/eq TBD]; qualitatively consistent with
// Dave Adamy's "EW 101 -- ES vs. SIGINT -- Part 2" (JED, Feb 2011, p. 52,
// Figure 3: gain vs. 3 dB beamwidth for a 55%-efficient parabolic dish),
// though that source is a graph rather than a closed-form equation. Same
// family as the Kraus and Tai & Pereira two-plane beamwidth-directivity
// approximations (D0 ~ K/(theta_az*theta_el)); the constant (commonly
// 26000-41253 across sources) depends on assumed beam shape/aperture
// efficiency.
// ---------------------------------------------------------------------------

void test_gain_from_beamwidth_10x10() {
    // θ_az = θ_el = 10°: G = 10*log10(30000 / 100) = 10*log10(300) = 24.77 dBi
    ASSERT_NEAR(gain_from_beamwidth(10.0_deg, 10.0_deg).value,
                10.0 * std::log10(300.0), 0.01);
}

void test_gain_from_beamwidth_1x1() {
    // θ_az = θ_el = 1°: G = 10*log10(30000) = 44.77 dBi
    ASSERT_NEAR(gain_from_beamwidth(1.0_deg, 1.0_deg).value,
                10.0 * std::log10(30000.0), 0.01);
}

void test_beamwidth_gain_consistency() {
    // beamwidth_from_gain (Tai & Pereira, 1D) and gain_from_beamwidth (pencil-beam, 2D)
    // are independent approximations from different models and do not round-trip.
    // What IS consistent: a higher-gain antenna has a narrower beamwidth, and
    // applying gain_from_beamwidth to that narrower beamwidth gives a higher gain
    // than applying it to the wider beamwidth of a lower-gain antenna.
    const Degrees bw_low  = beamwidth_from_gain(10.0_dB);
    const Degrees bw_high = beamwidth_from_gain(30.0_dB);
    ASSERT_TRUE(bw_high.value < bw_low.value);  // higher gain → narrower beam
    const Db g_from_low  = gain_from_beamwidth(bw_low,  bw_low);
    const Db g_from_high = gain_from_beamwidth(bw_high, bw_high);
    ASSERT_TRUE(g_from_high.value > g_from_low.value);  // consistent ordering
}

// ---------------------------------------------------------------------------
// Wavelength
// λ = c / f   where c = 2.998e8 m/s
// Source: physics; c = 299792458 m/s exactly (SI definition). No book
// citation applicable.
// ---------------------------------------------------------------------------

void test_wavelength_300mhz() {
    // 300 MHz → λ ≈ 1.0 m
    ASSERT_NEAR(wavelength(300.0_MHz).value, 2.99792458e8 / 300.0e6, 1e-6);
}

void test_wavelength_3ghz() {
    // 3000 MHz → λ ≈ 0.1 m
    ASSERT_NEAR(wavelength(3000.0_MHz).value, 2.99792458e8 / 3000.0e6, 1e-6);
}

void test_wavelength_halves_when_frequency_doubles() {
    const Meters lam1 = wavelength(1000.0_MHz);
    const Meters lam2 = wavelength(2000.0_MHz);
    ASSERT_NEAR(lam1.value / lam2.value, 2.0, 1e-9);
}

int main() {
    std::cout << "=== test_antenna ===\n";
    RUN_TEST(test_erp_dbm);
    RUN_TEST(test_erp_dbm_zero_gain);
    RUN_TEST(test_erp_dbw);
    RUN_TEST(test_dbi_to_dbd_known_value);
    RUN_TEST(test_dbd_to_dbi_known_value);
    RUN_TEST(test_dbi_dbd_roundtrip);
    RUN_TEST(test_beamwidth_from_gain_0dbi);
    RUN_TEST(test_beamwidth_from_gain_increases_with_lower_gain);
    RUN_TEST(test_gain_from_beamwidth_10x10);
    RUN_TEST(test_gain_from_beamwidth_1x1);
    RUN_TEST(test_beamwidth_gain_consistency);
    RUN_TEST(test_wavelength_300mhz);
    RUN_TEST(test_wavelength_3ghz);
    RUN_TEST(test_wavelength_halves_when_frequency_doubles);
    return test::summary();
}
