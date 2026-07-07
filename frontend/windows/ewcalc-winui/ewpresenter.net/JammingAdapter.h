#pragma once
// JammingAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/jamming_presenter.h"

namespace EwPresenterNet {

public value struct JammingOutput {
    System::String^ JsRatioStr;
    System::String^ SignalAtRxStr;
    System::String^ JammerAtRxStr;
    System::String^ OptimumBwStr;
    System::String^ DutyCycleStr;
    System::String^ BurnthroughRangeStr;
    bool Valid;
};

public delegate void JammingChangedHandler(JammingOutput output);

public ref class JammingAdapter sealed {
public:
    JammingAdapter();
    ~JammingAdapter();
    !JammingAdapter();

    void SetSignalErp      (double dbm)    { presenter_->set_signal_erp(dbm);          }
    void SetJammerErp      (double dbm)    { presenter_->set_jammer_erp(dbm);          }
    void SetSignalToRxDist (double km)     { presenter_->set_signal_to_rx_dist(km);    }
    void SetJammerToRxDist (double km)     { presenter_->set_jammer_to_rx_dist(km);    }
    void SetSignalTxHeight (double meters) { presenter_->set_signal_tx_height(meters); }
    void SetJammerHeight   (double meters) { presenter_->set_jammer_height(meters);    }
    void SetRxHeight       (double meters) { presenter_->set_rx_height(meters);        }
    void SetFrequency      (double mhz)    { presenter_->set_frequency(mhz);           }
    void SetRxGainSignal   (double db)     { presenter_->set_rx_gain_signal(db);       }
    void SetRxGainJammer   (double db)     { presenter_->set_rx_gain_jammer(db);       }
    void SetSignalBandwidth(double mhz)    { presenter_->set_signal_bandwidth(mhz);    }
    void SetHopRange       (double mhz)    { presenter_->set_hop_range(mhz);           }
    void SetJsThreshold    (double db)     { presenter_->set_js_threshold(db);          }

    property double DefaultSignalErp       { double get() { return presenter_->signal_erp_dbm();       } }
    property double DefaultJammerErp       { double get() { return presenter_->jammer_erp_dbm();       } }
    property double DefaultSignalToRxDist  { double get() { return presenter_->signal_to_rx_dist_km(); } }
    property double DefaultJammerToRxDist  { double get() { return presenter_->jammer_to_rx_dist_km(); } }
    property double DefaultSignalTxHeight  { double get() { return presenter_->signal_tx_height_m();   } }
    property double DefaultJammerHeight    { double get() { return presenter_->jammer_height_m();      } }
    property double DefaultRxHeight        { double get() { return presenter_->rx_height_m();          } }
    property double DefaultFrequency       { double get() { return presenter_->frequency_mhz();        } }
    property double DefaultSignalBandwidth { double get() { return presenter_->signal_bandwidth_mhz(); } }
    property double DefaultHopRange        { double get() { return presenter_->hop_range_mhz();        } }
    property double DefaultJsThreshold     { double get() { return presenter_->js_threshold_db();      } }

    property FieldValidationError SignalErpError       { FieldValidationError get() { return ToManaged(presenter_->signal_erp_error());         } }
    property FieldValidationError JammerErpError       { FieldValidationError get() { return ToManaged(presenter_->jammer_erp_error());         } }
    property FieldValidationError SignalDistError      { FieldValidationError get() { return ToManaged(presenter_->signal_to_rx_dist_error());  } }
    property FieldValidationError JammerDistError      { FieldValidationError get() { return ToManaged(presenter_->jammer_to_rx_dist_error());  } }
    property FieldValidationError SignalHeightError    { FieldValidationError get() { return ToManaged(presenter_->signal_tx_height_error());   } }
    property FieldValidationError JammerHeightError    { FieldValidationError get() { return ToManaged(presenter_->jammer_height_error());      } }
    property FieldValidationError RxHeightError        { FieldValidationError get() { return ToManaged(presenter_->rx_height_error());          } }
    property FieldValidationError FrequencyError       { FieldValidationError get() { return ToManaged(presenter_->frequency_error());          } }
    property FieldValidationError RxGainSignalError    { FieldValidationError get() { return ToManaged(presenter_->rx_gain_signal_error());     } }
    property FieldValidationError RxGainJammerError    { FieldValidationError get() { return ToManaged(presenter_->rx_gain_jammer_error());     } }
    property FieldValidationError SignalBandwidthError { FieldValidationError get() { return ToManaged(presenter_->signal_bandwidth_error());   } }
    property FieldValidationError HopRangeError        { FieldValidationError get() { return ToManaged(presenter_->hop_range_error());          } }
    property FieldValidationError JsThresholdError     { FieldValidationError get() { return ToManaged(presenter_->js_threshold_error());       } }

    property JammingOutput CurrentOutput { JammingOutput get() { return Snapshot(presenter_->output()); } }

    event JammingChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::JammingPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::JammingPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static JammingOutput Snapshot(const ewpresenter::JammingPresenter::Output& o) {
        JammingOutput s{};
        s.JsRatioStr          = ToManaged(o.js_ratio_str);
        s.SignalAtRxStr       = ToManaged(o.signal_at_rx_str);
        s.JammerAtRxStr       = ToManaged(o.jammer_at_rx_str);
        s.OptimumBwStr        = ToManaged(o.optimum_bw_str);
        s.DutyCycleStr        = ToManaged(o.duty_cycle_str);
        s.BurnthroughRangeStr = ToManaged(o.burnthrough_range_str);
        s.Valid               = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
