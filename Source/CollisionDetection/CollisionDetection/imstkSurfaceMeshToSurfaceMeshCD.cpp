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

#include "imstkSurfaceMeshToSurfaceMeshCD.h"
#include "imstkOctreeBasedCD.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
SurfaceMeshToSurfaceMeshCD::SurfaceMeshToSurfaceMeshCD(const std::shared_ptr<SurfaceMesh>&   meshA,
                                                       const std::shared_ptr<SurfaceMesh>&   meshB,
                                                       const std::shared_ptr<CollisionData>& colData) :
    CollisionDetection(CollisionDetection::Type::SurfaceMeshToSurfaceMesh, colData)
{
    if (!s_OctreeCD->hasCollisionPair(meshA->getGlobalIndex(), meshB->getGlobalIndex()))
    {
        addCollisionPairToOctree(std::static_pointer_cast<Geometry>(meshA),
                                 std::static_pointer_cast<Geometry>(meshB),
                                 getType(),
                                 colData);
    }
}
} // imstk
