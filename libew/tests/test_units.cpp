#include "test_main.h"
#include "libew/core/units.h"
#include <cmath>

using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// Arithmetic algebra
// ---------------------------------------------------------------------------
void test_dbm_plus_db() {
    const Dbm result = 20.0_dBm + 3.0_dB;
    ASSERT_NEAR(result.value, 23.0, 1e-9);
}

void test_db_plus_dbm() {
    const Dbm result = 3.0_dB + 20.0_dBm;
    ASSERT_NEAR(result.value, 23.0, 1e-9);
}

void test_dbm_minus_db() {
    const Dbm result = 20.0_dBm - 3.0_dB;
    ASSERT_NEAR(result.value, 17.0, 1e-9);
}

void test_dbm_minus_dbm_yields_db() {
    const Db result = 20.0_dBm - 14.0_dBm;
    ASSERT_NEAR(result.value, 6.0, 1e-9);
}

void test_db_addition() {
    const Db result = 10.0_dB + 5.0_dB;
    ASSERT_NEAR(result.value, 15.0, 1e-9);
}

void test_db_negation() {
    const Db result = -(3.0_dB);
    ASSERT_NEAR(result.value, -3.0, 1e-9);
}

// ---------------------------------------------------------------------------
// Unit conversions — round-trip and known values
// ---------------------------------------------------------------------------
void test_dbm_dbw_roundtrip() {
    const Dbm original = 30.0_dBm;
    const Dbm roundtrip = dbw_to_dbm(dbm_to_dbw(original));
    ASSERT_NEAR(roundtrip.value, original.value, 1e-9);
}

void test_30dbm_equals_1w() {
    // 30 dBm = 1 W
    const Watts w = dbm_to_watts(30.0_dBm);
    ASSERT_NEAR(w.value, 1.0, 1e-6);
}

void test_0dbm_equals_1mw() {
    // 0 dBm = 1 mW = 0.001 W
    const Watts w = dbm_to_watts(0.0_dBm);
    ASSERT_NEAR(w.value, 0.001, 1e-9);
}

void test_watts_to_dbm() {
    // 1 W = 30 dBm
    const Dbm p = watts_to_dbm(1.0_W);
    ASSERT_NEAR(p.value, 30.0, 1e-6);
}

void test_db_to_linear_0db() {
    ASSERT_NEAR(db_to_linear(0.0_dB), 1.0, 1e-9);
}

void test_db_to_linear_10db() {
    ASSERT_NEAR(db_to_linear(10.0_dB), 10.0, 1e-9);
}

void test_db_to_linear_3db() {
    // 3 dB ≈ 2.0 (actually 1.9953...)
    ASSERT_NEAR(db_to_linear(3.0_dB), std::pow(10.0, 0.3), 1e-9);
}

void test_linear_to_db_roundtrip() {
    const double ratio = 7.5;
    const Db db = linear_to_db(ratio);
    ASSERT_NEAR(db_to_linear(db), ratio, 1e-9);
}

void test_mhz_ghz_conversion() {
    const Mhz f = ghz_to_mhz(1.0_GHz);
    ASSERT_NEAR(f.value, 1000.0, 1e-9);
}

void test_km_m_conversion() {
    const Meters m = km_to_m(1.0_km);
    ASSERT_NEAR(m.value, 1000.0, 1e-9);
    const Km k = m_to_km(1000.0_m);
    ASSERT_NEAR(k.value, 1.0, 1e-9);
}

void test_dbm_dbw_offset() {
    // 0 dBW = 30 dBm
    const Dbm result = dbw_to_dbm(0.0_dBw);
    ASSERT_NEAR(result.value, 30.0, 1e-9);
    // 30 dBm = 0 dBW
    const Dbw result2 = dbm_to_dbw(30.0_dBm);
    ASSERT_NEAR(result2.value, 0.0, 1e-9);
}

void test_ordering() {
    ASSERT_TRUE(10.0_dBm > 5.0_dBm);
    ASSERT_TRUE(5.0_dBm < 10.0_dBm);
    ASSERT_TRUE(10.0_dBm == 10.0_dBm);
}

void test_dbsm_minus_dbsm_yields_db() {
    // RCS ratio: 10 dBsm - 4 dBsm = 6 dB
    const Db result = 10.0_dBsm - 4.0_dBsm;
    ASSERT_NEAR(result.value, 6.0, 1e-9);
}

void test_dbsm_minus_dbsm_self_is_zero() {
    const Db result = 5.0_dBsm - 5.0_dBsm;
    ASSERT_NEAR(result.value, 0.0, 1e-9);
}

int main() {
    std::cout << "=== test_units ===\n";
    RUN_TEST(test_dbm_plus_db);
    RUN_TEST(test_db_plus_dbm);
    RUN_TEST(test_dbm_minus_db);
    RUN_TEST(test_dbm_minus_dbm_yields_db);
    RUN_TEST(test_db_addition);
    RUN_TEST(test_db_negation);
    RUN_TEST(test_dbm_dbw_roundtrip);
    RUN_TEST(test_30dbm_equals_1w);
    RUN_TEST(test_0dbm_equals_1mw);
    RUN_TEST(test_watts_to_dbm);
    RUN_TEST(test_db_to_linear_0db);
    RUN_TEST(test_db_to_linear_10db);
    RUN_TEST(test_db_to_linear_3db);
    RUN_TEST(test_linear_to_db_roundtrip);
    RUN_TEST(test_mhz_ghz_conversion);
    RUN_TEST(test_km_m_conversion);
    RUN_TEST(test_dbm_dbw_offset);
    RUN_TEST(test_ordering);
    RUN_TEST(test_dbsm_minus_dbsm_yields_db);
    RUN_TEST(test_dbsm_minus_dbsm_self_is_zero);
    return test::summary();
}
