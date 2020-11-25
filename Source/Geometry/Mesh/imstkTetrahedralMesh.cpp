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
TetrahedralMesh::TetrahedralMesh(const std::string& name) : VolumetricMesh(Geometry::Type::TetrahedralMesh, name),
    m_tetrahedraIndices(std::make_shared<VecDataArray<int, 4>>())
{
}

void
TetrahedralMesh::initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                            std::shared_ptr<VecDataArray<int, 4>> tetrahedra,
                            bool computeAttachedSurfaceMesh)
{
    PointSet::initialize(vertices);
    this->setTetrahedraIndices(tetrahedra);

    if (computeAttachedSurfaceMesh)
    {
        this->computeAttachedSurfaceMesh();
    }

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
    Vec3d                          v[4];
    Mat4d                          A;
    double                         volume   = 0.0;
    const VecDataArray<double, 3>& vertices = *m_vertexPositions;
    for (const Vec4i& tetIndices : *m_tetrahedraIndices)
    {
        for (int i = 0; i < 4; ++i)
        {
            v[i] = vertices[tetIndices[i]];
        }

        A << v[0][0], v[0][1], v[0][2], 1, v[1][0], v[1][1], v[1][2], 1, v[2][0], v[2][1], v[2][2], 1, v[3][0], v[3][1], v[3][2], 1;

        const double det = A.determinant();
        if (det < 0)
        {
            LOG(WARNING) << "Tetrahedron is inverted, has negative volume!";
        }

        volume += std::abs(det) / 6;
    }

    return volume;
}

void
TetrahedralMesh::computeAttachedSurfaceMesh()
{
    this->m_attachedSurfaceMesh = std::make_shared<SurfaceMesh>();

    CHECK(this->extractSurfaceMesh(this->m_attachedSurfaceMesh))
        << "TetrahedralMesh::computeAttachedSurfaceMesh error: surface mesh was not extracted.";
}

bool
TetrahedralMesh::extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
                                    const bool                   enforceWindingConsistency /* = false*/)
{
    CHECK(surfaceMesh != nullptr)
        << "TetrahedralMesh::extractSurfaceMesh error: the surface mesh provided is not instantiated.";

    const std::vector<Vec3i> facePattern = {
        Vec3i(0, 1, 2), Vec3i(0, 1, 3), Vec3i(0, 2, 3), Vec3i(1, 2, 3)
    };

    // Find and store the tetrahedral faces that are unique
    const VecDataArray<int, 4>&           tetraIndices    = *this->getTetrahedraIndices();
    std::shared_ptr<VecDataArray<int, 3>> triangleIndices = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                 surfaceTri      = *triangleIndices;
    std::vector<size_t>                   surfaceTriTet;
    std::vector<size_t>                   tetRemainingVert;
    bool                                  unique;
    int                                   foundAt, tetId = 0;
    int                                   a, b, c;

    for (int i = 0; i < tetraIndices.size(); i++)
    {
        const Vec4i& tet = tetraIndices[i];
        // std::cout << "tet: " << tetId << std::endl;

        for (int t = 0; t < 4; ++t)
        {
            unique  = true;
            foundAt = 0;
            a       = tet[facePattern[t][0]];
            b       = tet[facePattern[t][1]];
            c       = tet[facePattern[t][2]];

            // search in reverse
            for (int i = surfaceTri.size() - 1; i != -1; i--)
            {
                const Vec3i& tri = surfaceTri[i];
                if (((tri[0] == a)
                     && ((tri[1] == b && tri[2] == c) || (tri[1] == c && tri[2] == b)))
                    || ((tri[1] == a)
                        && ((tri[0] == b && tri[2] == c) || (tri[0] == c && tri[2] == b)))
                    || ((tri[2] == a)
                        && ((tri[1] == b && tri[0] == c) || (tri[1] == c && tri[0] == b))))
                {
                    unique  = false;
                    foundAt = i;
                    break;
                }
            }

            if (unique)
            {
                surfaceTri.push_back(Vec3i(a, b, c));
                surfaceTriTet.push_back(tetId);
                tetRemainingVert.push_back(static_cast<size_t>(3 - t));
            }
            else
            {
                surfaceTri.erase(foundAt);
            }
        }
        tetId++;
    }

    // Arrange the surface triangle faces found in order
    Vec3d v0, v1, v2;
    Vec3d centroid;
    Vec3d normal;
    for (size_t faceId = 0; faceId < surfaceTri.size(); ++faceId)
    {
        v0 = this->getVertexPosition(surfaceTri[faceId][0]);
        v1 = this->getVertexPosition(surfaceTri[faceId][1]);
        v2 = this->getVertexPosition(surfaceTri[faceId][2]);

        centroid = (v0 + v1 + v2) / 3;

        normal = ((v0 - v1).cross(v0 - v2));

        if (normal.dot(centroid - this->getVertexPosition(tetRemainingVert.at(faceId))) > 0)
        {
            std::swap(surfaceTri[faceId][2], surfaceTri[faceId][1]);
        }
    }

    // Renumber the vertices
    std::list<int> uniqueVertIdList;
    for (const auto& face : surfaceTri)
    {
        uniqueVertIdList.push_back(face[0]);
        uniqueVertIdList.push_back(face[1]);
        uniqueVertIdList.push_back(face[2]);
    }
    uniqueVertIdList.sort();
    uniqueVertIdList.unique();

    int                                      vertId;
    std::list<int>::iterator                 it;
    std::shared_ptr<VecDataArray<double, 3>> vertPositions = std::make_shared<VecDataArray<double, 3>>();
    for (vertId = 0, it = uniqueVertIdList.begin(); it != uniqueVertIdList.end(); ++vertId, it++)
    {
        vertPositions->push_back(this->getVertexPosition(static_cast<size_t>(*it)));
        for (auto& face : surfaceTri)
        {
            for (size_t i = 0; i < 3; ++i)
            {
                if (face[i] == *it)
                {
                    face[i] = vertId;
                }
            }
        }
    }

    // Create and attach surface mesh
    surfaceMesh->initialize(vertPositions, triangleIndices);

    if (enforceWindingConsistency)
    {
        surfaceMesh->correctWindingOrder();
    }

    return true;
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
