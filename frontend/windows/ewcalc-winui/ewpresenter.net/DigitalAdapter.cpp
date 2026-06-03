// DigitalAdapter.cpp
// C4679: static virtual interface members (IDivisionOperators etc. on System::IntPtr)
// are not importable by C++/CLI; the affected interfaces are unused here.
#pragma warning(disable: 4679)
#include "DigitalAdapter.h"

#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

namespace EwPresenterNet {

static void DigitalDispatch(void* cookie, const ewpresenter::DigitalPresenter::Output* out) {
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(System::IntPtr(cookie));
    safe_cast<DigitalAdapter^>(h.Target)->FireChanged(*out);
}

DigitalAdapter::DigitalAdapter() : presenter_(new ewpresenter::DigitalPresenter()) {
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    void* cookie = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
    presenter_->set_on_change(MakeDigitalCB(&DigitalDispatch, cookie));
}

DigitalAdapter::~DigitalAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }
DigitalAdapter::!DigitalAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }

} // namespace EwPresenterNet
