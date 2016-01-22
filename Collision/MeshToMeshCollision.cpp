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

#include "MeshToMeshCollision.h"

// SimMedTK includes
#include "Core/CollisionManager.h"
#include "Collision/CollisionMoller.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/SurfaceTree.h"

// STL includes
#include <vector>

void MeshToMeshCollision::doComputeCollision(std::shared_ptr<CollisionManager> pairs)
{
    auto meshA = std::static_pointer_cast<MeshCollisionModel>(pairs->getFirst());
    auto meshB = std::static_pointer_cast<MeshCollisionModel>(pairs->getSecond());

    std::vector<MeshCollisionModel::NodePairType>
    intersectionNodes = meshA->getAABBTree()->getIntersectingNodes(meshB->getAABBTree());

    double depth;
    core::Vec3d normal;
    core::Vec3d contactPoint;
    for(auto & intersection : intersectionNodes)
    {
        auto nodeA = intersection.first;
        auto nodeB = intersection.second;

        std::vector<size_t> triangleListA;
        std::vector<size_t> triangleListB;

        nodeA->getIntersections(nodeB->getAabb(), triangleListA);
        nodeB->getIntersections(nodeA->getAabb(), triangleListB);

        for(const auto & i : triangleListA)
        {
            const core::Vec3d& normalA = meshA->getSurfaceNormal(i);
            if(normalA.isZero())
            {
                continue;
            }

            auto verticesA = meshA->getElementPositions(i);
            for(auto & j : triangleListB)
            {
                const core::Vec3d& normalB = meshB->getSurfaceNormal(j);
                if(normalB.isZero())
                {
                    continue;
                }

                auto verticesB = meshB->getElementPositions(j);

                // Chech for intersection
                if(CollisionMoller::tri2tri(verticesA[0], verticesA[1], verticesA[2],
                                              verticesB[0], verticesB[1], verticesB[2],
                                              depth, contactPoint, normal)
                  )
                {
                    pairs->addContact(std::abs(depth),contactPoint,0,normal);
                }
            }
        }
    }
}
