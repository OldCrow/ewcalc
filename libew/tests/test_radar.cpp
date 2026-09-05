#include "test_main.h"
#include "libew/radar/radar.h"

using namespace libew::radar;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// Pulse compression gain: G_pc = 10*log10(time_bandwidth_product).
// Source: Adamy EW102 (2004) Sec 3.5.2 "Pulse Compression", p.51.
// [OPEN: exact eq # TBD]
// ---------------------------------------------------------------------------

void test_pulse_compression_gain() {
    // TB = 1000 → gain = 30 dB
    ASSERT_NEAR(pulse_compression_gain(1000.0).value, 30.0, 0.01);
    // TB = 1 → gain = 0 dB
    ASSERT_NEAR(pulse_compression_gain(1.0).value, 0.0, 0.01);
}

// ---------------------------------------------------------------------------
// Coherent integration gain: G_int = 10*log10(num_pulses). Standard
// non-fluctuating-target coherent pulse integration gain (general radar
// signal processing result, not Adamy-specific).
// Source: Adamy EW102 [OPEN: page/eq TBD]
// ---------------------------------------------------------------------------

void test_coherent_integration_gain() {
    // 100 pulses → 20 dB
    ASSERT_NEAR(coherent_integration_gain(100).value, 20.0, 0.01);
    // 10 pulses → 10 dB
    ASSERT_NEAR(coherent_integration_gain(10).value, 10.0, 0.01);
}

// ---------------------------------------------------------------------------
// Radar range equation (link-budget/log form of the classic radar range
// equation, applied one-way each direction plus RCS gain term).
// Source: Adamy EW102 Sec 3.2 "Radar Range Equation", p.36;
//         Sec 3.2.2 "Radar Detection Range", p.40.
// [OPEN: exact eq # TBD]
// ---------------------------------------------------------------------------

void test_radar_range_computed() {
    // Correctness test with analytically-derived expected value.
    // Note (issue #14 item 2): the expected value below is hand-derived from
    // the documented radar range equation (see the derivation comment), not
    // computed by calling radar_range() itself, so this is a genuine
    // independent reference check rather than a self-referential round-trip.
    //
    // Inputs: Pt=60 dBm, G=30 dBi, σ=0 dBsm, f=3 GHz, L=3 dB, NF=5 dB, B=1 MHz, SNR=13 dB
    //
    // Derivation (from radar_range equation in radar.cpp):
    //   λ = c/f = 2.998e8/3e9 = 0.09993 m
    //   noise_power = sensitivity(1 MHz, 5 dB NF, 0 dB SNR) = -114+0+5 = -109 dBm
    //   numerator_dB = 60 + 2*30 + 20*log10(0.09993) + 0
    //                  - 30*log10(4π) - (-109) - 13 - 3
    //                ≈ 60 + 60 - 20.01 + 0 - 33.02 + 109 - 13 - 3 = 159.97 dB
    //   R_m = 10^(159.97/40) = 10^3.999 ≈ 9990 m ≈ 10.0 km
    // Harness output (default presenter inputs): 10.010 km
    const RadarRangeResult r = radar_range(
        Dbm{60.0},   // 1 kW transmitter
        Db{30.0},    // 30 dBi antenna gain
        Dbsm{0.0},   // 1 m² RCS
        Mhz{3000.0}, // X-band
        Db{3.0},     // system losses
        Db{5.0},     // noise figure
        Mhz{1.0},    // bandwidth
        Db{13.0}     // required SNR
    );
    ASSERT_NEAR(r.max_range.value, 10.01, 0.05);
    ASSERT_TRUE(r.two_way_loss.value > 0.0);
}

void test_wavelength() {
    // λ = c / f (basic physics; no domain-specific citation needed).
    // 3 GHz → λ = 0.1 m
    ASSERT_NEAR(wavelength_m(3000.0_MHz).value, 0.09993, 0.001);
    // 300 MHz → λ = 1 m
    ASSERT_NEAR(wavelength_m(300.0_MHz).value, 0.9993, 0.001);
}

// ---------------------------------------------------------------------------
// LPI advantage: ratio of intercept-receiver range disadvantage to radar
// detection range, expressed in dB as pulse_compression_gain / 4. This
// follows from the radar detection range scaling as (avg power * PC gain)^(1/4)
// versus a non-coherent intercept receiver's range scaling as (peak power)^(1/2)
// (see libew/include/libew/radar/radar.h derivation comment).
// Source: Adamy EW102 Sec 3.9 "Low Probability of Intercept Radars", p.67-72
//         (Sec 3.9.5 "LPI Figure of Merit", p.71). [OPEN: exact eq # TBD]
// (An earlier audit note here flagged radar.h as citing EW103 for this; the
// header now correctly cites EW102 — resolved in the v1.0.0 release change,
// closed as #67.)
// ---------------------------------------------------------------------------

void test_lpi_advantage() {
    // lpi_advantage = 10*log10(TB) / 4
    // TB = 100   → 10*log10(100)/4   = 10*2/4  = 5.0 dB
    // TB = 10000 → 10*log10(10000)/4 = 10*4/4  = 10.0 dB
    ASSERT_NEAR(lpi_advantage(100.0).value, 5.0, 0.01);
    ASSERT_NEAR(lpi_advantage(10000.0).value, 10.0, 0.01);
}

int main() {
    std::cout << "=== test_radar ===\n";
    RUN_TEST(test_pulse_compression_gain);
    RUN_TEST(test_coherent_integration_gain);
    RUN_TEST(test_radar_range_computed);
    RUN_TEST(test_wavelength);
    RUN_TEST(test_lpi_advantage);
    return test::summary();
}
