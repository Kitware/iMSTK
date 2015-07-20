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

#include "Core/smConfig.h"

#ifndef __linux__

#ifndef SM_NIUSB6008_INTERFACE_H

#define SM_NIUSB6008_INTERFACE_H

#include "smExternalDevices/DeviceInterface.h"
#include "Core/Event.h"
#include "Core/smEventData.h"
#include "Core/smPipe.h"

#include <NIDAQmx.h>

/// \brief National Instruments USB6008 related data
struct NIUSB6008Data
{
    bool on;
    float value[3];
};

/// \brief data related to tools used for laparoscopic surgery simulators
struct toolData
{
    char type[5];
    float32 min;
    float32 max;
    float32 invRange;
};

/// \brief National Instruments USB6008 interface class
class NIUSB6008Interface: public DeviceInterface
{

public:
    NIUSB6008Interface(int VBLaST_Task_ID);
    ~NIUSB6008Interface();

//     smPipe *NIUSB6008pipe; ///<
    int32 NI_error; ///<
    char NI_errBuff[2048]; ///<
    int initCount; ///<
    bool NI_on; ///<
    float32 minValue[2]; ///<
    float32 maxValue[2]; ///<
    float32 invRange[2]; ///<
    int32 count; ///<
    float64 sampdata[30]; ///<
    TaskHandle taskHandle; ///<
    float64 aveData[3]; ///<
    int nbrRegTool; ///<
    toolData *regTool; ///<
    int nbrTotalChannel; ///<
    toolData installedTool[8]; ///< total number of AI channels of the device = 8
    int nbrActiveChannel; ///<
    int activeChannel[3]; ///< maximum number of active channel is 3s
    int taskID;

public:

    /// \brief Initialize NIUSB6008 device
    void init()
    {
    };

    /// \brief start the device
    void exec()
    {
        this->run();
    };

    /// \brief !!
    void run();

    /// \brief handle event related to NIUSB6008 device
    void handleEvent(std::shared_ptr<core::Event> event) override
    {
    };

    /// \brief empty functions for now
    virtual void beginFrame()
    {
    };

    /// \brief empty functions for now
    virtual void endFrame()
    {
    };

    /// \brief !!
    void sendDataToPipe();

    /// \brief !!
    void initNI_Error(int32 error);

    /// \brief get tool calibration data from external file (hard-coded file name)
    void getToolCalibrationData();

    /// \brief !!
    void setTool();

    /// \brief get the data form registered tools
    void getToolData(int nc, int *ac);

    /// \brief !!
    friend int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle,
                                            int32 everyNsamplesEventType,
                                            uInt32 nSamples, void *callbackData);
    /// \brief !!
    friend int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status,
                                          void *callbackData);
};

#endif

#endif //__linux__
