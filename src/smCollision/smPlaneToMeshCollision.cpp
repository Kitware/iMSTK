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

#include "smPlaneToMeshCollision.h"

// SimMedTK includes
#include "smCollision/smCollisionMoller.h"
#include "smCore/smCollisionPair.h"
#include "smCollision/smMeshCollisionModel.h"
#include "smCollision/smPlaneCollisionModel.h"

// STL includes
#include <limits>

void smPlaneToMeshCollision::doComputeCollision(std::shared_ptr<smCollisionPair> pair)
{
    auto mesh = std::static_pointer_cast<smMeshCollisionModel>(pair->getFirst());
    auto plane = std::static_pointer_cast<smPlaneCollisionModel>(pair->getSecond());

    if (!mesh || !plane)
    {
        return;
    }

    double d;
    smVec3d planeNormal = plane->getPlaneModel()->getUnitNormal();
    smVec3d planePos = plane->getPlaneModel()->getPoint();

    smVec3d vert;
    pair->clearContacts();
    for (int i = 0; i < mesh->getVertices().size(); i++)//const auto& vertex : mesh->getVertices()
    {
        vert = mesh->getVertices()[i];
        d = planeNormal.dot(vert - planePos);

        if (d < std::numeric_limits<float>::epsilon())
        {
            pair->addContact(d, vert, i, planeNormal);// Create contact
        }
    }
    /*std::cout << "@ Collision detection\n";
    pair->printCollisionPairs();*/
}
