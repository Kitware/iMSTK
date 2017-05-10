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

#include "imstkVRPNArduinoDeviceClient.h"

#include "g3log/g3log.hpp"

namespace imstk
{
void
VRPNArduinoDeviceClient::initModule()
{
    auto fullDeviceIp = this->getName().c_str();

    m_vrpnAnalog = std::make_shared<vrpn_Analog_Remote>(fullDeviceIp);
    m_vrpnAnalog->register_change_handler(this, analogChangeHandler);

}

void
VRPNArduinoDeviceClient::runModule()
{
    m_vrpnAnalog->mainloop();
}

void
VRPNArduinoDeviceClient::cleanUpModule()
{

    m_vrpnAnalog->unregister_change_handler(this, analogChangeHandler);

    m_vrpnAnalog.reset();

}


void VRPN_CALLBACK
VRPNArduinoDeviceClient::analogChangeHandler(void *userData, const _vrpn_ANALOGCB a)
{
    auto deviceClient = reinterpret_cast<VRPNArduinoDeviceClient*>(userData);

    //deviceClient->m_ypr << a.channel[0], a.channel[1], a.channel[2];
    //deviceClient->m_accel << a.channel[3], a.channel[4], a.channel[5];
    deviceClient->m_roll = a.channel[0];

}


} // imstk
