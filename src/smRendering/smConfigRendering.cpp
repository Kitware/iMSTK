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
 
#include "smRendering/smConfigRendering.h"
#include "smShader/smShader.h"

smColor smColor::colorWhite(1.0, 1.0, 1.0, 1.0);
smColor smColor::colorBlue(0.0, 0.0, 1.0, 1.0);
smColor smColor::colorGreen(0.0, 1.0, 0.0, 1.0);
smColor smColor::colorRed(1.0, 0.0, 0.0, 1.0);
smColor smColor::colorGray(0.8, 0.8, 0.8, 1.0);
smColor smColor::colorYellow(1, 1, 0, 1);
smColor smColor::colorPink(1, 0, 1, 1);

void smRenderDetail::addShader(smUnifiedID p_shaderID)
{

    shaderEnable.push_back(true);
    shaders.push_back(p_shaderID);
}
void smRenderDetail::addVAO(smUnifiedID p_shaderID)
{

    VAOs.push_back(p_shaderID);
    VAOEnable.push_back(true);
}
