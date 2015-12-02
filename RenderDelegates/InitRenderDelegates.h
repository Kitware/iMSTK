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
#ifndef SMRENDERDELEGATES_CONFIG_H
#define SMRENDERDELEGATES_CONFIG_H

/// NOTE: This file should be only included once by the application.

#include "Core/Config.h"

#define SIMMEDTK_REGISTER_RENDER_DELEGATES() \
    SIMMEDTK_RUN_LOADER(register_OpenGLViewer); \
    SIMMEDTK_RUN_LOADER(register_AABBRenderDelegate); \
    SIMMEDTK_RUN_LOADER(register_MeshRenderDelegate); \
    SIMMEDTK_RUN_LOADER(register_PlaneRenderDelegate); \
    SIMMEDTK_RUN_LOADER(register_SceneObjectDeformableRenderDelegate); \
    SIMMEDTK_RUN_LOADER(register_StaticSceneObjectRenderDelegate); \
    SIMMEDTK_RUN_LOADER(register_SurfaceTreeRenderDelegate);

void InitRenderDelegates();

#endif // SMRENDERDELEGATES_CONFIG_H
