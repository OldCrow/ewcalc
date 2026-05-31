#pragma once
// ReceiverAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/receiver_presenter.h"
#include <vector>

namespace EwPresenterNet {

public value struct StageInput { double NoiseFigureDb; double GainDb; };

public value struct ReceiverOutput {
    System::String^ SensitivityStr;
    System::String^ CascadedNfStr;
    System::String^ Sfdr2Str;
    System::String^ Sfdr3Str;
    System::String^ DigitalDrStr;
    bool Valid;
};

public delegate void ReceiverChangedHandler(ReceiverOutput output);

public ref class ReceiverAdapter sealed {
public:
    ReceiverAdapter();
    ~ReceiverAdapter();
    !ReceiverAdapter();

    void SetBandwidth    (double mhz) { presenter_->set_bandwidth(RoundInput(mhz));       }
    void SetNoiseFigure  (double db)  { presenter_->set_noise_figure(RoundInput(db));     }
    void SetRequiredSnr  (double db)  { presenter_->set_required_snr(RoundInput(db));     }
    void SetSecondOrderIp(double dbm) { presenter_->set_second_order_ip(RoundInput(dbm)); }
    void SetThirdOrderIp (double dbm) { presenter_->set_third_order_ip(RoundInput(dbm));  }
    void SetAdcBits      (int bits)   { presenter_->set_adc_bits(bits);       }

    void SetStages(array<StageInput>^ stages) {
        std::vector<ewpresenter::ReceiverPresenter::StageInput> v;
        v.reserve(stages->Length);
        for each (auto s in stages) v.push_back({ s.NoiseFigureDb, s.GainDb });
        presenter_->set_stages(std::move(v));
    }

    array<StageInput>^ GetStages() {
        const auto& v = presenter_->stages();
        auto arr = gcnew array<StageInput>(static_cast<int>(v.size()));
        for (int i = 0; i < static_cast<int>(v.size()); ++i) {
            arr[i].NoiseFigureDb = v[i].noise_figure_db;
            arr[i].GainDb        = v[i].gain_db;
        }
        return arr;
    }

    property int    StageCount           { int    get() { return static_cast<int>(presenter_->stages().size()); } }
    property double DefaultBandwidth     { double get() { return presenter_->bandwidth_mhz();       } }
    property double DefaultNoiseFigure   { double get() { return presenter_->noise_figure_db();     } }
    property double DefaultRequiredSnr   { double get() { return presenter_->required_snr_db();     } }
    property double DefaultSecondOrderIp { double get() { return presenter_->second_order_ip_dbm(); } }
    property double DefaultThirdOrderIp  { double get() { return presenter_->third_order_ip_dbm();  } }
    property int    DefaultAdcBits       { int    get() { return presenter_->adc_bits();            } }

    property FieldValidationError BandwidthError     { FieldValidationError get() { return ToManaged(presenter_->bandwidth_error());       } }
    property FieldValidationError NoiseFigureError   { FieldValidationError get() { return ToManaged(presenter_->noise_figure_error());    } }
    property FieldValidationError RequiredSnrError   { FieldValidationError get() { return ToManaged(presenter_->required_snr_error());    } }
    property FieldValidationError SecondOrderIpError { FieldValidationError get() { return ToManaged(presenter_->second_order_ip_error()); } }
    property FieldValidationError ThirdOrderIpError  { FieldValidationError get() { return ToManaged(presenter_->third_order_ip_error());  } }

    property ReceiverOutput CurrentOutput { ReceiverOutput get() { return Snapshot(presenter_->output()); } }

    event ReceiverChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::ReceiverPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::ReceiverPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static ReceiverOutput Snapshot(const ewpresenter::ReceiverPresenter::Output& o) {
        ReceiverOutput s{};
        s.SensitivityStr = ToManaged(o.sensitivity_str);
        s.CascadedNfStr  = ToManaged(o.cascaded_nf_str);
        s.Sfdr2Str       = ToManaged(o.sfdr2_str);
        s.Sfdr3Str       = ToManaged(o.sfdr3_str);
        s.DigitalDrStr   = ToManaged(o.digital_dr_str);
        s.Valid          = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
