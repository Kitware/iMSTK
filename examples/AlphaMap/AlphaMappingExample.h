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

#ifndef ALPHAMAPPINGEXAMPLE_H
#define ALPHAMAPPINGEXAMPLE_H

#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSimulator.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smSceneObject.h"
#include "smSimulators/smDummySimulator.h"
#include "smExternalDevices/smPhantomInterface.h"
#include "smUtilities/smMotionTransformer.h"
#include "smShader/metalShader.h"


class AlphaMapExample: public smSimulationMain, public smCoreClass
{

public:
    smStaticSceneObject  *object1;
    smSDK* simmedtkSDK;
    smScene *scene1;
    smViewer *viewer;

    smPhantomInterface* hapticInterface;
    smHapticCameraTrans *motionTrans;

    AlphaMapExample();
    void initHapticCamMotion();

    virtual void simulateMain(smSimulationMainParam p_param)
    {
    }
    void draw() override;
    ~AlphaMapExample();
};

#endif
