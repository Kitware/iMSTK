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

#include "pzrMouseCameraController.h"

#include "Event/MouseButtonEvent.h"
#include "Event/MouseMoveEvent.h"

namespace mstk {
namespace Examples {
namespace Common {

pzrMouseCameraController::pzrMouseCameraController()
    : moveDistance{1}, lmbPressed{false}, rmbPressed{false}
{
}

pzrMouseCameraController::pzrMouseCameraController(std::shared_ptr<Camera> cam)
    : moveDistance{1},
      lmbPressed{false},
      rmbPressed{false},
      camera(cam)
{
}

void pzrMouseCameraController::setCamera(std::shared_ptr<Camera> cam)
{
    camera = cam;
}

void pzrMouseCameraController::setStepSize(float size)
{
    moveDistance = size;
}

void pzrMouseCameraController::handleEvent(std::shared_ptr<core::Event> event)
{
    assert(nullptr != event);
    assert(nullptr != camera);

    auto mouseButtonEvent = std::dynamic_pointer_cast<event::MouseButtonEvent>(event);
    if(mouseButtonEvent != nullptr)
    {
        if(event::MouseButton::Left ==
           mouseButtonEvent->getMouseButton())
        {
            lmbPressed = mouseButtonEvent->getPressed();
            coords = mouseButtonEvent->getWindowCoord().cast<float>();
        }
        else if(event::MouseButton::Right ==
                mouseButtonEvent->getMouseButton())
        {
            rmbPressed = mouseButtonEvent->getPressed();
            coords = mouseButtonEvent->getWindowCoord().cast<float>();
        }
        else
        {
            return;
        }
    }

    auto mouseMoveEvent = std::dynamic_pointer_cast<event::MouseMoveEvent>(event);
    if(mouseMoveEvent != nullptr)
    {
        auto newCoords = mouseMoveEvent->getWindowCoord().cast<float>();
        auto diff = coords - newCoords;

        if(lmbPressed && rmbPressed)
        {
            //pan x and y
            core::Vec3f v;
            v << diff, 0;
            this->camera->pan(v* moveDistance);
            //reset coords for next mouse move event
            coords = newCoords;
        }
        else if(lmbPressed)
        {
            //rotate
            // In radians
            this->camera->rotateFocusX(0.0174532925199433*(moveDistance * diff(1)));
            this->camera->rotateFocusY(0.0174532925199433*(moveDistance * diff(0)));
            //reset coords for next mouse move event
            coords = newCoords;
        }
        else if(rmbPressed)
        {
            //zoom
            this->camera->zoom(diff(1) * moveDistance);
            //reset coords for next mouse move event
            coords = newCoords;
        }
    }
}

}//Common
}//Examples
}//tk
