///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/
//
//#pragma once
//
//#include "imstkLoopThreadObject.h"
//
//#include <map>
//#include <string>
//
//#include <vrpn_Connection.h>
//#include <vrpn_MainloopContainer.h>
//
//namespace imstk
//{
/////
///// \brief Enumeration for device types
/////
//enum class DeviceType
//{
//    SpaceExplorer3DConnexion,
//    Navigator3DConnexion,
//    NovintFalcon,
//    PhantomOmni,
//    OSVR_HDK,
//    Arduino
//};
//
/////
///// \class VRPNDeviceServer
///// \brief Devices server using VRPN
/////
//class VRPNDeviceServer : public LoopThreadObject
//{
//public:
//
//    ///
//    /// \brief Constructor
//    ///
//    VRPNDeviceServer(const std::string& machine = "localhost", int port = vrpn_DEFAULT_LISTEN_PORT_NO) :
//        m_machine(machine),
//        m_port(port),
//        LoopThreadObject(machine + ":" + std::to_string(port))
//    {}
//
//    ///
//    /// \brief Destructor
//    ///
//    virtual ~VRPNDeviceServer() override = default;
//
//    ///
//    /// \brief Add device
//    ///
//    void addDevice(const std::string& deviceName, DeviceType deviceType, int id = 0);
//
//    ///
//    /// \brief Add serial device
//    ///
//    void addSerialDevice(const std::string& deviceName, DeviceType deviceType, const std::string& port = "COM6", int baudRate = 57600, int id = 0);
//
//protected:
//
//    ///
//    /// \brief Initialize the server module
//    ///
//    void initThread() override;
//
//    ///
//    /// \brief Run the server module
//    ///
//    void updateThread() override;
//
//    ///
//    /// \brief Clean the server module
//    ///
//    void stopThread() override;
//
//private:
//
//    ///
//    /// \struct
//    /// \brief TODO
//    ///
//    struct SerialInfo
//    {
//        int baudRate;
//        std::string port;
//    };
//
//    const std::string m_machine;                                       ///< machine name or IP
//    const int m_port;                                                  ///< connection port
//
//    std::map<std::string, std::pair<DeviceType, int>> m_deviceInfoMap; ///< list of iMSTK client info
//    std::map<std::string, SerialInfo> m_SerialInfoMap;
//    vrpn_Connection* m_serverConnection = nullptr;                     ///< VRPN server connection
//    vrpn_MainloopContainer* m_deviceConnections = nullptr;             ///< VRPN device connections
//};
//} // imstk
