#include "libew/antenna/antenna.h"
#include "libew/core/constants.h"
#include <cmath>

namespace libew::antenna {

Degrees beamwidth_from_gain(Db gain_dbi) noexcept {
    // Symmetric inverse of G ≈ 10*log10(29000 / (θ_az * θ_el)),
    // with θ_az == θ_el. 29000 per Adamy EW103 Sec 3.7 p.70 — his stated
    // choice among the rule-of-thumb constants (literature spans ~26000
    // to 41253; the underlying solution gives 28889).
    return Degrees{std::sqrt(29000.0 / std::pow(10.0, gain_dbi.value / 10.0))};
}

Db gain_from_beamwidth(Degrees az_beamwidth, Degrees el_beamwidth) noexcept {
    // G ≈ 10*log10(29000 / (θ_az * θ_el)) — constant per Adamy EW103
    // Sec 3.7 p.70 (see beamwidth_from_gain above).
    return Db{10.0 * std::log10(29000.0 / (az_beamwidth.value * el_beamwidth.value))};
}

Meters wavelength(Mhz frequency) noexcept {
    return Meters{constants::speed_of_light_m_s / (frequency.value * 1.0e6)};
}

} // namespace libew::antenna
