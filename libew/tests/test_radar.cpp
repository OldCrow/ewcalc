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

void test_radar_range_sanity() {
    // Sanity check: a modest radar should give a finite positive range
    const RadarRangeResult r = radar_range(
        Dbm{60.0},   // 1 kW transmitter (60 dBm)
        Db{30.0},    // 30 dBi antenna gain
        Dbsm{0.0},   // 0 dBsm = 1 m² RCS
        Mhz{3000.0}, // X-band (3 GHz)
        Db{3.0},     // 3 dB system losses
        Db{5.0},     // 5 dB noise figure
        Mhz{1.0},    // 1 MHz bandwidth
        Db{13.0}     // 13 dB SNR
    );
    // Should be in the range of tens to hundreds of km for these parameters
    ASSERT_TRUE(r.max_range.value > 10.0);
    ASSERT_TRUE(r.max_range.value < 1000.0);
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
    RUN_TEST(test_radar_range_sanity);
    RUN_TEST(test_wavelength);
    return test::summary();
}
