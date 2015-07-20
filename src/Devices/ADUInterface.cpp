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


#ifdef WIN32 //right now adutux(aduhid on linux) requires a different interface

#define __CPLUSPLUS

#include "smExternalDevices/smADUInterface.h"
#include <iostream>
#include <fstream>
#include <string>
#include "smExternalDevices/AduHid.h"

/// \brief default constructor
void smADUInterface::init()
{

}

/// \brief
void smADUInterface::exec()
{
    this->run();
}

/// \brief
smADUInterface::smADUInterface()
{
    isOpened = false;
    calibrationData = new ADUDeviceCalibrationData();
    deviceData = new ADUDeviceData();
    serialNumber = "B02363";

    calibrationData->minValue1 = 0;
    calibrationData->maxValue1 = 1;
    calibrationData->minValue2 = 0;
    calibrationData->maxValue2 = 1;

    if (openDevice(serialNumber) == Message::Success)
    {
        std::cout << "ADU USB Device Opened Successfully" << "\n";
    }
    else
    {
        std::cout << "Check the USB connection of the ADU device or the serial number: Couldn't initialize the device" << "\n";
    }

//     ADUpipe = new smPipe("ADU_Data", sizeof(ADUDeviceData), 10);
}

/// \brief
smADUInterface::smADUInterface(const std::string& calibrationFile)
{

// Read device serial number and calibration values
    isOpened = false;
    calibrationData = new ADUDeviceCalibrationData();
    deviceData = new ADUDeviceData();
    ifstream reader;
    reader.open(calibrationFile, ios::in);

    if (!reader)
    {
        std::cout << " ADU Device Calibration Data File Could Not Be Loaded For Reading" << "\n";
        std::cout << " Check the location for file : " << deviceData << "\n";
    }

    std::string buffer;
    int i;
    std::string t;
    std::string s;

    while (!reader.eof())
    {
        getline(reader, buffer);

        t = buffer;

        i = t.find("Serial Number:");

        if (i != std::string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            serialNumber = s;
            //Debug
            std::cout << "Serial Number of this device is" << serialNumber << "\n";
        }
        else
        {
            std::cout << "Couldn't find the serial number for this ADU Device -- Check calibration file" << "\n";
        }

        s.clear();
        i = t.find("AN0MIN:");

        if (i != std::string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->minValue1 = atoi(s.c_str());
        }

        s.clear();
        i = t.find("AN0MAX:");

        if (i != std::string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->maxValue1 = atoi(s.c_str());
        }

        s.clear();
        i = t.find("AN1MIN:");

        if (i != std::string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->minValue2 = atoi(s.c_str());
        }

        s.clear();
        i = t.find("AN1MAX:");

        if (i != std::string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->maxValue2 = atoi(s.c_str());
        }
    }

    reader.close();

    std::cout << "Calibration values are : " << calibrationData->minValue1 << " "
         << calibrationData->maxValue1 << " " << calibrationData->minValue2
         << " " << calibrationData->maxValue2 << "\n";

    if (openDevice(serialNumber) == Message::Success)
    {
        std::cout << "ADU USB Device Opened Successfully" << "\n";
    }
    else
    {
        std::cout << "Check the USB connection of the ADU device or the serial number: Couldn't initialize the device" << "\n";
    }

//     ADUpipe = new smPipe("ADU_Data", sizeof(ADUDeviceData), 10);
    sw = 0;
    updateFlag = 0;
}

/// \brief
smADUInterface::~smADUInterface()
{

    closeDevice();
}

/// \brief
int smADUInterface::openDevice(const std::string& serialNumber)
{

    deviceHandle = OpenAduDeviceBySerialNumber(serialNumber.c_str(), 0);

    if ((*((int *)&deviceHandle)) > 0)
    {
        isOpened = true;
        return Message::Success;
    }
    else
    {
        isOpened = false;
        return Message::Failure;
    }
}

/// \brief
int smADUInterface::closeDevice()
{

    CloseAduDevice(deviceHandle);
    return 0;
}

/// \brief
int* smADUInterface::readAnalogInputs()
{

    return 0;
}

/// \brief Support for two inputs
int smADUInterface::readAnalogInput(int channel)
{

    std::string command;
    smChar data[8];

    if (channel == 0)
    {
        command = "RUN00";
    }
    else
    {
        command = "RUN10";
    }

    WriteAduDevice(deviceHandle, command.c_str(), 5, 0, 0);
    memset(data, 0, sizeof(data));
    ReadAduDevice(deviceHandle, data, 5, 0, 0);

    return atoi(data);
}

/// \brief
void smADUInterface::runDevice()
{

    if (isOpened)
    {
        for (int i = 0; i < 2; i++)
        {
            deviceData->anValue[i] = readAnalogInput(i);
        }

        updateFlag = !updateFlag;
    }
}

/// \brief
void smADUInterface::sendDataToPipe()
{

//     ADUDeviceData *pipeData;
//     pipeData = (ADUDeviceData*)ADUpipe->beginWrite();
//     pipeData->calibration[0] = calibrationData->minValue1;
//     pipeData->calibration[1] = calibrationData->maxValue1;
//     pipeData->calibration[2] = calibrationData->minValue2;
//     pipeData->calibration[3] = calibrationData->maxValue2;
//     pipeData->anValue[0] = deviceData->anValue[0];
//     pipeData->anValue[1] = deviceData->anValue[1];
//     pipeData->deviceOpen = isOpened;
//     ADUpipe->endWrite(1);
//     ADUpipe->acknowledgeValueListeners();
}

/// \brief
void smADUInterface::run()
{

    while (1)
    {
        runDevice();
        sendDataToPipe();
    }
}

#endif // WIN32
