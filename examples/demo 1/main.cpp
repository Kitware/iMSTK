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

// Core SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSDK.h"
#include "smRendering/smViewer.h"
#include "smCore/smSceneObject.h"

// Include required types scene objects
#include "smSimulators/smVegaFemSceneObject.h"
#include "smCore/smStaticSceneObject.h"

// Include required simulators
#include "smSimulators/smVegaFemSimulator.h"
#include "smSimulators/smDummySimulator.h"

int main()
{
    //-------------------------------------------------------
    // 1. Create an instance of the SoFMIS framework/SDK
    // 2. Create viewer
    // 3. Create default scene (scene 0)
    //-------------------------------------------------------
    std::unique_ptr<smSDK> sdk = smSDK::createStandardSDK();

    //-------------------------------------------------------
    // Create scene actor 1:  fem scene object + fem simulator
    //-------------------------------------------------------

    // create a FEM simulator
    std::shared_ptr<smVegaFemSimulator> femSimulator = 
                            std::make_shared<smVegaFemSimulator>(sdk->getErrorLog());

    // create a Vega based FEM object and attach it to the fem simulator
    std::make_shared<smVegaFemSceneObject> femObject = 
                            std::make_shared<smVegaFemSceneObject>(sdk->getErrorLog(),
                            "../../../resources/vega/asianDragon/asianDragon.config");

    sdk->addSceneActor(femObject, femSimulator);

    //-------------------------------------------------------
    // Create scene actor 2:  plane + dummy simulator
    //-------------------------------------------------------

    // Create dummy simulator
    std::make_shared<smDummySimulator> staticSimulator =
                                std::make_shared<smDummySimulator>(sdk->getErrorLog());

    // create a static scene object
    std::shared_ptr<smStaticSceneObject> staticObject = 
                                                std::make_shared<smStaticSceneObject>();

    sdk->addSceneActor(staticObject, staticSimulator);

    //-------------------------------------------------------
    // Customize viewer
    //-------------------------------------------------------
    cstd::shared_ptr<smStaticSceneObject> viewer = sdk->getViewerInstance();

    viewer->viewerRenderDetail = viewer->viewerRenderDetail | 
                                                    SIMMEDTK_VIEWERRENDER_FADEBACKGROUND;

    viewer->setEventDispatcher(sdk->getEventDispatcher());

    //-------------------------------------------------------
    // Run the SDK
    //-------------------------------------------------------
    sdk->run();

    return 0;
}
