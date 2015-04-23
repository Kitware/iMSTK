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

#ifndef SMPBDOBJECTSIMULATOR_H
#define SMPBDOBJECTSIMULATOR_H

// SimMedTK includes
#include "smCore/smObjectSimulator.h"
#include "smCore/smEventHandler.h"

class smErrorLog;
class smPBDSurfaceSceneObject;
class smEvent;
class smDrawParam;

/// \brief Example Position based dynamics (PBD) simulator
class smPBDObjectSimulator: public smObjectSimulator
{

public:
    /// \brief constructor
    smPBDObjectSimulator(std::shared_ptr<smErrorLog> p_errorLog);

protected:
    /// \brief initialize the PBD object
    void initObject(std::shared_ptr<smPBDSurfaceSceneObject> p_object);
    /// \brief !!
    virtual void initCustom();

    /// \brief advance PBD simulator in a loop here
    virtual void run();

    /// \brief !! synchronize the buffers in the object..do not call by yourself.
    void syncBuffers();

    /// \brief handle key presses and other user events
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event);

    /// \brief render the PBD objects
    void draw(const smDrawParam &p_params);

};

#endif
