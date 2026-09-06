#pragma once
// DopplerAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/doppler_presenter.h"

namespace EwPresenterNet {

public value struct DopplerOutput {
    System::String^ DopplerShiftStr;
    System::String^ UnambiguousRangeStr;
    System::String^ BlindSpeedStr;
    System::String^ UnambiguousVelocityStr;
    System::String^ RangeResolutionStr;
    System::String^ CrossRangeAzStr;
    System::String^ CrossRangeElStr;
    bool Valid;
};

public delegate void DopplerChangedHandler(DopplerOutput output);

public ref class DopplerAdapter sealed {
public:
    DopplerAdapter();
    ~DopplerAdapter();
    !DopplerAdapter();

    void SetFrequency   (double mhz)  { presenter_->set_frequency(mhz);     }
    void SetRadialSpeed (double mS)   { presenter_->set_radial_speed(mS);   }
    void SetPrf         (double hz)   { presenter_->set_prf(hz);            }
    void SetBandwidth   (double mhz)  { presenter_->set_bandwidth(mhz);     }
    void SetTargetRange (double km)   { presenter_->set_target_range(km);   }
    void SetBeamwidthAz (double deg)  { presenter_->set_beamwidth_az(deg);  }
    void SetBeamwidthEl (double deg)  { presenter_->set_beamwidth_el(deg);  }

    property double DefaultFrequency   { double get() { return presenter_->frequency_mhz();    } }
    property double DefaultRadialSpeed { double get() { return presenter_->radial_speed_m_s(); } }
    property double DefaultPrf         { double get() { return presenter_->prf_hz();           } }
    property double DefaultBandwidth   { double get() { return presenter_->bandwidth_mhz();    } }
    property double DefaultTargetRange { double get() { return presenter_->target_range_km();  } }
    property double DefaultBeamwidthAz { double get() { return presenter_->beamwidth_az_deg(); } }
    property double DefaultBeamwidthEl { double get() { return presenter_->beamwidth_el_deg(); } }

    property FieldValidationError FrequencyError   { FieldValidationError get() { return ToManaged(presenter_->frequency_error());    } }
    property FieldValidationError RadialSpeedError { FieldValidationError get() { return ToManaged(presenter_->radial_speed_error()); } }
    property FieldValidationError PrfError         { FieldValidationError get() { return ToManaged(presenter_->prf_error());          } }
    property FieldValidationError BandwidthError   { FieldValidationError get() { return ToManaged(presenter_->bandwidth_error());    } }
    property FieldValidationError TargetRangeError { FieldValidationError get() { return ToManaged(presenter_->target_range_error()); } }
    property FieldValidationError BeamwidthAzError { FieldValidationError get() { return ToManaged(presenter_->beamwidth_az_error()); } }
    property FieldValidationError BeamwidthElError { FieldValidationError get() { return ToManaged(presenter_->beamwidth_el_error()); } }

    property DopplerOutput CurrentOutput { DopplerOutput get() { return Snapshot(presenter_->output()); } }

    event DopplerChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::DopplerPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::DopplerPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static DopplerOutput Snapshot(const ewpresenter::DopplerPresenter::Output& o) {
        DopplerOutput s{};
        s.DopplerShiftStr        = ToManaged(o.doppler_shift_str);
        s.UnambiguousRangeStr    = ToManaged(o.unambiguous_range_str);
        s.BlindSpeedStr          = ToManaged(o.blind_speed_str);
        s.UnambiguousVelocityStr = ToManaged(o.unambiguous_velocity_str);
        s.RangeResolutionStr     = ToManaged(o.range_resolution_str);
        s.CrossRangeAzStr        = ToManaged(o.cross_range_az_str);
        s.CrossRangeElStr        = ToManaged(o.cross_range_el_str);
        s.Valid                  = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
