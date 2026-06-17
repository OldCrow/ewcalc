// LocationAdapter.cpp
// C4679: static virtual interface members (IDivisionOperators etc. on System::IntPtr)
// are not importable by C++/CLI; the affected interfaces are unused here.
#pragma warning(disable: 4679)
#include "LocationAdapter.h"

#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

namespace EwPresenterNet {

static void LocationDispatch(void* cookie, const ewpresenter::LocationPresenter::Output* out) {
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(System::IntPtr(cookie));
    if (!h.IsAllocated) return;
    auto adapter = safe_cast<LocationAdapter^>(h.Target);
    if (adapter != nullptr) adapter->FireChanged(*out);
}

LocationAdapter::LocationAdapter() : presenter_(new ewpresenter::LocationPresenter()) {
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    try {
        void* cookie = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
        presenter_->set_on_change(MakeLocationCB(&LocationDispatch, cookie));
    } catch (...) {
        if (handle_.IsAllocated) handle_.Free();
        throw;
    }
}

LocationAdapter::~LocationAdapter() {
    if (presenter_) presenter_->set_on_change(nullptr);
    delete presenter_; presenter_ = nullptr;
    if (handle_.IsAllocated) handle_.Free();
    System::GC::SuppressFinalize(this);
}
LocationAdapter::!LocationAdapter() { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }

} // namespace EwPresenterNet
