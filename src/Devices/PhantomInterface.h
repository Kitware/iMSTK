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

#ifndef SMPHANTOMINTERFACE_H
#define SMPHANTOMINTERFACE_H

#include "smHapticInterface.h"
#include <HD/hd.h>
#include <HDU/hduVector.h>
#include <HDU/hduError.h>
#include "smCore/smEventHandler.h"

const int SM_MAX_PHANTOM_DEVICES = 4;

/// \brief class to use phantom omni device
class smPhantomInterface: public smHapticInterface
{

protected:
    smEvent *hapticEvent[SM_MAX_PHANTOM_DEVICES]; ///<
    smHapticOutEventData *hapticEventData[SM_MAX_PHANTOM_DEVICES]; ///<

public:
    bool forceEnabled;

    /// \brief constructor initialize the device
    smPhantomInterface();

    /// \brief destructor close the device; stop scheduler
    ~smPhantomInterface();

    /// \brief !!
    int openDevice();

    /// \brief !!
    int closeDevice();

    /// \brief !!
    int openDevice(int phantomNumber) ;

    /// \brief !!
    int openDevice(std::string phantomName);

    /// \brief start device scheduler
    int startDevice();

    /// \brief get phantom position
    int getPosition(smVec3 <smDouble>& d_pos);

    /// \brief get phantom orientation
    int getOreintation(smMatrix33 <smDouble> *d_rot);

    /// \brief get phantom transformation
    int getDeviceTransform(smMatrix44 <smDouble> *d_transform);

    HHD dHandle[SM_MAX_PHANTOM_DEVICES]; ///< handles for devices available
    int numPhantomDevices; ///< number of phantom devices
    hduVector3Dd  position[SM_MAX_PHANTOM_DEVICES]; ///< position
    hduVector3Dd  velocity[SM_MAX_PHANTOM_DEVICES]; ///< velocity
    hduVector3Dd  angles[SM_MAX_PHANTOM_DEVICES]; ///< !! angles of arms
    hduVector3Dd  force[SM_MAX_PHANTOM_DEVICES]; ///< force
    hduVector3Dd  torque[SM_MAX_PHANTOM_DEVICES];  ///< torque (if any)
    smDouble transform[SM_MAX_PHANTOM_DEVICES][16]; ///< transforms
    hapticDeviceData_t hapticDeviceData[SM_MAX_PHANTOM_DEVICES]; ///< haptic device data
    HDSchedulerHandle hapticCallbackHandle; ///< !!

    std::string phantomDeviceNames[SM_MAX_PHANTOM_DEVICES]; ///< names of phantoms

    /// \brief empty functions for now
    virtual void beginFrame() override
    {
    };

    /// \brief empty functions for now
    virtual void endFrame() override
    {
    };

    friend HDCallbackCode HDCALLBACK hapticCallback(void *pData); ///< !!

    /// \brief handle events related to phantom omni
    void handleEvent(std::shared_ptr<mstk::Event::smEvent> event) override;

    /// \brief initialize (nothing happens)
    void init() override;

    /// \brief start device
    void exec() override;

    /// \brief draw the phantom configuration for visualization
    void draw() override;

};

#endif
