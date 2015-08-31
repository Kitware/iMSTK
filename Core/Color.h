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
#ifndef SMCOLOR_H
#define SMCOLOR_H

#include "Core/Config.h"

struct Color
{
    union
    {
        float rgba[4];
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
    };
    Color();
    Color(float r, float g, float b, float a = 1.0);

    /// \brief Dark ratio. the valu is between 0 and 1.0
    void darken(float p_darkFactor);
    /// \brief lighten the color
    void lighten(float p_darkFactor);

    /// \brief returns the color value given with the index
    float operator()(int p_i);
    /// \brief setting
    Color &operator=(const Color &p_color);

    /// \brief converts to gl color
    template<typename GLColorType>
    GLColorType* toGLColor()
    {
        return static_cast<GLColorType*>(&rgba[0]);
    }

    template<typename GLColorType>
    const GLColorType* toGLColor() const
    {
        return static_cast<const GLColorType*>(&rgba[0]);
    }

    /// \brief set RGB color
    void setValue(float p_red, float p_green, float p_blue, float p_alpha);

    static Color colorWhite;
    static Color colorBlue;
    static Color colorGreen;
    static Color colorRed;
    static Color colorGray;
    static Color colorPink;
    static Color colorYellow;
};

#endif // SMCOLOR_H
