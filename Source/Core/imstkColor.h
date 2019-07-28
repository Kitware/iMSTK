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

#pragma once

#include <iostream>

/// \todo remove nameless union/struct in the future
#pragma warning(disable : 4201)
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

    ///
    /// \brief Constructor
    ///
    Color();
    Color(double r, double g, double b, double a = 1.0);

    ///
    /// \brief Equality operator
    ///
    Color& operator=(const Color& p_color);

    ///
    /// \brief Bitwise operator
    ///
    friend std::ostream& operator<<(std::ostream& os, const Color& c);

    ///
    /// \brief returns the color value given with the index
    ///
    double operator()(int p_i) const;

    ///
    /// \brief Dark ratio. the valu is between 0 and 1.0
    ///
    void darken(double p_darkFactor);

    ///
    /// \brief lighten the color
    ///
    void lighten(double p_darkFactor);

    ///
    /// \brief set RGB color
    ///
    void setValue(double p_red, double p_green, double p_blue, double p_alpha = 1.0);

    ///
    /// \brief get RGB color
    ///
    void getValue(double color[4]);

    ///
    /// \brief get RGB color
    ///
    const double* getValue() const;

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
};
#pragma warning(default : 4201)
}
