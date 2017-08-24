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

#ifdef iMSTK_USE_OPENHAPTICS

#include "imstkHDAPIDeviceServer.h"

#include <HD/hd.h>

#include "g3log/g3log.hpp"

namespace imstk
{
void
HDAPIDeviceServer::addDeviceClient(std::shared_ptr<HDAPIDeviceClient> client)
{
    m_deviceClients.push_back(client);
}

void
HDAPIDeviceServer::initModule()
{
    for(const auto& client : m_deviceClients)
    {
        client->init();
    }
    hdStartScheduler();
}

void
HDAPIDeviceServer::runModule()
{
    for(const auto& client : m_deviceClients)
    {
        client->run();
    }
}

void
HDAPIDeviceServer::cleanUpModule()
{
    hdStopScheduler();
    for(const auto& client : m_deviceClients)
    {
        client->cleanUp();
    }
}
} // imstk

#endif // ifdef iMSTK_USE_OMNI
