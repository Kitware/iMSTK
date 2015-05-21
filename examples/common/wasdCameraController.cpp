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

#include "smEvent/smKeyboardEvent.h"

namespace smtk {
namespace Examples {
namespace Common {

wasdCameraController::wasdCameraController()
    : moveDistance{1}
{
}

wasdCameraController::wasdCameraController(std::shared_ptr<smCamera> cam)
    : moveDistance{1},
      camera(cam)
{
}

void wasdCameraController::setCamera(std::shared_ptr<smCamera> cam)
{
    camera = cam;
}

void wasdCameraController::setStepSize(smUInt size)
{
    moveDistance = size;
}

void wasdCameraController::handleEvent(std::shared_ptr<smtk::Event::smEvent> event)
{
    assert(nullptr != event);
    assert(nullptr != camera);

    auto keyboardEvent = std::static_pointer_cast<smtk::Event::smKeyboardEvent>(event);
    if(keyboardEvent->getPressed())
    {
        switch(keyboardEvent->getKeyPressed())
        {
        case smtk::Event::smKey::W:
        {
            if(smtk::Event::smModKey::shift == (keyboardEvent->getModifierKey() & smtk::Event::smModKey::shift))
            {
                //Move the camera up
                camera->setCameraPos(camera->pos.x, camera->pos.y + moveDistance, camera->pos.z);
                camera->setCameraFocus(camera->fp.x, camera->fp.y + moveDistance, camera->fp.z);
            }
            else
            {
                //Move the camera forward
                camera->setCameraPos(camera->pos.x, camera->pos.y, camera->pos.z - moveDistance);
                camera->setCameraFocus(camera->fp.x, camera->fp.y, camera->fp.z - moveDistance);
            }
            camera->genViewMat();
            break;
        }
        case smtk::Event::smKey::A:
        {
            //Move the camera to the left
            camera->setCameraPos(camera->pos.x - moveDistance, camera->pos.y, camera->pos.z);
            camera->setCameraFocus(camera->fp.x - moveDistance, camera->fp.y, camera->fp.z);
            camera->genViewMat();
            break;
        }
        case smtk::Event::smKey::S:
        {
            if(smtk::Event::smModKey::shift == (keyboardEvent->getModifierKey() & smtk::Event::smModKey::shift))
            {
                //Move the camera down
                camera->setCameraPos(camera->pos.x, camera->pos.y - moveDistance, camera->pos.z);
                camera->setCameraFocus(camera->fp.x, camera->fp.y - moveDistance, camera->fp.z);
            }
            else
            {
                //Move the camera backward
                camera->setCameraPos(camera->pos.x, camera->pos.y, camera->pos.z + moveDistance);
                camera->setCameraFocus(camera->fp.x, camera->fp.y, camera->fp.z + moveDistance);
            }
            camera->genViewMat();
            break;
        }
        case smtk::Event::smKey::D:
        {
            //Move the camera to the right
            camera->setCameraPos(camera->pos.x + moveDistance, camera->pos.y, camera->pos.z);
            camera->setCameraFocus(camera->fp.x + moveDistance, camera->fp.y, camera->fp.z);
            camera->genViewMat();
            break;
        }
        default:
            break;
        }
    }
}

}//Common
}//Examples
}//smtk
