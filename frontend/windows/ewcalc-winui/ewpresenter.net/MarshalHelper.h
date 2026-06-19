#pragma once
// MarshalHelper.h

#include "ewpresenter/validation.h"
#include <string>
#include <cmath>

namespace EwPresenterNet {

public enum class FieldValidationError : System::Byte {
    None            = 0,
    BelowMinimum    = 1,
    AboveMaximum    = 2,
    InvalidZero     = 3,
    InvalidNegative = 4,
    NotFinite       = 5,
};

inline FieldValidationError ToManaged(ewpresenter::FieldError e) {
    switch (e) {
        case ewpresenter::FieldError::none:             return FieldValidationError::None;
        case ewpresenter::FieldError::below_minimum:    return FieldValidationError::BelowMinimum;
        case ewpresenter::FieldError::above_maximum:    return FieldValidationError::AboveMaximum;
        case ewpresenter::FieldError::invalid_zero:     return FieldValidationError::InvalidZero;
        case ewpresenter::FieldError::invalid_negative: return FieldValidationError::InvalidNegative;
        case ewpresenter::FieldError::not_finite:        return FieldValidationError::NotFinite;
    }
    return FieldValidationError::None;
}

/// Marshal a UTF-8 std::string to a managed System::String^.
/// marshal_as uses the ANSI code page (Windows-1252) which garbles multi-byte
/// UTF-8 sequences. Explicit UTF-8 decoding is required.
inline System::String^ ToManaged(const std::string& s) {
    if (s.empty()) return System::String::Empty;
    auto len   = static_cast<int>(s.size());
    auto bytes = gcnew array<System::Byte>(len);
    for (int i = 0; i < len; ++i)
        bytes[i] = static_cast<System::Byte>(static_cast<unsigned char>(s[i]));
    return System::Text::Encoding::UTF8->GetString(bytes);
}

/// Round a value to 6 significant figures to eliminate NumberBox float
/// precision artefacts (e.g. minimum=0.01 stored as 0.009999999776...).
inline double RoundInput(double v) noexcept {
    if (v == 0.0) return 0.0;
    const double mag = std::pow(10.0, 6.0 - std::floor(std::log10(std::abs(v))) - 1.0);
    return std::round(v * mag) / mag;
}

} // namespace EwPresenterNet
