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

#include "Core/BaseMesh.h"
#include "Core/Module.h"
#include "Devices/VRPNPhantomDevice.h"

#include <memory>
#include <chrono>

namespace mstk {
namespace Examples {
namespace Common {

class vrpnPhantomObjectController : public Module
{
public:
    vrpnPhantomObjectController(std::shared_ptr<VRPNPhantomDevice> p = nullptr,
                                std::shared_ptr<Core::BaseMesh> m = nullptr);

    void setPhantom(std::shared_ptr<VRPNPhantomDevice> p);
    void setMesh(std::shared_ptr<Core::BaseMesh> m);

    void setPollDelay(const std::chrono::milliseconds d);
    std::chrono::milliseconds getPollDelay();

    void setScalingFactor(double s);
    double getScalingFactor();

    void init() override;
    void beginFrame() override;
    void endFrame() override;
    void exec() override;

private:
    std::shared_ptr<VRPNPhantomDevice> phantom;
    std::shared_ptr<Core::BaseMesh> mesh;
    std::chrono::milliseconds delay;
    Eigen::Transform<double,3,Eigen::Affine> prevTrans;
    double scalingFactor;
};

} //Common
} //Examples
} //mstk