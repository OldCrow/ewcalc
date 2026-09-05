# Formulas and references

This document maps `ewcalc` calculator outputs to the equations implemented in `libew`. It records sources, assumptions, sign conventions, and unit conventions. Adamy book page/equation numbers are marked `[OPEN]` where the source family is known but a precise equation pin has not yet been verified.

## Propagation

### Free-space path loss (FSPL)
`L = 32.44 + 20*log10(d_km) + 20*log10(f_MHz)`

- Source: Dave Adamy, "EW 101 — ES vs. SIGINT — Part 2: Antenna and Range Considerations," *The Journal of Electronic Defense*, February 2011, p. 51 (LOS intercept-range formula, algebraically equivalent); standard Friis free-space path loss equation.
- Assumptions: unobstructed line-of-sight, isotropic reference antennas; antenna gain is applied separately.
- Units: distance in km, frequency in MHz, loss in dB (positive = loss).

### Two-ray ground reflection path loss
`L = 120 + 40*log10(d_km) - 20*log10(h_tx_m) - 20*log10(h_rx_m)`

- Source: Adamy, "EW 101 — ES vs. SIGINT — Part 2," JED, February 2011, p. 51 (2-ray intercept-range formula uses this exact 120 dB constant); standard plane-earth ground-reflection model.
- Assumptions: applies beyond the Fresnel-zone crossover distance; flat, reflective ground plane.
- Units: distance in km, antenna heights in meters, loss in dB.

### Fresnel-zone crossover distance
`d_FZ = h_tx_m * h_rx_m * f_MHz / 24000` km

- Source: Adamy EW101 [OPEN: page/eq TBD]. Derived as the range where FSPL and two-ray curves intersect; exact algebra gives approximately `/23885`, conventionally rounded to `/24000`.
- Units: heights in meters, frequency in MHz, result in km.

### Knife-edge diffraction loss
Piecewise Lee (1982) approximation to the Fresnel diffraction integral `J(v)`.

- Source: Adamy EW101 [OPEN: page/eq TBD]; Lee (1982); ITU-R P.526.
- Sign convention: `los_clearance > 0` means the knife edge is below the line-of-sight; `< 0` means obstruction above LOS.
- Units: distances in km, clearance in meters, frequency in MHz, loss in dB (positive = additional loss). At `v = 0`, loss is 6.02 dB.

### Earth bulge
`h_m = d1_km * d2_km / (2 * R_eff_km) * 1000`, with `R_eff = (4/3) * 6371 km`.

- Source: Adamy EW101 [OPEN: page/eq TBD]; standard 4/3-effective-earth-radius geometry; ITU-R P.526.
- Units: distances in km, result in meters (positive = earth bulge above the flat-earth chord).

### Radar/radio horizon range
`R_km = 4.122 * (sqrt(h_tx_m) + sqrt(h_rx_m))`

- Source: Adamy EW101 [OPEN: page/eq TBD]; standard k=4/3 earth-horizon formula.
- Units: heights in meters, result in km.

## Antenna

### Effective radiated power (ERP)
`ERP = P_tx + G_tx`

- Source: Adamy EW103 Sec. 5.2, p. 120 and Fig. 5.4, p. 121.
- Units: power in dBm or dBW, gain in dB; result uses the same power unit as input.

### dBi ↔ dBd gain reference conversion
`dBd = dBi - 2.15`, `dBi = dBd + 2.15`

- Source: IEEE Std 145 antenna terminology. The 2.15 dB offset is `10*log10(1.64)`, the half-wave dipole directivity over isotropic. Not treated in Adamy EW101–EW103 (verified 2026-09-06): his equations reference ERP directly and assume the user supplies the correct gain; his antenna tables are explicitly illustrative, with actual gain from antenna physics.

### Beamwidth from gain
`theta_3dB ≈ sqrt(30000 / 10^(G_dBi / 10))` degrees

- Source: Kraus/Tai-Pereira-family two-plane beamwidth-directivity rule of thumb, 30000 variant. This is the circular/symmetric inverse of `gain_from_beamwidth()` where `theta_az == theta_el`, using the same 30000 beam-shape constant.
- DELTA vs. Adamy: EW103 uses `29000/(theta_az*theta_el)` (Sec. 3.7, p. 70), noting the underlying solution gives 28889 with 29000 in common use. The two rules differ by 10*log10(30000/29000) = 0.15 dB. Retained as a documented difference pending the formula-fidelity sweep (see PLAN.md Provenance Framing).
- Assumptions: the presenter rejects gains below -6.35 dBi so the derived beamwidth does not exceed 360°.
- Units: gain in dBi, result in degrees.

### Gain from beamwidth
`G ≈ 10*log10(30000 / (theta_az_deg * theta_el_deg))`

- Source: Kraus/Tai-Pereira-family rule of thumb, 30000 variant; Adamy, "EW 101 — ES vs. SIGINT — Part 2," JED, February 2011, p. 52, Fig. 3 provides graphical context for gain vs. 3 dB beamwidth.
- DELTA vs. Adamy: EW103 Sec. 3.7, p. 70's 29000 form — see Beamwidth from gain above; same 0.15 dB difference and same pending decision.
- Units: beamwidths in degrees, result in dBi.

### Wavelength
`lambda_m = c / f_Hz`, with `c = 299792458 m/s`.

- Source: SI-defined speed of light.
- Units: frequency in MHz (converted to Hz), result in meters.

## Link budget

### One-way received power
`P_rx = P_tx + G_tx + G_rx - path_loss`

- Source: Adamy, "EW 101 — ES vs. SIGINT — Part 2," JED, February 2011, p. 51 (same link-budget algebra underlying Adamy's LOS/2-ray intercept-range formulas).
- Assumptions: path-loss model is selected by comparing link distance to Fresnel-zone crossover.
- Units: powers/gains in dBm/dB, result in dBm.

### Effective range
Maximum range where `P_rx == receiver sensitivity`, solved by inverting the one-way link budget.

- Source: Adamy, "EW 101 — ES vs. SIGINT — Part 2," JED, February 2011, p. 51: LOS `RI = Anti-log{[ERPT - 32 - 20log(F) + GR - S]/20}` and 2-ray `RI = Anti-log{[ERPT - 120 + 20log(hT) + 20log(hR) + GR - S]/40}`.
- Units: result in km.

## Receiver

### System sensitivity
`S_dBm = -114 + 10*log10(BW_MHz) + NF_dB + SNR_dB`

- Source: Dave Adamy, "EW 101 — ES vs. SIGINT — Part 3: Receiver Considerations," *The Journal of Electronic Defense*, March 2011, p. 58: `S = kTB + NF + Required RFSNR`.
- Units: bandwidth in MHz, NF/SNR in dB, result in dBm. `-114 dBm/MHz` is `kT` at 290 K (`-174 dBm/Hz`) plus 60 dB for MHz normalization.

### Cascaded noise figure
`NF_sys = NF1 + (NF2 - 1)/G1 + (NF3 - 1)/(G1*G2) + ...` in linear domain, converted to/from dB.

- Source: Friis (1944); Adamy EW102 [OPEN: page/eq TBD].
- Assumptions: stages are ordered front-to-back; gains and NFs are converted between dB and linear forms.
- Units: dB in, dB out.

### Digital dynamic range
`DR_dB = 6.02*N_bits + 1.76`

- Source: standard ADC quantization-noise SNR relation; Walden (1999); Adamy EW102/EW103 [OPEN: page/eq TBD].
- Units: bits in, dB out.

### Spurious-free dynamic range (SFDR)
`SFDR2 = (IIP2 - S) / 2`, `SFDR3 = 2*(IIP3 - S) / 3`

- Source: standard IP2/IP3 SFDR definitions; Adamy EW102 [OPEN: page/eq TBD].
- Assumptions: IM2 products grow at 2:1 slope and IM3 products at 3:1 slope relative to the fundamental.
- Units: intercept points and sensitivity in dBm, result in dB.

### Noise temperature conversions
`T_e = (NF_lin - 1) * 290 K`, `NF_dB = 10*log10(1 + T_e/290)`; passive loss: `T_e = (L_lin - 1) * T_phys`.

- Source: IEEE Std 686 noise-temperature convention; Adamy EW102 [OPEN: page/eq TBD].
- Units: NF in dB, temperatures in Kelvin.

## Jamming

### Communications J/S ratio
`J/S = J_rx - S_rx`, where each received power is `ERP + Rx_gain - path_loss`.

- Source: Adamy EW101 Ch. 9 "Jamming"; Adamy EW102 Sec. 5.8.1 "Jamming-to-Signal Ratio", p. 138 [OPEN: exact eq # TBD].
- Assumptions: signal and jammer paths are evaluated independently and may use different LOS/2-ray regimes.
- Sign convention: positive J/S favors the jammer.
- Units: powers/gains in dBm/dB; distances in km; heights in meters; frequency in MHz.

### Burnthrough range
LOS: `d = 10^((margin - 32.44 - 20*log10(f))/20)`; 2-ray: `d = 10^((margin - 120 + 20*log10(h_tx) + 20*log10(h_rx))/40)`.

- Source: Adamy EW101 Ch. 9 "Burn-Through"; Adamy EW102 Sec. 5.8, pp. 137-140 [OPEN: exact eq # TBD].
- Assumptions: jammer geometry is fixed; signal range is solved for `J/S = threshold`.
- Units: km, m, MHz, dB/dBm.

### Partial-band / spot jamming
`BW_opt = signal_bandwidth * 10^(single_channel_js/10)`, capped only by `hop_range_bandwidth`; `duty_cycle = min(BW_opt, hop_range) / hop_range`.

- Source: Adamy EW102 Sec. 5.9.1 "Jamming Frequency Hop Signals", pp. 141-146 [OPEN: exact eq # TBD].
- Assumptions: for `single_channel_js >= 0 dB`, surplus jammer power should widen bandwidth beyond the signal bandwidth to cover more hops (the corrected v0.7.0 behavior).
- Units: bandwidths in MHz, J/S in dB.

## Location

### CEP from AOA
`CEP ≈ 1.2 * R * tan(sigma_theta)`

- Source: Adamy EW102 Sec. 6.6.2 "Circular Error Probable", p. 182 [OPEN: exact eq # TBD].
- Assumptions: ideal two-receiver 90-degree crossing geometry; `R*tan(sigma_theta)` is RMS cross-range error.
- Units: bearing error in degrees, range in km, CEP in km.

### CEP from TDOA
`CEP ≈ c * sigma_t * R / (2 * B)`

- Source: Adamy EW102 Sec. 6.7.1 "TDOA System Accuracy", p. 183 [OPEN: exact eq # TBD].
- Assumptions: ideal perpendicular baseline-to-emitter geometry.
- Units: timing error in nanoseconds, range/baseline in km, CEP in km.

### CEP from EEP
`CEP ≈ 0.59 * (a + b)`

- Source: Adamy EW102 Sec. 6.4.3 "Elliptical Error Probable", p. 174 [OPEN: exact eq # TBD].
- Assumptions: `a` and `b` are 1-sigma semi-major/semi-minor axes; approximation is accurate to about 1% for axis ratio `a/b <= 4`.
- Units: axes and CEP in km.

## Radar

### Radar range equation
`20*log10(R_max) = (P_t + 2G + 20*log10(lambda) + sigma - 30*log10(4*pi) - noise_power - SNR - L_sys) / 4`

- Source: Adamy EW102 Sec. 3.2 "Radar Range Equation", p. 36; Sec. 3.2.2 "Radar Detection Range", p. 40 [OPEN: exact eq # TBD].
- Assumptions: monostatic radar with the same antenna gain for transmit and receive; `noise_power = sensitivity - SNR`.
- Units: powers/gains in dBm/dBi/dB, RCS in dBsm, frequency/bandwidth in MHz, range in km.

### Pulse compression gain
`G_pc = 10*log10(time_bandwidth_product)`

- Source: Adamy EW102 Sec. 3.5.2 "Pulse Compression", p. 51 [OPEN: exact eq # TBD].
- Units: dimensionless time-bandwidth product in, dB out.

### Coherent integration gain
`G_int = 10*log10(num_pulses)`

- Source: standard non-fluctuating-target coherent pulse-integration result [OPEN: page/eq TBD].
- Units: pulse count in, dB out.

### LPI advantage
`LPI advantage = pulse_compression_gain / 4 = 10*log10(TB) / 4`

- Source: Adamy EW102 Sec. 3.9 "Low Probability of Intercept Radars", pp. 67-72; Sec. 3.9.5 "LPI Figure of Merit", p. 71 [OPEN: exact eq # TBD].
- Assumptions: compares matched-filter pulse-compression radar with non-coherent energy-detecting intercept receiver; advantage collapses to 0 dB if the intercept receiver also uses a matched filter.
- Units: time-bandwidth product in, dB out.

## Detection

### Required SNR — Albersheim's equation
`A = ln(0.62/Pfa)`, `B = ln(Pd/(1-Pd))`,
`SNR_dB = -5*log10(N) + (6.2 + 4.545/sqrt(N + 0.44)) * log10(A + 0.12*A*B + 1.7*B)`

- Source: W. J. Albersheim, "Closed-Form Approximation to Robertson's Detection Characteristics", Proc. IEEE 69(7), 1981, as reproduced in M. A. Richards, "Noncoherent Integration Gain, and its Approximation" (2010), eq. 12. Note the third constant is 4.545, not the oft-misquoted 4.54.
- Assumptions: nonfluctuating target, linear detector, noncoherent integration of N pulses; error < 0.2 dB for 1e-7 <= Pfa <= 1e-3, 0.1 <= Pd <= 0.9, 1 <= N <= 8096; also serviceable for square-law detectors (~0.2 dB apart).
- Units: probabilities and pulse count in, dB out.

### Required SNR — Shnidman's equation (Swerling 0–4)
`K = inf (Sw0), 1 (Sw1), N (Sw2), 2 (Sw3), 2N (Sw4)`; `alpha = 0.25 if N >= 40 else 0`;
`eta = sqrt(-0.8*ln(4*Pfa*(1-Pfa))) + sign(Pd-0.5)*sqrt(-0.8*ln(4*Pd*(1-Pd)))`;
`X_inf = eta*(eta + 2*sqrt(N/2 + alpha - 0.25))`;
`C1 = {[(17.7006*Pd - 18.4496)*Pd + 14.5339]*Pd - 3.525}/K`;
`C2 = (1/K)*{exp(27.31*Pd - 25.14) + (Pd - 0.8)*[0.7*ln(1e-5/Pfa) + (2N-20)/80]}` (only for Pd > 0.872);
`SNR_dB = 10*log10(X_inf/N) + C1 + C2`

- Source: D. A. Shnidman, "Determination of Required SNR Values", IEEE Trans. AES 38(3), 2002.
- Validation: the implementation is gated in `libew/tests/test_radar.cpp` against an exact Marcum-Q / noncentral-chi-square oracle (`scripts/detection_oracle.py`); worst observed error over the 60-point grid is 0.30 dB, inside Shnidman's published 0.5 dB envelope (0.1 <= Pd <= 0.99, 1e-9 <= Pfa <= 1e-3, 1 <= N <= 100).
- Units: probabilities, pulse count and Swerling case in, per-pulse dB out.

### Fluctuation loss
`L_f = SNR_required(Swerling case) - SNR_required(Swerling 0)`

- Source: definitional, computed via Shnidman's equation above; concept per Swerling-model radar detection theory (e.g. Richards, Fundamentals of Radar Signal Processing, ch. 6).
- Units: dB.

### Dwell time and hits per scan
`T_D = theta_az / scan_rate`, `hits = T_D * PRF`

- Source: standard scanning-radar geometry (Adamy EW101 dwell-time treatment [OPEN: page/eq TBD]).
- Units: beamwidth in degrees, scan rate in deg/s, PRF in Hz; dwell in seconds, hits dimensionless.

### False-alarm rate
`FAR = Pfa * B`

- Source: standard result — one independent detection opportunity per resolution time 1/B (e.g. Skolnik, Introduction to Radar Systems [OPEN: page/eq TBD]).
- Units: probability and MHz in, false alarms per second out.

## Doppler & Resolution

### Doppler shift (two-way)
`f_d = 2 * v_r * f / c`

- Source: standard radar Doppler relation (Richards, Fundamentals of Radar Signal Processing; Skolnik); Adamy EW102 radar chapter as EW-series anchor [OPEN: sec/page TBD].
- Assumptions: monostatic radar, two-way path (twice the one-way shift); closing target positive.
- Units: frequency in MHz, speed in m/s, shift in Hz.

### Unambiguous range
`R_u = c / (2 * PRF)`

- Source: standard pulse-radar relation (Richards; Skolnik); Adamy EW102 anchor [OPEN: sec/page TBD].
- Units: PRF in Hz, range in km.

### Blind speed and unambiguous velocity
`v_b = lambda * PRF / 2` (first blind speed; multiples n·v_b),
`v_u = lambda * PRF / 4` (Doppler unambiguous within ±PRF/2)

- Source: standard MTI/pulse-Doppler relations (Richards; Skolnik); Adamy EW102 anchor [OPEN: sec/page TBD].
- Note: together with R_u these express the Doppler dilemma — R_u·v_u = c·lambda/8 is PRF-invariant (guarded by test_radar.cpp).
- Units: frequency in MHz, PRF in Hz, speeds in m/s.

### Range resolution
`dR = c / (2 * B)`

- Source: standard waveform-bandwidth resolution relation (Richards; Skolnik); Adamy EW102 anchor [OPEN: sec/page TBD].
- Assumptions: compressed bandwidth B; for an uncompressed pulse B ≈ 1/tau gives dR = c·tau/2.
- Units: bandwidth in MHz, resolution in m.

### Cross-range resolution
`dX = R * theta_3dB` (theta in radians)

- Source: real-beam small-angle arc length (Richards; Skolnik); Adamy EW102 resolution-cell treatment as anchor [OPEN: sec/page TBD].
- Units: range in km, beamwidth in degrees, resolution in m. The az and el extents together with dR bound the resolution cell.

## Digital/DSSS

### Eb/N0 ↔ SNR
`Eb/N0 = SNR + 10*log10(BW/R_b)`; inverse `SNR = Eb/N0 - 10*log10(BW/R_b)`.

- Source: Adamy EW102 Sec. 5.6.6 "Signal-to-Noise Ratio", p. 128; Sec. 5.6.7 "Bit-Error Rate Versus RF SNR", p. 128 [OPEN: exact eq # TBD]; standard digital communications relation.
- Assumptions: `BW` is receiver noise bandwidth for the measured SNR.
- Units: `BW` and `R_b` use the same scale, stored as MHz/Mcps-equivalent; ratios are dimensionless, results in dB.

### DSSS process gain
`PG = 10*log10(chip_rate / data_rate)`

- Source: Adamy EW102 Sec. 5.7.3 "Direct Sequence Spread Spectrum Signals", p. 136 [OPEN: exact eq # TBD].
- Units: chip rate and data rate in the same scale, result in dB.

### DSSS jamming margin and required J/S
`JM = PG - Eb/N0_required - implementation_loss`; `J/S_required = -JM`.

- Source: Adamy EW102 Sec. 5.9.3 "Jamming DSSS Signals", pp. 147-149 [OPEN: exact eq # TBD]; Adamy EW103 book-level comms-jamming context [OPEN: page/eq TBD].
- Sign convention: positive jamming margin means spreading gain exceeds jammer advantage; positive required J/S means the jammer must exceed signal power at the receiver.
- Units: all terms in dB.
