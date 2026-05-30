#pragma once

/// @file validation.h
/// @brief Input validation types for the ewpresenter layer.
///
/// Presenters store raw double inputs and validate them against physical bounds
/// before passing them to libew. No exceptions are thrown; all errors are
/// expressed as FieldError values.

#include <string_view>

namespace ewpresenter {

/// Reason an input field failed validation.
enum class FieldError : uint8_t {
    none,           ///< Value is valid
    below_minimum,  ///< Value is less than the physical minimum for this field
    above_maximum,  ///< Value exceeds the physical maximum for this field
    invalid_zero,   ///< Value is zero where it must be strictly positive
    invalid_negative ///< Value is negative where it must be non-negative
};

/// Returns a short human-readable label for a FieldError.
[[nodiscard]] constexpr std::string_view describe(FieldError e) noexcept {
    switch (e) {
        case FieldError::none:             return "";
        case FieldError::below_minimum:    return "below minimum";
        case FieldError::above_maximum:    return "above maximum";
    case FieldError::invalid_zero:     return "must be positive";
        case FieldError::invalid_negative: return "must not be negative";
    }
    return "unknown";
}

/// Validate a value against optional lower and upper bounds.
/// Pass 0 for min_val to skip lower-bound check on strictly-positive fields;
/// use validate_positive() for zero-exclusion.
[[nodiscard]] constexpr FieldError validate_bounds(
    double value, double min_val, double max_val) noexcept
{
    if (value < min_val) return FieldError::below_minimum;
    if (value > max_val) return FieldError::above_maximum;
    return FieldError::none;
}

/// Validate that a value is strictly positive.
[[nodiscard]] constexpr FieldError validate_positive(double value) noexcept {
    if (value <= 0.0) return FieldError::invalid_zero;
    return FieldError::none;
}

/// Validate that a value is non-negative.
[[nodiscard]] constexpr FieldError validate_non_negative(double value) noexcept {
    if (value < 0.0) return FieldError::invalid_negative;
    return FieldError::none;
}

/// Convenience: validate positive AND within bounds.
[[nodiscard]] constexpr FieldError validate_positive_bounded(
    double value, double min_val, double max_val) noexcept
{
    if (value <= 0.0) return FieldError::invalid_zero;
    return validate_bounds(value, min_val, max_val);
}

} // namespace ewpresenter
