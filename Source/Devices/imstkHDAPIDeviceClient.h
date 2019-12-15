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

#ifndef imstkHDAPIDeviceClient_h
#define imstkHDAPIDeviceClient_h

#include <map>

#include "imstkDeviceClient.h"

#include <HD/hd.h>

#include <memory>

namespace imstk
{
struct HD_state
{
    HDdouble pos[3];
    HDdouble vel[3];
    HDdouble trans[16];
    HDint buttons;
};

///
/// \class HDAPIDeviceClient
/// \brief Subclass of DeviceClient for phantom omni
///
class HDAPIDeviceClient : public DeviceClient
{
public:

    ///
    /// \brief Constructor/Destructor
    ///
    HDAPIDeviceClient(std::string name) : DeviceClient(name, "localhost") {}
    virtual ~HDAPIDeviceClient() {}

protected:

    friend class HDAPIDeviceServer;

    ///
    /// \brief Initialize the phantom omni device
    ///
    void init();

    ///
    /// \brief Use callback to get tracking data from phantom omni
    ///
    void run();

    ///
    /// \brief Closes the phantom omni device
    ///
    void cleanUp();

private:
    ///
    /// \brief Phantom omni device api callback
    ///
    static HDCallbackCode HDCALLBACK hapticCallback(void* pData);

    HHD      m_handle; ///< device handle
    HD_state m_state;  ///< device reading state
};
}

#endif // ifndef imstkHDAPIDeviceClient_h
#endif // ifdef iMSTK_USE_OpenHaptics
