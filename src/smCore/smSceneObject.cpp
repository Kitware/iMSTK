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

#include "smCore/smSceneObject.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smSDK.h"

///attach the simulator to the  object
void smSceneObject::attachObjectSimulator(smObjectSimulator *p_objectSim)
{

    p_objectSim->addObject(this);
}

///release the simulator from the object
void smSceneObject::releaseObjectSimulator()
{

    objectSim->removeObject(this);
    objectSim = NULL;
}

///get a reference to the simulator
smObjectSimulator* smSceneObject::getObjectSimulator()
{

    return objectSim;
}

///attach the custom renderer to the scene object
void smSceneObject::attachCustomRenderer(smCustomRenderer *p_customeRenderer)
{

    customRender = p_customeRenderer;
}

///release the custom renderer from the class
void smSceneObject::releaseCustomeRenderer()
{
    customRender = NULL;
}

smSceneObject::smSceneObject()
{

    type = SIMMEDTK_SMSCENEBOJECT;
    objectSim = NULL;
    customRender = NULL;
    smSDK::registerSceneObject(this);
    memBlock = new smMemoryBlock(NULL);
    flags.isViewerInit = false;
    flags.isSimulatorInit = false;
    name = QString("SceneObject") + QString().setNum(uniqueId.ID);
}
