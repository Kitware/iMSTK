// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Color.h"

Color Color::colorWhite(1.0, 1.0, 1.0, 1.0);
Color Color::colorBlue(0.0, 0.0, 1.0, 1.0);
Color Color::colorGreen(0.0, 1.0, 0.0, 1.0);
Color Color::colorRed(1.0, 0.0, 0.0, 1.0);
Color Color::colorGray(0.8, 0.8, 0.8, 1.0);
Color Color::colorYellow(1, 1, 0, 1);
Color Color::colorPink(1, 0, 1, 1);

Color::Color()
{
    rgba[0] = 0.8f;
    rgba[1] = 0.8f;
    rgba[2] = 0.8f;
    rgba[3] = 1.0f;
}

Color::Color( float r, float g, float b, float a )
{
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
}

void Color::darken( float p_darkFactor )
{

    rgba[0] = ( rgba[1] - rgba[1] * ( p_darkFactor ) );
    rgba[1] = ( rgba[2] - rgba[2] * ( p_darkFactor ) );
    rgba[2] = ( rgba[3] - rgba[3] * ( p_darkFactor ) );
    rgba[0] = ( rgba[0] < 0 ? 0 : rgba[0] );
    rgba[1] = ( rgba[1] < 0 ? 0 : rgba[1] );
    rgba[2] = ( rgba[2] < 0 ? 0 : rgba[2] );
}

void Color::lighten( float p_darkFactor )
{

    rgba[0] = rgba[1] + rgba[1] * ( p_darkFactor );
    rgba[1] = rgba[2] + rgba[2] * ( p_darkFactor );
    rgba[2] = rgba[3] + rgba[3] * ( p_darkFactor );

    rgba[0] = ( rgba[0] > 1.0 ? 1.0 : rgba[0] );
    rgba[1] = ( rgba[1] < 1.0 ? 1.0 : rgba[1] );
    rgba[2] = ( rgba[2] < 1.0 ? 1.0 : rgba[2] );
}

float Color::operator()( int p_i )
{
    if ( p_i < 0 || p_i > 3 )
    {
        return -1;
    }

    return rgba[p_i];
}

Color &Color::operator=(const Color &p_color )
{
    rgba[0] = p_color.rgba[0];
    rgba[1] = p_color.rgba[1];
    rgba[2] = p_color.rgba[2];
    rgba[3] = p_color.rgba[3];
    return *this;
}

GLfloat *Color::toGLColor()
{
    return static_cast<GLfloat*>(rgba);
}

const GLfloat *Color::toGLColor() const
{
    return static_cast<const GLfloat*>(rgba);
}

void Color::setValue( float p_red, float p_green, float p_blue, float p_alpha )
{
    rgba[0] = p_red;
    rgba[1] = p_green;
    rgba[2] = p_blue;
    rgba[3] = p_alpha;
}
