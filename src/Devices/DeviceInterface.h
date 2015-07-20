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

#ifndef SMINTERFACE_H
#define SMINTERFACE_H

// SimMedTK includes
#include "Core/Config.h"
#include "Core/Module.h"

/// \brief Abstract base interface class with virtual functions.
/// Device specific implementation should be done by instantiating this class
class DeviceInterface: public Module
{

public:
    enum class Message
    {
        Unknown = -2,
        Failure = -1,
        Success = 0
    };

public:
    /// \brief constructor
    DeviceInterface();

    /// \brief destructor
    virtual ~DeviceInterface()
    {

    };

    /// \brief open the device
    virtual Message openDevice()
    {
        return Message::Unknown;
    }

    /// \brief close the device
    virtual Message closeDevice()
    {
        return Message::Unknown;
    }

    /// \brief write data (for ADU interface device)
    virtual Message write(void */*Interfacehandle*/, int /*port*/, void */*data*/)
    {
        return  Message::Unknown;
    };

    /// \brief read data (for ADU interface device)
    virtual Message read(void */*Interfacehandle*/, int /*port*/, void */*data*/)
    {
        return Message::Unknown;
    } ;

protected :
    bool driverInstalled; ///< true if device driver is installed
};

#endif
