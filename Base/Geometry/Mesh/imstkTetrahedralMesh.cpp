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

namespace imstk
{

void
TetrahedralMesh::initialize(const std::vector<Vec3d>& vertices,
                            const std::vector<TetraArray>& tetrahedra,
                            bool computeAttachedSurfaceMesh)
{
    Mesh::initialize(vertices);
    this->setTetrahedraVertices(tetrahedra);

    if (computeAttachedSurfaceMesh)
    {
        this->computeAttachedSurfaceMesh();
    }
}

void
TetrahedralMesh::clear()
{
    Mesh::clear();
    m_tetrahedraVertices.clear();
}

void
TetrahedralMesh::print() const
{
    Mesh::print();

    LOG(INFO) << "Number of tetrahedra: " << this->getNumTetrahedra();
    LOG(INFO) << "Tetrahedra:";
    for (auto &tet : m_tetrahedraVertices)
    {
        LOG(INFO) << tet.at(0) << ", "
                  << tet.at(1) << ", "
                  << tet.at(2) << ", "
                  << tet.at(3);
    }
}

double
TetrahedralMesh::getVolume() const
{
    Vec3d v[4];
    Mat4d A;
    double volume = 0.0;
    for (const TetraArray& tetVertices : m_tetrahedraVertices)
    {
        for (int i = 0; i < 4; i++)
        {
            v[i] = this->getVertexPosition(tetVertices[i]);
        }

        A << v[0][0], v[0][1], v[0][2], 1,
             v[1][0], v[1][1], v[1][2], 1,
             v[2][0], v[2][1], v[2][2], 1,
             v[3][0], v[3][1], v[3][2], 1;

        double det = A.determinant();
        if (det < 0)
        {
            LOG(WARNING) << "Tetrahedron is inverted, has negative volume!";
        }

        volume += std::abs(det)/6;
    }

    return volume;
}

void
TetrahedralMesh::computeAttachedSurfaceMesh()
{
    this->m_attachedSurfaceMesh = std::make_shared<imstk::SurfaceMesh>();
    if (!this->extractSurfaceMesh(this->m_attachedSurfaceMesh))
    {
        LOG(WARNING) << "TetrahedralMesh::computeAttachedSurfaceMesh error: surface mesh was not extracted.";
    }
}

bool
TetrahedralMesh::extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh)
{
    if (!surfaceMesh)
    {
        LOG(WARNING) << "TetrahedralMesh::extractSurfaceMesh error: the surface mesh provided is not instantiated.";
        return false;
    }

    using triArray = SurfaceMesh::TriangleArray;
    const std::vector<triArray> facePattern = { triArray{ { 0, 1, 2 } }, triArray{ { 0, 1, 3 } }, triArray{ { 0, 2, 3 } }, triArray{ { 1, 2, 3 } } };

    // Find and store the tetrahedral faces that are unique
    auto vertArray = this->getTetrahedraVertices();
    std::vector<triArray> surfaceTri;
    std::vector<size_t> surfaceTriTet;
    std::vector<size_t> tetRemainingVert;
    bool unique = true;
    size_t foundAt = 0, tetId = 0;
    size_t a, b, c;

    for (auto &tetVertArray : vertArray)
    {
        //std::cout << "tet: " << tetId << std::endl;

        for (int t = 0; t < 4; ++t)
        {
            unique = true;
            foundAt = 0;
            a = tetVertArray[facePattern[t][0]];
            b = tetVertArray[facePattern[t][1]];
            c = tetVertArray[facePattern[t][2]];

            // search in reverse
            for (int i = surfaceTri.size() - 1; i >= 0; i--)
            {
                const auto &triA = surfaceTri.at(i);
                if (((triA[0] == a) && ((triA[1] == b && triA[2] == c) || (triA[1] == c && triA[2] == b))) ||
                    ((triA[1] == a) && ((triA[0] == b && triA[2] == c) || (triA[0] == c && triA[2] == b))) ||
                    ((triA[2] == a) && ((triA[1] == b && triA[0] == c) || (triA[1] == c && triA[0] == b))))
                {
                    unique = false;
                    foundAt = i;
                    break;
                }
            }

            if (unique)
            {
                surfaceTri.push_back(triArray{ { a, b, c } });
                surfaceTriTet.push_back(tetId);
                tetRemainingVert.push_back(3 - t);
            }
            else
            {
                surfaceTri.erase(surfaceTri.begin() + foundAt);
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
        v0 = this->getVertexPosition(surfaceTri.at(faceId)[0]);
        v1 = this->getVertexPosition(surfaceTri.at(faceId)[1]);
        v2 = this->getVertexPosition(surfaceTri.at(faceId)[2]);

        centroid = (v0 + v1 + v2) / 3;

        normal = ((v0 - v1).cross(v0 - v2));

        if (normal.dot(centroid - this->getVertexPosition(tetRemainingVert.at(faceId))) > 0)
        {
            // swap
            int tmpIndex = surfaceTri[faceId][2];
            surfaceTri[faceId][2] = surfaceTri[faceId][1];
            surfaceTri[faceId][2] = tmpIndex;
        }
    }

    // Renumber the vertices
    std::list<size_t> uniqueVertIdList;
    for (const auto &face : surfaceTri)
    {
        uniqueVertIdList.push_back(face[0]);
        uniqueVertIdList.push_back(face[1]);
        uniqueVertIdList.push_back(face[2]);
    }
    uniqueVertIdList.sort();
    uniqueVertIdList.unique();

    size_t vertId;
    std::list<size_t>::iterator it;
    std::vector<Vec3d> vertPositions;
    for (vertId = 0, it = uniqueVertIdList.begin(); it != uniqueVertIdList.end(); ++vertId, it++)
    {
        vertPositions.push_back(this->getVertexPosition(*it));
        for (auto &face : surfaceTri)
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
    surfaceMesh->initialize(vertPositions, surfaceTri);

    return true;
}

void
TetrahedralMesh::computeBarycentricWeights(const size_t& tetId, const Vec3d& pos,
                                           WeightsArray& weights) const
{
    const TetraArray& tetVertices = m_tetrahedraVertices.at(tetId);
    Vec3d v[4];
    double det;

    for (int i = 0; i < 4; i++)
    {
        v[i] = this->getVertexPosition(tetVertices[i]);
    }

    Mat4d A;
    A << v[0][0], v[0][1], v[0][2], 1,
         v[1][0], v[1][1], v[1][2], 1,
         v[2][0], v[2][1], v[2][2], 1,
         v[3][0], v[3][1], v[3][2], 1;

    det = A.determinant();

    for (int i = 0; i < 4; i++)
    {
        Mat4d B = A;
        B(i, 0) = pos[0];
        B(i, 1) = pos[1];
        B(i, 2) = pos[2];
        weights[i] = B.determinant() / det;
    }
}

void
TetrahedralMesh::computeTetrahedronBoundingBox(const size_t& tetId, Vec3d& min, Vec3d& max) const
{
    auto v1 = this->getVertexPosition(m_tetrahedraVertices.at(tetId)[0]);
    auto v2 = this->getVertexPosition(m_tetrahedraVertices.at(tetId)[1]);
    auto v3 = this->getVertexPosition(m_tetrahedraVertices.at(tetId)[2]);
    auto v4 = this->getVertexPosition(m_tetrahedraVertices.at(tetId)[3]);

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

void
TetrahedralMesh::setTetrahedraVertices(const std::vector<TetraArray>& tetrahedra)
{
    m_tetrahedraVertices = tetrahedra;
}
const std::vector<TetrahedralMesh::TetraArray>&
TetrahedralMesh::getTetrahedraVertices() const
{
    return m_tetrahedraVertices;
}

const TetrahedralMesh::TetraArray&
TetrahedralMesh::getTetrahedronVertices(const size_t& tetId) const
{
    return m_tetrahedraVertices.at(tetId);
}

int
TetrahedralMesh::getNumTetrahedra() const
{
    return m_tetrahedraVertices.size();
}

} // imstk
