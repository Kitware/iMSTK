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
#include "imstkCollisionUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkGeometryUtilities.h"

struct EdgePair
{
    EdgePair(uint32_t a1, uint32_t a2, uint32_t b1, uint32_t b2)
    {
        edgeA[0] = a1;
        edgeA[1] = a2;
        edgeB[0] = b1;
        edgeB[1] = b2;

        edgeAId = getIdA();
        edgeBId = getIdB();
    }

    ///
    /// \brief Reversible edges are equivalent, reversible vertices in the edges are equivalent as well
    /// EdgePair(0,1,5,2)==EdgePair(1,0,5,2)==EdgePair(1,0,2,5)==...
    ///
    bool operator==(const EdgePair& other) const
    {
        return (edgeAId == other.edgeAId && edgeBId == other.edgeBId)
               || (edgeAId == other.edgeBId && edgeBId == other.edgeAId);
    }

    // These functions return a unique int for an edge, order doesn't matter
    // ie: f(vertexId1, vertexId2)=f(vertexId2, vertexId1)
    const uint32_t getIdA() const
    {
        const uint32_t max = std::max(edgeA[0], edgeA[1]);
        const uint32_t min = std::min(edgeA[0], edgeA[1]);
        return max * (max + 1) / 2 + min;
    }

    const uint32_t getIdB() const
    {
        const uint32_t max = std::max(edgeB[0], edgeB[1]);
        const uint32_t min = std::min(edgeB[0], edgeB[1]);
        return max * (max + 1) / 2 + min;
    }

    uint32_t edgeA[2];
    uint32_t edgeAId;
    uint32_t edgeB[2];
    uint32_t edgeBId;
};

namespace std
{
template<>
struct hash<EdgePair>
{
    // EdgePair has 4 uints to hash, they bound the same range, 0 to max vertices of a mesh
    // A complete unique hash split into 4, would limit us to 256 max vertices so we will have
    // collisions but they will be unlikely given small portions of the mesh are in contact at
    // any one time
    std::size_t operator()(const EdgePair& k) const
    {
        // Shift by 8 each time, there will be overlap every 256 ints
        //return ((k.edgeA[0] ^ (k.edgeA[1] << 8)) ^ (k.edgeB[0] << 16)) ^ (k.edgeB[1] << 24);

        // The edge ids are more compact since f(1,0)=f(0,1) there are fewer permutations,
        // This should allow up to ~360 max vertices..., not that much better
        return (k.edgeAId ^ (k.edgeBId << 16));
    }
};
}

namespace imstk
{
SurfaceMeshToSurfaceMeshCD::SurfaceMeshToSurfaceMeshCD()
{
    setRequiredInputType<SurfaceMesh>(0);
    setRequiredInputType<SurfaceMesh>(1);

    // By default generate contact data for both sides
    setGenerateCD(true, true);
}

void
SurfaceMeshToSurfaceMeshCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>          geomA,
    std::shared_ptr<Geometry>          geomB,
    CDElementVector<CollisionElement>& elementsA,
    CDElementVector<CollisionElement>& elementsB)
{
    std::shared_ptr<SurfaceMesh>             surfMeshA    = std::dynamic_pointer_cast<SurfaceMesh>(geomA);
    std::shared_ptr<VecDataArray<double, 3>> verticesAPtr = surfMeshA->getVertexPositions();
    VecDataArray<double, 3>&                 verticesA    = *verticesAPtr;
    std::shared_ptr<VecDataArray<int, 3>>    indicesAPtr  = surfMeshA->getTriangleIndices();
    const VecDataArray<int, 3>&              indicesA     = *indicesAPtr;

    std::shared_ptr<SurfaceMesh>             surfMeshB    = std::dynamic_pointer_cast<SurfaceMesh>(geomB);
    std::shared_ptr<VecDataArray<double, 3>> verticesBPtr = surfMeshB->getVertexPositions();
    VecDataArray<double, 3>&                 verticesB    = *verticesBPtr;
    std::shared_ptr<VecDataArray<int, 3>>    indicesBPtr  = surfMeshB->getTriangleIndices();
    const VecDataArray<int, 3>&              indicesB     = *indicesBPtr;

    std::unordered_set<EdgePair> edges;
    for (int i = 0; i < indicesA.size(); i++)
    {
        const Vec3i& cellA = indicesA[i];
        for (int j = 0; j < indicesB.size(); j++)
        {
            const Vec3i& cellB = indicesB[j];

            // vtContact needs to be checked both ways but eeContact is symmetric
            std::pair<Vec2i, Vec2i> eeContact;
            std::pair<int, Vec3i>   vtContact;
            std::pair<Vec3i, int>   tvContact;
            const int               contactType = CollisionUtils::triangleToTriangle(cellA, cellB,
                verticesA[cellA[0]], verticesA[cellA[1]], verticesA[cellA[2]],
                verticesB[cellB[0]], verticesB[cellB[1]], verticesB[cellB[2]],
                eeContact, vtContact, tvContact);

            // If you want to visualize the cells in contact
            // report triangle vs triangle instead
            /* CellIndexElement elemB;
            elemB.idCount = 3;
            elemB.cellType = IMSTK_TRIANGLE;
            elemB.ids[0] = cellB[0];
            elemB.ids[1] = cellB[1];
            elemB.ids[2] = cellB[2];
            CellIndexElement elemA;
            elemA.idCount = 3;
            elemA.cellType = IMSTK_TRIANGLE;
            elemA.ids[0] = cellA[0];
            elemA.ids[1] = cellA[1];
            elemA.ids[2] = cellA[2];
            elementsA.unsafeAppend(elemA);
            elementsB.unsafeAppend(elemB);*/

            // Type 1, vertex-triangle contact
            if (contactType == 1)
            {
                CellIndexElement elemA;
                elemA.idCount  = 1;
                elemA.cellType = IMSTK_VERTEX;
                elemA.ids[0]   = vtContact.first;

                CellIndexElement elemB;
                elemB.idCount  = 3;
                elemB.cellType = IMSTK_TRIANGLE;
                elemB.ids[0]   = vtContact.second[0];
                elemB.ids[1]   = vtContact.second[1];
                elemB.ids[2]   = vtContact.second[2];

                elementsA.unsafeAppend(elemA);
                elementsB.unsafeAppend(elemB);
            }
            // Type 0, edge-edge contact
            else if (contactType == 0)
            {
                // Create an edge pair and hash it to see if we already have this contact from
                // another triangle
                const EdgePair edgePair = {
                    static_cast<uint32_t>(eeContact.first[0]),
                    static_cast<uint32_t>(eeContact.first[1]),
                    static_cast<uint32_t>(eeContact.second[0]),
                    static_cast<uint32_t>(eeContact.second[1]) };
                if (edges.count(edgePair) == 0)
                {
                    CellIndexElement elemA;
                    elemA.idCount  = 2;
                    elemA.cellType = IMSTK_EDGE;
                    elemA.ids[0]   = eeContact.first[0];
                    elemA.ids[1]   = eeContact.first[1];

                    CellIndexElement elemB;
                    elemB.idCount  = 2;
                    elemB.cellType = IMSTK_EDGE;
                    elemB.ids[0]   = eeContact.second[0];
                    elemB.ids[1]   = eeContact.second[1];

                    elementsA.unsafeAppend(elemA);
                    elementsB.unsafeAppend(elemB);
                    edges.insert(edgePair);
                }
            }
            // Type 3, triangle-vertex contact
            else if (contactType == 2)
            {
                CellIndexElement elemA;
                elemA.idCount  = 3;
                elemA.cellType = IMSTK_TRIANGLE;
                elemA.ids[0]   = tvContact.first[0];
                elemA.ids[1]   = tvContact.first[1];
                elemA.ids[2]   = tvContact.first[2];

                CellIndexElement elemB;
                elemB.idCount  = 1;
                elemB.cellType = IMSTK_VERTEX;
                elemB.ids[0]   = tvContact.second;

                elementsA.unsafeAppend(elemA);
                elementsB.unsafeAppend(elemB);
            }
            //else
            //{
            //    // This case is hit in one edge case
            //    LOG(WARNING) << "Contact without intersection!";
            //}
        }
    }
}
}