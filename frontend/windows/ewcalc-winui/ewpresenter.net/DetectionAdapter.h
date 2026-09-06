#pragma once
// DetectionAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/detection_presenter.h"

namespace EwPresenterNet {

public value struct DetectionOutput {
    System::String^ RequiredSnrStr;
    System::String^ RequiredSnrAlbersheimStr;
    System::String^ FluctuationLossStr;
    System::String^ DwellTimeStr;
    System::String^ HitsPerScanStr;
    System::String^ FarStr;
    bool Valid;
};

public delegate void DetectionChangedHandler(DetectionOutput output);

public ref class DetectionAdapter sealed {
public:
    DetectionAdapter();
    ~DetectionAdapter();
    !DetectionAdapter();

    void SetPd          (double pd)    { presenter_->set_pd(pd);              }
    void SetPfaExponent (double x)     { presenter_->set_pfa_exponent(x);     }
    void SetNumPulses   (int n)        { presenter_->set_num_pulses(n);       }
    void SetSwerlingCase(int c)        { presenter_->set_swerling_case(c);    }
    void SetBeamwidth   (double deg)   { presenter_->set_beamwidth(deg);      }
    void SetScanRate    (double degS)  { presenter_->set_scan_rate(degS);     }
    void SetPrf         (double hz)    { presenter_->set_prf(hz);             }
    void SetBandwidth   (double mhz)   { presenter_->set_bandwidth(mhz);      }

    property double DefaultPd           { double get() { return presenter_->pd();              } }
    property double DefaultPfaExponent  { double get() { return presenter_->pfa_exponent();    } }
    property int    DefaultNumPulses    { int    get() { return presenter_->num_pulses();      } }
    property int    DefaultSwerlingCase { int    get() { return presenter_->swerling_case();   } }
    property double DefaultBeamwidth    { double get() { return presenter_->beamwidth_deg();   } }
    property double DefaultScanRate     { double get() { return presenter_->scan_rate_deg_s(); } }
    property double DefaultPrf          { double get() { return presenter_->prf_hz();          } }
    property double DefaultBandwidth    { double get() { return presenter_->bandwidth_mhz();   } }

    property FieldValidationError PdError           { FieldValidationError get() { return ToManaged(presenter_->pd_error());            } }
    property FieldValidationError PfaExponentError  { FieldValidationError get() { return ToManaged(presenter_->pfa_exponent_error());  } }
    property FieldValidationError NumPulsesError    { FieldValidationError get() { return ToManaged(presenter_->num_pulses_error());    } }
    property FieldValidationError SwerlingCaseError { FieldValidationError get() { return ToManaged(presenter_->swerling_case_error()); } }
    property FieldValidationError BeamwidthError    { FieldValidationError get() { return ToManaged(presenter_->beamwidth_error());     } }
    property FieldValidationError ScanRateError     { FieldValidationError get() { return ToManaged(presenter_->scan_rate_error());     } }
    property FieldValidationError PrfError          { FieldValidationError get() { return ToManaged(presenter_->prf_error());           } }
    property FieldValidationError BandwidthError    { FieldValidationError get() { return ToManaged(presenter_->bandwidth_error());     } }

    property DetectionOutput CurrentOutput { DetectionOutput get() { return Snapshot(presenter_->output()); } }

    event DetectionChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::DetectionPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::DetectionPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static DetectionOutput Snapshot(const ewpresenter::DetectionPresenter::Output& o) {
        DetectionOutput s{};
        s.RequiredSnrStr           = ToManaged(o.required_snr_str);
        s.RequiredSnrAlbersheimStr = ToManaged(o.required_snr_albersheim_str);
        s.FluctuationLossStr       = ToManaged(o.fluctuation_loss_str);
        s.DwellTimeStr             = ToManaged(o.dwell_time_str);
        s.HitsPerScanStr           = ToManaged(o.hits_per_scan_str);
        s.FarStr                   = ToManaged(o.far_str);
        s.Valid                    = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
