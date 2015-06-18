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

// SimMedTK includes
#include "smCore/smCollisionPair.h"
#include "smCollision/smMeshToMeshCollision.h"
#include "smCollision/smCollisionMoller.h"
#include "smCollision/smMeshCollisionModel.h"

// STL includes
#include <vector>

void smMeshToMeshCollision::doComputeCollision(std::shared_ptr<smCollisionPair> pairs)
{
    auto meshA = std::static_pointer_cast<smMeshCollisionModel>(pairs->getFirst());
    auto meshB = std::static_pointer_cast<smMeshCollisionModel>(pairs->getSecond());

    std::vector<smMeshCollisionModel::NodePairType>
    intersectionNodes = meshA->getAABBTree()->getIntersectingNodes(meshB->getAABBTree());

    double depth;
    smVec3d normal;
    smVec3d contactPoint;
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
            const smVec3d& normalA = meshA->getNormal(i);
            if(normalA.isZero())
            {
                continue;
            }

            auto verticesA = meshA->getTrianglePositions(i);
            for(auto & j : triangleListB)
            {
                const smVec3d& normalB = meshB->getNormal(j);
                if(normalB.isZero())
                {
                    continue;
                }

                auto verticesB = meshB->getTrianglePositions(j);

                // Chech for intersection
                if(smCollisionMoller::tri2tri(verticesA[0], verticesA[1], verticesA[2],
                                              verticesB[0], verticesB[1], verticesB[2],
                                              depth, contactPoint, normal)
                  )
                {
                    //pairs->addContact(std::abs(depth), contactPoint, normal);
                }
            }
        }
    }
}
