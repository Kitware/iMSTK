/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"

#include <iostream>

/// \todo remove nameless union/struct in the future
#ifdef WIN32
#pragma warning(disable : 4201)
#endif
namespace imstk
{
///
/// \struct Color
///
/// \brief Color in RGB space
///
struct Color
{
    union
    {
        double rgba[4];
        struct
        {
            double r;
            double g;
            double b;
            double a;
        };
    };

    Color();
    Color(const double red,
          const double green,
          const double blue,
          const double alpha = 1.0);
    Color(const double* rgba_color);
    Color(const Color& color, const double alpha);

    ///
    /// \brief Equality operator
    ///
    Color& operator=(const Color& p_color);

    operator Eigen::Matrix<unsigned char, 3, 1>()
    {
        return Eigen::Matrix<unsigned char, 3, 1>(
            static_cast<unsigned char>(r * 255.0),
            static_cast<unsigned char>(g * 255.0),
            static_cast<unsigned char>(b * 255.0));
    }
    operator Eigen::Matrix<unsigned char, 4, 1>()
    {
        return Eigen::Matrix<unsigned char, 4, 1>(
            static_cast<unsigned char>(r * 255.0),
            static_cast<unsigned char>(g * 255.0),
            static_cast<unsigned char>(b * 255.0),
            static_cast<unsigned char>(a * 255.0));
    }

    ///
    /// \brief Bitwise operator
    ///
    friend std ::ostream& operator<<(std::ostream& os, const Color& c);

    ///
    /// \brief returns the color value given with the index
    ///
    double operator()(const int p_i) const;

    ///
    /// \brief set RGB color
    ///
    void setValue(const double p_red,
                  const double p_green,
                  const double p_blue,
                  const double p_alpha = 1.0);

    ///
    /// \brief get RGB color
    ///
    void getValue(double color[4]);

    ///
    /// \brief get RGB color
    ///
    const double* getValue() const;

    ///
    /// \Get the RGB hex in string format
    ///
    std::string rgbHex();

    static Color darken(const Color color, const double factor);
    static Color lighten(const Color color, const double factor);

    static Color clamp(const Color color, const Color min, const Color max);

    ///
    /// \brief interpolate between two colors by ratio t
    ///
    static Color lerpRgba(const Color& start, const Color& end, const double t);
    static Color lerpRgb(const Color& start, const Color& end, const double t);

    /// Various commonly used colors
    static Color White;
    static Color Black;
    static Color DarkGray;
    static Color LightGray;
    static Color Blue;
    static Color Green;
    static Color Red;
    static Color Pink;
    static Color Orange;
    static Color Yellow;
    static Color Teal;
    static Color Marigold;
    static Color Bone;
    static Color YellowBone;
    static Color Blood;
    static Color LightSkin;
    static Color DarkSkin;
};
#ifdef WIN32
#pragma warning(default : 4201)
#endif

///
/// \brief Multiplication operators
///
Color operator*(const Color& color_lhs, const Color& color_rhs);
Color operator*(const Color& color_lhs, const double intensity_rhs);
Color operator*=(const Color& color_lhs, const Color& color_rhs);
Color operator*=(const Color& color_lhs, const double intensity_rhs);

///
/// \brief Addition operators
///
Color operator+(const Color& color_lhs, const Color& color_rhs);
Color operator+(const Color& intensity_lhs, const double intensity_rhs);
Color operator+=(const Color& color_lhs, const Color& color_rhs);
Color operator+=(const Color& intensity_lhs, const double intensity_rhs);

///
/// \brief Subtraction operators
///
Color operator-(const Color& color_lhs, const Color& color_rhs);
Color operator-(const Color& color_rhs, const double intensity_lhs);
Color operator-=(const Color& color_lhs, const Color& color_rhs);
Color operator-=(const Color& color_rhs, const double intensity_lhs);

///
/// \brief Comparison operator
///
bool operator==(const Color& color_lhs, const Color& color_rhs);
bool operator!=(const Color& color_lhs, const Color& color_rhs);
} // namespace imstk
