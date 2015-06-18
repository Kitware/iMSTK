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

#include "smEvent/smMouseButtonEvent.h"
#include "smEvent/smMouseMoveEvent.h"
#include "smCore/smMath.h"

namespace smtk {
namespace Examples {
namespace Common {

pzrMouseCameraController::pzrMouseCameraController()
    : moveDistance{1}, lmbPressed{false}, rmbPressed{false}
{
}

pzrMouseCameraController::pzrMouseCameraController(std::shared_ptr<smCamera> cam)
    : moveDistance{1},
      camera(cam),
      lmbPressed{false},
      rmbPressed{false}
{
}

void pzrMouseCameraController::setCamera(std::shared_ptr<smCamera> cam)
{
    camera = cam;
}

void pzrMouseCameraController::setStepSize(float size)
{
    moveDistance = size;
}

void pzrMouseCameraController::handleEvent(std::shared_ptr<smtk::Event::smEvent> event)
{
    assert(nullptr != event);
    assert(nullptr != camera);

    auto mouseButtonEvent = std::dynamic_pointer_cast<Event::smMouseButtonEvent>(event);
    if(mouseButtonEvent != nullptr)
    {
        if(Event::smMouseButton::Left ==
           mouseButtonEvent->getMouseButton())
        {
            lmbPressed = mouseButtonEvent->getPressed();
            coords = mouseButtonEvent->getWindowCoord().cast<float>();
        }
        else if(Event::smMouseButton::Right ==
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

    auto mouseMoveEvent = std::dynamic_pointer_cast<Event::smMouseMoveEvent>(event);
    if(mouseMoveEvent != nullptr)
    {
        smVec2f diff;
        smVec2f newCoords;

        newCoords = mouseMoveEvent->getWindowCoord().cast<float>();
        diff = coords - newCoords;

        if(lmbPressed && rmbPressed)
        {
            //pan x and y
            this->camera->pan(smVec3f(diff(0), diff(1), 0) * moveDistance);
            //reset coords for next mouse move event
            coords = newCoords;
        }
        else if(lmbPressed)
        {            
            //rotate
            this->camera->rotateFocusX(SM_DEGREES2RADIANS(moveDistance * diff(1)));
            this->camera->rotateFocusY(SM_DEGREES2RADIANS(moveDistance * diff(0)));
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
}//smtk
