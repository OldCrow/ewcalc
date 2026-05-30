#include "test_main.h"
#include "libew/propagation/propagation.h"
#include <cmath>

using namespace libew::propagation;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// FSPL: L = 32.44 + 20*log10(d_km) + 20*log10(f_MHz)
// Source: Adamy EW101; also derived from Friis transmission equation.
// ---------------------------------------------------------------------------

void test_fspl_known_values() {
    // At 1 km, 1 MHz: L = 32.44 + 0 + 0 = 32.44 dB  (formula baseline)
    ASSERT_NEAR(free_space_path_loss(1.0_km, 1.0_MHz).value, 32.44, 0.01);

    // At 10 km, 100 MHz: L = 32.44 + 20 + 40 = 92.44 dB
    ASSERT_NEAR(free_space_path_loss(10.0_km, 100.0_MHz).value, 92.44, 0.01);

    // At 100 km, 1 GHz = 1000 MHz: L = 32.44 + 40 + 60 = 132.44 dB
    ASSERT_NEAR(free_space_path_loss(100.0_km, 1000.0_MHz).value, 132.44, 0.01);
}

void test_fspl_double_range_adds_6dB() {
    // 20*log10(2d) - 20*log10(d) = 20*log10(2) = 6.021 dB
    const Db l1 = free_space_path_loss(10.0_km, 100.0_MHz);
    const Db l2 = free_space_path_loss(20.0_km, 100.0_MHz);
    ASSERT_NEAR(l2.value - l1.value, 20.0 * std::log10(2.0), 0.001);
}

void test_fspl_double_frequency_adds_6dB() {
    // Same range, double frequency → +6 dB
    const Db l1 = free_space_path_loss(50.0_km, 100.0_MHz);
    const Db l2 = free_space_path_loss(50.0_km, 200.0_MHz);
    ASSERT_NEAR(l2.value - l1.value, 20.0 * std::log10(2.0), 0.001);
}

// ---------------------------------------------------------------------------
// 2-Ray: L = 120 + 40*log10(d_km) - 20*log10(h_tx) - 20*log10(h_rx)
// Source: Adamy EW101; derived from two-ray ground-reflection geometry.
// ---------------------------------------------------------------------------

void test_two_ray_known_values() {
    // At 1 km, h_tx=10 m, h_rx=10 m:
    // L = 120 + 0 - 20 - 20 = 80 dB
    ASSERT_NEAR(two_ray_path_loss(1.0_km, 10.0_m, 10.0_m).value, 80.0, 0.01);

    // At 10 km, h_tx=10 m, h_rx=10 m:
    // L = 120 + 40 - 20 - 20 = 120 dB
    ASSERT_NEAR(two_ray_path_loss(10.0_km, 10.0_m, 10.0_m).value, 120.0, 0.01);
}

void test_two_ray_double_range_adds_12dB() {
    // 40*log10(2d) - 40*log10(d) = 40*log10(2) = 12.041 dB
    const Db l1 = two_ray_path_loss(10.0_km, 10.0_m, 10.0_m);
    const Db l2 = two_ray_path_loss(20.0_km, 10.0_m, 10.0_m);
    ASSERT_NEAR(l2.value - l1.value, 40.0 * std::log10(2.0), 0.001);
}

void test_two_ray_double_height_subtracts_6dB() {
    // Doubling either antenna height reduces loss by 6 dB (from -20*log10)
    const Db l1 = two_ray_path_loss(10.0_km, 10.0_m, 10.0_m);
    const Db l2 = two_ray_path_loss(10.0_km, 20.0_m, 10.0_m);
    ASSERT_NEAR(l1.value - l2.value, 20.0 * std::log10(2.0), 0.001);
}

// ---------------------------------------------------------------------------
// Fresnel zone crossover: d_FZ = h_tx * h_rx * f_MHz / 24000
// Source: Adamy EW101.
// ---------------------------------------------------------------------------

void test_fresnel_zone_formula() {
    // At h_tx=10 m, h_rx=10 m, f=100 MHz:
    // d_FZ = 10 * 10 * 100 / 24000 = 10000/24000 = 0.41667 km
    ASSERT_NEAR(fresnel_zone_distance(10.0_m, 10.0_m, 100.0_MHz).value, 10.0*10.0*100.0/24000.0, 1e-6);

    // At h_tx=30 m, h_rx=10 m, f=300 MHz:
    // d_FZ = 30 * 10 * 300 / 24000 = 90000/24000 = 3.75 km
    ASSERT_NEAR(fresnel_zone_distance(30.0_m, 10.0_m, 300.0_MHz).value, 3.75, 0.001);
}

void test_regime_selection() {
    // h_tx=10m, h_rx=10m, f=100 MHz → FZ = 0.4167 km
    // At 1 km (> FZ): 2-ray regime
    ASSERT_TRUE(is_two_ray_regime(1.0_km, 10.0_m, 10.0_m, 100.0_MHz));
    // At 0.1 km (< FZ): LOS regime
    ASSERT_FALSE(is_two_ray_regime(0.1_km, 10.0_m, 10.0_m, 100.0_MHz));
}

void test_path_loss_selects_correct_model() {
    // Below Fresnel zone: path_loss == FSPL
    const Km short_range{0.1};
    const Mhz freq{100.0};
    const Meters h{10.0};
    ASSERT_NEAR(path_loss(short_range, h, h, freq).value,
                free_space_path_loss(short_range, freq).value, 1e-9);

    // Above Fresnel zone: path_loss == two_ray
    const Km long_range{5.0};
    ASSERT_NEAR(path_loss(long_range, h, h, freq).value,
                two_ray_path_loss(long_range, h, h).value, 1e-9);
}

// ---------------------------------------------------------------------------
// Knife-edge diffraction: Lee approximation to Fresnel integral.
// Source: Adamy EW101; Lee (1982) approximation to J(v).
//
// The Fresnel parameter: v = h * sqrt(2*(d1+d2)/(λ*d1*d2))
// where h > 0 = edge above LOS (obstruction), h < 0 = edge below LOS (clearance).
//
// Reference values derived analytically from the Lee formula:
//   For v = 0:  Gd = 20*log10(0.5*exp(0)) = -6.02 dB → loss = 6.02 dB
//               (knife edge exactly on line of sight = 6 dB diffraction loss)
// ---------------------------------------------------------------------------

void test_knife_edge_on_los() {
    // When the knife edge lies exactly on the line of sight, loss ≈ 6 dB.
    // At v = 0: Gd = 20*log10(0.5 * exp(0)) = 20*log10(0.5) = -6.021 dB, loss = 6.021 dB
    // Geometry: choose d1, d2, f such that los_clearance = 0 gives v = 0.
    // los_clearance = 0 m → h = 0 → v = 0 regardless of d1, d2, λ
    const Db loss = knife_edge_diffraction_loss(10.0_km, 10.0_km, Meters{0.0}, 300.0_MHz);
    ASSERT_NEAR(loss.value, 6.02, 0.05);
}

void test_knife_edge_clear_los() {
    // Edge well below LOS: v << -1, should give near-zero loss.
    // Use large positive clearance (LOS well above edge).
    const Db loss = knife_edge_diffraction_loss(10.0_km, 10.0_km, 1000.0_m, 100.0_MHz);
    ASSERT_NEAR(loss.value, 0.0, 0.1);
}

void test_knife_edge_negative_v_approx() {
    // For v in (-1, 0): Gd = 20*log10(0.5 - 0.62*v)
    // Choose geometry giving v ≈ -0.5:
    // Use LOS height = +5 m (edge below LOS), short paths at high frequency
    // so that |v| is controlled. Use clearance = +5 m to get negative v.
    // Verify loss is less than 6 dB (clearance → partial gain).
    const Db loss = knife_edge_diffraction_loss(5.0_km, 5.0_km, 5.0_m, 1000.0_MHz);
    ASSERT_TRUE(loss.value < 6.02);   // clearance above LOS → less than 6 dB loss
    ASSERT_TRUE(loss.value >= 0.0);   // loss is non-negative
}

int main() {
    std::cout << "=== test_propagation ===\n";
    RUN_TEST(test_fspl_known_values);
    RUN_TEST(test_fspl_double_range_adds_6dB);
    RUN_TEST(test_fspl_double_frequency_adds_6dB);
    RUN_TEST(test_two_ray_known_values);
    RUN_TEST(test_two_ray_double_range_adds_12dB);
    RUN_TEST(test_two_ray_double_height_subtracts_6dB);
    RUN_TEST(test_fresnel_zone_formula);
    RUN_TEST(test_regime_selection);
    RUN_TEST(test_path_loss_selects_correct_model);
    RUN_TEST(test_knife_edge_on_los);
    RUN_TEST(test_knife_edge_clear_los);
    RUN_TEST(test_knife_edge_negative_v_approx);
    return test::summary();
}
