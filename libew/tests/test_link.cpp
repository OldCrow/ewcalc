#include "test_main.h"
#include "libew/link/link.h"
#include "libew/propagation/propagation.h"
#include <cmath>

using namespace libew::link;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// one_way_link — received power
//
// LOS regime formula: Prx = Ptx + Gtx + Grx - FSPL
// For a link at short range (below Fresnel crossover), path loss == FSPL.
// Reference: Adamy EW101 one-way link budget equation.
// ---------------------------------------------------------------------------

void test_one_way_link_los_power() {
    // Tx=20 dBm, Gtx=0 dB, Grx=0 dB, 1 km, f=100 MHz, h=10m/10m
    // FZ = 10*10*100/24000 = 0.417 km → 1 km is 2-ray regime
    // Use 0.1 km (below FZ) to stay in LOS:
    // FSPL(0.1 km, 100 MHz) = 32.44 + 20*log10(0.1) + 20*log10(100)
    //                       = 32.44 - 20 + 40 = 52.44 dB
    // Prx = 20 + 0 + 0 - 52.44 = -32.44 dBm
    const auto r = one_way_link(
        20.0_dBm, 0.0_dB, 0.0_dB,
        0.1_km, 10.0_m, 10.0_m, 100.0_MHz);
    ASSERT_NEAR(r.received_power.value, -32.44, 0.05);
    ASSERT_FALSE(r.two_ray_regime);
}

void test_one_way_link_gain_adds_linearly() {
    // Adding 3 dB to Gtx raises Prx by exactly 3 dB
    const auto r1 = one_way_link(20.0_dBm, 0.0_dB, 0.0_dB,
                                  0.1_km, 10.0_m, 10.0_m, 100.0_MHz);
    const auto r2 = one_way_link(20.0_dBm, 3.0_dB, 0.0_dB,
                                  0.1_km, 10.0_m, 10.0_m, 100.0_MHz);
    ASSERT_NEAR(r2.received_power.value - r1.received_power.value, 3.0, 1e-9);
}

void test_one_way_link_path_loss_matches_fspl_in_los() {
    // In LOS regime, the reported path_loss must equal the standalone FSPL result.
    const Km d{0.1};
    const Mhz f{100.0};
    const Meters h{10.0};
    const auto r = one_way_link(20.0_dBm, 0.0_dB, 0.0_dB, d, h, h, f);
    const Db fspl = libew::propagation::free_space_path_loss(d, f);
    ASSERT_NEAR(r.path_loss.value, fspl.value, 1e-9);
}

void test_one_way_link_two_ray_regime_flag() {
    // h_tx=10m, h_rx=10m, f=100 MHz → FZ = 0.417 km
    // At 1 km: 2-ray regime; at 0.1 km: LOS regime
    const auto los = one_way_link(0.0_dBm, 0.0_dB, 0.0_dB,
                                   0.1_km, 10.0_m, 10.0_m, 100.0_MHz);
    const auto two_ray = one_way_link(0.0_dBm, 0.0_dB, 0.0_dB,
                                       1.0_km, 10.0_m, 10.0_m, 100.0_MHz);
    ASSERT_FALSE(los.two_ray_regime);
    ASSERT_TRUE(two_ray.two_ray_regime);
}

void test_one_way_link_fresnel_zone_reported() {
    // Fresnel zone distance is returned in the result
    const auto r = one_way_link(0.0_dBm, 0.0_dB, 0.0_dB,
                                 0.5_km, 10.0_m, 10.0_m, 100.0_MHz);
    // FZ = 10 * 10 * 100 / 24000 = 0.4167 km
    ASSERT_NEAR(r.fresnel_zone_distance.value, 10.0*10.0*100.0/24000.0, 1e-6);
}

// ---------------------------------------------------------------------------
// effective_range
//
// Solves for the range at which received power equals receiver sensitivity.
// Source: Adamy EW101 effective range derivation.
// ---------------------------------------------------------------------------

void test_effective_range_sanity() {
    // A link with reasonable parameters should give a finite positive range.
    const auto r = effective_range(
        20.0_dBm, 0.0_dB, 0.0_dB,
        10.0_m, 10.0_m, 100.0_MHz, Dbm{-100.0});
    ASSERT_TRUE(r.range.value > 0.0);
    ASSERT_TRUE(r.range.value < 10000.0);
}

void test_effective_range_los_inversion() {
    // LOS regime requires: effective_range < Fresnel zone crossover distance.
    // Use tall antennas (large FZ) and low Tx power (short range).
    //
    // h_tx=h_rx=100 m, f=100 MHz → FZ = 100*100*100/24000 = 41.7 km
    // Ptx=20 dBm, sens=-50 dBm → margin = 70 dB
    // LOS range = 10^((70 - 32.44 - 20*log10(100)) / 20) = 10^((70-32.44-40)/20)
    //           = 10^(-2.44/20) = 10^-0.122 = 0.755 km  (< 41.7 km FZ → LOS) ✓
    const auto r = effective_range(
        20.0_dBm, 0.0_dB, 0.0_dB,
        100.0_m, 100.0_m, 100.0_MHz, Dbm{-50.0});
    ASSERT_FALSE(r.two_ray_regime);
    // Verify: received power at the returned range must equal the sensitivity
    const auto check = one_way_link(
        20.0_dBm, 0.0_dB, 0.0_dB,
        r.range, 100.0_m, 100.0_m, 100.0_MHz);
    ASSERT_NEAR(check.received_power.value, -50.0, 0.5);
}

void test_effective_range_two_ray_regime() {
    // With tall antennas and low frequency, FZ is large and can be below
    // effective range, pushing the result into the 2-ray regime.
    // h_tx=100m, h_rx=100m, f=3000 MHz → FZ = 100*100*3000/24000 = 1250 km
    // That's too large; use f=100 MHz, h=1000m:
    // FZ = 1000*1000*100/24000 = 4166 km — still large
    // Use h=100m, f=100MHz → FZ = 100*100*100/24000 = 41.7 km
    // Ptx=40dBm (10W), Grx=0, sensitivity=-80dBm → margin=120dB
    // LOS range >> 41.7 km → 2-ray regime result
    const auto r = effective_range(
        40.0_dBm, 0.0_dB, 0.0_dB,
        100.0_m, 100.0_m, 100.0_MHz, Dbm{-80.0});
    ASSERT_TRUE(r.two_ray_regime);
    ASSERT_TRUE(r.range.value > 0.0);
}

void test_effective_range_zero_frequency_returns_nan() {
    // Zero frequency must return NaN, not +inf or crash.
    const auto r = effective_range(
        20.0_dBm, 0.0_dB, 0.0_dB,
        10.0_m, 10.0_m, Mhz{0.0}, Dbm{-100.0});
    ASSERT_TRUE(std::isnan(r.range.value));
}

void test_effective_range_zero_height_returns_nan() {
    // Zero antenna height must return NaN, not trigger UB or +inf.
    const auto r = effective_range(
        20.0_dBm, 0.0_dB, 0.0_dB,
        Meters{0.0}, 10.0_m, 100.0_MHz, Dbm{-100.0});
    ASSERT_TRUE(std::isnan(r.range.value));
}

int main() {
    std::cout << "=== test_link ===\n";
    RUN_TEST(test_one_way_link_los_power);
    RUN_TEST(test_one_way_link_gain_adds_linearly);
    RUN_TEST(test_one_way_link_path_loss_matches_fspl_in_los);
    RUN_TEST(test_one_way_link_two_ray_regime_flag);
    RUN_TEST(test_one_way_link_fresnel_zone_reported);
    RUN_TEST(test_effective_range_sanity);
    RUN_TEST(test_effective_range_los_inversion);
    RUN_TEST(test_effective_range_two_ray_regime);
    RUN_TEST(test_effective_range_zero_frequency_returns_nan);
    RUN_TEST(test_effective_range_zero_height_returns_nan);
    return test::summary();
}
