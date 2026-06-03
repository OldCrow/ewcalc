#include "test_main.h"
#include "libew/radar/radar.h"

using namespace libew::radar;
using namespace libew::units;
using namespace libew::units::literals;

void test_pulse_compression_gain() {
    // TB = 1000 → gain = 30 dB
    ASSERT_NEAR(pulse_compression_gain(1000.0).value, 30.0, 0.01);
    // TB = 1 → gain = 0 dB
    ASSERT_NEAR(pulse_compression_gain(1.0).value, 0.0, 0.01);
}

void test_coherent_integration_gain() {
    // 100 pulses → 20 dB
    ASSERT_NEAR(coherent_integration_gain(100).value, 20.0, 0.01);
    // 10 pulses → 10 dB
    ASSERT_NEAR(coherent_integration_gain(10).value, 10.0, 0.01);
}

void test_radar_range_computed() {
    // Correctness test with analytically-derived expected value.
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
    // 3 GHz → λ = 0.1 m
    ASSERT_NEAR(wavelength_m(3000.0_MHz).value, 0.09993, 0.001);
    // 300 MHz → λ = 1 m
    ASSERT_NEAR(wavelength_m(300.0_MHz).value, 0.9993, 0.001);
}

int main() {
    std::cout << "=== test_radar ===\n";
    RUN_TEST(test_pulse_compression_gain);
    RUN_TEST(test_coherent_integration_gain);
    RUN_TEST(test_radar_range_computed);
    RUN_TEST(test_wavelength);
    return test::summary();
}
