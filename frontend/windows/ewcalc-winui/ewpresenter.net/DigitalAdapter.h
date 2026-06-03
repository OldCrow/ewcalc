#pragma once
// DigitalAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/digital_presenter.h"

namespace EwPresenterNet {

public value struct DigitalOutput {
    System::String^ EbNoStr;
    System::String^ ProcessGainStr;
    System::String^ JammingMarginStr;
    System::String^ RequiredJsStr;
    bool Valid;
};

public delegate void DigitalChangedHandler(DigitalOutput output);

public ref class DigitalAdapter sealed {
public:
    DigitalAdapter();
    ~DigitalAdapter();
    !DigitalAdapter();

    void SetSnr               (double db)   { presenter_->set_snr(RoundInput(db));                      }
    void SetBandwidth         (double mhz)  { presenter_->set_bandwidth(RoundInput(mhz));               }
    void SetDataRate          (double mbps) { presenter_->set_data_rate(RoundInput(mbps));              }
    void SetChipRate          (double mcps) { presenter_->set_chip_rate(RoundInput(mcps));              }
    void SetRequiredEbNo      (double db)   { presenter_->set_required_eb_no(RoundInput(db));           }
    void SetImplementationLoss(double db)   { presenter_->set_implementation_loss(RoundInput(db));      }

    property double DefaultSnr                { double get() { return presenter_->snr_db();                   } }
    property double DefaultBandwidth          { double get() { return presenter_->bandwidth_mhz();            } }
    property double DefaultDataRate           { double get() { return presenter_->data_rate_mhz();            } }
    property double DefaultChipRate           { double get() { return presenter_->chip_rate_mhz();            } }
    property double DefaultRequiredEbNo       { double get() { return presenter_->required_eb_no_db();        } }
    property double DefaultImplementationLoss { double get() { return presenter_->implementation_loss_db();   } }

    property DigitalOutput CurrentOutput { DigitalOutput get() { return Snapshot(presenter_->output()); } }

    event DigitalChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::DigitalPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::DigitalPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static DigitalOutput Snapshot(const ewpresenter::DigitalPresenter::Output& o) {
        DigitalOutput s{};
        s.EbNoStr          = ToManaged(o.eb_no_str);
        s.ProcessGainStr   = ToManaged(o.process_gain_str);
        s.JammingMarginStr = ToManaged(o.jamming_margin_str);
        s.RequiredJsStr    = ToManaged(o.required_js_str);
        s.Valid            = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
