#include "test_main.h"
#include "libew/receiver/receiver.h"
#include <array>
#include <cmath>

using namespace libew::receiver;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// Sensitivity: S = -114 + 10*log10(BW_MHz) + NF_dB + SNR_dB
// Source: Adamy EW101. The -114 dBm/MHz baseline = kT at 290 K
//         = -174 dBm/Hz + 60 dB for MHz normalisation.
// ---------------------------------------------------------------------------

void test_sensitivity_1mhz_zero_nf_zero_snr() {
    // At 1 MHz BW, 0 dB NF, 0 dB SNR: -114 + 10*log10(1) + 0 + 0 = -114 dBm
    ASSERT_NEAR(system_sensitivity(1.0_MHz, 0.0_dB, 0.0_dB).value, -114.0, 0.1);
}

void test_sensitivity_10mhz_bandwidth() {
    // 10 MHz BW, 0 dB NF, 0 dB SNR: -114 + 10 = -104 dBm
    ASSERT_NEAR(system_sensitivity(10.0_MHz, 0.0_dB, 0.0_dB).value, -104.0, 0.1);
}

void test_sensitivity_with_nf_and_snr() {
    // BW=0.1 MHz, NF=6.5 dB, SNR=15 dB:
    // S = -114 + 10*log10(0.1) + 6.5 + 15 = -114 - 10 + 6.5 + 15 = -102.5 dBm
    ASSERT_NEAR(system_sensitivity(0.1_MHz, 6.5_dB, 15.0_dB).value, -102.5, 0.01);
}

void test_sensitivity_additive() {
    // Each NF dB and each SNR dB adds linearly to the sensitivity
    const Dbm s1 = system_sensitivity(1.0_MHz, 5.0_dB, 10.0_dB);
    const Dbm s2 = system_sensitivity(1.0_MHz, 6.0_dB, 10.0_dB);
    ASSERT_NEAR(s2.value - s1.value, 1.0, 0.001);
}

// ---------------------------------------------------------------------------
// Cascaded noise figure (Friis formula):
// NF_sys = NF1 + (NF2-1)/G1 + (NF3-1)/(G1*G2) + ...
// Source: Friis (1944); Adamy EW101.
//
// For a lossless passive element of L dB: NF = L dB, gain = -L dB.
// Derivation of test values is done analytically from the Friis formula.
// ---------------------------------------------------------------------------

void test_cascaded_nf_perfect_preamp() {
    // If stage 1 is a perfect (noiseless) preamp with infinite gain,
    // NF_sys should equal NF1 = 0 dB. Approximate with G=60 dB, NF1=0 dB:
    const Stage preamp  = {0.0_dB, 60.0_dB};
    const Stage backend = {20.0_dB, Db{0.0}};
    const Db nf = cascaded_noise_figure(preamp, backend);
    // NF_sys = 1 + (10^2 - 1)/10^6 ≈ 1 + 0 = 1 linear = 0 dB
    ASSERT_NEAR(nf.value, 0.0, 0.01);
}

void test_cascaded_nf_dominant_first_stage() {
    // High-gain first stage dominates: NF_sys ≈ NF1
    // Preamp: NF=3 dB, G=20 dB; backend: NF=10 dB
    // Friis (linear): NF = 10^0.3 + (10^1.0 - 1)/10^2.0
    //               = 1.9953 + 9/100 = 1.9953 + 0.09 = 2.0853
    // 10*log10(2.0853) = 3.19 dB
    const Stage s1 = {3.0_dB, 20.0_dB};
    const Stage s2 = {10.0_dB, Db{0.0}};
    ASSERT_NEAR(cascaded_noise_figure(s1, s2).value, 10.0*std::log10(2.0853), 0.02);
}

void test_cascaded_nf_four_stage_chain() {
    // Four-stage chain: feed loss (1 dB), preamp (NF=3 dB, G=20 dB),
    // cable loss (10 dB), receiver (NF=11 dB).
    //
    // Friis linear:
    //   NF1 = 10^0.1 = 1.2589,  G1 = 10^(-0.1) = 0.7943
    //   NF2 = 10^0.3 = 1.9953,  G2 = 10^2.0  = 100
    //   NF3 = 10^1.0 = 10.0,    G3 = 10^(-1.0) = 0.1
    //   NF4 = 10^1.1 = 12.589
    //
    //   NF_sys = 1.2589
    //          + (1.9953-1)/0.7943           = 1.2535
    //          + (10.0 -1)/(0.7943*100)      = 0.1133
    //          + (12.589-1)/(0.7943*100*0.1) = 1.4590
    //        = 4.0847 linear = 6.11 dB
    const std::array<Stage, 4> chain = {{
        {Db{1.0},  Db{-1.0}},
        {Db{3.0},  Db{20.0}},
        {Db{10.0}, Db{-10.0}},
        {Db{11.0}, Db{0.0}}
    }};
    ASSERT_NEAR(cascaded_noise_figure(std::span<const Stage>{chain}).value, 6.11, 0.05);
}

// ---------------------------------------------------------------------------
// Digital dynamic range: DR = 6.02*N + 1.76 dB
// Source: Walden (1999); widely reproduced in EW and DSP texts.
// ---------------------------------------------------------------------------

void test_digital_dr_1bit() {
    // 1 bit: DR = 6.02 + 1.76 = 7.78 dB
    ASSERT_NEAR(digital_dynamic_range(1).value, 7.78, 0.01);
}

void test_digital_dr_8bit() {
    // 8 bits: DR = 6.02*8 + 1.76 = 49.92 dB
    ASSERT_NEAR(digital_dynamic_range(8).value, 49.92, 0.01);
}

void test_digital_dr_12bit() {
    // 12 bits: DR = 6.02*12 + 1.76 = 72.24 + 1.76 = 74.00 dB
    ASSERT_NEAR(digital_dynamic_range(12).value, 74.00, 0.01);
}

void test_digital_dr_each_bit_adds_6db() {
    // Each additional bit adds 6.02 dB
    const Db dr8  = digital_dynamic_range(8);
    const Db dr9  = digital_dynamic_range(9);
    ASSERT_NEAR(dr9.value - dr8.value, 6.02, 0.001);
}

// ---------------------------------------------------------------------------
// SFDR
// 2nd order: SFDR2 = (1/2) * (IIP2 - S)   — IM2 grows at 2:1 slope
// 3rd order: SFDR3 = (2/3) * (IIP3 - S)   — IM3 grows at 3:1 slope
// Source: Razavi RF Microelectronics; Pozar Microwave Engineering.
// ---------------------------------------------------------------------------

void test_sfdr_third_order_derivation() {
    // sensitivity = -100 dBm, IP3 = 20 dBm
    // SFDR3 = (2/3) * (20 - (-100)) = (2/3) * 120 = 80 dB
    ASSERT_NEAR(analog_sfdr_third_order(Dbm{-100.0}, Dbm{20.0}).value, 80.0, 0.001);
}

void test_sfdr_second_order_derivation() {
    // sensitivity = -100 dBm, IP2 = 50 dBm
    // SFDR2 = (1/2) * (50 - (-100)) = (1/2) * 150 = 75 dB
    ASSERT_NEAR(analog_sfdr_second_order(Dbm{-100.0}, Dbm{50.0}).value, 75.0, 0.001);
}

void test_sfdr_linearity_third_order() {
    // Each dB increase in IP3 adds 2/3 dB to SFDR3
    const Db sfdr1 = analog_sfdr_third_order(Dbm{-100.0}, Dbm{10.0});
    const Db sfdr2 = analog_sfdr_third_order(Dbm{-100.0}, Dbm{11.0});
    ASSERT_NEAR(sfdr2.value - sfdr1.value, 2.0/3.0, 0.001);
}

void test_sfdr_linearity_second_order() {
    // Each dB increase in IP2 adds 1/2 dB to SFDR2
    const Db sfdr1 = analog_sfdr_second_order(Dbm{-100.0}, Dbm{10.0});
    const Db sfdr2 = analog_sfdr_second_order(Dbm{-100.0}, Dbm{11.0});
    ASSERT_NEAR(sfdr2.value - sfdr1.value, 0.5, 0.001);
}

// ---------------------------------------------------------------------------
// Noise temperature conversions
//
// T_e = (NF_lin - 1) * T_ref,  T_ref = 290 K
// NF  = 10·log10(1 + T_e / T_ref)
// Source: IEEE standard; Adamy EW102.
// ---------------------------------------------------------------------------

void test_nf_from_noise_temp_at_t_ref() {
    // T_e = 290 K = T_ref → NF_lin = 2 → NF_dB = 10*log10(2) = 3.010 dB
    ASSERT_NEAR(nf_from_noise_temp(Kelvin{290.0}).value,
                10.0 * std::log10(2.0), 0.001);
}

void test_nf_from_noise_temp_zero() {
    // T_e = 0 K → NF = 0 dB (noiseless)
    ASSERT_NEAR(nf_from_noise_temp(Kelvin{0.0}).value, 0.0, 1e-9);
}

void test_noise_temp_from_nf_roundtrip() {
    // noise_temp_from_nf and nf_from_noise_temp are exact inverses
    const Db nf_original{6.5};
    const Kelvin te = noise_temp_from_nf(nf_original);
    const Db nf_recovered = nf_from_noise_temp(te);
    ASSERT_NEAR(nf_recovered.value, nf_original.value, 1e-9);
}

void test_noise_temp_from_nf_0db() {
    // 0 dB NF → T_e = (1 - 1) * 290 = 0 K
    ASSERT_NEAR(noise_temp_from_nf(0.0_dB).value, 0.0, 1e-9);
}

void test_loss_noise_temp_at_standard_temp() {
    // At T_phys = 290 K: T_e = (L_lin - 1) * 290 = (NF_lin - 1) * T_ref
    // This must equal the result of noise_temp_from_nf for the same loss value.
    const Db loss{3.0}; // 3 dB
    const Kelvin te_loss = loss_noise_temp(loss, Kelvin{290.0});
    const Kelvin te_nf   = noise_temp_from_nf(loss);
    ASSERT_NEAR(te_loss.value, te_nf.value, 1e-6);
}

void test_loss_noise_temp_at_zero_kelvin() {
    // Cold component: T_phys = 0 K → T_e = 0 regardless of loss
    ASSERT_NEAR(loss_noise_temp(10.0_dB, Kelvin{0.0}).value, 0.0, 1e-9);
}

void test_loss_noise_temp_better_than_standard_when_cold() {
    // A cold component adds less noise than the same loss at 290 K
    const Db loss{3.0};
    const Kelvin te_cold = loss_noise_temp(loss, Kelvin{100.0});
    const Kelvin te_warm = loss_noise_temp(loss, Kelvin{290.0});
    ASSERT_TRUE(te_cold.value < te_warm.value);
}

int main() {
    std::cout << "=== test_receiver ===\n";
    RUN_TEST(test_sensitivity_1mhz_zero_nf_zero_snr);
    RUN_TEST(test_sensitivity_10mhz_bandwidth);
    RUN_TEST(test_sensitivity_with_nf_and_snr);
    RUN_TEST(test_sensitivity_additive);
    RUN_TEST(test_cascaded_nf_perfect_preamp);
    RUN_TEST(test_cascaded_nf_dominant_first_stage);
    RUN_TEST(test_cascaded_nf_four_stage_chain);
    RUN_TEST(test_digital_dr_1bit);
    RUN_TEST(test_digital_dr_8bit);
    RUN_TEST(test_digital_dr_12bit);
    RUN_TEST(test_digital_dr_each_bit_adds_6db);
    RUN_TEST(test_sfdr_third_order_derivation);
    RUN_TEST(test_sfdr_second_order_derivation);
    RUN_TEST(test_sfdr_linearity_third_order);
    RUN_TEST(test_sfdr_linearity_second_order);
    RUN_TEST(test_nf_from_noise_temp_at_t_ref);
    RUN_TEST(test_nf_from_noise_temp_zero);
    RUN_TEST(test_noise_temp_from_nf_roundtrip);
    RUN_TEST(test_noise_temp_from_nf_0db);
    RUN_TEST(test_loss_noise_temp_at_standard_temp);
    RUN_TEST(test_loss_noise_temp_at_zero_kelvin);
    RUN_TEST(test_loss_noise_temp_better_than_standard_when_cold);
    return test::summary();
}
