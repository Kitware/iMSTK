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
#ifndef COLLISIONDETECTIONSPATIALHASHING_H
#define COLLISIONDETECTIONSPATIALHASHING_H

// STL includes
#include <memory>
#include <chrono>
#include <thread>

// SimMedTK includes
#include "Core/SDK.h"
#include "Core/Config.h"
#include "Core/ErrorLog.h"
#include "Core/CoreClass.h"
#include "Core/Simulator.h"
#include "Core/StaticSceneObject.h"
#include "Core/SceneObject.h"
#include "Simulators/DummySimulator.h"
#include "Collision/SpatialHashCollision.h"
#include "Rendering/Viewer.h"

class CollisionDetectionSpatialHashing: public smSimulationMain
{

public:
    CollisionDetectionSpatialHashing();
    virtual ~CollisionDetectionSpatialHashing(){}

    void draw ();
    void simulateMain (const smSimulationMainParam &p_param) override;
    void run();

public:
    std::shared_ptr<smSDK>                  sdk;
    std::shared_ptr<smStaticSceneObject>    modelA;
    std::shared_ptr<smStaticSceneObject>    modelB;
    std::shared_ptr<smScene>                scene;
    std::shared_ptr<smViewer>               viewer;
    std::shared_ptr<smDummySimulator>       defaultSimulator;
    std::shared_ptr<smSimulator>            simulator;
    std::shared_ptr<smSpatialHashCollision> spatialHashing;
    smInt moveObj;
};

#endif // COLLISIONDETECTIONSPATIALHASHING_H

