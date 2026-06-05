#pragma once
// RadarAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/radar_presenter.h"

namespace EwPresenterNet {

public value struct RadarOutput {
    System::String^ MaxRangeStr;
    System::String^ TwoWayLossStr;
    System::String^ PulseCompressionGainStr;
    System::String^ CoherentIntegrationGainStr;
    System::String^ LpiAdvantageStr;
    System::String^ TargetRcsStr;
    bool Valid;
};

public delegate void RadarChangedHandler(RadarOutput output);

public ref class RadarAdapter sealed {
public:
    RadarAdapter();
    ~RadarAdapter();
    !RadarAdapter();

    void SetTxPower          (double dbm)  { presenter_->set_tx_power(RoundInput(dbm));              }
    void SetAntennaGain      (double dbi)  { presenter_->set_antenna_gain(RoundInput(dbi));          }
    void SetTargetRcs        (double dbsm) { presenter_->set_target_rcs(RoundInput(dbsm));           }
    void SetFrequency        (double mhz)  { presenter_->set_frequency(RoundInput(mhz));             }
    void SetSystemLosses     (double db)   { presenter_->set_system_losses(RoundInput(db));          }
    void SetNoiseFigure      (double db)   { presenter_->set_noise_figure(RoundInput(db));           }
    void SetBandwidth        (double mhz)  { presenter_->set_bandwidth(RoundInput(mhz));             }
    void SetRequiredSnr      (double db)   { presenter_->set_required_snr(RoundInput(db));           }
    void SetTimeBandwidthProd(double tb)   { presenter_->set_time_bandwidth_product(RoundInput(tb)); }
    void SetNumPulses        (int n)       { presenter_->set_num_pulses(n);              }

    property double DefaultTxPower           { double get() { return presenter_->tx_power_dbm();           } }
    property double DefaultAntennaGain       { double get() { return presenter_->antenna_gain_dbi();       } }
    property double DefaultTargetRcs         { double get() { return presenter_->target_rcs_dbsm();        } }
    property double DefaultFrequency         { double get() { return presenter_->frequency_mhz();          } }
    property double DefaultSystemLosses      { double get() { return presenter_->system_losses_db();       } }
    property double DefaultNoiseFigure       { double get() { return presenter_->noise_figure_db();        } }
    property double DefaultBandwidth         { double get() { return presenter_->bandwidth_mhz();          } }
    property double DefaultRequiredSnr       { double get() { return presenter_->required_snr_db();        } }
    property double DefaultTimeBandwidthProd { double get() { return presenter_->time_bandwidth_product(); } }
    property int    DefaultNumPulses         { int    get() { return presenter_->num_pulses();             } }

    property RadarOutput CurrentOutput { RadarOutput get() { return Snapshot(presenter_->output()); } }

    event RadarChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::RadarPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::RadarPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static RadarOutput Snapshot(const ewpresenter::RadarPresenter::Output& o) {
        RadarOutput s{};
        s.MaxRangeStr                = ToManaged(o.max_range_str);
        s.TwoWayLossStr              = ToManaged(o.two_way_loss_str);
        s.PulseCompressionGainStr    = ToManaged(o.pulse_compression_gain_str);
        s.CoherentIntegrationGainStr = ToManaged(o.coherent_integration_gain_str);
        s.LpiAdvantageStr            = ToManaged(o.lpi_advantage_str);
        s.TargetRcsStr               = ToManaged(o.target_rcs_str);
        s.Valid                      = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
