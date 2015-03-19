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

#ifdef _WIN32 //right now adutux(aduhid on linux) requires a different interface

#ifndef SM_ADU_INTERFACE_H

#define SM_ADU_INTERFACE_H

#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include "smCore/smDoubleBuffer.h"

/// \brief holds data for calibration of ADU device
struct ADUDeviceCalibrationData
{
    smUInt maxValue1;
    smUInt minValue1;
    smUInt maxValue2;
    smUInt minValue2;
};

/// \device Container for holding the device data
struct ADUDeviceData
{
    smBool deviceOpen;
    smUInt calibration[4];
    smUInt anValue[3];
};

/// \device Interface class for the ADU device
class smADUInterface: public smDeviceInterface
{

public:
    /// \brief constructor
    smADUInterface();

    /// \brief constructor Open the ADU device with all necessary data recorded in a file
    smADUInterface(const smString& calibrationFile);

    /// \brief destructor
    ~smADUInterface();

    /// \brief Open the ADU device specified by the serial number
    smInt openDevice(const smString& serialNumber);

    /// \brief Close the ADU device
    smInt closeDevice();

    /// \brief Function to be called in a thread for getting device data
    void runDevice();

    /// \brief !!
    smInt readAnalogInput(smInt channel);

    /// \brief !!
    smInt *readAnalogInputs();

    smInt sw; ///< !!
    smBool updateFlag; ///< !!
    void *deviceHandle; ///< !!
    ADUDeviceCalibrationData *calibrationData; ///< calibration data
    ADUDeviceData *deviceData; ///< device data
    smString serialNumber; ///< serial number of the ADU device
    smBool isOpened; ///< !!

    /// \brief !!
    void init();

    /// \brief !!
    void exec();

    /// \briefempty functions for now
    virtual void beginFrame() {};

    /// \brief empty functions for now
    virtual void endFrame() {};

    /// \brief !!
    void draw(const smDrawParam &p_params) {};

    /// \brief
    void run();

    /// \brief
    void handleEvent(smEvent *p_event) {};

public:
    smPipe *ADUpipe; ///< !!

    /// \brief !!
    void sendDataToPipe();
};

#endif

#endif // WIN32 include
