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

#include "smCore/smConfig.h"

#ifndef SIMMEDTK_OPERATINGSYSTEM_LINUX

#ifndef SM_NIUSB6008_INTERFACE_H

#define SM_NIUSB6008_INTERFACE_H

#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smDoubleBuffer.h"

#include <NIDAQmx.h>

/// \brief National Instruments USB6008 related data
struct NIUSB6008Data
{
    smBool on;
    smFloat value[3];
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
class smNIUSB6008Interface: public smDeviceInterface
{

public:
    smNIUSB6008Interface(int VBLaST_Task_ID);
    ~smNIUSB6008Interface();

    smPipe *NIUSB6008pipe; ///<
    int32 NI_error; ///<
    char NI_errBuff[2048]; ///<
    smInt initCount; ///<
    smBool NI_on; ///<
    float32 minValue[2]; ///<
    float32 maxValue[2]; ///<
    float32 invRange[2]; ///<
    int32 count; ///<
    float64 sampdata[30]; ///<
    TaskHandle taskHandle; ///<
    float64 aveData[3]; ///<
    smInt nbrRegTool; ///<
    toolData *regTool; ///<
    smInt nbrTotalChannel; ///<
    toolData installedTool[8]; ///< total number of AI channels of the device = 8
    smInt nbrActiveChannel; ///<
    smInt activeChannel[3]; ///< maximum number of active channel is 3s
    smInt taskID;

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
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> event) override
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
    void getToolData(smInt nc, smInt *ac);

    /// \brief !!
    friend int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle,
                                            int32 everyNsamplesEventType,
                                            uInt32 nSamples, void *callbackData);
    /// \brief !!
    friend int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status,
                                          void *callbackData);
};

#endif

#endif //SIMMEDTK_OPERATINGSYSTEM_LINUX
