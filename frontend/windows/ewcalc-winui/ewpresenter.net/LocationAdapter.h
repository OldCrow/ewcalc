#pragma once
// LocationAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/location_presenter.h"

namespace EwPresenterNet {

public value struct LocationOutput {
    System::String^ CepAoaStr;
    System::String^ CepEepStr;
    bool Valid;
};

public delegate void LocationChangedHandler(LocationOutput output);

public ref class LocationAdapter sealed {
public:
    LocationAdapter();
    ~LocationAdapter();
    !LocationAdapter();

    void SetRmsBearingError(double degrees) { presenter_->set_rms_bearing_error(RoundInput(degrees)); }
    void SetAoaRange       (double km)      { presenter_->set_aoa_range(RoundInput(km));              }
    void SetSemiMajor      (double km)      { presenter_->set_semi_major(RoundInput(km));             }
    void SetSemiMinor      (double km)      { presenter_->set_semi_minor(RoundInput(km));             }

    property double DefaultRmsBearingError { double get() { return presenter_->rms_bearing_error_deg(); } }
    property double DefaultAoaRange        { double get() { return presenter_->aoa_range_km();          } }
    property double DefaultSemiMajor       { double get() { return presenter_->semi_major_km();         } }
    property double DefaultSemiMinor       { double get() { return presenter_->semi_minor_km();         } }

    property FieldValidationError RmsBearingError { FieldValidationError get() { return ToManaged(presenter_->rms_bearing_error()); } }
    property FieldValidationError AoaRangeError   { FieldValidationError get() { return ToManaged(presenter_->aoa_range_error());   } }
    property FieldValidationError SemiMajorError  { FieldValidationError get() { return ToManaged(presenter_->semi_major_error());  } }
    property FieldValidationError SemiMinorError  { FieldValidationError get() { return ToManaged(presenter_->semi_minor_error());  } }

    property LocationOutput CurrentOutput { LocationOutput get() { return Snapshot(presenter_->output()); } }

    event LocationChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::LocationPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::LocationPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static LocationOutput Snapshot(const ewpresenter::LocationPresenter::Output& o) {
        LocationOutput s{};
        s.CepAoaStr = ToManaged(o.cep_aoa_str);
        s.CepEepStr = ToManaged(o.cep_eep_str);
        s.Valid     = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
