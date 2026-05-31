// RadarAdapter.cpp
#include "RadarAdapter.h"

#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

namespace EwPresenterNet {

static void RadarDispatch(void* cookie, const ewpresenter::RadarPresenter::Output* out) {
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(System::IntPtr(cookie));
    safe_cast<RadarAdapter^>(h.Target)->FireChanged(*out);
}

RadarAdapter::RadarAdapter() : presenter_(new ewpresenter::RadarPresenter()) {
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    void* cookie = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
    presenter_->set_on_change(MakeRadarCB(&RadarDispatch, cookie));
}

RadarAdapter::~RadarAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }
RadarAdapter::!RadarAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }

} // namespace EwPresenterNet
