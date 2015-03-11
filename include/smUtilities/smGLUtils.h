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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMGLUTILS_H
#define SMGLUTILS_H

// SimMedTK includes
#include "smCore/smCoreClass.h"
#include "smCore/smConfig.h"
#include "smUtilities/smVector.h"
#include "smUtilities/smMatrix.h"

/// \brief stores the information about the ground
struct smGroundRenderInfo;

/// \brief opengl rendering utilities are here.
class smGLUtils : public smCoreClass
{
public:
    /// \brief checks the openGL error. if there is an error then it returns
    /// \brief the error text otherwise it returns NULL
    static bool queryGLError(smChar*err);

    /// \brief  taken from glProgramming.com.  Checks the extension.
    static smBool QueryExtension(char *extName);

    /// \brief  fade background draw with a size groundRenderInfo.scale
    static void fadeBackgroundDraw();

    /// \brief draw quad on screen with color and left, botton, right, top positions
    static void drawQuadOnScreen(smColor p_color,
                                 smFloat p_left,
                                 smFloat p_bottom,
                                 smFloat p_right,
                                 smFloat p_top);

    /// \brief draw unit quad
    static void drawUnitQuadOnScreen();

    static void queryProjectionMatrix(smMatrix44f &p_matrix);
    /// \brief to query current model view matrix. p_matrix will have the final values.
    static void queryModelViewMatrix(smMatrix44f &p_matrix);
};

#endif
