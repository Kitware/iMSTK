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

#ifndef SMTK_EXAMPLES_COMMON_WASDCAMERACONTROLLER_H
#define SMTK_EXAMPLES_COMMON_WASDCAMERACONTROLLER_H

#include "Core/CoreClass.h"
#include "Rendering/Camera.h"

namespace smtk {
namespace Examples {
namespace Common {

/// \brief A camera controller that is manipulated by the W, A, S and D keys
/// on the keyboard
///
/// \detail W = forward, A = left, S = backward, D = right,
/// Shift + W = up, Shift + S = down
/// To use this class, it must be registered with the event system.
///
class wasdCameraController : public smCoreClass
{
public:
    /// \brief Default constructor
    ///
    wasdCameraController();

    /// \brief Default constructor
    ///
    /// \param cam Pointer to camera to be controlled
    wasdCameraController(std::shared_ptr<smCamera> cam);

    /// \brief Event handling function from smCoreClass
    ///
    /// \param event Event to handle from the main event system
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> event) override;

    /// \brief Set the camera to be controlled
    ///
    /// \param cam Pointer to camera to be controlled
    void setCamera(std::shared_ptr<smCamera> cam);

    /// \brief Set the step size that the camera moves with each key press
    ///
    /// \param size Size in generic units to move
    void setStepSize(float size);

private:
    std::shared_ptr<smCamera> camera; ///< Pointer to camera being controlled
    float moveDistance;
};

}//Common
}//Examples
}//tk

#endif
