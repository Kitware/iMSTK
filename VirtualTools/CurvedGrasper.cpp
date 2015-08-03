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

#include "VirtualTools/CurvedGrasper.h"

// Eigen includes
#include "Eigen/Geometry"

// SimMedTK includes
#include "Core/SDK.h"
#include "Event/HapticEvent.h"
#include "Event/KeyboardEvent.h"

CurvedGrasper::CurvedGrasper(size_t p_PhantomID,
                             const std::string& p_pivotModelFileName,
                             const std::string& p_lowerModelFileName,
                             const std::string& p_upperModelFileName)
{

    angle = 0;
    maxangle = 10 * 3.14 / 360;
    this->phantomID = p_PhantomID;

    Matrix33d rot;
    mesh_pivot = new SurfaceMesh(Core::BaseMesh::MeshType::Rigid, nullptr);
    mesh_pivot->loadMesh(p_pivotModelFileName, Core::BaseMesh::MeshFileType::ThreeDS);
    mesh_pivot->scale(core::Vec3d(0.5, 0.5, 0.5));
    rot = Eigen::AngleAxisd(-M_PI_2, core::Vec3d::UnitX()).matrix();
    mesh_pivot->rotate(rot);
    rot = Eigen::AngleAxisd(-M_PI_2, core::Vec3d::UnitZ()).matrix();
    mesh_pivot->rotate(rot);

    mesh_upperJaw = new SurfaceMesh(Core::BaseMesh::MeshType::Rigid, nullptr);
    mesh_upperJaw->loadMesh(p_upperModelFileName, Core::BaseMesh::MeshFileType::ThreeDS);
    mesh_upperJaw->scale(core::Vec3d(0.5, 0.5, 0.5));
    rot = Eigen::AngleAxisd(-M_PI_2, core::Vec3d::UnitY()).matrix();
    mesh_upperJaw->rotate(rot);
    rot = Eigen::AngleAxisd(-M_PI_2, core::Vec3d::UnitZ()).matrix();
    mesh_upperJaw->rotate(rot);

    mesh_lowerJaw = new SurfaceMesh(Core::BaseMesh::MeshType::Rigid, nullptr);
    mesh_lowerJaw->loadMesh(p_lowerModelFileName, Core::BaseMesh::MeshFileType::ThreeDS);
    mesh_lowerJaw->scale(core::Vec3d(0.5, 0.5, 0.5));
    rot = Eigen::AngleAxisd(-M_PI_2, core::Vec3d::UnitY()).matrix();
    mesh_lowerJaw->rotate(rot);
    rot = Eigen::AngleAxisd(-M_PI_2, core::Vec3d::UnitZ()).matrix();
    mesh_lowerJaw->rotate(rot);

    meshContainer_pivot.name = "CurvedGrasperPivot";
    meshContainer_pivot.mesh = mesh_pivot;

    meshContainer_upperJaw.name = "CurvedGrasperUpper";
    meshContainer_upperJaw.mesh = mesh_upperJaw;

    meshContainer_lowerJaw.name = "CurvedGrasperLower";
    meshContainer_lowerJaw.mesh = mesh_lowerJaw;

    //for interface
#ifdef smNIUSB6008DAQ
    NIUSB6008pipeReg.regType = SIMMEDTK_PIPE_BYREF;
    NIUSB6008pipeReg.listenerObject = this->mesh_lowerJaw;
#endif

    DAQdataID = 0;
}

void CurvedGrasper::handleEvent(std::shared_ptr<core::Event> p_event)
{
    if(!this->isListening())
    {
        return;
    }

    auto hapticEvent = std::static_pointer_cast<event::HapticEvent>(p_event);
    if(hapticEvent != nullptr && hapticEvent->getDeviceId() == this->phantomID)
    {
        MeshContainer *containerLower = this->getMeshContainer("CurvedGrasperLower");
        MeshContainer *containerUpper = this->getMeshContainer("CurvedGrasperUpper");
        Matrix44d godPosMat = hapticEvent->getTransform();
        transRot = godPosMat;
        pos = hapticEvent->getPosition();
        vel = hapticEvent->getVelocity();
        buttonState[0] = hapticEvent->getButtonState(0);
        buttonState[1] = hapticEvent->getButtonState(1);

        if (buttonState[0])
        {
            angle -= 0.05;

            if (angle < 0.0)
            {
                angle = 0.0;
            }
        }

        if (buttonState[1])
        {
            angle += 0.05;

            if (angle > 30.0)
            {
                angle = 30.0;
            }
        }

        containerLower->offsetRotY =  angle / 360.0; //angle*maxangle;
        containerUpper->offsetRotY =  -angle / 360.0; //-angle*maxangle;
        return;
    }

    auto keyboardEvent = std::static_pointer_cast<event::KeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case event::Key::Num1:
            {
                this->eventHandler->detachEvent(core::EventType::Haptic,shared_from_this());
                this->getRenderDetail()->renderType = this->getRenderDetail()->renderType
                & ( ~SIMMEDTK_RENDER_NONE );
                break;
            }

            case event::Key::Num2:
            {
                this->eventHandler->attachEvent(core::EventType::Haptic,shared_from_this());
                this->getRenderDetail()->renderType = this->getRenderDetail()->renderType
                | SIMMEDTK_RENDER_NONE;
                break;
            }
            default:
                break;
        }
    }
}

void CurvedGrasper::updateOpenClose()
{

    if (buttonState[0])
    {
        angle -= 0.05;

        if (angle < 0.0)
        {
            angle = 0.0;
        }
    }

    if (buttonState[1])
    {
        angle += 0.05;

        if (angle > 30.0)
        {
            angle = 30.0;
        }
    }

#ifdef smNIUSB6008DAQ
    NIUSB6008Data *NI_Data;

    if (NIUSB6008pipeReg.data.nbrElements > 0)
    {
        NI_Data = (NIUSB6008Data  *)NIUSB6008pipeReg.data.dataLocation;

        if (NI_Data->on)
        {
            angle = NI_Data->value[DAQdataID] * 30.0;
        }
    }

#endif
}
