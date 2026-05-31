#pragma once
// PropagationAdapter.h

#include "MarshalHelper.h"
#include "ewpresenter/propagation_presenter.h"

namespace EwPresenterNet {

public value struct PropagationOutput {
    System::String^ FsplStr;
    System::String^ TwoRayLossStr;
    System::String^ FresnelZoneStr;
    System::String^ PathLossStr;
    System::String^ RegimeStr;
    bool            TwoRayRegime;
    bool            Valid;
};

public delegate void PropagationChangedHandler(PropagationOutput output);

public ref class PropagationAdapter sealed {
public:
    PropagationAdapter();
    ~PropagationAdapter();
    !PropagationAdapter();

    void SetDistance (double km)     { presenter_->set_distance(km);     }
    void SetFrequency(double mhz)    { presenter_->set_frequency(mhz);   }
    void SetTxHeight (double meters) { presenter_->set_tx_height(meters); }
    void SetRxHeight (double meters) { presenter_->set_rx_height(meters); }

    property double DefaultDistance  { double get() { return presenter_->distance_km();   } }
    property double DefaultFrequency { double get() { return presenter_->frequency_mhz(); } }
    property double DefaultTxHeight  { double get() { return presenter_->tx_height_m();   } }
    property double DefaultRxHeight  { double get() { return presenter_->rx_height_m();   } }

    property FieldValidationError DistanceError  { FieldValidationError get() { return ToManaged(presenter_->distance_error());   } }
    property FieldValidationError FrequencyError { FieldValidationError get() { return ToManaged(presenter_->frequency_error());  } }
    property FieldValidationError TxHeightError  { FieldValidationError get() { return ToManaged(presenter_->tx_height_error());  } }
    property FieldValidationError RxHeightError  { FieldValidationError get() { return ToManaged(presenter_->rx_height_error());  } }

    property PropagationOutput CurrentOutput {
        PropagationOutput get() { return Snapshot(presenter_->output()); }
    }

    event PropagationChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::PropagationPresenter::Output& o) {
        Changed(Snapshot(o));
    }

private:
    ewpresenter::PropagationPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static PropagationOutput Snapshot(const ewpresenter::PropagationPresenter::Output& o) {
        PropagationOutput s{};
        s.FsplStr        = ToManaged(o.fspl_str);
        s.TwoRayLossStr  = ToManaged(o.two_ray_loss_str);
        s.FresnelZoneStr = ToManaged(o.fresnel_zone_str);
        s.PathLossStr    = ToManaged(o.path_loss_str);
        s.RegimeStr      = ToManaged(o.regime_str);
        s.TwoRayRegime   = o.two_ray_regime;
        s.Valid          = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
