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

#ifndef SMDUMMYSIMULATOR_H
#define SMDUMMYSIMULATOR_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"

/// \brief Example simulator. This dummy simulator works on static scene objects for now.
class smDummySimulator: public smObjectSimulator
{

public:
    /// \brief constructor
    smDummySimulator(std::shared_ptr<smErrorLog> p_errorLog);
    virtual ~smDummySimulator(){}

protected:
    virtual void beginSim() override;

    /// \brief !!
    virtual void initCustom() override;

    /// \brief advance the simulator in time in a loop here
    virtual void run() override;

    /// \brief !!
    void endSim() override;

    /// \brief synchronize the buffers in the object (do not call by yourself).
    void syncBuffers() override;

    /// \brief catch events such as key presses and other user inputs
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;
};

#endif
