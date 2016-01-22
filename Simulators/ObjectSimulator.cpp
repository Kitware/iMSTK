// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#include "Simulators/ObjectSimulator.h"
#include "SceneModels/SceneObject.h"

ObjectSimulator::ObjectSimulator():
    enabled(false),
    isObjectSimInitialized(false),
    timeStep(.01),
    timerPerFrame(0.0),
    framesPerSecond(0.0),
    frameCounter(0),
    totalTime(0.0),
    executionTypeStatusChanged(false),
    executionType(ExecutionType::SyncMode)
{
    this->name = "Controller-" + std::to_string(CoreClass::getUniqueId()->getId());
    this->type = core::ClassType::Simulator;
}

//---------------------------------------------------------------------------
void ObjectSimulator::addModel(std::shared_ptr< SceneObject > model)
{
    if(!model)
    {
        // TODO: log this
        return;
    }

    model->attachObjectSimulator(this->safeDownCast<ObjectSimulator>());
    this->simulatedModels.emplace_back(model);
}

//---------------------------------------------------------------------------
void ObjectSimulator::removeModel(const std::shared_ptr< SceneObject > &model)
{
    if(!model)
    {
        // TODO: log this
        return;
    }

    auto item = std::find(std::begin(this->simulatedModels),
                          std::end(this->simulatedModels), model);

    if(item != std::end(this->simulatedModels))
    {
        this->simulatedModels.erase(item);
    }
    else
    {
        // TODO: Log this.
    }
}

//---------------------------------------------------------------------------
void ObjectSimulator::setExecutionType(const ObjectSimulator::ExecutionType &type)
{
    this->executionType = type;
    this->executionTypeStatusChanged = true;
}

//---------------------------------------------------------------------------
ObjectSimulator::ExecutionType ObjectSimulator::getExecutionType() const
{
    return this->executionType;
}

//---------------------------------------------------------------------------
bool ObjectSimulator::isEnabled()
{
    return this->enabled;
}

//---------------------------------------------------------------------------
void ObjectSimulator::setEnabled(bool value)
{
    this->enabled = value;
}

//---------------------------------------------------------------------------
void ObjectSimulator::initialize()
{
    if(!this->isObjectSimInitialized)
    {
        this->isObjectSimInitialized = true;
    }
}

//---------------------------------------------------------------------------
void ObjectSimulator::syncBuffers() {}

//---------------------------------------------------------------------------
void ObjectSimulator::exec()
{
    this->beginExecution();
    this->run();
    this->endExecution();
}

//---------------------------------------------------------------------------
void ObjectSimulator::beginExecution()
{
    frameCounter++;
    timer.start();
}

//---------------------------------------------------------------------------
void ObjectSimulator::endExecution()
{
    timerPerFrame = timer.elapsed();
    totalTime += timerPerFrame;

    if(totalTime > 1.0)
    {
        framesPerSecond = frameCounter;
        frameCounter = 0.0;
        totalTime = 0.0;
    }
}

//---------------------------------------------------------------------------
void ObjectSimulator::setTimeStep(const double newTimeStep)
{
    this->timeStep = newTimeStep;
}

//---------------------------------------------------------------------------
double ObjectSimulator::getTimeStep() const
{
    return this->timeStep;
}

//---------------------------------------------------------------------------
void ObjectSimulator::run()
{
    for(auto & model : this->simulatedModels)
    {
        model->update(this->timeStep);
    }
}
