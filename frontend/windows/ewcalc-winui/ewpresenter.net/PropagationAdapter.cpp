// PropagationAdapter.cpp
// C4679: static virtual interface members (IDivisionOperators etc. on System::IntPtr)
// are not importable by C++/CLI; the affected interfaces are unused here.
#pragma warning(disable: 4679)
#include "PropagationAdapter.h"

// Include NativeCallbacks.h in a native-only section so its lambdas
// (which capture only void* and fn ptr) compile without managed context.
#pragma managed(push, off)
#include "NativeCallbacks.h"
#pragma managed(pop)

// Everything below is managed context — GCHandle, safe_cast etc. are legal here.
namespace EwPresenterNet {

static void PropagationDispatch(
    void* cookie, const ewpresenter::PropagationPresenter::Output* out)
{
    auto h = System::Runtime::InteropServices::GCHandle::FromIntPtr(
                 System::IntPtr(cookie));
    if (!h.IsAllocated) return;
    auto self = safe_cast<PropagationAdapter^>(h.Target);
    if (self != nullptr) self->FireChanged(*out);
}

PropagationAdapter::PropagationAdapter()
    : presenter_(new ewpresenter::PropagationPresenter())
{
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    try {
        void* cookie =
            System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
        presenter_->set_on_change(MakePropagationCB(&PropagationDispatch, cookie));
    } catch (...) {
        if (handle_.IsAllocated) handle_.Free();
        throw;
    }
}

PropagationAdapter::~PropagationAdapter() {
    if (presenter_) presenter_->set_on_change(nullptr);
    delete presenter_; presenter_ = nullptr;
    if (handle_.IsAllocated) handle_.Free();
    System::GC::SuppressFinalize(this);
}

PropagationAdapter::!PropagationAdapter() {
    if (presenter_) presenter_->set_on_change(nullptr);
    delete presenter_; presenter_ = nullptr;
    if (handle_.IsAllocated) handle_.Free();
}

} // namespace EwPresenterNet
