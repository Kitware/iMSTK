/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#pragma once

#include <functional>
#include <vtkCommand.h>

namespace imstk
{
class SimulationManager;

/// \brief TODO
class OpenVRCommand : public vtkCommand
{
public:
    static OpenVRCommand* New()
    {
        return new OpenVRCommand;
    }

    ///
    /// \brief Set the simulation manager
    ///
    void SetSimulationManager(SimulationManager* manager = nullptr)
    {
        m_simManager = manager;
    }

    ///
    /// \brief TODO
    ///
    virtual void Execute(
        vtkObject*    caller,
        unsigned long eventId,
        void*         callData) override;

protected:

    ///
    /// \brief Constructor
    ///
    OpenVRCommand() = default;

    ///
    /// \brief Destructor
    ///
    virtual ~OpenVRCommand() = default;

    SimulationManager* m_simManager = nullptr; ///> SimulationManager owning the current simulation being interacted with
};
} // imstk
