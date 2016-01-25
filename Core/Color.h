// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef CORE_COLOR_H
#define CORE_COLOR_H

#include "Core/Config.h"

namespace imstk {

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

    /// \brief get RGB color
    void getValue(float color[4])
    {
        color[0] = rgba[0];
        color[1] = rgba[1];
        color[2] = rgba[2];
        color[3] = rgba[3];
    }

    /// \brief get RGB color
    float *getValue()
    {
        return rgba;
    }
    /// \brief get RGB color
    const float *getValue() const
    {
        return rgba;
    }
    static Color colorWhite;
    static Color colorBlue;
    static Color colorGreen;
    static Color colorRed;
    static Color colorGray;
    static Color colorPink;
    static Color colorYellow;
};

}

#endif // CORE_COLOR_H
