// This file is part of the iMSTK project.
//
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

#include "Devices/VRPNDeviceServer.h"

// STL includes
#include <thread>

// VRPN includes
#include <vrpn_Connection.h>
#include <vrpn_Tracker_Filter.h>
#include <vrpn_3DConnexion.h>
#include <vrpn_Tracker_RazerHydra.h>
#include <vrpn_Xkeys.h>
#include <vrpn_Tracker_OSVRHackerDevKit.h>

#ifdef VRPN_USE_PHANTOM_SERVER
#include <server_src/vrpn_Phantom.h>
#endif

namespace imstk {

VRPNDeviceServer::VRPNDeviceServer()
{
    this->name = "VRPNDeviceServer";
    this->connection = vrpn_create_server_connection();
}

//---------------------------------------------------------------------------
bool VRPNDeviceServer::addDeviceClient(
        const std::shared_ptr<VRPNDeviceClient> newDeviceClient,
        const bool addFiltering)
{
    std::string newDeviceURL = newDeviceClient->getDeviceURL();

    // Check that it is localhost
    std::string newDeviceIP = newDeviceURL.substr(newDeviceURL.find_last_of("@") + 1);
    if (newDeviceIP != "localhost")
    {
        std::cerr<< "addDeviceClient error: VRPNDeviceServer can only communicate "
                 << "with devices connected locally (devicename@localhost)." << std::endl;
        return EXIT_FAILURE;
    }

    // Check that name not already use by a deviceClient
    std::string newDeviceName = newDeviceURL.substr(0, newDeviceURL.find_last_of("@"));
    if( this->analogDevicesList.find(newDeviceName) != this->analogDevicesList.end()
            && this->trackerDevicesList.find(newDeviceName) != this->trackerDevicesList.end()
            && this->buttonDevicesList.find(newDeviceName) != this->buttonDevicesList.end()
            )
    {
        std::cerr<< "addDeviceClient error: name already use by another client ("
                 << newDeviceName << ")" << std::endl;
        return EXIT_FAILURE;
    }

    // Instantiate a VRPN device depending on the device type
    std::shared_ptr<vrpn_Analog> vrpnAnalogDevice;
    std::shared_ptr<vrpn_Tracker> vrpnTrackerDevice;
    std::shared_ptr<vrpn_Button> vrpnButtonDevice;
    switch( newDeviceClient->getDeviceType() )
    {
    case DeviceType::SPACE_EXPLORER_3DCONNEXION:
        vrpnAnalogDevice = std::make_shared<vrpn_3DConnexion_SpaceExplorer>
                (newDeviceName.c_str(), this->connection);
    break;
    case DeviceType::NAVIGATOR_3DCONNEXION:
        vrpnAnalogDevice = std::make_shared<vrpn_3DConnexion_Navigator>
                (newDeviceName.c_str(), this->connection);
    break;
    case DeviceType::RAZER_HYDRA:
        vrpnTrackerDevice = std::make_shared<vrpn_Tracker_RazerHydra>
                (newDeviceName.c_str(), this->connection);
    break;
    case DeviceType::XKEYS_XK3:
        vrpnButtonDevice = std::make_shared<vrpn_Xkeys_XK3>
                (newDeviceName.c_str(), this->connection);
    break;
    case DeviceType::OSVR_HDK:
        vrpnTrackerDevice = std::make_shared<vrpn_Tracker_OSVRHackerDevKit>
                (newDeviceName.c_str(), this->connection);
    break;
    case DeviceType::PHANTOM_OMNI:
#ifdef VRPN_USE_PHANTOM_SERVER
        vrpnDevice = std::make_shared<vrpn_Phantom> // TODO
                (newDeviceName.c_str(), this->connection, 60.0f, "Default PHANToM");
#else
        std::cerr<< "addDeviceClient error: needs VRPN_USE_PHANTOM_SERVER to be true "
                 << "to connect a Panthom omni device." << std::endl;
        return EXIT_FAILURE;
#endif
    break;
    default:
        std::cerr<< "addDeviceClient error: unknown device type."
                 << std::endl;
        return EXIT_FAILURE;
    }

    // Add vrpn device in list
    if(vrpnAnalogDevice != nullptr)
    {
        this->analogDevicesList[newDeviceName] = vrpnAnalogDevice;
        std::cout<<newDeviceName<<" successfully added."<<std::endl;
    }
    else if(vrpnTrackerDevice != nullptr)
    {
        this->trackerDevicesList[newDeviceName] = vrpnTrackerDevice;
        std::cout<<newDeviceName<<" successfully added."<<std::endl;
    }
    else if(vrpnButtonDevice != nullptr)
    {
        this->buttonDevicesList[newDeviceName] = vrpnButtonDevice;
        std::cout<<newDeviceName<<" successfully added."<<std::endl;
    }
    else
    {
        std::cerr<< "addDeviceClient error: could not instantiate VRPN device."
                 << std::endl;
        return EXIT_FAILURE;
    }

    // Add One Euro Filter
    if( addFiltering && vrpnTrackerDevice != nullptr)
    {
        std::string filterName = newDeviceName +"_Filter";

        std::shared_ptr<vrpn_Tracker_FilterOneEuro> filter =
                std::make_shared<vrpn_Tracker_FilterOneEuro>
                (filterName.c_str(), this->connection, newDeviceName.c_str(), 7);

        this->trackerDevicesList[filterName] = filter;
        std::cout<<filterName<<" successfully added."<<std::endl;
    }
    else if( addFiltering )
    {
        std::cerr<< "addDeviceClient warning: can not filter device which is not a tracker."
                 << std::endl;
    }

    return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
void VRPNDeviceServer::exec()
{
    while(!this->terminateExecution)
    {
        for( auto& device : this->analogDevicesList )
        {
            device.second->mainloop();
        }
        for( auto& device : this->trackerDevicesList )
        {
            device.second->mainloop();
        }
        for( auto& device : this->buttonDevicesList )
        {
            device.second->mainloop();
        }
        this->connection->mainloop();

        std::this_thread::sleep_for(this->pollDelay);
    }

    // connections allocated with vrpn_create_server_connection()
    // must decrement their reference to be auto-deleted by VRPN
    std::cout<<"VRPNDeviceServer: removing connection references"<<std::endl;
    this->connection->removeReference();

    std::cout<<"VRPNDeviceServer: closing server"<<std::endl;
    delete(this->connection);

    this->terminationCompleted = true;
}

}
