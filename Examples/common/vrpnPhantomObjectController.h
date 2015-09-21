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

/// \brief Controls a 3D mesh using data from a Phantom communicating using VRPN
///
class vrpnPhantomObjectController : public Module
{
public:
    vrpnPhantomObjectController(std::shared_ptr<VRPNPhantomDevice> p = nullptr,
                                std::shared_ptr<Core::BaseMesh> m = nullptr);

    /// \brief Set the pointer to the Phantom device to use
    ///
    /// \param p A shared pointer to an allocated VRPNPhantomDevice object
    ///
    void setPhantom(std::shared_ptr<VRPNPhantomDevice> p);

    /// \brief Set the pointer to the mesh to control
    ///
    /// \param m A shared pointer to an allocated BaseMesh object
    ///
    void setMesh(std::shared_ptr<Core::BaseMesh> m);

    /// \brief Set the polling delay of the controller to get new data from the
    /// Phantom device
    ///
    /// \param d The new polling delay to set
    ///
    void setPollDelay(const std::chrono::milliseconds d);

    /// \brief Get the current polling delay
    ///
    /// \return The currently set polling delay
    std::chrono::milliseconds getPollDelay();

    /// \brief Set how much to scale the physical movement by in 3D space
    ///
    /// \param s The new scaling factor to set
    ///
    void setScalingFactor(double s);

    /// \brief Get the current scaling factor
    ///
    /// \return The currently set scaling factor
    double getScalingFactor();

    void init() override;
    void beginFrame() override;
    void endFrame() override;
    void exec() override;

private:
    std::shared_ptr<VRPNPhantomDevice> phantom; ///< Pointer to phantom device
    std::shared_ptr<Core::BaseMesh> mesh; ///< Pointer to controlled mesh
    std::chrono::milliseconds delay; ///< Polling delay
    double scalingFactor; ///< Scaling factor for physical to 3D translation
    core::Quaterniond pRot; ///< Previous rotation quaternion from phantom
    core::Vec3d pPos; ///< Previous position from phantom
    double pScale; ///< Previous scaling factor for use with pPos
};

} //Common
} //Examples
} //mstk