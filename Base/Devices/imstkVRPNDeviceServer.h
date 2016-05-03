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

#ifndef imstkVRPNDeviceServer_h
#define imstkVRPNDeviceServer_h

#include <vector>
#include <map>
#include <typeinfo>

#include <vrpn_Connection.h>
#include <vrpn_MainloopContainer.h>

#include "imstkModule.h"
#include "imstkVRPNDeviceClient.h"

namespace imstk {

///
/// \class VRPNDeviceServer
/// \brief Devices server using VRPN
///
class VRPNDeviceServer : public Module
{
public:

    VRPNDeviceServer(std::string name = "VRPN Device Server"): Module(name) {}

    virtual ~VRPNDeviceServer() {}

    bool addDeviceClient(const std::shared_ptr<VRPNDeviceClient> deviceClient);

protected:

    void initModule() override;
    void runModule() override;
    void cleanUpModule() override;

private:

    vrpn_Connection * m_connection; //!< VRPN server connection
    vrpn_MainloopContainer* m_devices; //!< VRPN devices connection

    std::vector<std::shared_ptr<VRPNDeviceClient>> m_clientsList; //!< list of iMSTK client info
};
}

#endif // ifndef imstkVRPNDeviceServer_h
