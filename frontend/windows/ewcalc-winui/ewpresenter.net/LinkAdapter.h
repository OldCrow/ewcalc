#pragma once
// LinkAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/link_presenter.h"

namespace EwPresenterNet {

public value struct LinkOutput {
    System::String^ ReceivedPowerStr;
    System::String^ PathLossStr;
    System::String^ FresnelZoneStr;
    System::String^ RegimeStr;
    System::String^ EffectiveRangeStr;
    System::String^ RangeRegimeStr;
    System::String^ LinkMarginStr;
    bool Valid;
};

public delegate void LinkChangedHandler(LinkOutput output);

public ref class LinkAdapter sealed {
public:
    LinkAdapter();
    ~LinkAdapter();
    !LinkAdapter();

    void SetTxPower      (double dbm)    { presenter_->set_tx_power(RoundInput(dbm));       }
    void SetTxGain       (double db)     { presenter_->set_tx_gain(RoundInput(db));         }
    void SetRxGain       (double db)     { presenter_->set_rx_gain(RoundInput(db));         }
    void SetDistance     (double km)     { presenter_->set_distance(RoundInput(km));        }
    void SetTxHeight     (double meters) { presenter_->set_tx_height(RoundInput(meters));   }
    void SetRxHeight     (double meters) { presenter_->set_rx_height(RoundInput(meters));   }
    void SetFrequency    (double mhz)    { presenter_->set_frequency(RoundInput(mhz));      }
    void SetRxSensitivity(double dbm)    { presenter_->set_rx_sensitivity(RoundInput(dbm)); }

    property double DefaultTxPower       { double get() { return presenter_->tx_power_dbm();       } }
    property double DefaultTxGain        { double get() { return presenter_->tx_gain_db();         } }
    property double DefaultRxGain        { double get() { return presenter_->rx_gain_db();         } }
    property double DefaultDistance      { double get() { return presenter_->distance_km();        } }
    property double DefaultTxHeight      { double get() { return presenter_->tx_height_m();        } }
    property double DefaultRxHeight      { double get() { return presenter_->rx_height_m();        } }
    property double DefaultFrequency     { double get() { return presenter_->frequency_mhz();      } }
    property double DefaultRxSensitivity { double get() { return presenter_->rx_sensitivity_dbm(); } }

    property FieldValidationError TxPowerError       { FieldValidationError get() { return ToManaged(presenter_->tx_power_error());       } }
    property FieldValidationError TxGainError        { FieldValidationError get() { return ToManaged(presenter_->tx_gain_error());        } }
    property FieldValidationError RxGainError        { FieldValidationError get() { return ToManaged(presenter_->rx_gain_error());        } }
    property FieldValidationError DistanceError      { FieldValidationError get() { return ToManaged(presenter_->distance_error());       } }
    property FieldValidationError TxHeightError      { FieldValidationError get() { return ToManaged(presenter_->tx_height_error());      } }
    property FieldValidationError RxHeightError      { FieldValidationError get() { return ToManaged(presenter_->rx_height_error());      } }
    property FieldValidationError FrequencyError     { FieldValidationError get() { return ToManaged(presenter_->frequency_error());      } }
    property FieldValidationError RxSensitivityError { FieldValidationError get() { return ToManaged(presenter_->rx_sensitivity_error()); } }

    property LinkOutput CurrentOutput { LinkOutput get() { return Snapshot(presenter_->output()); } }

    event LinkChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::LinkPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::LinkPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static LinkOutput Snapshot(const ewpresenter::LinkPresenter::Output& o) {
        LinkOutput s{};
        s.ReceivedPowerStr  = ToManaged(o.received_power_str);
        s.PathLossStr       = ToManaged(o.path_loss_str);
        s.FresnelZoneStr    = ToManaged(o.fresnel_zone_str);
        s.RegimeStr         = ToManaged(o.regime_str);
        s.EffectiveRangeStr = ToManaged(o.effective_range_str);
        s.RangeRegimeStr    = ToManaged(o.range_regime_str);
        s.LinkMarginStr     = ToManaged(o.link_margin_str);
        s.Valid             = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
