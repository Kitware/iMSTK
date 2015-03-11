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
#include "smCore/smConfig.h"
#include "smCore/smModule.h"
#include "smCore/smEventHandler.h"

/// Declarations and constants for the device
const int   SIMMEDTK_MSG_UNKNOWN = -2;
const int   SIMMEDTK_MSG_FAILURE = -1;
const int   SIMMEDTK_MSG_SUCCESS =  0;

/// \brief Abstract base interface class with virtual functions.
/// Device specific implementation should be done by instantiating this class
class smDeviceInterface: public smModule, smEventHandler
{

public:
    /// \brief constructor
    smDeviceInterface();

    /// \brief destructor
    virtual ~smDeviceInterface()
    {

    };

    /// \brief open the device
    virtual int openDevice()
    {
        return SIMMEDTK_MSG_UNKNOWN;
    }

    /// \brief close the device
    virtual int closeDevice()
    {
        return SIMMEDTK_MSG_UNKNOWN;
    }

    /// \brief write data (for ADU interface device)
    virtual int write(void *Interfacehandle, smInt port, void *data)
    {
        return  SIMMEDTK_MSG_UNKNOWN;
    };

    /// \brief read data (for ADU interface device)
    virtual int read(void *Interfacehandle, smInt port, void *data)
    {
        return SIMMEDTK_MSG_UNKNOWN;
    } ;

protected :
    smBool driverInstalled; ///< true if device driver is installed
};

#endif
