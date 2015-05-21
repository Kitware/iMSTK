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
demo1Scene::demo1Scene()
{
    //Create an instance of the SoFMIS framework/SDK
    sdk = smSDK::createSDK();

    //Create a new scene to work in
    scene = sdk->createScene();

    //Create a viewer to see the scene
    sdk->addViewer(&viewer);

    //-------------------------------------------------------
    // Create scene object 1:  fem scene object
    //-------------------------------------------------------

    /// create a FEM simulator
    femSim = new smVegaFemSimulator(sdk->getErrorLog());
    sdk->registerObjectSim(femSim);

    /// create a Vega based FEM object and attach it to the fem simulator
    femobj = new smVegaFemSceneObject(sdk->getErrorLog(),
                                      "../../../resources/vega/asianDragon/asianDragon.config");
    femobj->attachObjectSimulator(femSim);

    sdk->registerSceneObject(femobj);

    //-------------------------------------------------------
    // Create scene object 2: static plane
    //-------------------------------------------------------

    // Create dummy simulator
    defaultSimulator = std::make_shared<smDummySimulator>(sdk->getErrorLog());
    sdk->registerObjectSim(defaultSimulator);



    /// add the FEM object to the scene
    scene->addSceneObject(femobj);

    /// create a simulator module and register FEM simulator
    simulator = sdk->createSimulator();
    simulator->registerObjectSimulator(femSim);

    /// create a viewer
    viewer.viewerRenderDetail = viewer.viewerRenderDetail | SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;
    viewer.setEventDispatcher(sdk->getEventDispatcher());

    /// run the SDK
    sdk->run();
}

void buildDemo1Scene()
{
    vegaFemExample *ve = new vegaFemExample();
    delete ve;
}

