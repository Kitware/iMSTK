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
#include "Core/Config.h"
#include "Core/CoreClass.h"

/// \brief The dispacther class is responsible of taking care of the tasks that need to
///     proppgated over the whole modules. For instance, adding a new phyiscs at run-time.
class Dispatcher: public CoreClass
{

public:
    ///
    /// \brief Constructor
    ///
    Dispatcher();

    ///
    /// \brief This function takes care of handling of system level tasks
    ///     before or after every frame
    /// \param p_caller        p_caller has pointer to the object which the
    ///     function is called
    /// \param p_callerState   p_callerState is stores the state of the at the
    ///     moment of the function called. Such as whether it is the beginning
    ///     of the simulator frameor end of the simulator frame for now etc..
    ///
    core::ResultDispatcher handle(std::shared_ptr<CoreClass> p_caller,
                                  core::CallerState p_callerState);

private:
    ///
    /// \brief handle viewer events
    /// \param p_caller this is the caller class
    /// \param p_callerState it indicates the state of the
    ///     called which is predefined at the Config.h
    ///
    void handleViewer(std::shared_ptr<CoreClass> p_caller,
                      core::CallerState p_callerState);

    ///
    /// \brief Handle simulator events
    /// \param p_caller this is the caller class
    /// \param p_callerState it indicates the state of the
    ///     called which is predefined at the Config.h
    ///
    void handleSimulator(std::shared_ptr<CoreClass> p_caller,
                         core::CallerState p_callerState);

    ///
    /// \brief Handle collision detection events
    /// \param p_caller this is the caller class
    /// \param p_callerState it indicates the state of the
    ///     called which is predefined at the Config.h
    ///
    void handleCollisionDetection(std::shared_ptr<CoreClass> p_caller,
                                  core::CallerState p_callerState);

    ///
    /// \brief Handle collision response events
    /// \param p_caller this is the caller class
    /// \param p_callerState it indicates the state of the
    ///     called which is predefined at the Config.h
    ///
    void handleCollisionResponse(std::shared_ptr<CoreClass> p_caller,
                                 core::CallerState p_callerState);

    ///
    /// \brief Handle all events
    ///
    void handleAll();

};

#endif
