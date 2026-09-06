// DetectionAdapter.cpp
// C4679: static virtual interface members (IDivisionOperators etc. on System::IntPtr)
// are not importable by C++/CLI; the affected interfaces are unused here.
#pragma warning(disable: 4679)
#include "DetectionAdapter.h"

#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

namespace EwPresenterNet {

static void DetectionDispatch(void* cookie, const ewpresenter::DetectionPresenter::Output* out) {
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(System::IntPtr(cookie));
    if (!h.IsAllocated) return;
    auto adapter = safe_cast<DetectionAdapter^>(h.Target);
    if (adapter != nullptr) adapter->FireChanged(*out);
}

DetectionAdapter::DetectionAdapter() : presenter_(new ewpresenter::DetectionPresenter()) {
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    try {
        void* cookie = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
        presenter_->set_on_change(MakeDetectionCB(&DetectionDispatch, cookie));
    } catch (...) {
        if (handle_.IsAllocated) handle_.Free();
        throw;
    }
}

DetectionAdapter::~DetectionAdapter() {
    if (presenter_) presenter_->set_on_change(nullptr);
    delete presenter_; presenter_ = nullptr;
    if (handle_.IsAllocated) handle_.Free();
    System::GC::SuppressFinalize(this);
}
DetectionAdapter::!DetectionAdapter() { if (presenter_) presenter_->set_on_change(nullptr); delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }

} // namespace EwPresenterNet
