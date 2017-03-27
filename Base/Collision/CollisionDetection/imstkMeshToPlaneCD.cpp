/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version B.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-B.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#include "imstkMeshToPlaneCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkPlane.h"
#include "imstkMesh.h"

#include <g3log/g3log.hpp>

namespace imstk
{

void
MeshToPlaneCD::computeCollisionData()
{
    // Clear collisionData
    m_colData.clearAll();

    // Get plane properties
    auto planePos = m_plane->getPosition();

    // TODO: Fix this issue of extra computation in future
    auto rotatedNormal = m_plane->getOrientation()*m_plane->getNormal();

    size_t nodeId = 0;
    for (const auto& p : m_mesh->getVertexPositions())
    {
        auto peneDistance = (planePos - p).dot(rotatedNormal);
        if (peneDistance <= 0.0)
        {
            m_colData.MAColData.push_back({ nodeId, rotatedNormal * -peneDistance });
        }
        nodeId++;
    }

}

} // imstk