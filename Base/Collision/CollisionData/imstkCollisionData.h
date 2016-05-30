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

#ifndef imstkCollisionData_h
#define imstkCollisionData_h

#include "imstkMath.h"

namespace imstk {

struct PositionDirectionCollisionData
{
    Vec3d position;
    Vec3d direction;
    double penetrationDepth;
};

struct VertexDirectionCollisionData
{
    size_t vertexId;
    Vec3d direction;
    double penetrationDepth;
};

struct VertexTriangleCollisionData
{
    size_t vertexId;
    size_t triangleId;
    double penetrationDepth;
};

struct EdgeEdgeCollisionData
{
    size_t edgeId1;
    size_t edgeId2;
    double shortestDistance;
};

class CollisionData
{
public:

    void clearAll()
    {
        PDColData.clear();
        VDColData.clear();
        VTColData.clear();
        EEColData.clear();
    }

    std::vector<PositionDirectionCollisionData> PDColData; //!< Position Direction collision data
    std::vector<VertexDirectionCollisionData> VDColData;   //!< Vertex Direction collision data
    std::vector<VertexTriangleCollisionData> VTColData;    //!< Vertex Triangle collision data
    std::vector<EdgeEdgeCollisionData> EEColData;          //!< Edge Edge collision data
};
}

#endif // ifndef imstkCollisionData_h
