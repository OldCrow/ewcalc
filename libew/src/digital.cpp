#include "libew/digital/digital.h"
#include <cmath>

namespace libew::digital {

Db eb_no_from_snr(Db snr, Mhz bandwidth, Mhz data_rate) noexcept {
    // Eb/N₀ = SNR + 10·log10(BW / Rb)
    return Db{snr.value + 10.0 * std::log10(bandwidth.value / data_rate.value)};
}

Db snr_from_eb_no(Db eb_no, Mhz bandwidth, Mhz data_rate) noexcept {
    // SNR = Eb/N₀ - 10·log10(BW / Rb) = Eb/N₀ + 10·log10(Rb / BW)
    return Db{eb_no.value - 10.0 * std::log10(bandwidth.value / data_rate.value)};
}

Db dsss_process_gain(Mhz chip_rate, Mhz data_rate) noexcept {
    // PG = 10·log10(chip_rate / data_rate)
    return Db{10.0 * std::log10(chip_rate.value / data_rate.value)};
}

Db dsss_jamming_margin(Db process_gain, Db required_eb_no,
                       Db implementation_loss) noexcept {
    // JM = PG - Eb/N₀_req - L_impl
    return Db{process_gain.value - required_eb_no.value - implementation_loss.value};
}

Db dsss_required_js(Db process_gain, Db required_eb_no,
                    Db implementation_loss) noexcept {
    // J/S_req = -JM = Eb/N₀_req + L_impl - PG
    return Db{required_eb_no.value + implementation_loss.value - process_gain.value};
}

} // namespace libew::digital
