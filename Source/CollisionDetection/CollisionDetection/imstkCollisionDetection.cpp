/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkCollisionDetection.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkCollisionData.h"
#include "imstkOctreeBasedCD.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskNode.h"

namespace imstk
{
CollisionDetection::CollisionDetection(const CollisionDetection::Type& type, std::shared_ptr<CollisionData> colData) : m_type(type),
    m_colData((colData == nullptr) ? std::make_shared<CollisionData>() : colData),
    m_taskNode(std::make_shared<TaskNode>(std::bind(&CollisionDetection::computeCollisionData, this), "CollisionDetection"))
{
}

// Static functions ==>
void
CollisionDetection::addCollisionPairToOctree(const std::shared_ptr<Geometry>&      geomA,
                                             const std::shared_ptr<Geometry>&      geomB,
                                             const Type                            collisionType,
                                             const std::shared_ptr<CollisionData>& collisionData)
{
    auto addToOctree =
        [&](std::shared_ptr<Geometry> geom) {
            if (!s_OctreeCD->hasGeometry(geom->getGlobalIndex()))
            {
                if (geom->getTypeName() == "PointSet")
                {
                    s_OctreeCD->addPointSet(std::dynamic_pointer_cast<PointSet>(geom));
                }
                else if (geom->getTypeName() == "SurfaceMesh")
                {
                    s_OctreeCD->addTriangleMesh(std::dynamic_pointer_cast<SurfaceMesh>(geom));
                }
                else if (std::dynamic_pointer_cast<AnalyticalGeometry>(geom) != nullptr)
                {
                    s_OctreeCD->addAnalyticalGeometry(geom);
                }
            }
        };

    addToOctree(geomA);
    addToOctree(geomB);
    s_OctreeCD->addCollisionPair(geomA, geomB, collisionType, collisionData);
}

void
CollisionDetection::updateInternalOctreeAndDetectCollision()
{
    if (s_OctreeCD->getNumCollisionPairs() > 0)
    {
        s_OctreeCD->update();
        s_OctreeCD->detectCollision();
    }
}

void
CollisionDetection::clearInternalOctree()
{
    s_OctreeCD->clear();
}

// Static octree
std::shared_ptr<OctreeBasedCD> CollisionDetection::s_OctreeCD = std::make_shared<OctreeBasedCD>(Vec3d(0, 0, 0), 100.0, 0.1, 1);
}
