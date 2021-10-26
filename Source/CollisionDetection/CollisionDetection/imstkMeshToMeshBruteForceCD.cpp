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

#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkCollisionUtils.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

#include <unordered_set>

namespace imstk
{
///
/// \brief Hash together a pair of edges
///
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
}

namespace std
{
template<>
struct hash<imstk::EdgePair>
{
    // EdgePair has 4 uints to hash, they bound the same range, 0 to max vertices of a mesh
    // A complete unique hash split into 4, would limit us to 256 max vertices so we will have
    // collisions but they will be unlikely given small portions of the mesh are in contact at
    // any one time
    std::size_t operator()(const imstk::EdgePair& k) const
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
struct PointSetData
{
    PointSetData(std::shared_ptr<PointSet> pointSet);

    // Get geometry A data
    std::shared_ptr<PointSet> m_pointSet;
    const VecDataArray<double, 3>& vertices;
};

struct SurfMeshData
{
    SurfMeshData(std::shared_ptr<SurfaceMesh> surfMesh);

    // Get geometry B data
    std::shared_ptr<SurfaceMesh> m_surfMesh;
    const VecDataArray<int, 3>& cells;
    const VecDataArray<double, 3>& vertices;
    const std::vector<std::set<size_t>>& vertexFaces;
    const VecDataArray<double, 3>& faceNormals;
};

PointSetData::PointSetData(std::shared_ptr<PointSet> pointSet) :
    m_pointSet(pointSet),
    vertices(*pointSet->getVertexPositions())
{
}

SurfMeshData::SurfMeshData(std::shared_ptr<SurfaceMesh> surfMesh) :
    m_surfMesh(surfMesh),
    cells(*surfMesh->getTriangleIndices()),
    vertices(*surfMesh->getVertexPositions()),
    vertexFaces(surfMesh->getVertexNeighborTriangles()),
    faceNormals(*surfMesh->getCellNormals())
{
}

///
/// \brief Compute the pseudonormal of the vertex given by vertexIndex
///
static Vec3d
vertexPseudoNormalFromTriangle(const int vertexIndex, const SurfMeshData& surfMeshData)
{
    // Compute the pseudonormal on the mesh at the point
    // Identify incident faces to the point and weight sum their normals
    double sum  = 0.0;
    Vec3d  nSum = Vec3d::Zero();
    for (size_t neighborFaceIndex : surfMeshData.vertexFaces[vertexIndex])
    {
        Vec3i cell = surfMeshData.cells[neighborFaceIndex];
        // Ensure vertexIndex is in 0
        if (cell[1] == vertexIndex)
        {
            std::swap(cell[0], cell[1]);
        }
        else if (cell[2] == vertexIndex)
        {
            std::swap(cell[0], cell[2]);
        }

        const Vec3d  ab    = (surfMeshData.vertices[cell[1]] - surfMeshData.vertices[vertexIndex]).normalized();
        const Vec3d  bc    = (surfMeshData.vertices[cell[2]] - surfMeshData.vertices[vertexIndex]).normalized();
        const double angle = acos(ab.dot(bc));
        const Vec3d  n     = angle * surfMeshData.faceNormals[neighborFaceIndex];

        sum  += n.norm();
        nSum += n;
    }
    return nSum / sum;
}

///
/// \brief Compute the pseudonormal of the edge given by vertexIds
///
static Vec3d
edgePseudoNormalFromTriangle(const Vec2i& vertexIds, const SurfMeshData& surfMeshData)
{
    // Find the two cells that have both vertexIds
    double sum  = 0.0;
    Vec3d  nSum = Vec3d::Zero();
    for (size_t neighborFaceIndex : surfMeshData.vertexFaces[vertexIds[0]])
    {
        const Vec3i& cell     = surfMeshData.cells[neighborFaceIndex];
        bool         found[2] = { false, false };
        for (int j = 0; j < 3; j++)
        {
            if (cell[j] == vertexIds[0])
            {
                found[0] = true;
            }
            else if (cell[j] == vertexIds[1])
            {
                found[1] = true;
            }
        }
        // If it contains both vertices its a face to the edge
        if (found[0] && found[1])
        {
            const Vec3d n = PI * surfMeshData.faceNormals[neighborFaceIndex];
            sum  += n.norm();
            nSum += n;
        }
    }
    return nSum / sum;
}

///
/// \brief Returns signed distance, reports caseType (ie: which element type
/// is nearest, vertex, edge, or triangle)
/// \param position to measure signed distance from
/// \param SurfaceMesh to measure signed distance too
/// \param nearest element case, vertex-0, edge-1, triangle-2
/// \param vertexIds of nearest element (maximum 3 verts)
///
static double
polySignedDist(const Vec3d& pos, const SurfMeshData& surfMeshData,
               int& caseType, Vec3i& vIds)
{
    int    closestCell     = -1;
    Vec3d  closestPt       = Vec3d::Zero();
    double minSqrDist      = IMSTK_DOUBLE_MAX;
    int    closestCellCase = -1;

    // Find the closest point out of all elements
    // \todo: We could early reject backface cull all triangles (this is effectively case 6 done early)
    for (int j = 0; j < surfMeshData.cells.size(); j++)
    {
        const Vec3i& cell = surfMeshData.cells[j];
        const Vec3d& x1   = surfMeshData.vertices[cell[0]];
        const Vec3d& x2   = surfMeshData.vertices[cell[1]];
        const Vec3d& x3   = surfMeshData.vertices[cell[2]];

        int          ptOnTriangleCaseType;
        const Vec3d  closestPtOnTri = CollisionUtils::closestPointOnTriangle(pos, x1, x2, x3, ptOnTriangleCaseType);
        const double sqrDist = (closestPtOnTri - pos).squaredNorm();
        if (sqrDist < minSqrDist)
        {
            minSqrDist      = sqrDist;
            closestPt       = closestPtOnTri;
            closestCell     = j;
            closestCellCase = ptOnTriangleCaseType;
        }
    }

    // We use the normal of the nearest element to determine sign, but we can't just use the
    // normal as there are discontinuities at the edges and vertices. We instead use the
    // "angle-weighted psuedonormal" given adjacent elements

    // Closest element is a vertex
    if (closestCellCase == 0 || closestCellCase == 1 || closestCellCase == 2)
    {
        int vertexIndex = surfMeshData.cells[closestCell][0]; // a
        if (closestCellCase == 1)
        {
            vertexIndex = surfMeshData.cells[closestCell][1]; // b
        }
        else if (closestCellCase == 2)
        {
            vertexIndex = surfMeshData.cells[closestCell][2]; // c
        }

        const Vec3d psuedoN = vertexPseudoNormalFromTriangle(vertexIndex, surfMeshData);
        caseType = 0;
        vIds[0]  = vertexIndex;
        return (pos - closestPt).dot(psuedoN);
    }
    // Closest element is an edge
    else if (closestCellCase == 3 || closestCellCase == 4 || closestCellCase == 5)
    {
        Vec2i vertexIds = { surfMeshData.cells[closestCell][0], surfMeshData.cells[closestCell][1] }; // ab
        if (closestCellCase == 4)
        {
            vertexIds = { surfMeshData.cells[closestCell][1], surfMeshData.cells[closestCell][2] }; // bc
        }
        else if (closestCellCase == 5)
        {
            vertexIds = { surfMeshData.cells[closestCell][2], surfMeshData.cells[closestCell][0] }; // ca
        }

        const Vec3d psuedoN = edgePseudoNormalFromTriangle(vertexIds, surfMeshData);
        caseType = 1;
        vIds[0]  = vertexIds[0];
        vIds[1]  = vertexIds[1];
        return (pos - closestPt).dot(psuedoN);
    }
    // Closest element is the triangle
    else if (closestCellCase == 6)
    {
        //const Vec3d psuedoN = (2.0 * PI * faceNormals[closestCell]).normalized();
        const Vec3d psuedoN = surfMeshData.faceNormals[closestCell]; // Assume normalized
        caseType = 2;
        vIds[0]  = surfMeshData.cells[closestCell][0];
        vIds[1]  = surfMeshData.cells[closestCell][1];
        vIds[2]  = surfMeshData.cells[closestCell][2];
        return (pos - closestPt).dot(psuedoN);
    }
    // Should only ever occur if there are no elements
    else
    {
        caseType = -1;
        return IMSTK_DOUBLE_MAX;
    }
}

MeshToMeshBruteForceCD::MeshToMeshBruteForceCD()
{
    setRequiredInputType<PointSet>(0);
    setRequiredInputType<SurfaceMesh>(1);
}

void
MeshToMeshBruteForceCD::computeCollisionDataAB(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    // Broad phase collision
    if (doBroadPhaseCollisionCheck(geomA, geomB))
    {
        auto pointSet = std::dynamic_pointer_cast<PointSet>(geomA);
        auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(geomB);
        surfMesh->computeTrianglesNormals();
        surfMesh->computeVertexNeighborTriangles();

        // Narrow phase
        if (m_generateVertexTriangleContacts)
        {
            if (m_vertexInside.size() < pointSet->getNumVertices())
            {
                m_vertexInside = std::vector<bool>(pointSet->getNumVertices(), false);
            }
            else
            {
                for (int i = 0; i < m_vertexInside.size(); i++)
                {
                    m_vertexInside[i] = false;
                }
            }

            vertexToTriangleTest(geomA, geomB, elementsA, elementsB);

            if (m_generateEdgeEdgeContacts)
            {
                if (geomA->getTypeName() == "LineMesh")
                {
                    lineMeshEdgeToTriangleTest(geomA, geomB, elementsA, elementsB);
                }
                else if (geomA->getTypeName() == "SurfaceMesh")
                {
                    surfMeshEdgeToTriangleTest(geomA, geomB, elementsA, elementsB);
                }
            }
        }
    }
}

void
MeshToMeshBruteForceCD::vertexToTriangleTest(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    PointSetData pointSetData(std::dynamic_pointer_cast<PointSet>(geomA));
    SurfMeshData surfMeshData(std::dynamic_pointer_cast<SurfaceMesh>(geomB));

    // For every vertex
    for (int i = 0; i < pointSetData.vertices.size(); i++)
    {
        const Vec3d& p = pointSetData.vertices[i];

        int          caseType   = -1;
        Vec3i        vertexIds  = Vec3i::Zero();
        const double signedDist = polySignedDist(p, surfMeshData, caseType, vertexIds);
        if (signedDist <= 0.0)
        {
            if (caseType == 0)
            {
                CellIndexElement elemA;
                elemA.ids[0]   = i;
                elemA.idCount  = 1;
                elemA.cellType = IMSTK_VERTEX;

                CellIndexElement elemB;
                elemB.ids[0]   = vertexIds[0];
                elemB.idCount  = 1;
                elemB.cellType = IMSTK_VERTEX;

                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                m_vertexInside[i] = true;
            }
            else if (caseType == 1)
            {
                CellIndexElement elemA;
                elemA.ids[0]   = i;
                elemA.idCount  = 1;
                elemA.cellType = IMSTK_VERTEX;

                CellIndexElement elemB;
                elemB.ids[0]   = vertexIds[0];
                elemB.ids[1]   = vertexIds[1];
                elemB.idCount  = 2;
                elemB.cellType = IMSTK_EDGE;

                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                m_vertexInside[i] = true;
            }
            else if (caseType == 2)
            {
                CellIndexElement elemA;
                elemA.ids[0]   = i;
                elemA.idCount  = 1;
                elemA.cellType = IMSTK_VERTEX;

                CellIndexElement elemB;
                elemB.ids[0]   = vertexIds[0];
                elemB.ids[1]   = vertexIds[1];
                elemB.ids[2]   = vertexIds[2];
                elemB.idCount  = 3;
                elemB.cellType = IMSTK_TRIANGLE;

                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
                m_vertexInside[i] = true;
            }
        }
    }
}

void
MeshToMeshBruteForceCD::lineMeshEdgeToTriangleTest(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    SurfMeshData surfMeshBData(std::dynamic_pointer_cast<SurfaceMesh>(geomB));

    // Get geometry A data
    std::shared_ptr<LineMesh>                lineMesh = std::dynamic_pointer_cast<LineMesh>(geomA);
    std::shared_ptr<VecDataArray<double, 3>> meshAVerticesPtr = lineMesh->getVertexPositions();
    const VecDataArray<double, 3>&           meshAVertices    = *meshAVerticesPtr;
    std::shared_ptr<VecDataArray<int, 2>>    meshACellsPtr    = lineMesh->getLinesIndices();
    VecDataArray<int, 2>&                    meshACells       = *meshACellsPtr;

    const int triEdgePattern[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };

    // For every edge/line segment of the line mesh
    for (int i = 0; i < meshACells.size(); i++)
    {
        const Vec2i& edgeA = meshACells[i];

        // Only check edges that don't exist totally inside
        if (!m_vertexInside[edgeA[0]] && !m_vertexInside[edgeA[1]])
        {
            double minSqrDist    = IMSTK_DOUBLE_MAX;
            int    closestTriId  = -1;
            int    closestEdgeId = -1;

            // For every triangle/cell of meshB
            for (int j = 0; j < surfMeshBData.cells.size(); j++)
            {
                const Vec3i& cellB = surfMeshBData.cells[j];

                // For every edge of that triangle
                for (int k = 0; k < 3; k++)
                {
                    const Vec2i edgeB(cellB[triEdgePattern[k][0]], cellB[triEdgePattern[k][1]]);

                    // Compute the closest point on the two edges
                    // Check the case, the edges must be within each others bounds/ranges
                    Vec3d ptA, ptB;
                    if (CollisionUtils::edgeToEdgeClosestPoints(
                        meshAVertices[edgeA[0]], meshAVertices[edgeA[1]],
                        surfMeshBData.vertices[edgeB[0]], surfMeshBData.vertices[edgeB[1]],
                        ptA, ptB) == 0)
                    {
                        // Find the closest element to this point on the edge
                        const double sqrDist = (ptB - ptA).squaredNorm();
                        // Use the closest one only
                        if (sqrDist < minSqrDist)
                        {
                            // Check if the point on the oppositie edge nearest to edgeB is inside B
                            int          caseType   = -1;
                            Vec3i        vIds       = Vec3i::Zero();
                            const double signedDist = polySignedDist(ptA, surfMeshBData, caseType, vIds);
                            if (signedDist <= 0.0)
                            {
                                minSqrDist    = sqrDist;
                                closestTriId  = j;
                                closestEdgeId = k;
                            }
                        }
                    }
                }
            }

            if (closestTriId != -1)
            {
                CellIndexElement elemA;
                elemA.ids[0]   = edgeA[0];
                elemA.ids[1]   = edgeA[1];
                elemA.idCount  = 2;
                elemA.cellType = IMSTK_EDGE;

                CellIndexElement elemB;
                elemB.ids[0]   = surfMeshBData.cells[closestTriId][triEdgePattern[closestEdgeId][0]];
                elemB.ids[1]   = surfMeshBData.cells[closestTriId][triEdgePattern[closestEdgeId][1]];
                elemB.idCount  = 2;
                elemB.cellType = IMSTK_EDGE;

                elementsA.push_back(elemA);
                elementsB.push_back(elemB);
            }
        }
    }
}

void
MeshToMeshBruteForceCD::surfMeshEdgeToTriangleTest(
    std::shared_ptr<Geometry>      geomA,
    std::shared_ptr<Geometry>      geomB,
    std::vector<CollisionElement>& elementsA,
    std::vector<CollisionElement>& elementsB)
{
    SurfMeshData surfMeshBData(std::dynamic_pointer_cast<SurfaceMesh>(geomB));

    // Get geometry A data
    std::shared_ptr<SurfaceMesh>             surfMeshA = std::dynamic_pointer_cast<SurfaceMesh>(geomA);
    std::shared_ptr<VecDataArray<double, 3>> meshAVerticesPtr = surfMeshA->getVertexPositions();
    const VecDataArray<double, 3>&           meshAVertices    = *meshAVerticesPtr;
    std::shared_ptr<VecDataArray<int, 3>>    meshACellsPtr    = surfMeshA->getTriangleIndices();
    VecDataArray<int, 3>&                    meshACells       = *meshACellsPtr;

    std::unordered_set<EdgePair> hashedEdges;

    const int triEdgePattern[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };
    if (m_generateEdgeEdgeContacts)
    {
        // For every edge/line segment of the line mesh
        for (int i = 0; i < meshACells.size(); i++)
        {
            const Vec3i& cellA = meshACells[i];

            // For every edge of triangle A
            for (int j = 0; j < 3; j++)
            {
                const Vec2i edgeA = Vec2i(cellA[triEdgePattern[j][0]], cellA[triEdgePattern[j][1]]);

                // Only check edges that don't exist totally inside
                if (!m_vertexInside[edgeA[0]] && !m_vertexInside[edgeA[1]])
                {
                    double minSqrDist    = IMSTK_DOUBLE_MAX;
                    int    closestTriId  = -1;
                    int    closestEdgeId = -1;

                    // For every triangle/cell of meshB
                    for (int k = 0; k < surfMeshBData.cells.size(); k++)
                    {
                        const Vec3i& cellB = surfMeshBData.cells[k];

                        // For every edge of that triangle
                        for (int l = 0; l < 3; l++)
                        {
                            const Vec2i edgeB(cellB[triEdgePattern[l][0]], cellB[triEdgePattern[l][1]]);

                            // Compute the closest point on the two edges
                            // Check the case, the edges must be within each others bounds/ranges
                            Vec3d ptA, ptB;
                            if (CollisionUtils::edgeToEdgeClosestPoints(
                                meshAVertices[edgeA[0]], meshAVertices[edgeA[1]],
                                surfMeshBData.vertices[edgeB[0]], surfMeshBData.vertices[edgeB[1]],
                                ptA, ptB) == 0)
                            {
                                // Find the closest element to this point on the edge
                                const double sqrDist = (ptB - ptA).squaredNorm();
                                // Use the closest one only
                                if (sqrDist < minSqrDist)
                                {
                                    // Check if the point on the oppositie edge nearest to edgeB is inside B
                                    int          caseType   = -1;
                                    Vec3i        vIds       = Vec3i::Zero();
                                    const double signedDist = polySignedDist(ptA, surfMeshBData, caseType, vIds);
                                    if (signedDist <= 0.0)
                                    {
                                        minSqrDist    = sqrDist;
                                        closestTriId  = k;
                                        closestEdgeId = l;
                                    }
                                }
                            }
                        }
                    }

                    if (closestTriId != -1)
                    {
                        // Before inserting check if it already exists
                        EdgePair edgePair(
                            edgeA[0], edgeA[1],
                            surfMeshBData.cells[closestTriId][triEdgePattern[closestEdgeId][0]],
                            surfMeshBData.cells[closestTriId][triEdgePattern[closestEdgeId][1]]);
                        if (hashedEdges.count(edgePair) == 0)
                        {
                            CellIndexElement elemA;
                            elemA.ids[0]   = edgeA[0];
                            elemA.ids[1]   = edgeA[1];
                            elemA.idCount  = 2;
                            elemA.cellType = IMSTK_EDGE;

                            CellIndexElement elemB;
                            elemB.ids[0]   = surfMeshBData.cells[closestTriId][triEdgePattern[closestEdgeId][0]];
                            elemB.ids[1]   = surfMeshBData.cells[closestTriId][triEdgePattern[closestEdgeId][1]];
                            elemB.idCount  = 2;
                            elemB.cellType = IMSTK_EDGE;

                            elementsA.push_back(elemA);
                            elementsB.push_back(elemB);

                            hashedEdges.insert(edgePair);
                        }
                    }
                }
            }
        }
    }
}

bool
MeshToMeshBruteForceCD::doBroadPhaseCollisionCheck(
    std::shared_ptr<Geometry> geomA,
    std::shared_ptr<Geometry> geomB) const
{
    const auto mesh1 = std::dynamic_pointer_cast<PointSet>(geomA);
    const auto mesh2 = std::dynamic_pointer_cast<PointSet>(geomB);

    // Edge Case Ex: One point vs non-manifold SurfaceMesh (like a single triangle or plane)
    if (mesh1->getNumVertices() == 1 || mesh2->getNumVertices() == 1)
    {
        return true;
    }

    Vec3d min1, max1;
    mesh1->computeBoundingBox(min1, max1);

    Vec3d min2, max2;
    mesh2->computeBoundingBox(min2, max2);

    // Padding here helps with thin vs thin geometry
    min1 -= m_padding;
    max1 += m_padding;
    min2 -= m_padding;
    max2 += m_padding;

    return CollisionUtils::testAABBToAABB(
        min1[0], max1[0],
        min1[1], max1[1],
        min1[2], max1[2],
        min2[0], max2[0],
        min2[1], max2[1],
        min2[2], max2[2]);
}
}