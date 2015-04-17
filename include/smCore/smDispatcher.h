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

#ifndef SMDISPATCHER_H
#define SMDISPATCHER_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"

///The dispacther class is responsible of taking care of the
///tasks that need to proppgated over the whole modules.For instance, adding a new
///phyiscs at run-time
class smDispatcher: public smCoreClass
{

private:
    ///handle viewer events
    /// \param p_caller this is the caller class
    /// \param p_callerState it indicates the state of the called which is predefined at the smConfig.h
    void  handleViewer(std::shared_ptr<smCoreClass> p_caller, smCallerState p_callerState);

    ///handle simulator events
    void  handleSimulator(std::shared_ptr<smCoreClass> p_caller, smCallerState p_callerState);

    ///handle collision detection events
    void  handleCollisionDetection(std::shared_ptr<smCoreClass> p_caller, smCallerState p_callerState);

    ///handle all events
    void  handleAll();

public:
    smDispatcher();

public:
    ///handle all event and call corresponding events
    smDispathcerResult handle(std::shared_ptr<smCoreClass> p_caller, smCallerState p_callerState);

};

#endif
