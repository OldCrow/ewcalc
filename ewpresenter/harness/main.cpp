#include "ewpresenter/ewpresenter.h"
#include <iostream>
#include <string>

// ============================================================================
// Helper: print a labelled section header
// ============================================================================
static void section(const char* title) {
    std::cout << "\n=== " << title << " ===\n";
}
static void row(const char* label, const std::string& value) {
    std::cout << "  " << label << ": " << value << "\n";
}

int main() {
    std::cout << "ewpresenter harness — default-input smoke test\n";

    // -----------------------------------------------------------------------
    // 1. Propagation
    //    Default inputs: 32.6 km, 100 MHz, 10 m / 10 m
    // -----------------------------------------------------------------------
    section("Propagation");
    {
        ewpresenter::PropagationPresenter p;
        // Register callback to confirm it fires
        bool fired = false;
        p.set_on_change([&fired](const ewpresenter::PropagationPresenter::Output&) {
            fired = true;
        });
        p.set_distance(32.6);    // triggers callback
        p.set_frequency(100.0);
        p.set_tx_height(10.0);
        p.set_rx_height(10.0);

        const auto& out = p.output();
        row("Valid",         out.valid ? "yes" : "NO");
        row("FSPL",          out.fspl_str);
        row("2-Ray loss",    out.two_ray_loss_str);
        row("Fresnel zone",  out.fresnel_zone_str);
        row("Path loss",     out.path_loss_str);
        row("Regime",        out.regime_str);
        row("Callback fired",fired ? "yes" : "NO");
    }

    // -----------------------------------------------------------------------
    // 2. Link budget
    //    Default: 20 dBm Tx, 0/0 dB gains, 32.6 km, 10/10 m, 100 MHz, -120.5 dBm sens
    // -----------------------------------------------------------------------
    section("Link Budget");
    {
        ewpresenter::LinkPresenter p;
        p.set_tx_power(20.0);
        p.set_tx_gain(0.0);
        p.set_rx_gain(0.0);
        p.set_distance(32.6);
        p.set_tx_height(10.0);
        p.set_rx_height(10.0);
        p.set_frequency(100.0);
        p.set_rx_sensitivity(-120.5);

        const auto& out = p.output();
        row("Valid",           out.valid ? "yes" : "NO");
        row("Received power",  out.received_power_str);
        row("Path loss",       out.path_loss_str);
        row("Fresnel zone",    out.fresnel_zone_str);
        row("Regime",          out.regime_str);
        row("Link margin",     out.link_margin_str);
        row("Effective range", out.effective_range_str);
        row("Range regime",    out.range_regime_str);
    }

    // -----------------------------------------------------------------------
    // 3. Receiver
    //    Default: 0.1 MHz BW, 6.5 dB NF, 15 dB SNR, IP2=50, IP3=20, 12-bit ADC
    // -----------------------------------------------------------------------
    section("Receiver");
    {
        ewpresenter::ReceiverPresenter p;
        const auto& out = p.output();
        row("Valid",          out.valid ? "yes" : "NO");
        row("Sensitivity",    out.sensitivity_str);
        row("Cascaded NF",    out.cascaded_nf_str);
        row("SFDR2",          out.sfdr2_str);
        row("SFDR3",          out.sfdr3_str);
        row("Digital DR",     out.digital_dr_str);
    }

    // -----------------------------------------------------------------------
    // 4. Jamming
    //    Default: signal ERP 30 dBm, jammer ERP 50 dBm, 5 km / 50 km, 100 MHz
    // -----------------------------------------------------------------------
    section("Jamming");
    {
        ewpresenter::JammingPresenter p;
        const auto& out = p.output();
        row("Valid",          out.valid ? "yes" : "NO");
        row("J/S ratio",      out.js_ratio_str);
        row("Signal at Rx",   out.signal_at_rx_str);
        row("Jammer at Rx",   out.jammer_at_rx_str);
        row("Optimum BW",     out.optimum_bw_str);
        row("Duty cycle",     out.duty_cycle_str);
    }

    // -----------------------------------------------------------------------
    // 5. Location
    //    Default: 1° RMS bearing, 100 km range; EEP 2/1 km
    // -----------------------------------------------------------------------
    section("Location");
    {
        ewpresenter::LocationPresenter p;
        const auto& out = p.output();
        row("Valid",    out.valid ? "yes" : "NO");
        row("CEP(AOA)", out.cep_aoa_str);
        row("CEP(EEP)", out.cep_eep_str);
    }

    // -----------------------------------------------------------------------
    // 6. Radar
    //    Default: 60 dBm, 30 dBi, 0 dBsm, 3 GHz, 3 dB losses, 5 dB NF, 1 MHz, 13 dB SNR
    // -----------------------------------------------------------------------
    section("Radar");
    {
        ewpresenter::RadarPresenter p;
        const auto& out = p.output();
        row("Valid",               out.valid ? "yes" : "NO");
        row("Max range",           out.max_range_str);
        row("Two-way loss",        out.two_way_loss_str);
        row("PC gain (TB=100)",    out.pulse_compression_gain_str);
        row("Coh. int. gain (N=16)", out.coherent_integration_gain_str);
    }

    // -----------------------------------------------------------------------
    // 7. Validation — inject a bad input and verify the presenter invalidates
    // -----------------------------------------------------------------------
    section("Validation");
    {
        ewpresenter::PropagationPresenter p;
        p.set_distance(-5.0);  // invalid: negative
        row("Rejected negative distance",
            p.output().valid ? "FAIL (still valid)" : "PASS (invalidated)");
        row("Distance error",
            std::string(ewpresenter::describe(p.distance_error())));

        p.set_distance(32.6);  // restore
        row("Restored valid",
            p.output().valid ? "PASS" : "FAIL");
    }

    std::cout << "\nHarness complete.\n";
    return 0;
}
