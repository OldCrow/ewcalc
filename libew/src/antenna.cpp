#include "libew/antenna/antenna.h"
#include "libew/core/constants.h"
#include <cmath>

namespace libew::antenna {

Degrees beamwidth_from_gain(Db gain_dbi) noexcept {
    // Spherical cap approximation: θ = 10^((11.1 - G_dBi) / 20)
    return Degrees{std::pow(10.0, (11.1 - gain_dbi.value) / 20.0)};
}

Db gain_from_beamwidth(Degrees az_beamwidth, Degrees el_beamwidth) noexcept {
    // G ≈ 10*log10(30000 / (θ_az * θ_el))
    return Db{10.0 * std::log10(30000.0 / (az_beamwidth.value * el_beamwidth.value))};
}

Meters wavelength(Mhz frequency) noexcept {
    return Meters{constants::speed_of_light_m_s / (frequency.value * 1.0e6)};
}

} // namespace libew::antenna
