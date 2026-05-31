// CallbackBridge.h
// Pure native header — no managed types, no #include of any managed header.
// Included by both the managed adapter .cpp and the native callback .cpp.
//
// Pattern: each adapter allocates a CallbackBridge on the native heap.
// The bridge holds a function pointer (set by the managed side) and a cookie.
// The native presenter calls the bridge; the bridge calls back into managed
// code through the function pointer.

#pragma once
#include <functional>

// Forward-declare the native output types so this header stays pure-native.
namespace ewpresenter {
    struct PropagationPresenter; // not needed here, outputs are passed by ref below
}

// A plain C-style trampoline: native code calls OnOutput(cookie, ...).
// The managed adapter sets up OnOutput to be a static managed method address.
typedef void (*BridgeFn)(void* cookie, const void* output_ptr);

struct CallbackBridge {
    BridgeFn fn;
    void*    cookie;

    void Call(const void* output_ptr) const {
        if (fn) fn(cookie, output_ptr);
    }
};
