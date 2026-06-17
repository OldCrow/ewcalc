#pragma once

/// @file presenter_base.h
/// @brief CRTP base providing update_field() for ewpresenter scalar setters.
///
/// Every standard scalar setter follows the same four-step pattern:
///   1. Store the raw value.
///   2. Validate and record the FieldError.
///   3. Recompute derived outputs.
///   4. Fire the on_change callback.
///
/// PresenterBase<Derived> collapses those four steps into a single
/// update_field() call, eliminating ~200 lines of mechanical repetition
/// across the eight presenters.
///
/// Usage:
/// @code
///   class MyPresenter : public PresenterBase<MyPresenter> {
///     ...
///     void set_gain(double dbi) noexcept {
///         update_field(dbi, gain_dbi_, gain_err_,
///                      validate_bounds(dbi, -10.0, 60.0));
///     }
///   private:
///     void recompute() noexcept;  // required by PresenterBase
///     void fire() noexcept;       // required by PresenterBase
///   };
/// @endcode

#include "ewpresenter/validation.h"

namespace ewpresenter {

/// CRTP base for all ewpresenter classes.
///
/// @tparam Derived  Concrete presenter class. Must implement:
///                  - void recompute() noexcept
///                  - void fire() noexcept
template<typename Derived>
class PresenterBase {
protected:
    /// Assign @p value and @p validated to the referenced members, then
    /// trigger recompute and the on_change callback.
    ///
    /// The validation result is computed by the caller (at the call site
    /// the validate_* call is inlined by the compiler, so there is zero
    /// runtime overhead compared to the expanded four-line form).
    void update_field(double value, double& storage, FieldError& error,
                      FieldError validated) noexcept {
        storage = value;
        error   = validated;
        static_cast<Derived*>(this)->recompute();
        static_cast<Derived*>(this)->fire();
    }
};

} // namespace ewpresenter
