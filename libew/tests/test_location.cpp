#include "test_main.h"
#include "libew/location/location.h"
#include <cmath>
#include <numbers>

using namespace libew::location;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// CEP from AOA bearing error (ideal geometry: two receivers 90° apart,
// equidistant from emitter).
//
// Formula: CEP = 1.2 * R * tan(σ_θ)
// Source: Adamy EW101, standard DF geometry approximation.
//
// The 1.2 coefficient comes from combining the 2D Gaussian CEP factor
// sqrt(2*ln(2)) ≈ 1.177 with a modest geometry correction; Adamy rounds to 1.2.
// ---------------------------------------------------------------------------

void test_cep_aoa_formula_derivation() {
    // For R=100 km, σ=1°:
    // CEP = 1.2 * 100 * tan(1°) = 1.2 * 100 * 0.017455 = 2.095 km
    const double expected = 1.2 * 100.0 * std::tan(1.0 * std::numbers::pi / 180.0);
    ASSERT_NEAR(cep_from_rms_bearing_error(1.0_deg, 100.0_km).value, expected, 0.001);
}

void test_cep_aoa_proportional_to_range() {
    // CEP scales linearly with range (tan(σ) factor is fixed)
    const Km cep50  = cep_from_rms_bearing_error(1.0_deg, 50.0_km);
    const Km cep100 = cep_from_rms_bearing_error(1.0_deg, 100.0_km);
    ASSERT_NEAR(cep100.value / cep50.value, 2.0, 1e-9);
}

void test_cep_aoa_proportional_to_tan_error() {
    // Doubling the bearing error approximately doubles the CEP (for small angles)
    const Km cep1 = cep_from_rms_bearing_error(1.0_deg, 100.0_km);
    const Km cep2 = cep_from_rms_bearing_error(2.0_deg, 100.0_km);
    // tan(2°)/tan(1°) ≈ 2 for small angles
    const double ratio = std::tan(2.0 * std::numbers::pi/180.0)
                       / std::tan(1.0 * std::numbers::pi/180.0);
    ASSERT_NEAR(cep2.value / cep1.value, ratio, 0.001);
}

void test_cep_aoa_zero_error() {
    // Zero bearing error → zero CEP
    ASSERT_NEAR(cep_from_rms_bearing_error(0.0_deg, 100.0_km).value, 0.0, 1e-9);
}

// ---------------------------------------------------------------------------
// CEP from EEP (Elliptical Error Probable).
//
// Formula: CEP ≈ 0.59 * (a + b)   where a ≥ b are the 1-sigma semi-axes.
// Source: Adamy EW101; approximation accurate to ~1% for a/b ≤ 4.
//
// For a = b (circular): CEP = 0.59 * 2σ = 1.18σ, consistent with
// the theoretical CEP = σ * sqrt(2*ln(2)) = 1.177σ for a 2D isotropic Gaussian.
// ---------------------------------------------------------------------------

void test_cep_eep_formula_derivation() {
    // a=2 km, b=1 km: CEP = 0.59 * 3 = 1.77 km
    ASSERT_NEAR(cep_from_eep(2.0_km, 1.0_km).value, 1.77, 0.001);
}

void test_cep_eep_circular() {
    // For a = b = σ: CEP = 0.59 * 2σ = 1.18σ
    // Theoretical value for 2D isotropic Gaussian: CEP = 1.1774 * σ
    // The 0.59*(a+b) approximation gives 1.18σ ≈ 1.177σ (error < 0.3%)
    ASSERT_NEAR(cep_from_eep(1.0_km, 1.0_km).value, 1.18, 0.001);
}

void test_cep_eep_scales_with_axes() {
    // Doubling both axes doubles the CEP
    const Km cep1 = cep_from_eep(2.0_km, 1.0_km);
    const Km cep2 = cep_from_eep(4.0_km, 2.0_km);
    ASSERT_NEAR(cep2.value / cep1.value, 2.0, 1e-9);
}

int main() {
    std::cout << "=== test_location ===\n";
    RUN_TEST(test_cep_aoa_formula_derivation);
    RUN_TEST(test_cep_aoa_proportional_to_range);
    RUN_TEST(test_cep_aoa_proportional_to_tan_error);
    RUN_TEST(test_cep_aoa_zero_error);
    RUN_TEST(test_cep_eep_formula_derivation);
    RUN_TEST(test_cep_eep_circular);
    RUN_TEST(test_cep_eep_scales_with_axes);
    return test::summary();
}
