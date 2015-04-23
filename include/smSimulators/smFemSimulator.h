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

#ifndef SMFEMSIMULATOR_H
#define SMFEMSIMULATOR_H

// STL includes
#include <memory>

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"

namespace smtk{
namespace Event{
class smEventHandler;
class smEvent;
}}

/// \brief Example FEM simulator
class smFemSimulator: public smObjectSimulator
{
private:
    smVec3d hapticPosition;
    smBool hapticButtonPressed;
    std::shared_ptr<smtk::Event::smEventHandler> eventHandler;

public:
    /// \brief constructor
    smFemSimulator(std::shared_ptr<smErrorLog> p_errorLog);

protected:
    /// \brief !!
    virtual void beginSim();

    /// \brief !!
    virtual void initCustom();

    /// \brief run the fem simulation in a loop here
    virtual void run();

    /// \brief !!
    void endSim();

    /// \brief synchronize the buffers in the object (do not call by yourself).
    void syncBuffers();

    /// \brief !!
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event);
};

#endif
