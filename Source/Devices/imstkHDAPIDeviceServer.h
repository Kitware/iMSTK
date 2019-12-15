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

#ifndef imstkHDAPIDeviceServer_h
#define imstkHDAPIDeviceServer_h

#include <vector>
#include <map>
#include <typeinfo>

// imstk
#include "imstkModule.h"
#include "imstkHDAPIDeviceClient.h"

namespace imstk
{
///
/// \class HDAPIDeviceServer
/// \brief Devices server using HDAPI
///
class HDAPIDeviceServer : public Module
{
public:

    ///
    /// \brief Constructor
    ///
    HDAPIDeviceServer() : Module("HDAPIDeviceServer")
    {}

    ///
    /// \brief Destructor
    ///
    virtual ~HDAPIDeviceServer() {}

    ///
    /// \brief Add device client
    ///
    void addDeviceClient(std::shared_ptr<HDAPIDeviceClient> client);

protected:

    ///
    /// \brief Initialize the server module
    ///
    void initModule() override;

    ///
    /// \brief Run the server module
    ///
    void runModule() override;

    ///
    /// \brief Clean the server module
    ///
    void cleanUpModule() override;

private:

    std::vector<std::shared_ptr<HDAPIDeviceClient>> m_deviceClients; ///< list of OpenHaptics
};
} // imstk

#endif // ifndef imstkHDAPIDeviceServer_h
#endif // ifdef iMSTK_USE_OpenHaptics
