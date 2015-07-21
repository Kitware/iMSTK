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

#include "Devices/DeviceInterface.h"
#include "Core/Event.h"

/// \brief holds data for calibration of ADU device
struct ADUDeviceCalibrationData
{
    unsigned int maxValue1;
    unsigned int minValue1;
    unsigned int maxValue2;
    unsigned int minValue2;
};

/// \device Container for holding the device data
struct ADUDeviceData
{
    bool deviceOpen;
    unsigned int calibration[4];
    unsigned int anValue[3];
};

/// \device Interface class for the ADU device
class ADUInterface: public DeviceInterface
{

public:
    /// \brief constructor
    ADUInterface();

    /// \brief constructor Open the ADU device with all necessary data recorded in a file
    ADUInterface(const std::string& calibrationFile);

    /// \brief destructor
    ~ADUInterface();

    /// \brief Open the ADU device specified by the serial number
    int openDevice(const std::string& serialNumber);

    /// \brief Close the ADU device
    int closeDevice();

    /// \brief Function to be called in a thread for getting device data
    void runDevice();

    /// \brief !!
    int readAnalogInput(int channel);

    /// \brief !!
    int *readAnalogInputs();

    int sw; ///< !!
    bool updateFlag; ///< !!
    void *deviceHandle; ///< !!
    ADUDeviceCalibrationData *calibrationData; ///< calibration data
    ADUDeviceData *deviceData; ///< device data
    std::string serialNumber; ///< serial number of the ADU device
    bool isOpened; ///< !!

    /// \brief !!
    void init() override;

    /// \brief !!
    void exec() override;

    /// \briefempty functions for now
    virtual void beginFrame() override {};

    /// \brief empty functions for now
    virtual void endFrame() override {};

    /// \brief !!
    void draw() override {};

    /// \brief
    void run() override;

    /// \brief
    void handleEvent(std::shared_ptr<core::Event> event)  override{};

public:
//     smPipe *ADUpipe; ///< !!

    /// \brief !!
    void sendDataToPipe();
};

#endif

#endif // WIN32 include
