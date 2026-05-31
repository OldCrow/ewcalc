// Helpers/FieldErrorConverter.cs
using EwPresenterNet;
using Microsoft.UI;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Media;
using System;

namespace EwCalc.Helpers;

public sealed class FieldErrorToBrushConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, string language)
    {
        if (value is FieldValidationError err && err != FieldValidationError.None)
        {
            if (Application.Current.Resources.TryGetValue("SystemFillColorCriticalBrush", out var brush))
                return brush;
            return new SolidColorBrush(Colors.Red);
        }
        return new SolidColorBrush(Colors.Transparent);
    }
    public object ConvertBack(object value, Type targetType, object parameter, string language)
        => throw new NotSupportedException();
}

public sealed class FieldErrorToTooltipConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, string language)
    {
        if (value is FieldValidationError err && err != FieldValidationError.None)
        {
            return err switch
            {
                FieldValidationError.BelowMinimum    => "Value is below the minimum",
                FieldValidationError.AboveMaximum    => "Value exceeds the maximum",
                FieldValidationError.InvalidZero     => "Value must be positive",
                FieldValidationError.InvalidNegative => "Value must not be negative",
                _                                    => string.Empty,
            };
        }
        return string.Empty;
    }
    public object ConvertBack(object value, Type targetType, object parameter, string language)
        => throw new NotSupportedException();
}

public sealed class RegimeConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, string language)
        => value is bool b && b ? "2-ray" : "LOS";
    public object ConvertBack(object value, Type targetType, object parameter, string language)
        => throw new NotSupportedException();
}
