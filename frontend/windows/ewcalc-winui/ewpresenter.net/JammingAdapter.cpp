// JammingAdapter.cpp
#include "JammingAdapter.h"

#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

namespace EwPresenterNet {

static void JammingDispatch(void* cookie, const ewpresenter::JammingPresenter::Output* out) {
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(System::IntPtr(cookie));
    safe_cast<JammingAdapter^>(h.Target)->FireChanged(*out);
}

JammingAdapter::JammingAdapter() : presenter_(new ewpresenter::JammingPresenter()) {
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    void* cookie = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
    presenter_->set_on_change(MakeJammingCB(&JammingDispatch, cookie));
}

JammingAdapter::~JammingAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }
JammingAdapter::!JammingAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }

} // namespace EwPresenterNet
