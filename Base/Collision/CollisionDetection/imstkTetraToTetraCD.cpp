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

#include "imstkTetraToTetraCD.h"

#include "imstkCollisionData.h"
#include "imstkMath.h"

#include <g3log/g3log.hpp>

namespace imstk {

TetraToTetraCD::TetraToTetraCD(std::shared_ptr<TetrahedralMesh> meshA,
                               std::shared_ptr<TetrahedralMesh> meshB,
                               CollisionData& colData) :
    CollisionDetection(CollisionDetection::Type::MeshToMesh, colData), //is TetrahedralMeshToTetrahedralMesh type needed?
    m_meshA(meshA),
    m_meshB(meshB)
{
}

void
TetraToTetraCD::findCollisionsForMeshWithinHashTable(const std::shared_ptr<TetrahedralMesh> mesh, size_t idOffset)
{
    Vec3d min, max; //bounding box of a tetrahedron
    Vec3d vPos;
    const auto eps = MACHINE_PRECISION;
    const double eps2 = 1e-10;
    
    //tetrahedron belonging part of penetration type does not change
    auto cType = static_cast<PointTetrahedronCollisionData::CollisionType>(idOffset > 0);

    auto nodesMeshA = m_meshA->getVertexPositions();
    auto nodesMeshB = m_meshB->getVertexPositions();

    for (size_t tId = 0; tId < mesh->getNumTetrahedra(); ++tId) //TODO: parallelize!
    {
        TetrahedralMesh::TetraArray vInd = mesh->getTetrahedronVertices(tId);
        for (int i = 0; i < 4; i++) //if idOffset!=0 ?
        {
            vInd[i] += idOffset;
        }
        mesh->computeTetrahedronBoundingBox(tId, min, max);
        std::vector<size_t> collP = m_hashTable.getPointsInAABB(min, max);
        assert(collP.size() >= 4);
        if (collP.size() > 4)
        {
            for (size_t vId : collP)
            {
                //vertex does not belong to this tetrahedron
                if (vId != vInd[0] &&
                    vId != vInd[1] &&
                    vId != vInd[2] &&
                    vId != vInd[3])
                {                    
                    //this determines vertex belonging part of the penetration type
                    //and gets vertex position
                    if (vId < m_meshA->getNumVertices())
                    {
                        vPos = nodesMeshA[vId];
                        cType = static_cast<PointTetrahedronCollisionData::CollisionType>((cType & 1) + 0);
                    }
                    else
                    {
                        vId -= m_meshA->getNumVertices();
                        vPos = nodesMeshB[vId];
                        cType = static_cast<PointTetrahedronCollisionData::CollisionType>((cType & 1) + 2);
                    }

                    TetrahedralMesh::WeightsArray bCoord; //barycentric coordinates of the vertex in tetrahedron
                    mesh->computeBarycentricWeights(tId, vPos, bCoord);
                    if (bCoord[0] >= -eps &&
                        bCoord[1] >= -eps &&
                        bCoord[2] >= -eps &&
                        bCoord[3] >= -eps)
                    {
                        auto coordSum = bCoord[0] + bCoord[1] + bCoord[2] + bCoord[3];
                        assert(coordSum <= 1 + eps2 && coordSum >= 1 - eps2);

                        PointTetrahedronCollisionData ptColl = { cType, vId, tId, bCoord };
                        m_colData.PTColData.push_back(ptColl);
                    }
                } //if not this tetrahedron
            } //for vertices
        }
    } //for tetrahedra
}

void
TetraToTetraCD::computeCollisionData()
{
    m_hashTable.clear();
    m_hashTable.insertPoints(m_meshA->getVertexPositions());
    m_hashTable.insertPoints(m_meshB->getVertexPositions());

    m_colData.clearAll();
    this->findCollisionsForMeshWithinHashTable(m_meshA, 0);
    this->findCollisionsForMeshWithinHashTable(m_meshB, m_meshA->getNumVertices());
}

}
