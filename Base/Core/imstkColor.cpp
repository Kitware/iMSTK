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

#include <g3log/g3log.hpp>

namespace imstk
{

bool isColorRangeCorrect( double c )
{
    return (c >= 0 && c <= 1.0);
}

Color Color::White(1.0, 1.0, 1.0, 1.0);
Color Color::Black(0.0, 0.0, 0.0, 1.0);
Color Color::DarkGray(0.8, 0.8, 0.8, 1.0);
Color Color::LightGray(0.3, 0.3, 0.3, 1.0);
Color Color::Blue(0.0, 0.0, 1.0, 1.0);
Color Color::Green(0.0, 1.0, 0.0, 1.0);
Color Color::Red(1.0, 0.0, 0.0, 1.0);
Color Color::Yellow(1.0, 1.0, 0.0, 1.0);
Color Color::Pink(1.0, 0.0, 1.0, 1.0);

Color::Color()
{
    rgba[0] = 0.8f;
    rgba[1] = 0.8f;
    rgba[2] = 0.8f;
    rgba[3] = 1.0f;
}

Color::Color( double r, double g, double b, double a )
{
    bool redGood = isColorRangeCorrect(r);
    bool greenGood = isColorRangeCorrect(g);
    bool blueGood = isColorRangeCorrect(b);
    bool alphaGood = isColorRangeCorrect(a);
    if(!redGood || !greenGood || !blueGood || !alphaGood)
    {
        LOG(WARNING) << "Can not set Color: value outside of [0.0, 1.0] range.";
        return;
    }

    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
}

Color &
Color::operator=(const Color &p_color )
{
    rgba[0] = p_color.rgba[0];
    rgba[1] = p_color.rgba[1];
    rgba[2] = p_color.rgba[2];
    rgba[3] = p_color.rgba[3];
    return *this;
}

double
Color::operator()( int p_i ) const
{
    if ( p_i < 0 || p_i > 3 )
    {
        return -1;
    }

    return rgba[p_i];
}

std::ostream& operator<<(std::ostream& os, const Color& c)
{
    os << "R = " << c.r << '\n'
       << "G = " << c.g << '\n'
       << "B = " << c.b << '\n'
       << "\u03B1 = " << c.a ;
    return os;
}

void
Color::darken( double p_darkFactor )
{
    rgba[0] = ( rgba[1] - rgba[1] * ( p_darkFactor ) );
    rgba[1] = ( rgba[2] - rgba[2] * ( p_darkFactor ) );
    rgba[2] = ( rgba[3] - rgba[3] * ( p_darkFactor ) );
    rgba[0] = ( rgba[0] < 0 ? 0 : rgba[0] );
    rgba[1] = ( rgba[1] < 0 ? 0 : rgba[1] );
    rgba[2] = ( rgba[2] < 0 ? 0 : rgba[2] );
}

void
Color::lighten( double p_darkFactor )
{
    rgba[0] = rgba[1] + rgba[1] * ( p_darkFactor );
    rgba[1] = rgba[2] + rgba[2] * ( p_darkFactor );
    rgba[2] = rgba[3] + rgba[3] * ( p_darkFactor );

    rgba[0] = ( rgba[0] > 1.0 ? 1.0 : rgba[0] );
    rgba[1] = ( rgba[1] < 1.0 ? 1.0 : rgba[1] );
    rgba[2] = ( rgba[2] < 1.0 ? 1.0 : rgba[2] );
}

void
Color::setValue( double p_red, double p_green, double p_blue, double p_alpha )
{
    bool redGood = isColorRangeCorrect(p_red);
    bool greenGood = isColorRangeCorrect(p_green);
    bool blueGood = isColorRangeCorrect(p_blue);
    bool alphaGood = isColorRangeCorrect(p_alpha);
    if(!redGood || !greenGood || !blueGood || !alphaGood)
    {
        LOG(WARNING) << "Can not set Color: value outside of [0.0, 1.0] range.";
        return;
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

const double *
Color::getValue() const
{
    return rgba;
}

}
