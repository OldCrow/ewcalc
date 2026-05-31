#pragma once
// MarshalHelper.h

#include "ewpresenter/validation.h"
#include <string>
#include <msclr/marshal_cppstd.h>

namespace EwPresenterNet {

public enum class FieldValidationError : System::Byte {
    None            = 0,
    BelowMinimum    = 1,
    AboveMaximum    = 2,
    InvalidZero     = 3,
    InvalidNegative = 4,
};

inline FieldValidationError ToManaged(ewpresenter::FieldError e) {
    switch (e) {
        case ewpresenter::FieldError::none:             return FieldValidationError::None;
        case ewpresenter::FieldError::below_minimum:    return FieldValidationError::BelowMinimum;
        case ewpresenter::FieldError::above_maximum:    return FieldValidationError::AboveMaximum;
        case ewpresenter::FieldError::invalid_zero:     return FieldValidationError::InvalidZero;
        case ewpresenter::FieldError::invalid_negative: return FieldValidationError::InvalidNegative;
    }
    return FieldValidationError::None;
}

inline System::String^ ToManaged(const std::string& s) {
    return msclr::interop::marshal_as<System::String^>(s);
}

} // namespace EwPresenterNet
