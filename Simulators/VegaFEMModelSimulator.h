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

#ifndef SMVEGAFEMSIMULATOR_H
#define SMVEGAFEMSIMULATOR_H

// SimMedTK includes
#include "Core/Config.h"
#include "Simulators/ObjectSimulator.h"
#include "Core/ErrorLog.h"
#include "Core/Vector.h"

class ToolCoupler;
class VegaFemSceneObject;

/// \brief Interface class between Vega and SimMedTK core
class VegaFEMModelSimulator : public ObjectSimulator
{

public:
    ///
    /// \brief constructor
    ///
    VegaFEMModelSimulator();

    ///
    /// \brief !!
    ///
    virtual void initialize() override;

    ///
    /// \brief handle the keyboard and haptic button press events
    ///
    void handleEvent(std::shared_ptr<core::Event> p_event) override;

    ///
    /// \brief Set haptic tool
    ///
    void setHapticTool(std::shared_ptr<ToolCoupler> tool);

    ///
    /// \brief Get haptic tool
    ///
    std::shared_ptr<ToolCoupler> getHapticTool() const;

    ///
    /// \brief Update haptic forces
    ///
    void updateHapticForces(std::shared_ptr<VegaFemSceneObject> sceneObject);

private:
    core::Vec3d hapticPosition; ///< position of the haptic device end effector
    bool hapticButtonPressed; ///< true of the button is pressed

    std::shared_ptr<ToolCoupler> hapticTool;
};

#endif
