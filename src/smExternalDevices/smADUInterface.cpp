/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================

 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */


#ifdef WIN32 //right now adutux(aduhid on linux) requires a different interface

#define __CPLUSPLUS

#include "smExternalDevices/smADUInterface.h"
#include <iostream>
#include <fstream>
#include "smExternalDevices/AduHid.h"

using namespace std;

/// \brief default constructor
void smADUInterface::init()
{

}

/// \brief
void smADUInterface::exec()
{

    this->start();
}

/// \brief
smADUInterface::smADUInterface()
{

    serialNumber = new smChar[10];
    isOpened = false;
    calibrationData = new ADUDeviceCalibrationData();
    deviceData = new ADUDeviceData();
    serialNumber = "B02363";

    calibrationData->minValue1 = 0;
    calibrationData->maxValue1 = 1;
    calibrationData->minValue2 = 0;
    calibrationData->maxValue2 = 1;

    if (openDevice(serialNumber) == SIMMEDTK_MSG_SUCCESS)
    {
        cout << "ADU USB Device Opened Successfully" << endl;
    }
    else
    {
        cout << "Check the USB connection of the ADU device or the serial number: Couldn't initialize the device" << endl;
    }

    ADUpipe = new smPipe("ADU_Data", sizeof(ADUDeviceData), 10);
}

/// \brief
smADUInterface::smADUInterface(char *calibrationFile)
{

// Read device serial number and calibration values
    serialNumber = new smChar[10];
    isOpened = false;
    calibrationData = new ADUDeviceCalibrationData();
    deviceData = new ADUDeviceData();
    ifstream reader;
    reader.open(calibrationFile, ios::in);

    if (!reader)
    {
        cout << " ADU Device Calibration Data File Could Not Be Loaded For Reading" << endl;
        cout << " Check the location for file : " << deviceData << endl;
    }

    smChar buffer[100];
    smInt i;
    string t;
    string s;

    while (!reader.eof())
    {
        reader.getline(buffer, 50);

        t = buffer;

        i = t.find("Serial Number:");

        if (i != string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            strcpy(serialNumber, s.c_str());
            //Debug
            cout << "Serial Number of this device is" << serialNumber << endl;
        }
        else
        {
            cout << "Couldn't find the serial number for this ADU Device -- Check calibration file" << endl;
        }

        s.clear();
        i = t.find("AN0MIN:");

        if (i != string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->minValue1 = atoi(s.c_str());
        }

        s.clear();
        i = t.find("AN0MAX:");

        if (i != string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->maxValue1 = atoi(s.c_str());
        }

        s.clear();
        i = t.find("AN1MIN:");

        if (i != string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->minValue2 = atoi(s.c_str());
        }

        s.clear();
        i = t.find("AN1MAX:");

        if (i != string::npos)
        {
            i = t.find(":");
            s.assign(t, i + 1, t.size());
            calibrationData->maxValue2 = atoi(s.c_str());
        }
    }

    reader.close();

    cout << "Calibration values are : " << calibrationData->minValue1 << " "
         << calibrationData->maxValue1 << " " << calibrationData->minValue2
         << " " << calibrationData->maxValue2 << endl;

    if (openDevice(serialNumber) == SIMMEDTK_MSG_SUCCESS)
    {
        cout << "ADU USB Device Opened Successfully" << endl;
    }
    else
    {
        cout << "Check the USB connection of the ADU device or the serial number: Couldn't initialize the device" << endl;
    }

    ADUpipe = new smPipe("ADU_Data", sizeof(ADUDeviceData), 10);
    sw = 0;
    updateFlag = 0;
}

/// \brief
smADUInterface::~smADUInterface()
{

    closeDevice();
}

/// \brief
smInt smADUInterface::openDevice(char *serialNumber)
{

    deviceHandle = OpenAduDeviceBySerialNumber(serialNumber, 0);

    if ((*((int *)&deviceHandle)) > 0)
    {
        isOpened = true;
        return SIMMEDTK_MSG_SUCCESS;
    }
    else
    {
        isOpened = false;
        return SIMMEDTK_MSG_FAILURE;
    }
}

/// \brief
smInt smADUInterface::closeDevice()
{

    CloseAduDevice(deviceHandle);
    return 0;
}

/// \brief
smInt* smADUInterface::readAnalogInputs()
{

    return 0;
}

/// \brief Support for two inputs
smInt smADUInterface::readAnalogInput(int channel)
{

    smChar *command;
    smChar data[8];

    if (channel == 0)
    {
        command = "RUN00";
    }
    else
    {
        command = "RUN10";
    }

    WriteAduDevice(deviceHandle, command, 5, 0, 0);
    memset(data, 0, sizeof(data));
    ReadAduDevice(deviceHandle, data, 5, 0, 0);

    return atoi(data);
}

/// \brief
void smADUInterface::runDevice()
{

    if (isOpened)
    {
        for (smInt i = 0; i < 2; i++)
        {
            deviceData->anValue[i] = readAnalogInput(i);
        }

        updateFlag = !updateFlag;
    }
}

/// \brief
void smADUInterface::sendDataToPipe()
{

    ADUDeviceData *pipeData;
    pipeData = (ADUDeviceData*)ADUpipe->beginWrite();
    pipeData->calibration[0] = calibrationData->minValue1;
    pipeData->calibration[1] = calibrationData->maxValue1;
    pipeData->calibration[2] = calibrationData->minValue2;
    pipeData->calibration[3] = calibrationData->maxValue2;
    pipeData->anValue[0] = deviceData->anValue[0];
    pipeData->anValue[1] = deviceData->anValue[1];
    pipeData->deviceOpen = isOpened;
    ADUpipe->endWrite(1);
    ADUpipe->acknowledgeValueListeners();
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
