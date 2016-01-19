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

#include "PlaneToMeshCollision.h"

// SimMedTK includes
#include "Collision/CollisionMoller.h"
#include "Core/CollisionManager.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/PlaneCollisionModel.h"

// STL includes
#include <limits>

void PlaneToMeshCollision::doComputeCollision(std::shared_ptr<CollisionManager> pair)
{
    auto meshModel = std::static_pointer_cast<MeshCollisionModel>(pair->getFirst());
    auto planeModel = std::static_pointer_cast<PlaneCollisionModel>(pair->getSecond());

    if (!meshModel || !planeModel)
    {
        return;
    }

    core::Vec3d normal = planeModel->getPlaneModel()->getUnitNormal();
    core::Vec3d contactPoint = planeModel->getPlaneModel()->getPoint();

    pair->clearContacts();
    const auto &vertices = meshModel->getVertices();
    for (size_t i = 0, end = vertices.size(); i < end; ++i)
    {
        auto vertex = vertices[i];
        auto d = normal.dot(vertex - contactPoint);

        if (d < std::numeric_limits<float>::epsilon())
        {
            pair->addContact(meshModel, d, vertex, i, normal);// Create contact
            pair->addContact(planeModel, d, vertex-d*normal, i, -normal);// Create contact
        }
    }
}
