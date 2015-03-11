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

// STD includes
#include <cmath>

// SimMedTK includes
#include "smRendering/smLight.h"
#include "smUtilities/smQuaternion.h"

smVec3f smLight::defaultDir(0, 0, -1.0);
smVec3f smLight::defaultUpDir(0, 1, 0.0);
smVec3f smLight::defaultTransDir(1, 0, 0.0);

void smLight::updateDirection()
{
    smFloat angle;
    smVec3f dirNorm = direction.normalized();

    angle = std::acos(dirNorm.dot(defaultDir));
    smVec3f axisOfRot = dirNorm.cross(defaultDir).normalized();

    smQuaternionf rot = getRotationQuaternion(-angle,axisOfRot);

    upVector = rot*defaultUpDir;
    transverseDir = rot*defaultTransDir;
}
