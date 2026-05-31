// PropagationAdapter.cpp
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
    auto h    = System::Runtime::InteropServices::GCHandle::FromIntPtr(
                    System::IntPtr(cookie));
    auto self = safe_cast<PropagationAdapter^>(h.Target);
    self->FireChanged(*out);
}

PropagationAdapter::PropagationAdapter()
    : presenter_(new ewpresenter::PropagationPresenter())
{
    handle_ = System::Runtime::InteropServices::GCHandle::Alloc(this);
    void* cookie =
        System::Runtime::InteropServices::GCHandle::ToIntPtr(handle_).ToPointer();
    presenter_->set_on_change(MakePropagationCB(&PropagationDispatch, cookie));
}

PropagationAdapter::~PropagationAdapter() {
    delete presenter_; presenter_ = nullptr;
    if (handle_.IsAllocated) handle_.Free();
}

PropagationAdapter::!PropagationAdapter() {
    delete presenter_; presenter_ = nullptr;
    if (handle_.IsAllocated) handle_.Free();
}

} // namespace EwPresenterNet
