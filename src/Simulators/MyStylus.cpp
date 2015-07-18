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

// Eigen includes
#include "Eigen/Geometry"

// SimMedTK includes
#include "MyStylus.h"
#include "Mesh/SurfaceMesh.h"
#include "Core/SDK.h"
#include "Core/Matrix.h"
#include "Event/HapticEvent.h"
#include "Event/KeyboardEvent.h"

MyStylus::MyStylus(const smString& p_shaft, const smString& p_lower, const smString& p_upper)
{
    angle = 0;
    smMatrix33d rot = Eigen::AngleAxisd(-SM_PI_HALF, smVec3d::UnitX()).matrix();

    smSurfaceMesh *mesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    mesh->loadMesh(p_shaft, SM_FILETYPE_3DS);
    mesh->assignTexture("hookCautery");
    mesh->scale(smVec3d(0.2, 0.2, 0.2));
    mesh->rotate(rot);

    smSurfaceMesh *lowerMesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    lowerMesh->loadMesh(p_lower, SM_FILETYPE_3DS);
    lowerMesh->assignTexture("metal");
    lowerMesh->scale(smVec3d(0.2, 0.2, 0.2));
    lowerMesh->rotate(rot);

    smSurfaceMesh *upperMesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    upperMesh->loadMesh(p_upper, SM_FILETYPE_3DS);
    upperMesh->assignTexture("metal");
    upperMesh->scale(smVec3d(0.2, 0.2, 0.2));
    upperMesh->rotate(rot);

    meshContainer.name = "HookCauteryPivot";
    meshContainer.mesh = mesh;
    meshContainer.posOffsetPos[2] = 2;

    meshContainerLower.name = "HookCauteryLower";
    meshContainerLower.mesh = lowerMesh;
    meshContainerLower.preOffsetPos[2] = 0.0;
    meshContainerLower.posOffsetPos[2] = -3.5;

    meshContainerUpper.name = "HookCauteryUpper";
    meshContainerUpper.mesh = upperMesh;
    meshContainerUpper.preOffsetPos[2] = 0.0;
    meshContainerUpper.posOffsetPos[2] = -3.5;

    addMeshContainer(&meshContainer);
    addMeshContainer(meshContainer.name, &meshContainerLower);
    addMeshContainer(meshContainer.name, &meshContainerUpper);

    this->listening = true;
}

void MyStylus::updateOpenClose()
{
    if ( buttonState[1] )
    {
        if ( angle < 1.0 )
        {
            angle += 0.000004;
        }
        else
        {
            angle = 1.0;
        }
    }

    if ( buttonState[0] )
    {
        if ( angle > 0.00001 )
        {
            angle -= 0.000004;
        }
        else
        {
            angle = 0.0;
        }
    }
    std::cout << angle << "\n";
}

//This function is not fixed for a reason....I'll give you a hint...try to match the brackets
void MyStylus::handleEvent (std::shared_ptr<smtk::Event::smEvent> p_event)
{
    if(!this->isListening())
    {
        return;
    }

    auto hapticEvent = std::static_pointer_cast<smtk::Event::smHapticEvent>(p_event);
    if(hapticEvent != nullptr && hapticEvent->getDeviceId() == this->phantomID)
    {
        smMeshContainer *containerLower = this->getMeshContainer ( "HookCauteryLower" );
        smMeshContainer *containerUpper = this->getMeshContainer ( "HookCauteryUpper" );
        transRot = hapticEvent->getTransform();

        pos = hapticEvent->getPosition();

        vel = hapticEvent->getVelocity();

        buttonState[0] = hapticEvent->getButtonState(0);
        buttonState[1] = hapticEvent->getButtonState(1);
        buttonState[2] = hapticEvent->getButtonState(2);
        buttonState[3] = hapticEvent->getButtonState(3);

        if ( buttonState[1] )
        {
            if ( angle < 1.0 )
            {
                angle += 0.000004;
            }
            else
            {
                angle = 1.0;
            }
        }

        if ( buttonState[0] )
        {
            if ( angle > 0.00001 )
            {
                angle -= 0.000004;
            }
            else
            {
                angle = 0.0;
            }
        }

        containerLower->offsetRotX = angle * 25;
        containerUpper->offsetRotX = -angle * 25;
        return;
    }

    auto keyboardEvent = std::static_pointer_cast<smtk::Event::smKeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case smtk::Event::smKey::Num1:
            {
                this->eventHandler->detachEvent(smtk::Event::EventType::Haptic,shared_from_this());
                this->getRenderDetail()->renderType = this->getRenderDetail()->renderType & ( ~SIMMEDTK_RENDER_NONE );
                break;
            }

            case smtk::Event::smKey::Num2:
            {
                this->eventHandler->attachEvent(smtk::Event::EventType::Haptic,shared_from_this());
                this->getRenderDetail()->renderType = this->getRenderDetail()->renderType | SIMMEDTK_RENDER_NONE;
                break;
            }
            default:
                break;
        }
    }
}

HookCautery::HookCautery(const smString& p_pivot)
{
    smMatrix33d rot = Eigen::AngleAxisd(-SM_PI_HALF, smVec3d::UnitX()).matrix();

    smSurfaceMesh *mesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    mesh->loadMesh(p_pivot, SM_FILETYPE_3DS);
    mesh->assignTexture("metal");
    mesh->scale(smVec3d(0.2, 0.2, 0.2));
    mesh->rotate(rot);

    meshContainer.name = "HookCauteryPivot";
    meshContainer.mesh = mesh;
    meshContainer.posOffsetPos[2] = 2;

    addMeshContainer(&meshContainer);
}

void HookCautery::handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event)
{
    if(!this->isListening())
    {
        return;
    }

    auto hapticEvent = std::static_pointer_cast<smtk::Event::smHapticEvent>(p_event);
    if(hapticEvent != nullptr && hapticEvent->getDeviceId() == this->phantomID)
    {
        transRot = hapticEvent->getTransform();

        pos = hapticEvent->getPosition();

        vel = hapticEvent->getVelocity();

        buttonState[0] = hapticEvent->getButtonState(0);
        buttonState[1] = hapticEvent->getButtonState(1);
        buttonState[2] = hapticEvent->getButtonState(2);
        buttonState[3] = hapticEvent->getButtonState(3);

        return;
    }

    auto keyboardEvent = std::static_pointer_cast<smtk::Event::smKeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case smtk::Event::smKey::Num1:
            {
                this->eventHandler->detachEvent(smtk::Event::EventType::Haptic,shared_from_this());
                this->getRenderDetail()->renderType = this->getRenderDetail()->renderType & ( ~SIMMEDTK_RENDER_NONE );
                break;
            }

            case smtk::Event::smKey::Num2:
            {
                this->eventHandler->attachEvent(smtk::Event::EventType::Haptic,shared_from_this());
                this->getRenderDetail()->renderType = this->getRenderDetail()->renderType | SIMMEDTK_RENDER_NONE;
                break;
            }
            default:
                break;
        }
    }
}
