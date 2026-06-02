#pragma once

/// @file digital.h
/// @brief Digital link and spread-spectrum EW calculations.
///
/// Covers Eb/N₀ ↔ SNR conversion for digital links, and DSSS process gain
/// and jamming margin analysis (EW102/103 scope).

#include "libew/core/units.h"

namespace libew::digital {

using namespace libew::units;

// ---------------------------------------------------------------------------
// Eb/N₀ — energy-per-bit to noise density ratio
// ---------------------------------------------------------------------------

/// Eb/N₀ from carrier SNR measured over a noise bandwidth.
///
/// Eb/N₀ = SNR + 10·log10(BW / Rb)
///
/// Both BW and Rb use the Mhz type (Mcps and Mbps share the same 10⁶/s scale).
/// When BW == Rb (noise bandwidth equals data rate), Eb/N₀ = SNR.
///
/// @param snr        Carrier-to-noise ratio in bandwidth bw (dB)
/// @param bandwidth  Receiver noise bandwidth (MHz)
/// @param data_rate  Information bit rate (Mbps — stored as Mhz)
/// @return Eb/N₀ (dB)
[[nodiscard]] Db eb_no_from_snr(Db snr, Mhz bandwidth, Mhz data_rate) noexcept;

/// SNR from Eb/N₀ and the bandwidth-to-data-rate ratio (inverse of eb_no_from_snr).
///
/// SNR = Eb/N₀ − 10·log10(BW / Rb)
///
/// @param eb_no      Energy-per-bit to noise density ratio (dB)
/// @param bandwidth  Receiver noise bandwidth (MHz)
/// @param data_rate  Information bit rate (Mbps — stored as Mhz)
/// @return Carrier SNR in bandwidth (dB)
[[nodiscard]] Db snr_from_eb_no(Db eb_no, Mhz bandwidth, Mhz data_rate) noexcept;

// ---------------------------------------------------------------------------
// DSSS — direct-sequence spread spectrum
// ---------------------------------------------------------------------------

/// DSSS process gain.
///
/// PG = 10·log10(chip_rate / data_rate)
///
/// chip_rate is in Mcps (stored as Mhz); data_rate is in Mbps (stored as Mhz).
/// Both quantities are rates in units of 10⁶ per second, so their ratio is
/// dimensionless and the division is exact.
///
/// @param chip_rate  Spread-spectrum chip rate (Mcps — stored as Mhz)
/// @param data_rate  Information bit rate (Mbps — stored as Mhz)
/// @return Process gain (dB)
[[nodiscard]] Db dsss_process_gain(Mhz chip_rate, Mhz data_rate) noexcept;

/// DSSS jamming margin.
///
/// JM = PG − Eb/N₀_required − implementation_loss
///
/// A positive margin means the spread-spectrum gain exceeds the jammer's
/// advantage; a negative margin means the jammer can defeat the waveform.
///
/// @param process_gain       Spread-spectrum process gain (dB)
/// @param required_eb_no     Required Eb/N₀ for acceptable BER (dB)
/// @param implementation_loss Losses from non-ideal implementation (dB, positive)
/// @return Jamming margin (dB)
[[nodiscard]] Db dsss_jamming_margin(Db process_gain, Db required_eb_no,
                                     Db implementation_loss) noexcept;

/// Required J/S ratio to defeat DSSS jamming margin.
///
/// J/S_required = −JM = Eb/N₀_required + implementation_loss − PG
///
/// A positive result means the jammer must have a positive J/S advantage
/// at the receiver to overcome the spreading gain.
///
/// @param process_gain       Spread-spectrum process gain (dB)
/// @param required_eb_no     Required Eb/N₀ for acceptable BER (dB)
/// @param implementation_loss Losses from non-ideal implementation (dB, positive)
/// @return Required J/S ratio (dB)
[[nodiscard]] Db dsss_required_js(Db process_gain, Db required_eb_no,
                                  Db implementation_loss) noexcept;

} // namespace libew::digital
