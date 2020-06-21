/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkColor.h"
#include "imstkLogger.h"
#include <iomanip>
#include <sstream>

namespace imstk
{
bool
isColorRangeCorrect(double c)
{
    return (c >= 0 && c <= 1.0);
}

Color Color::White(1.0, 1.0, 1.0, 1.0);
Color Color::Black(0.0, 0.0, 0.0, 1.0);
Color Color::DarkGray(0.3, 0.3, 0.3, 1.0);
Color Color::LightGray(0.8, 0.8, 0.8, 1.0);
Color Color::Blue(0.0, 0.0, 1.0, 1.0);
Color Color::Green(0.0, 1.0, 0.0, 1.0);
Color Color::Red(1.0, 0.0, 0.0, 1.0);
Color Color::Yellow(1.0, 1.0, 0.0, 1.0);
Color Color::Orange(1.0, 0.6, 0.0, 1.0);
Color Color::Pink(1.0, 0.0, 1.0, 1.0);
Color Color::Teal(0.5, 1.0, 0.8, 1.0);
Color Color::Marigold(0.9, 0.9, 0.4);

Color::Color()
{
    rgba[0] = 0.8f;
    rgba[1] = 0.8f;
    rgba[2] = 0.8f;
    rgba[3] = 1.0f;
}

Color::Color(const double r, const double g, const double b, const double a)
{
    setValue(r, g, b, a);
}

Color::Color(const double* rgba)
{
    setValue(rgba[0], rgba[1], rgba[2], rgba[3]);
}

Color::Color(const Color& color, const double a)
{
    setValue(color.r, color.g, color.b, a);
}

Color&
Color::operator=(const Color& p_color)
{
    rgba[0] = p_color.rgba[0];
    rgba[1] = p_color.rgba[1];
    rgba[2] = p_color.rgba[2];
    rgba[3] = p_color.rgba[3];
    return *this;
}

double
Color::operator()(int p_i) const
{
    if (p_i < 0 || p_i > 3)
    {
        return -1;
    }

    return rgba[p_i];
}

std::ostream&
operator<<(std::ostream& os, const Color& c)
{
    os << "R = " << c.r << '\n'
       << "G = " << c.g << '\n'
       << "B = " << c.b << '\n'
       << "A = " << c.a;
    return os;
}

void
Color::darken(const double p_darkFactor)
{
    rgba[0] = (rgba[1] - rgba[1] * (p_darkFactor) );
    rgba[1] = (rgba[2] - rgba[2] * (p_darkFactor) );
    rgba[2] = (rgba[3] - rgba[3] * (p_darkFactor) );
    rgba[0] = (rgba[0] < 0 ? 0 : rgba[0]);
    rgba[1] = (rgba[1] < 0 ? 0 : rgba[1]);
    rgba[2] = (rgba[2] < 0 ? 0 : rgba[2]);
}

void
Color::lighten(const double p_darkFactor)
{
    rgba[0] = rgba[1] + rgba[1] * (p_darkFactor);
    rgba[1] = rgba[2] + rgba[2] * (p_darkFactor);
    rgba[2] = rgba[3] + rgba[3] * (p_darkFactor);

    rgba[0] = (rgba[0] > 1.0 ? 1.0 : rgba[0]);
    rgba[1] = (rgba[1] < 1.0 ? 1.0 : rgba[1]);
    rgba[2] = (rgba[2] < 1.0 ? 1.0 : rgba[2]);
}

void
Color::setValue(const double p_red,
                const double p_green,
                const double p_blue,
                const double p_alpha)
{
    if (!isColorRangeCorrect(p_red)
        || !isColorRangeCorrect(p_green)
        || !isColorRangeCorrect(p_blue)
        || !isColorRangeCorrect(p_alpha))
    {
        LOG(WARNING) << "Can not set Color: value outside of [0.0, 1.0] range.";
    }

    rgba[0] = p_red;
    rgba[1] = p_green;
    rgba[2] = p_blue;
    rgba[3] = p_alpha;
}

void
Color::getValue(double color[4])
{
    color[0] = rgba[0];
    color[1] = rgba[1];
    color[2] = rgba[2];
    color[3] = rgba[3];
}

const double*
Color::getValue() const
{
    return rgba;
}

Color
Color::lerpRgba(const Color& start, const Color& end, const double t)
{
    return start + (end - start) * t;
}

Color
Color::lerpRgb(const Color& start, const Color& end, const double t)
{
    return Color(start + (end - start) * t, 1.0);
}

std::string
Color::rgbHex()
{
    std::stringstream ss;
    const int         red   = static_cast<int>(r * 255.0);
    const int         green = static_cast<int>(g * 255.0);
    const int         blue  = static_cast<int>(b * 255.0);
    ss << std::setfill('0') << std::setw(2) << std::hex << red;
    ss << std::setfill('0') << std::setw(2) << std::hex << green;
    ss << std::setfill('0') << std::setw(2) << std::hex << blue;
    return ss.str();
}

Color
operator*(const Color& color_lhs, const Color& color_rhs)
{
    Color results;
    results.r = color_lhs.r * color_rhs.r;
    results.g = color_lhs.g * color_rhs.g;
    results.b = color_lhs.b * color_rhs.b;
    results.a = color_lhs.a * color_rhs.a;
    return results;
}

Color
operator*(const Color& color_lhs, const double intensity_rhs)
{
    Color results;
    results.r = color_lhs.r * intensity_rhs;
    results.g = color_lhs.g * intensity_rhs;
    results.b = color_lhs.b * intensity_rhs;
    results.a = color_lhs.a * intensity_rhs;
    return results;
}

Color
operator*=(const Color& color_lhs, const Color& color_rhs)
{
    Color results;
    results.r = color_lhs.r * color_rhs.r;
    results.g = color_lhs.g * color_rhs.g;
    results.b = color_lhs.b * color_rhs.b;
    results.a = color_lhs.a * color_rhs.a;
    return results;
}

Color
operator*=(const Color& color_lhs, const double intensity_rhs)
{
    Color results;
    results.r = color_lhs.r * intensity_rhs;
    results.g = color_lhs.g * intensity_rhs;
    results.b = color_lhs.b * intensity_rhs;
    results.a = color_lhs.a * intensity_rhs;
    return results;
}

Color
operator+(const Color& color_lhs, const Color& color_rhs)
{
    Color results;
    results.r = color_lhs.r + color_rhs.r;
    results.g = color_lhs.g + color_rhs.g;
    results.b = color_lhs.b + color_rhs.b;
    results.a = color_lhs.a + color_rhs.a;
    return results;
}

Color
operator+(const Color& color_lhs, const double intensity_rhs)
{
    Color results;
    results.r = color_lhs.r + intensity_rhs;
    results.g = color_lhs.g + intensity_rhs;
    results.b = color_lhs.b + intensity_rhs;
    results.a = color_lhs.a + intensity_rhs;
    return results;
}

Color
operator+=(const Color& color_lhs, const Color& color_rhs)
{
    Color results;
    results.r = color_lhs.r + color_rhs.r;
    results.g = color_lhs.g + color_rhs.g;
    results.b = color_lhs.b + color_rhs.b;
    results.a = color_lhs.a + color_rhs.a;
    return results;
}

Color
operator+=(const Color& color_lhs, const double intensity_rhs)
{
    Color results;
    results.r = color_lhs.r + intensity_rhs;
    results.g = color_lhs.g + intensity_rhs;
    results.b = color_lhs.b + intensity_rhs;
    results.a = color_lhs.a + intensity_rhs;
    return results;
}

Color
operator-(const Color& color_lhs, const Color& color_rhs)
{
    Color results;
    results.r = color_lhs.r - color_rhs.r;
    results.g = color_lhs.g - color_rhs.g;
    results.b = color_lhs.b - color_rhs.b;
    results.a = color_lhs.a - color_rhs.a;
    return results;
}

Color
operator-(const Color& color_lhs, const double intensity_rhs)
{
    Color results;
    results.r = color_lhs.r - intensity_rhs;
    results.g = color_lhs.g - intensity_rhs;
    results.b = color_lhs.b - intensity_rhs;
    results.a = color_lhs.a - intensity_rhs;
    return results;
}

Color
operator-=(const Color& color_lhs, const Color& color_rhs)
{
    Color results;
    results.r = color_lhs.r - color_rhs.r;
    results.g = color_lhs.g - color_rhs.g;
    results.b = color_lhs.b - color_rhs.b;
    results.a = color_lhs.a - color_rhs.a;
    return results;
}

Color
operator-=(const Color& color_lhs, const double intensity_rhs)
{
    Color results;
    results.r = color_lhs.r - intensity_rhs;
    results.g = color_lhs.g - intensity_rhs;
    results.b = color_lhs.b - intensity_rhs;
    results.a = color_lhs.a - intensity_rhs;
    return results;
}
}
