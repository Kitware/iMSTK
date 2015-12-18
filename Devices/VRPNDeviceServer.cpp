// This file is part of the SimMedTK project.
// Copyright (c) Kitware, Inc.
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

#include "Devices/VRPNDeviceServer.h"

// STL includes
#include <thread>

// VRPN includes
#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>
#include <vrpn_Tracker_Filter.h>
#include <vrpn_Tracker_RazerHydra.h>
#include <vrpn_Xkeys.h>

#ifdef VRPN_USE_PHANTOM_SERVER
#include <server_src/vrpn_Phantom.h>
#endif

VRPNDeviceServer::VRPNDeviceServer() : connection(nullptr)
{
    this->name = "VRPNDeviceServer";

}

//---------------------------------------------------------------------------
void VRPNDeviceServer::exec()
{
    connection = vrpn_create_server_connection();

    if(!connection)
    {
        this->terminateExecution = true;
        this->terminationCompleted = true;
        return;
    }

    // Create the various device objects
    std::shared_ptr<vrpn_3DConnexion_SpaceExplorer> explorer =
    std::make_shared<vrpn_3DConnexion_SpaceExplorer>("explorer",
                                                         this->connection);
    std::shared_ptr<vrpn_3DConnexion_Navigator> navigator =
    std::make_shared<vrpn_3DConnexion_Navigator>("navigator",
                                                        this->connection);
    std::shared_ptr<vrpn_Tracker_RazerHydra> razer =
        std::make_shared<vrpn_Tracker_RazerHydra>("razer", this->connection);
    std::shared_ptr<vrpn_Tracker_FilterOneEuro> razerFiltered =
        std::make_shared<vrpn_Tracker_FilterOneEuro>("razer_filtered",
                                                     this->connection,
                                                     "*razer",
                                                     7);
    std::shared_ptr<vrpn_Xkeys_XK3> xkeys =
        std::make_shared<vrpn_Xkeys_XK3>("xkeys", this->connection);

#ifdef VRPN_USE_PHANTOM_SERVER
    std::shared_ptr<vrpn_Phantom> phantom =
        std::make_shared<vrpn_Phantom>("Phantom0",
                                       this->connection,
                                       60.0f,
                                       "Default PHANToM");
    std::shared_ptr<vrpn_Tracker_FilterOneEuro> phantomFiltered =
        std::make_shared<vrpn_Tracker_FilterOneEuro>("phantom_filtered",
                                                     this->connection,
                                                     "*phantom",
                                                     7);
#endif

    while(!this->terminateExecution)
    {
        explorer->mainloop();
        navigator->mainloop();
        razer->mainloop();
        razerFiltered->mainloop();
        xkeys->mainloop();

#ifdef VRPN_USE_PHANTOM_SERVER
        phantom->mainloop();
        phantomFiltered->mainloop();
#endif

        this->connection->mainloop();

        std::this_thread::sleep_for(this->pollDelay);
    }

    // connections allocated with vrpn_create_server_connection()
    // must decrement their reference to be auto-deleted by VRPN
    connection->removeReference();

    this->terminationCompleted = true;
}
