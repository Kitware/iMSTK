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

#ifndef COLLISIONDETECTIONEXAMPLE_H
#define COLLISIONDETECTIONEXAMPLE_H

#include "Core/Config.h"
#include "Core/ErrorLog.h"
#include "Core/CoreClass.h"
#include "Core/Simulator.h"
#include "Core/StaticSceneObject.h"
#include "Core/SceneObject.h"
#include "Simulators/DummySimulator.h"
#include "ExternalDevices/PhantomInterface.h"
#include "Core/MotionTransformer.h"
#include "Collision/SpatialGrid.h"


class CollisionDetectionExample: public smSimulationMain, public smCoreClass
{

public:
    smStaticSceneObject  *object1;
    smStaticSceneObject  *object2;
    smSDK* simmedtkSDK;
    smScene *scene1;
    smDummySimulator *dummySim;
    smViewer *viewer;
    smSimulator *simulator;
    smPipeRegisteration myCollInformation;
    smPhantomInterface* hapticInterface;
    smHapticCameraTrans *motionTrans;
    smSpatialGrid *spatGrid;
    smLattice *lat;
    smLattice *lat2;

    CollisionDetectionExample();
    void initHapticCamMotion();

    virtual void simulateMain(smSimulationMainParam p_param)
    {
    }

    void CollisionDetectionExample::draw();
    ~CollisionDetectionExample();
};

void collisionDetectionExample();

#endif
