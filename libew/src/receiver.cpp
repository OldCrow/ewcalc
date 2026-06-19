#include "libew/receiver/receiver.h"
#include "libew/core/constants.h"
#include "libew/core/units.h"
#include <cmath>

namespace libew::receiver {

Dbm system_sensitivity(Mhz bandwidth, Db noise_figure, Db required_snr) noexcept {
    // S = -114 + 10*log10(BW/1MHz) + NF + SNR_min
    // (-114 dBm/MHz = -174 dBm/Hz + 60 dB for MHz baseline)
    return Dbm{
        -114.0
        + 10.0 * std::log10(bandwidth.value)
        + noise_figure.value
        + required_snr.value
    };
}

Db cascaded_noise_figure(Stage s1, Stage s2) noexcept {
    // Friis: NF_sys = NF1_lin + (NF2_lin - 1) / G1_lin
    const double nf1 = units::db_to_linear(s1.noise_figure);
    const double nf2 = units::db_to_linear(s2.noise_figure);
    const double g1  = units::db_to_linear(s1.gain);
    return units::linear_to_db(nf1 + (nf2 - 1.0) / g1);
}

Db cascaded_noise_figure(std::span<const Stage> stages) noexcept {
    if (stages.empty()) return Db{0.0};
    if (stages.size() == 1) return stages[0].noise_figure;

    // Accumulate Friis from front to back
    double nf_sys = units::db_to_linear(stages[0].noise_figure);
    double g_cumulative = units::db_to_linear(stages[0].gain);

    for (std::size_t i = 1; i < stages.size(); ++i) {
        const double nf_i = units::db_to_linear(stages[i].noise_figure);
        nf_sys += (nf_i - 1.0) / g_cumulative;
        g_cumulative *= units::db_to_linear(stages[i].gain);
    }

    return units::linear_to_db(nf_sys);
}

Db analog_sfdr_second_order(Dbm sensitivity, Dbm second_order_ip) noexcept {
    // SFDR2 = (1/2) * (IP2_dBm - S_dBm)
    // IM2 products grow at 2:1 slope; SFDR2 = (IIP2 - S) / (2 - 1) * (1/2) = (IIP2 - S) / 2
    // Derivation: IM2 floor = 2*P_in - IIP2 = S  =>  P_in = (IIP2 + S)/2
    //             SFDR2 = P_in - S = (IIP2 - S)/2
    // Source: Razavi RF Microelectronics; Pozar Microwave Engineering.
    return Db{0.5 * (second_order_ip - sensitivity).value};
}

Db analog_sfdr_third_order(Dbm sensitivity, Dbm third_order_ip) noexcept {
    // SFDR3 = (2/3) * (IP3_dBm - S_dBm)
    return Db{(2.0 / 3.0) * (third_order_ip - sensitivity).value};
}

Db digital_dynamic_range(int num_bits) noexcept {
    // DR ≈ 6.02 * N + 1.76  (dB)
    return Db{6.02 * static_cast<double>(num_bits) + 1.76};
}

Kelvin noise_temp_from_nf(Db noise_figure) noexcept {
    // T_e = (NF_lin - 1) * T_ref
    const double nf_lin = units::db_to_linear(noise_figure);
    return Kelvin{(nf_lin - 1.0) * constants::standard_temperature_K};
}

Db nf_from_noise_temp(Kelvin noise_temp) noexcept {
    // NF_dB = 10*log10(1 + T_e / T_ref)
    return Db{
        10.0 * std::log10(1.0 + noise_temp.value / constants::standard_temperature_K)
    };
}

Kelvin loss_noise_temp(Db loss_db, Kelvin phys_temp) noexcept {
    // T_e = (L_lin - 1) * T_phys
    const double l_lin = units::db_to_linear(loss_db);
    return Kelvin{(l_lin - 1.0) * phys_temp.value};
}

} // namespace libew::receiver
