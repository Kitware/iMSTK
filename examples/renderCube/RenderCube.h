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

#include "../common/wasdCameraController.h"

class RenderCube
    : public smSimulationMain,
      public smCoreClass
{
public:
    RenderCube();
    ~RenderCube();
    void simulateMain(const smSimulationMainParam &p_param) override;
private:
    std::shared_ptr<smSDK> sdk;
    std::shared_ptr<smViewer> viewer;
    std::shared_ptr<smScene> scene1;
    std::shared_ptr<smLight> light;
    std::shared_ptr<smCamera> sceneCamera;
    std::shared_ptr<smStaticSceneObject> cube;
    std::shared_ptr<smtk::Examples::Common::wasdCameraController> camCtl;

    void setupLights();
    void setupCamera();
};

void runRenderCube();

#endif
