// NativeCallbacks.h
// Purely native header — zero managed types.
// Defines the factory functions that create std::function callbacks
// wrapping a plain C function pointer + void* cookie.
// The lambda inside each factory captures only native types (void*, fn ptr).

#pragma once
#include <functional>
#include "ewpresenter/propagation_presenter.h"
#include "ewpresenter/link_presenter.h"
#include "ewpresenter/receiver_presenter.h"
#include "ewpresenter/jamming_presenter.h"
#include "ewpresenter/location_presenter.h"
#include "ewpresenter/radar_presenter.h"
#include "ewpresenter/digital_presenter.h"

// One typedef per presenter output type.
typedef void (*PropagationCB)(void*, const ewpresenter::PropagationPresenter::Output*);
typedef void (*LinkCB)       (void*, const ewpresenter::LinkPresenter::Output*);
typedef void (*ReceiverCB)   (void*, const ewpresenter::ReceiverPresenter::Output*);
typedef void (*JammingCB)    (void*, const ewpresenter::JammingPresenter::Output*);
typedef void (*LocationCB)   (void*, const ewpresenter::LocationPresenter::Output*);
typedef void (*RadarCB)      (void*, const ewpresenter::RadarPresenter::Output*);
typedef void (*DigitalCB)    (void*, const ewpresenter::DigitalPresenter::Output*);

// Factory: returns a std::function<> that calls fn(cookie, &output).
// The lambda captures only native types — safe inside #pragma managed(off).
inline std::function<void(const ewpresenter::PropagationPresenter::Output&)>
MakePropagationCB(PropagationCB fn, void* cookie) {
    return [fn, cookie](const ewpresenter::PropagationPresenter::Output& o) { fn(cookie, &o); };
}

inline std::function<void(const ewpresenter::LinkPresenter::Output&)>
MakeLinkCB(LinkCB fn, void* cookie) {
    return [fn, cookie](const ewpresenter::LinkPresenter::Output& o) { fn(cookie, &o); };
}

inline std::function<void(const ewpresenter::ReceiverPresenter::Output&)>
MakeReceiverCB(ReceiverCB fn, void* cookie) {
    return [fn, cookie](const ewpresenter::ReceiverPresenter::Output& o) { fn(cookie, &o); };
}

inline std::function<void(const ewpresenter::JammingPresenter::Output&)>
MakeJammingCB(JammingCB fn, void* cookie) {
    return [fn, cookie](const ewpresenter::JammingPresenter::Output& o) { fn(cookie, &o); };
}

inline std::function<void(const ewpresenter::LocationPresenter::Output&)>
MakeLocationCB(LocationCB fn, void* cookie) {
    return [fn, cookie](const ewpresenter::LocationPresenter::Output& o) { fn(cookie, &o); };
}

inline std::function<void(const ewpresenter::RadarPresenter::Output&)>
MakeRadarCB(RadarCB fn, void* cookie) {
    return [fn, cookie](const ewpresenter::RadarPresenter::Output& o) { fn(cookie, &o); };
}

inline std::function<void(const ewpresenter::DigitalPresenter::Output&)>
MakeDigitalCB(DigitalCB fn, void* cookie) {
    return [fn, cookie](const ewpresenter::DigitalPresenter::Output& o) { fn(cookie, &o); };
}
