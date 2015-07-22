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

class CollisionDetectionSpatialHashing: public SimulationMain
{

public:
    CollisionDetectionSpatialHashing();
    virtual ~CollisionDetectionSpatialHashing(){}

    void draw ();
    void simulateMain (const SimulationMainParam &p_param) override;
    void run();

public:
    std::shared_ptr<SDK>                  sdk;
    std::shared_ptr<StaticSceneObject>    modelA;
    std::shared_ptr<StaticSceneObject>    modelB;
    std::shared_ptr<Scene>                scene;
    std::shared_ptr<Viewer>               viewer;
    std::shared_ptr<DummySimulator>       defaultSimulator;
    std::shared_ptr<Simulator>            simulator;
    std::shared_ptr<SpatialHashCollision> spatialHashing;
    int moveObj;
};

#endif // COLLISIONDETECTIONSPATIALHASHING_H

