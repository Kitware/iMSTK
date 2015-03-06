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

#ifndef RENDER_CUBE_H
#define RENDER_CUBE_H

#include "smCore/smSDK.h"

class RenderCube
    : public smSimulationMain,
      public smCoreClass,
      public smEventHandler
{
public:
    RenderCube();
    ~RenderCube();
    void simulateMain(smSimulationMainParam p_param);
    void handleEvent(smEvent *p_event);
private:
    smSDK* simmedtkSDK;
    smScene *scene1;
    smStaticSceneObject cube;
    smViewer viewer;

    void setupLights();
    void setupCamera();
};

void runRenderCube();

#endif
