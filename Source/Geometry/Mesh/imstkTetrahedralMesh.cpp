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

#include "imstkTetrahedralMesh.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
TetrahedralMesh::TetrahedralMesh(const std::string& name) : VolumetricMesh(name),
    m_tetrahedraIndices(std::make_shared<VecDataArray<int, 4>>())
{
}

void
TetrahedralMesh::initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                            std::shared_ptr<VecDataArray<int, 4>> tetrahedra)
{
    PointSet::initialize(vertices);
    this->setTetrahedraIndices(tetrahedra);

    m_removedMeshElems.resize(tetrahedra->size(), false);
}

void
TetrahedralMesh::clear()
{
    PointSet::clear();
    m_tetrahedraIndices->clear();
}

void
TetrahedralMesh::print() const
{
    PointSet::print();

    LOG(INFO) << "Number of tetrahedra: " << this->getNumTetrahedra();
    LOG(INFO) << "Tetrahedra:";
    for (auto& tet : *m_tetrahedraIndices)
    {
        LOG(INFO) << tet[0] << ", " << tet[1] << ", " << tet[2] << ", " << tet[3];
    }
}

double
TetrahedralMesh::getVolume()
{
    double                         volume   = 0.0;
    const VecDataArray<double, 3>& vertices = *m_vertexPositions;
    for (const Vec4i& tet : *m_tetrahedraIndices)
    {
        const double tetVol = tetVolume(vertices[tet[0]], vertices[tet[1]], vertices[tet[2]], vertices[tet[3]]);
        if (tetVol < 0.0)
        {
            LOG(WARNING) << "Tetrahedron is inverted, has negative volume!";
        }
        volume += tetVol;
    }

    return volume;
}

std::shared_ptr<SurfaceMesh>
TetrahedralMesh::extractSurfaceMesh()
{
    const std::array<Vec3i, 4> facePattern = {
        Vec3i(0, 1, 2), Vec3i(0, 1, 3), Vec3i(0, 2, 3), Vec3i(1, 2, 3)
    };
    const std::array<int, 4>   unusedVert = { 3, 2, 1, 0 };

    // Find and store the tetrahedral faces that are unique
    const VecDataArray<int, 4>&              tetraIndices   = *this->getTetrahedraIndices();
    std::shared_ptr<VecDataArray<double, 3>> tetVerticesPtr = getVertexPositions();
    const VecDataArray<double, 3>&           tetVertices    = *tetVerticesPtr;
    std::shared_ptr<VecDataArray<int, 3>>    triIndicesPtr  = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                    triIndices     = *triIndicesPtr;
    //std::vector<size_t>                   surfaceTriTet;

    // Gives surfaceTri id/faceid -> index of unused vert for face (4 verts per tet, one will be unused)
    std::vector<size_t> tetRemainingVert;

    bool unique;
    int  foundAt;
    int  a, b, c;

    // For every tetrahedron
    for (int i = 0; i < tetraIndices.size(); i++)
    {
        const Vec4i& tet = tetraIndices[i];

        // For every triangle face of the tetrahedron
        for (int t = 0; t < 4; ++t)
        {
            unique  = true;
            foundAt = 0;
            a       = tet[facePattern[t][0]];
            b       = tet[facePattern[t][1]];
            c       = tet[facePattern[t][2]];

            // Search in reverse for matching face (consider using hash/unordered or ordered binary tree instead)
            for (int j = triIndices.size() - 1; j != -1; j--)
            {
                const Vec3i& tri = triIndices[j];
                // Checks all equivalence permutations
                if (((tri[0] == a)
                     && ((tri[1] == b && tri[2] == c) || (tri[1] == c && tri[2] == b)))
                    || ((tri[1] == a)
                        && ((tri[0] == b && tri[2] == c) || (tri[0] == c && tri[2] == b)))
                    || ((tri[2] == a)
                        && ((tri[1] == b && tri[0] == c) || (tri[1] == c && tri[0] == b))))
                {
                    unique  = false;
                    foundAt = j;
                    break;
                }
            }

            // If not found yet, insert as potentially unique face
            if (unique)
            {
                triIndices.push_back(Vec3i(a, b, c));
                //surfaceTriTet.push_back(tetId);
                tetRemainingVert.push_back(tet[unusedVert[t]]);
            }
            // If found, erase face, it is not unique anymore
            else
            {
                triIndices.erase(foundAt);
                tetRemainingVert.erase(tetRemainingVert.begin() + foundAt);
            }
        }
    }
    // Finally we end up with a set of unique faces, surfaceTri

    // Ensure all faces are have correct windings (such that interior vertex of the tet is inside)
    for (int i = 0; i < triIndices.size(); i++)
    {
        const Vec3d& v0       = tetVertices[triIndices[i][0]];
        const Vec3d& v1       = tetVertices[triIndices[i][1]];
        const Vec3d& v2       = tetVertices[triIndices[i][2]];
        const Vec3d  normal   = ((v1 - v0).cross(v2 - v0));
        const Vec3d  centroid = (v0 + v1 + v2) / 3.0;

        // Vertex that does not contribute to the face
        const Vec3d unusedVertex = tetVertices[tetRemainingVert.at(i)];

        // If the normal is correct, it should be pointing in the same direction as the (face centroid-unusedVertex)
        if (normal.dot(centroid - unusedVertex) < 0)
        {
            std::swap(triIndices[i][2], triIndices[i][1]);
        }
    }

    // All the existing triangles are still pointing to the old vertex buffer
    // we need to reindex and make a new vertex buffer

    // Create a map of old to new indices
    std::unordered_map<int, int> oldToNewVertId;
    for (int i = 0; i < triIndices.size(); i++)
    {
        Vec3i& face = triIndices[i];

        // If the vertex hasn't been reassigned
        if (oldToNewVertId.count(face[0]) == 0)
        {
            // Use size as new index
            const int newVertexId = oldToNewVertId.size();
            oldToNewVertId[face[0]] = newVertexId;
            face[0] = newVertexId; // Relabel the old one
        }
        // If the vertex has already been reassigned
        else
        {
            face[0] = oldToNewVertId[face[0]];
        }

        if (oldToNewVertId.count(face[1]) == 0)
        {
            const int newVertexId = oldToNewVertId.size();
            oldToNewVertId[face[1]] = newVertexId;
            face[1] = newVertexId;
        }
        else
        {
            face[1] = oldToNewVertId[face[1]];
        }

        if (oldToNewVertId.count(face[2]) == 0)
        {
            const int newVertexId = oldToNewVertId.size();
            oldToNewVertId[face[2]] = newVertexId;
            face[2] = newVertexId;
        }
        else
        {
            face[2] = oldToNewVertId[face[2]];
        }
    }

    auto                     triVerticesPtr = std::make_shared<VecDataArray<double, 3>>(oldToNewVertId.size());
    VecDataArray<double, 3>& triVertices    = *triVerticesPtr;

    for (auto vertIndexPair : oldToNewVertId)
    {
        const int tetVertId = vertIndexPair.first;
        const int triVertId = vertIndexPair.second;
        // Copy the vertex over
        triVertices[triVertId] = tetVertices[tetVertId];
    }

    // \todo: Copy over attributes (can't be done yet as type copying of data arrays is not possible)

    // Create and attach surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    surfMesh->initialize(triVerticesPtr, triIndicesPtr);
    return surfMesh;
}

void
TetrahedralMesh::computeBarycentricWeights(const size_t& tetId, const Vec3d& pos,
                                           WeightsArray& weights) const
{
    const Vec4i& tetIndices = (*m_tetrahedraIndices)[tetId];
    Vec3d        v[4];
    double       det;

    const VecDataArray<double, 3>& vertices = *m_vertexPositions;
    for (int i = 0; i < 4; ++i)
    {
        v[i] = vertices[tetIndices[i]];
    }

    Mat4d A;
    A << v[0][0], v[0][1], v[0][2], 1, v[1][0], v[1][1], v[1][2], 1, v[2][0], v[2][1], v[2][2], 1, v[3][0], v[3][1], v[3][2], 1;

    det = A.determinant();

    for (int i = 0; i < 4; ++i)
    {
        Mat4d B = A;
        B(i, 0)    = pos[0];
        B(i, 1)    = pos[1];
        B(i, 2)    = pos[2];
        weights[i] = B.determinant() / det;
    }
}

void
TetrahedralMesh::computeTetrahedronBoundingBox(const size_t& tetId, Vec3d& min, Vec3d& max) const
{
    const VecDataArray<double, 3>& vertices     = *m_vertexPositions;
    const VecDataArray<int, 4>&    tetraIndices = *m_tetrahedraIndices;
    auto                           v1 = vertices[tetraIndices[tetId][0]];
    auto                           v2 = vertices[tetraIndices[tetId][1]];
    auto                           v3 = vertices[tetraIndices[tetId][2]];
    auto                           v4 = vertices[tetraIndices[tetId][3]];

    std::array<double, 4> arrayx = { v1[0], v2[0], v3[0], v4[0] };
    std::array<double, 4> arrayy = { v1[1], v2[1], v3[1], v4[1] };
    std::array<double, 4> arrayz = { v1[2], v2[2], v3[2], v4[2] };

    min[0] = *std::min_element(arrayx.begin(), arrayx.end());
    min[1] = *std::min_element(arrayy.begin(), arrayy.end());
    min[2] = *std::min_element(arrayz.begin(), arrayz.end());

    max[0] = *std::max_element(arrayx.begin(), arrayx.end());
    max[1] = *std::max_element(arrayy.begin(), arrayy.end());
    max[2] = *std::max_element(arrayz.begin(), arrayz.end());
}

const Vec4i&
TetrahedralMesh::getTetrahedronIndices(const size_t tetId) const
{
    return (*m_tetrahedraIndices)[tetId];
}

Vec4i&
TetrahedralMesh::getTetrahedronIndices(const size_t tetId)
{
    return (*m_tetrahedraIndices)[tetId];
}

size_t
TetrahedralMesh::getNumTetrahedra() const
{
    return m_tetrahedraIndices->size();
}
}  // namespace imstk
