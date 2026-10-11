#pragma once
// RadarJammingAdapter.h
#include "MarshalHelper.h"
#include "ewpresenter/radar_jamming_presenter.h"

namespace EwPresenterNet {

public value struct RadarJammingOutput {
    System::String^ JsRatioStr;
    System::String^ JsMarginStr;
    System::String^ BurnthroughRangeStr;
    System::String^ StatusStr;
    bool JammerEffective;
    bool Valid;
};

public delegate void RadarJammingChangedHandler(RadarJammingOutput output);

public ref class RadarJammingAdapter sealed {
public:
    RadarJammingAdapter();
    ~RadarJammingAdapter();
    !RadarJammingAdapter();

    // Geometry: 0 = self-protection, 1 = stand-off (presenter kSelfProtection / kStandOff).
    void SetGeometry     (int g)        { presenter_->set_geometry(g);        }
    void SetRadarErp     (double dbm)   { presenter_->set_radar_erp(dbm);     }
    void SetJammerErp    (double dbm)   { presenter_->set_jammer_erp(dbm);    }
    void SetTargetRcs    (double dbsm)  { presenter_->set_target_rcs(dbsm);   }
    void SetTargetRange  (double km)    { presenter_->set_target_range(km);   }
    void SetJammerRange  (double km)    { presenter_->set_jammer_range(km);   }
    void SetMainbeamGain (double db)    { presenter_->set_mainbeam_gain(db);  }
    void SetSidelobeGain (double db)    { presenter_->set_sidelobe_gain(db);  }
    void SetJsThreshold  (double db)    { presenter_->set_js_threshold(db);   }

    property int    Geometry            { int    get() { return presenter_->geometry();          } }
    property double DefaultRadarErp     { double get() { return presenter_->radar_erp_dbm();     } }
    property double DefaultJammerErp    { double get() { return presenter_->jammer_erp_dbm();    } }
    property double DefaultTargetRcs    { double get() { return presenter_->target_rcs_dbsm();   } }
    property double DefaultTargetRange  { double get() { return presenter_->target_range_km();   } }
    property double DefaultJammerRange  { double get() { return presenter_->jammer_range_km();   } }
    property double DefaultMainbeamGain { double get() { return presenter_->mainbeam_gain_db();  } }
    property double DefaultSidelobeGain { double get() { return presenter_->sidelobe_gain_db();  } }
    property double DefaultJsThreshold  { double get() { return presenter_->js_threshold_db();   } }

    property FieldValidationError GeometryError      { FieldValidationError get() { return ToManaged(presenter_->geometry_error());      } }
    property FieldValidationError RadarErpError      { FieldValidationError get() { return ToManaged(presenter_->radar_erp_error());     } }
    property FieldValidationError JammerErpError     { FieldValidationError get() { return ToManaged(presenter_->jammer_erp_error());    } }
    property FieldValidationError TargetRcsError     { FieldValidationError get() { return ToManaged(presenter_->target_rcs_error());    } }
    property FieldValidationError TargetRangeError   { FieldValidationError get() { return ToManaged(presenter_->target_range_error());  } }
    property FieldValidationError JammerRangeError   { FieldValidationError get() { return ToManaged(presenter_->jammer_range_error());  } }
    property FieldValidationError MainbeamGainError  { FieldValidationError get() { return ToManaged(presenter_->mainbeam_gain_error()); } }
    property FieldValidationError SidelobeGainError  { FieldValidationError get() { return ToManaged(presenter_->sidelobe_gain_error()); } }
    property FieldValidationError JsThresholdError   { FieldValidationError get() { return ToManaged(presenter_->js_threshold_error());  } }

    property RadarJammingOutput CurrentOutput { RadarJammingOutput get() { return Snapshot(presenter_->output()); } }

    event RadarJammingChangedHandler^ Changed;

internal:
    void FireChanged(const ewpresenter::RadarJammingPresenter::Output& o) { Changed(Snapshot(o)); }

private:
    ewpresenter::RadarJammingPresenter* presenter_;
    System::Runtime::InteropServices::GCHandle handle_;

    static RadarJammingOutput Snapshot(const ewpresenter::RadarJammingPresenter::Output& o) {
        RadarJammingOutput s{};
        s.JsRatioStr           = ToManaged(o.js_ratio_str);
        s.JsMarginStr          = ToManaged(o.js_margin_str);
        s.BurnthroughRangeStr  = ToManaged(o.burnthrough_range_str);
        s.StatusStr            = ToManaged(o.status_str);
        s.JammerEffective      = o.jammer_effective;
        s.Valid                = o.valid;
        return s;
    }
};

} // namespace EwPresenterNet
