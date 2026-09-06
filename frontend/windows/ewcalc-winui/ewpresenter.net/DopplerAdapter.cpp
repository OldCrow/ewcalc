// DopplerAdapter.cpp
// C4679: static virtual interface members (IDivisionOperators etc. on System::IntPtr)
// are not importable by C++/CLI; the affected interfaces are unused here.
#pragma warning(disable: 4679)
#include "DopplerAdapter.h"

#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

namespace EwPresenterNet {

static void DopplerDispatch(void* cookie, const ewpresenter::DopplerPresenter::Output* out) {
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(System::IntPtr(cookie));
    if (!h.IsAllocated) return;
    auto adapter = safe_cast<DopplerAdapter^>(h.Target);
    if (adapter != nullptr) adapter->FireChanged(*out);
}

DopplerAdapter::DopplerAdapter() : presenter_(new ewpresenter::DopplerPresenter()) {
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    try {
        void* cookie = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
        presenter_->set_on_change(MakeDopplerCB(&DopplerDispatch, cookie));
    } catch (...) {
        if (handle_.IsAllocated) handle_.Free();
        throw;
    }
}

DopplerAdapter::~DopplerAdapter() {
    if (presenter_) presenter_->set_on_change(nullptr);
    delete presenter_; presenter_ = nullptr;
    if (handle_.IsAllocated) handle_.Free();
    System::GC::SuppressFinalize(this);
}
DopplerAdapter::!DopplerAdapter() { if (presenter_) presenter_->set_on_change(nullptr); delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }

} // namespace EwPresenterNet
