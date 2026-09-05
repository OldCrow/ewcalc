/// @file test_formatter.cpp
/// @brief Size-bound tests for every formatter function.
///
/// Two invariants must hold for the C bridge to remain safe:
///   1. Every formatter output fits in the 64-byte internal snprintf buffer
///      used inside formatter.cpp (the `fmt()` helper).
///   2. Every formatter output fits in the 80-byte EWP_STR_MAX bridge buffers
///      (i.e., size() < 64 and size() < 80 are equivalent today; both are
///      checked so that a future inner-buffer enlargement doesn't silently
///      violate the bridge contract).
///
/// Inputs are chosen to be the most extreme values accepted by the presenters
/// so that maximum output width is exercised.

#include "test_main.h"
#include "ewpresenter/formatter.h"

using namespace ewpresenter;
using namespace libew::units;

static constexpr std::size_t kInnerBuf  = 64;  ///< fmt() internal buffer in formatter.cpp
static constexpr std::size_t kBridgeBuf = 80;  ///< EWP_STR_MAX in ewcalc_bridge.h

// ---------------------------------------------------------------------------
// Helper macro: assert both size constraints in one call.
// ---------------------------------------------------------------------------
#define ASSERT_SIZE(s) \
    do { \
        ASSERT_TRUE((s).size() < kInnerBuf); \
        ASSERT_TRUE((s).size() < kBridgeBuf); \
    } while(false)

// ---------------------------------------------------------------------------
// Power
// ---------------------------------------------------------------------------
void test_format_dbm() {
    ASSERT_SIZE(format_dbm(Dbm{-200.0}));   // large negative
    ASSERT_SIZE(format_dbm(Dbm{ 200.0}));   // large positive
    ASSERT_SIZE(format_dbm(Dbm{-200.0}, 3));
}

void test_format_dbw() {
    ASSERT_SIZE(format_dbw(Dbw{-200.0}));
    ASSERT_SIZE(format_dbw(Dbw{ 200.0}));
}

void test_format_watts() {
    // Sub-threshold: displayed as mW
    ASSERT_SIZE(format_watts(Watts{0.0001}));
    ASSERT_SIZE(format_watts(Watts{0.999}));
    // Above threshold: displayed as W
    ASSERT_SIZE(format_watts(Watts{1.0}));
    ASSERT_SIZE(format_watts(Watts{100000.0}));
    // Negative sub-threshold
    ASSERT_SIZE(format_watts(Watts{-0.5}));
}

// ---------------------------------------------------------------------------
// Gain / loss
// ---------------------------------------------------------------------------
void test_format_db() {
    ASSERT_SIZE(format_db(Db{-100.0}));
    ASSERT_SIZE(format_db(Db{ 100.0}));
    ASSERT_SIZE(format_db(Db{-100.0}, 3));
}

// Regression test for issue #8: IEEE 754 negative zero must not render as "-0.0".
void test_format_db_negative_zero() {
    ASSERT_TRUE(format_db(Db{-0.0}) == "0.0 dB");
}

void test_format_dbsm() {
    ASSERT_SIZE(format_dbsm(Dbsm{-40.0}));
    ASSERT_SIZE(format_dbsm(Dbsm{ 60.0}));
}

// ---------------------------------------------------------------------------
// Frequency
// ---------------------------------------------------------------------------
void test_format_mhz() {
    ASSERT_SIZE(format_mhz(Mhz{0.1}));
    ASSERT_SIZE(format_mhz(Mhz{100000.0}));
    ASSERT_SIZE(format_mhz(Mhz{100000.0}, 3));
}

void test_format_ghz() {
    ASSERT_SIZE(format_ghz(Ghz{0.0001}));
    ASSERT_SIZE(format_ghz(Ghz{100.0}));
}

// ---------------------------------------------------------------------------
// Distance
// ---------------------------------------------------------------------------
void test_format_km() {
    ASSERT_SIZE(format_km(Km{0.01}));
    ASSERT_SIZE(format_km(Km{10000.0}));
    ASSERT_SIZE(format_km(Km{10000.0}, 3));
}

void test_format_m() {
    ASSERT_SIZE(format_m(Meters{0.1}));
    ASSERT_SIZE(format_m(Meters{100000.0}));
}

// ---------------------------------------------------------------------------
// Angle
// ---------------------------------------------------------------------------
void test_format_degrees() {
    ASSERT_SIZE(format_degrees(Degrees{0.0}));
    ASSERT_SIZE(format_degrees(Degrees{360.0}));
    ASSERT_SIZE(format_degrees(Degrees{360.0}, 4));
}

// ---------------------------------------------------------------------------
// Dimensionless
// ---------------------------------------------------------------------------
void test_format_percent() {
    ASSERT_SIZE(format_percent(0.0));
    ASSERT_SIZE(format_percent(1.0));        // 100.000 %
    ASSERT_SIZE(format_percent(0.9999, 6));  // 99.990000 % — high decimal count
}

// ---------------------------------------------------------------------------
// Temperature
// ---------------------------------------------------------------------------
void test_format_seconds() {
    ASSERT_SIZE(format_seconds(Seconds{0.0}));
    ASSERT_SIZE(format_seconds(Seconds{1.5}));       // "1.500 s"
    ASSERT_SIZE(format_seconds(Seconds{0.0556}));    // "55.6 ms"
    ASSERT_SIZE(format_seconds(Seconds{1.23e-5}));   // "12.3 µs"
    ASSERT_SIZE(format_seconds(Seconds{1.0e9}));
    // Band checks
    ASSERT_TRUE(format_seconds(Seconds{1.5}).find(" s") != std::string::npos);
    ASSERT_TRUE(format_seconds(Seconds{0.0556}).find("ms") != std::string::npos);
    ASSERT_TRUE(format_seconds(Seconds{1.23e-5}).find("\xc2\xb5s") != std::string::npos);
    ASSERT_TRUE(format_seconds(Seconds{0.0}) == "0.000 s");
}

void test_format_hz() {
    ASSERT_SIZE(format_hz(0.0));
    ASSERT_SIZE(format_hz(1.0));       // "1.00 Hz"
    ASSERT_SIZE(format_hz(2500.0));    // "2.50 kHz"
    ASSERT_SIZE(format_hz(3.1e6));     // "3.10 MHz"
    ASSERT_SIZE(format_hz(1.0e12));
    ASSERT_TRUE(format_hz(2500.0).find("kHz") != std::string::npos);
    ASSERT_TRUE(format_hz(3.1e6).find("MHz") != std::string::npos);
}

void test_format_mps() {
    ASSERT_SIZE(format_mps(0.0));
    ASSERT_SIZE(format_mps(-3000.0));
    ASSERT_SIZE(format_mps(1.0e9, 6));
    ASSERT_TRUE(format_mps(15.0) == "15.0 m/s");
    ASSERT_TRUE(format_mps(-0.0) == "0.0 m/s");   // negative-zero normalization
}

void test_format_count() {
    ASSERT_SIZE(format_count(0.0));
    ASSERT_SIZE(format_count(55.55));
    ASSERT_SIZE(format_count(1.0e15, 3));
    // 55.55 is 55.5499… in binary — rounds down; avoid the half-way case.
    ASSERT_TRUE(format_count(55.56) == "55.6");
    ASSERT_TRUE(format_count(-0.0) == "0.0");   // negative-zero normalization
}

void test_format_kelvin() {
    ASSERT_SIZE(format_kelvin(Kelvin{0.0}));
    ASSERT_SIZE(format_kelvin(Kelvin{100000.0}));
    ASSERT_SIZE(format_kelvin(Kelvin{100000.0}, 2));
}

// ---------------------------------------------------------------------------
// Boolean / status (these are string literals, trivially short)
// ---------------------------------------------------------------------------
void test_format_regime() {
    ASSERT_SIZE(format_regime(true));
    ASSERT_SIZE(format_regime(false));
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== test_formatter ===\n";

    RUN_TEST(test_format_dbm);
    RUN_TEST(test_format_dbw);
    RUN_TEST(test_format_watts);
    RUN_TEST(test_format_db);
    RUN_TEST(test_format_db_negative_zero);
    RUN_TEST(test_format_dbsm);
    RUN_TEST(test_format_mhz);
    RUN_TEST(test_format_ghz);
    RUN_TEST(test_format_km);
    RUN_TEST(test_format_m);
    RUN_TEST(test_format_degrees);
    RUN_TEST(test_format_percent);
    RUN_TEST(test_format_seconds);
    RUN_TEST(test_format_hz);
    RUN_TEST(test_format_mps);
    RUN_TEST(test_format_count);
    RUN_TEST(test_format_kelvin);
    RUN_TEST(test_format_regime);

    return test::summary();
}
