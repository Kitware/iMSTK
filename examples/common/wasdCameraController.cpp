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

#include "wasdCameraController.h"

#include "Event/KeyboardEvent.h"

namespace mstk {
namespace Examples {
namespace Common {

wasdCameraController::wasdCameraController()
    : moveDistance{1.0}
{
}

wasdCameraController::wasdCameraController(std::shared_ptr<smCamera> cam)
    : moveDistance{1.0},
      camera(cam)
{
}

void wasdCameraController::setCamera(std::shared_ptr<smCamera> cam)
{
    camera = cam;
}

void wasdCameraController::setStepSize(float size)
{
    moveDistance = size;
}

void wasdCameraController::handleEvent(std::shared_ptr<mstk::Event::smEvent> event)
{
    assert(nullptr != event);
    assert(nullptr != camera);

    auto keyboardEvent = std::static_pointer_cast<mstk::Event::smKeyboardEvent>(event);
    if(keyboardEvent->getPressed())
    {
        smVec3f dispVec = smVec3f::Zero(); //Vector to store displacement of camera
        switch(keyboardEvent->getKeyPressed())
        {
        case mstk::Event::smKey::W:
        {
            if(mstk::Event::smModKey::shift == (keyboardEvent->getModifierKey() & mstk::Event::smModKey::shift))
            {
                //Move the camera up
                dispVec(1) = moveDistance;
            }
            else
            {
                //Move the camera forward
                dispVec(2) = -moveDistance;
            }
            break;
        }
        case mstk::Event::smKey::A:
        {
            //Move the camera to the left
            dispVec(0) = -moveDistance;
            break;
        }
        case mstk::Event::smKey::S:
        {
            if(mstk::Event::smModKey::shift == (keyboardEvent->getModifierKey() & mstk::Event::smModKey::shift))
            {
                //Move the camera down
                dispVec(1) = -moveDistance;
            }
            else
            {
                //Move the camera backward
                dispVec(2) = moveDistance;
            }
            break;
        }
        case mstk::Event::smKey::D:
        {
            //Move the camera to the right
            dispVec(0) = moveDistance;
            break;
        }
        default:
            return;
        }

        //apply displacements
        camera->pan(dispVec);
    }
}

}//Common
}//Examples
}//tk
