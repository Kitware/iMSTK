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

#include "smConfig.h"

struct smColor
{
    union
    {
        smFloat rgba[4];
        struct
        {
            smFloat r;
            smFloat g;
            smFloat b;
            smFloat a;
        };
    };
    smColor();
    smColor(smFloat r, smFloat g, smFloat b, smFloat a = 1.0);

    /// \brief Dark ratio. the valu is between 0 and 1.0
    void darken(smFloat p_darkFactor);
    /// \brief lighten the color
    void lighten(smFloat p_darkFactor);

    /// \brief returns the color value given with the index
    smFloat operator()(smInt p_i);
    /// \brief setting
    smColor &operator=(const smColor &p_color);
    /// \brief converts to gl color
    smGLFloat* toGLColor();
    const smGLFloat* toGLColor() const;
    /// \brief set RGB color
    void setValue(smFloat p_red, smFloat p_green, smFloat p_blue, smFloat p_alpha);

    static smColor colorWhite;
    static smColor colorBlue;
    static smColor colorGreen;
    static smColor colorRed;
    static smColor colorGray;
    static smColor colorPink;
    static smColor colorYellow;
};

#endif // SMCOLOR_H
