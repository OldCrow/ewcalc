#include "libew/antenna/antenna.h"
#include "libew/core/constants.h"
#include <cmath>

namespace libew::antenna {

Degrees beamwidth_from_gain(Db gain_dbi) noexcept {
    // Symmetric inverse of G ≈ 10*log10(30000 / (θ_az * θ_el)),
    // with θ_az == θ_el.
    return Degrees{std::sqrt(30000.0 / std::pow(10.0, gain_dbi.value / 10.0))};
}

Db gain_from_beamwidth(Degrees az_beamwidth, Degrees el_beamwidth) noexcept {
    // G ≈ 10*log10(30000 / (θ_az * θ_el))
    return Db{10.0 * std::log10(30000.0 / (az_beamwidth.value * el_beamwidth.value))};
}

Meters wavelength(Mhz frequency) noexcept {
    return Meters{constants::speed_of_light_m_s / (frequency.value * 1.0e6)};
}

} // namespace libew::antenna
