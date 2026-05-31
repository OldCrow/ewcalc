// LinkAdapter.cpp
// C4679: static virtual interface members (IDivisionOperators etc. on System::IntPtr)
// are not importable by C++/CLI; the affected interfaces are unused here.
#pragma warning(disable: 4679)
#include "LinkAdapter.h"

#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

namespace EwPresenterNet {

static void LinkDispatch(void* cookie, const ewpresenter::LinkPresenter::Output* out) {
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(System::IntPtr(cookie));
    safe_cast<LinkAdapter^>(h.Target)->FireChanged(*out);
}

LinkAdapter::LinkAdapter() : presenter_(new ewpresenter::LinkPresenter()) {
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    void* cookie = System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
    presenter_->set_on_change(MakeLinkCB(&LinkDispatch, cookie));
}

LinkAdapter::~LinkAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }
LinkAdapter::!LinkAdapter()  { delete presenter_; presenter_ = nullptr; if (handle_.IsAllocated) handle_.Free(); }

} // namespace EwPresenterNet
