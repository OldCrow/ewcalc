#pragma once
// AntennaAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/antenna_presenter.h"

namespace EwPresenterNet {

public value struct AntennaOutput {
    System::String^ ErpStr;
    System::String^ BeamwidthFromGainStr;
    System::String^ GainFromBeamwidthStr;
    System::String^ WavelengthStr;
    bool Valid;
};

public delegate void AntennaChangedHandler(AntennaOutput output);

public ref class AntennaAdapter sealed {
public:
    AntennaAdapter();
    ~AntennaAdapter();
    !AntennaAdapter();

    void SetGain        (double dbi) { presenter_->set_gain(RoundInput(dbi));          }
    void SetAzBeamwidth (double deg) { presenter_->set_az_beamwidth(RoundInput(deg));  }
    void SetElBeamwidth (double deg) { presenter_->set_el_beamwidth(RoundInput(deg));  }
    void SetTxPower     (double dbm) { presenter_->set_tx_power(RoundInput(dbm));      }
    void SetFrequency   (double mhz) { presenter_->set_frequency(RoundInput(mhz));     }

    property double DefaultGain        { double get() { return presenter_->gain_dbi();          } }
    property double DefaultAzBeamwidth { double get() { return presenter_->az_beamwidth_deg();  } }
    property double DefaultElBeamwidth { double get() { return presenter_->el_beamwidth_deg();  } }
    property double DefaultTxPower     { double get() { return presenter_->tx_power_dbm();      } }
    property double DefaultFrequency   { double get() { return presenter_->frequency_mhz();     } }

    property AntennaOutput CurrentOutput { AntennaOutput get() { return Snapshot(presenter_->output()); } }

    event AntennaChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::AntennaPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::AntennaPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static AntennaOutput Snapshot(const ewpresenter::AntennaPresenter::Output& o) {
        AntennaOutput s{};
        s.ErpStr                = ToManaged(o.erp_str);
        s.BeamwidthFromGainStr  = ToManaged(o.beamwidth_from_gain_str);
        s.GainFromBeamwidthStr  = ToManaged(o.gain_from_beamwidth_str);
        s.WavelengthStr         = ToManaged(o.wavelength_str);
        s.Valid                 = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
