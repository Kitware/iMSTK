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

#include "vegaFemExample.h"

/// \brief Create a scene with one fem object using vega library
vegaFemExample::vegaFemExample()
{

    //Create an instance of the SoFMIS framework/SDK
    sofmisSDK = smSDK::createSDK();

    //Create a new scene to work in
    scene1 = sofmisSDK->createScene();

    //Create a viewer to see the scene
    viewer = sofmisSDK->createViewer();

    /// create a FEM simulator
    femSim = new smVegaFemSimulator(sofmisSDK->getErrorLog());

    /// set the dispatcher for FEM. it will be used for sending events
    femSim->setDispatcher(sofmisSDK->getEventDispatcher());
    sofmisSDK->getEventDispatcher()->registerEventHandler(femSim, SIMMEDTK_EVENTTYPE_HAPTICOUT);

    /// create a Vega based FEM object and attach it to the fem simulator
    femobj = new smVegaFemSceneObject(sofmisSDK->getErrorLog(),
                                      "../../../resources/vega/asianDragon/asianDragon.config");
    femobj->attachObjectSimulator(femSim);

    /// add the FEM object to the scene
    scene1->addSceneObject(femobj);

    /// create a simulator module and register FEM simulator
    simulator = sofmisSDK->createSimulator();
    simulator->registerObjectSimulator(femSim);

    /// create a viewer
    viewer->viewerRenderDetail = viewer->viewerRenderDetail | SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;
    viewer->setEventDispatcher(sofmisSDK->getEventDispatcher());

    /// run the SDK
    sofmisSDK->run();
}

void VegaFemExample()
{
    vegaFemExample *ve = new vegaFemExample();
    delete ve;
}

