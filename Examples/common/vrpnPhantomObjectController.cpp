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

#include "vrpnPhantomObjectController.h"

#include "Core/RenderDelegate.h"

#include <thread>

namespace mstk {
namespace Examples {
namespace Common {

vrpnPhantomObjectController::vrpnPhantomObjectController(
    std::shared_ptr<VRPNPhantomDevice> p, std::shared_ptr<Core::BaseMesh> m)
    : phantom(p),
      mesh(m),
      scalingFactor(1.0),
      pScale(scalingFactor),
      delay(std::chrono::milliseconds(100))
{
    pRot = core::Quaterniond(0, 0, 0, 0);
    pPos = core::Vec3d(0, 0, 0);
}

void vrpnPhantomObjectController::init()
{
}

void vrpnPhantomObjectController::beginFrame()
{
}

void vrpnPhantomObjectController::endFrame()
{
}

void vrpnPhantomObjectController::exec()
{
    assert(phantom);
    assert(mesh);

    while(!terminateExecution)
    {
        using AffTrans3d = Eigen::Transform<double,3,Eigen::Affine>;
        //calculate new transformation
        core::Quaterniond rot = phantom->getOrientation();
        core::Vec3d pos = phantom->getPosition();
        double scale = scalingFactor; //store in case it is changed during this
        //create the transform for the current data
        AffTrans3d trans = Eigen::Translation3d(pos * scale) * rot;

        //exclude any zero data that comes at initialization
        if(!(((core::Vec3d::Zero() == rot.vec()) && (0 == rot.w())) &&
             (core::Vec3d::Zero() == pos)))
        {
            //don't inverse the transform until there is non-zero data
            if(!(((core::Vec3d::Zero() == pRot.vec()) && (0 == pRot.w())) &&
                 (core::Vec3d::Zero() == pPos)))
            {
                AffTrans3d pTrans = Eigen::Translation3d(pPos * pScale) * pRot;
                mesh->transform(pTrans.inverse()); //inverse prev transform
            }
            mesh->transform(trans); //apply new transformation
            mesh->getRenderDelegate()->modified(); //tell the renderer to update

            //store this transformation so it can be inverted before the next
            pPos = pos;
            pRot = rot;
            pScale = scale;
        }

        std::this_thread::sleep_for(delay);
    }
}

void vrpnPhantomObjectController::setPollDelay(const std::chrono::milliseconds d)
{
    delay = d;
}

std::chrono::milliseconds vrpnPhantomObjectController::getPollDelay()
{
    return delay;
}

void vrpnPhantomObjectController::setScalingFactor(double s)
{
    scalingFactor = s;
}

double vrpnPhantomObjectController::getScalingFactor()
{
    return scalingFactor;
}

void vrpnPhantomObjectController::setPhantom(std::shared_ptr<VRPNPhantomDevice> p)
{
    phantom = p;
}

void vrpnPhantomObjectController::setMesh(std::shared_ptr<Core::BaseMesh> m)
{
    mesh = m;
}


} //Common
} //Examples
} //mstk