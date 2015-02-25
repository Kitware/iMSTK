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

#include "smCore/smObjectSimulator.h"
#include "smRendering/smViewer.h"
#include "smCore/smSDK.h"

void smObjectSimulator::initDraw(smDrawParam p_params)
{

    p_params.rendererObject->addText(name);
}

void smObjectSimulator::draw(smDrawParam p_params)
{

    smString fps(name + " FPS: " + std::to_string(this->FPS));
    p_params.rendererObject->updateText(name, fps);
}

smObjectSimulator::smObjectSimulator(smErrorLog *p_log)
{

    this->log = p_log;
    smSDK::registerObjectSim(this);
    name = "objecSimulator" + std::to_string(uniqueId.ID);

    type = SIMMEDTK_SMOBJECTSIMULATOR;
    isObjectSimInitialized = false;
    threadPriority = SIMMEDTK_THREAD_NORMALPRIORITY;

    objectsSimulated.clear();

    //this statement is very important do not delete this
    setAutoDelete(false);
    FPS = 0.0;
    frameCounter = 0;
    totalTime = 0.0;
    timer.start();
    enabled = false;
    executionTypeStatusChanged = false;
    execType = SIMMEDTK_SIMEXECUTION_SYNCMODE;
}
