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
    sdk = smSDK::createSDK();

    //Create a new scene to work in
    scene1 = sdk->createScene();

    //Create a viewer to see the scene
    viewer = sdk->createViewer();

    /// create a FEM simulator
    femSim = std::make_shared<smVegaFemSimulator>(sdk->getErrorLog());

    /// set the dispatcher for FEM. it will be used for sending events
//     femSim->setDispatcher(sdk->getEventDispatcher());
//     sdk->getEventDispatcher()->registerEventHandler(femSim, SIMMEDTK_EVENTTYPE_HAPTICOUT);

    /// create a Vega based FEM object and attach it to the fem simulator
    femobj = std::make_shared<smVegaFemSceneObject>(sdk->getErrorLog(),
                                      "res/asianDragon.config");
    femobj->attachObjectSimulator(femSim);

    /// add the FEM object to the scene
    scene1->addSceneObject(femobj);

    /// create a simulator module and register FEM simulator
    simulator = sdk->createSimulator();
    simulator->registerObjectSimulator(femSim);

    /// create a viewer
    viewer->viewerRenderDetail = viewer->viewerRenderDetail & 
                                                    SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;  
//     viewer.setEventDispatcher(sdk->getEventDispatcher());

    /// run the SDK
    sdk->run();
}

void VegaFemExample()
{
    vegaFemExample *ve = new vegaFemExample();
    delete ve;
}

