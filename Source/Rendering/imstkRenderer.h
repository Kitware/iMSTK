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

#ifndef imstkRenderer_h
#define imstkRenderer_h

#include "imstkTextureManager.h"
#include "imstkMath.h"

#include <memory>

namespace imstk
{
class Renderer
{
public:
    ///
    /// \brief Enumerations for the render mode
    ///
    enum Mode
    {
        EMPTY,
        DEBUG,
        SIMULATION
    };

    ///
    /// \brief Get renderer
    ///
    virtual std::shared_ptr<Renderer> getRenderer();

    ///
    /// \brief Set rendering mode
    ///
    virtual void setMode(Renderer::Mode mode, bool enableVR);

    ///
    /// \brief Get rendering mode
    ///
    virtual const Renderer::Mode& getMode();

    ///
    /// \brief Update background colors
    ///
    virtual void updateBackground(const Vec3d color1, const Vec3d color2 = Vec3d::Zero(), const bool gradientBackground = false){};

protected:
    Renderer::Mode m_currentMode = Renderer::Mode::EMPTY;
};
}

#endif