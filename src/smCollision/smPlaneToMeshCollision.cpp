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

#include "smCollision/smPlaneToMeshCollision.h"

// SimMedTK includes
#include "smCollision/smCollisionMoller.h"
#include "smCollision/smCollisionPair.h"
#include "smCollision/smMeshCollisionModel.h"
#include "smCollision/smPlaneCollisionModel.h"

// STL includes
#include <limits>

void smPlaneToMeshCollision::doComputeCollision(std::shared_ptr<smCollisionPair> pair)
{
    auto mesh = std::static_pointer_cast<smMeshCollisionModel>(pair->getFirst());
    auto plane = std::static_pointer_cast<smPlaneCollisionModel>(pair->getFirst());

    if (!mesh || !plane)
    {
        return;
    }

    double d;
    smVec3d planeNormal = plane->getNormal();
    float planeOffset = planeNormal.dot(plane->getPosition());
    for (const auto& vertex : mesh->getVertices())
    {
        d = planeNormal.dot(vertex) - planeOffset;
        if (d < std::numeric_limits<float>::epsilon())
        {
            // Create contact
            pair->addContact(-d, vertex, planeNormal);
        }
    }

}