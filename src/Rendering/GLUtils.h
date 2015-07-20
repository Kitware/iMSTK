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
#include "Core/CoreClass.h"
#include "Core/Config.h"
#include "Core/Vector.h"
#include "Core/Matrix.h"

/// \brief stores the information about the ground
struct smGroundRenderInfo;

/// \brief opengl rendering utilities are here.
class GLUtils : public CoreClass
{
public:
    // OpenGL 3.0 adds the concept of indexed strings and
    // deprecates calls to glGetString( GL_EXTENSIONS ), which
    // will now generate GL_INVALID_ENUM.
    GLUtils()
    {
        int numExt;
        glGetIntegerv( GL_NUM_EXTENSIONS, &numExt );
        for(int i = 0; i < numExt; ++i)
        {
            openGLExtensions.emplace_back(
                reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS,i)));
        }
    }
    ~GLUtils();

public:
    /// \brief checks the openGL error. if there is an error then it returns
    /// \brief the error text otherwise it returns NULL
    static bool queryGLError(std::string& err);

    /// \brief  taken from glProgramming.com.  Checks the extension.
    bool QueryExtension(const std::string& extName);

    /// \brief  fade background draw with a size groundRenderInfo.scale
    static void fadeBackgroundDraw();

    /// \brief draw quad on screen with color and left, botton, right, top positions
    static void drawQuadOnScreen(Color p_color,
                                 float p_left,
                                 float p_bottom,
                                 float p_right,
                                 float p_top);

    /// \brief draw unit quad
    static void drawUnitQuadOnScreen();

    static void queryProjectionMatrix(Matrix44f &p_matrix);
    /// \brief to query current model view matrix. p_matrix will have the final values.
    static void queryModelViewMatrix(Matrix44f &p_matrix);

private:
    std::vector<std::string> openGLExtensions;
};

#endif
