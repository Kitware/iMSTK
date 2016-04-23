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

namespace imstk {
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

bool
TetrahedralMesh::extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh)
{
    if (!surfaceMesh)
    {
        LOG(WARNING) << "Cannot extract SurfaceMesh: The surface mesh provided is not instantiated!";
        return false;
    }
    using triArray = SurfaceMesh::TriangleArray;

    const std::vector<triArray> facePattern = { triArray{ { 0, 1, 2 } }, triArray{ { 0, 1, 3 } }, triArray{ { 0, 2, 3 } }, triArray{ { 1, 2, 3 } } };

    // Find number of common vertices
    auto getNumCommonVerts = [facePattern](const TetraArray& array1, const TetraArray& array2, triArray& commonFace) -> int
    {
        int numCommonVerts = 0;
        std::array<bool, 4> tmpFace = {{0,0,0,0}};
        for (size_t i = 0; i < 4; ++i)
        {
            if (array1[i] == array2[0] || array1[i] == array2[1] || array1[i] == array2[2] || array1[i] == array2[3])
            {
                tmpFace[i] = true;
                numCommonVerts++;
            }
        }
        if (numCommonVerts == 3)
        {
            for (size_t i = 0; i < 4; ++i)
            {
                if (!tmpFace[i])
                {
                    for (size_t j = 0; j < 3; ++j)
                    {
                        commonFace[j] = array1[facePattern[3-i][j]];
                    }
                }
            }
        }
        return numCommonVerts;
    };

    // Find the common face irrespective of the order
    auto findCommonFace = [facePattern](const TetraArray& tetVertArray, const triArray& triVertArray) -> int
    {
        for (size_t i = 0; i < 4; ++i)
        {
            if (tetVertArray[i] != triVertArray[0] && tetVertArray[i] != triVertArray[1] && tetVertArray[i] != triVertArray[2])
            {
                for (size_t j = 0; j < 4; ++j)
                {
                    if (i != facePattern[j].at(0) && i != facePattern[j].at(1) && i != facePattern[j].at(2))
                    {
                        return (int)j;
                    }
                }
            }
        }
        LOG(WARNING) << "There is no common face!";
        return -1;// something wrong if you reach this point
    };

    // Find and store the tetrahedral faces that are unique
    auto vertArray = this->getTetrahedraVertices();
    std::vector<triArray> surfaceTri;
    std::vector<int> surfaceTriTet;
    std::vector<int> tetRemainingVert;
    triArray possibleFaces[4];
    triArray commonFace;
    bool foundFaces[4];

    for (size_t tetId = 0; tetId < this->getNumTetrahedra(); ++tetId)
    {
        auto tetVertArray = vertArray.at(tetId);
        foundFaces[0] = foundFaces[1] = foundFaces[2] = foundFaces[3] = false;

        for (size_t tetIdInner = 0 ; tetIdInner < this->getNumTetrahedra(); ++tetIdInner)
        {
            if (tetId == tetIdInner)
            {
                continue;
            }
            auto tetVertArrayInner = vertArray.at(tetIdInner);

            // check if there is common face
            if (getNumCommonVerts(tetVertArray, tetVertArrayInner, commonFace) == 3)
            {
                foundFaces[findCommonFace(tetVertArray, commonFace)] = true;
            }

            // break if all the faces are already found
            if (foundFaces[0] && foundFaces[1] && foundFaces[2] && foundFaces[3])
            {
                break;
            }
        }

        // break if all the faces are already found
        if (foundFaces[0] && foundFaces[1] && foundFaces[2] && foundFaces[3])
        {
            break;
        }
        else
        {
            for (size_t faceId = 0; faceId < 4; ++faceId)
            {
                possibleFaces[faceId] = triArray{ {
                        tetVertArray[facePattern[faceId].at(0)],
                        tetVertArray[facePattern[faceId].at(1)],
                        tetVertArray[facePattern[faceId].at(2)] } };

                if (foundFaces[faceId] == false)
                {
                    surfaceTri.push_back(possibleFaces[faceId]);
                    surfaceTriTet.push_back(tetId);
                    tetRemainingVert.push_back(3 - faceId);
                }
            }
        }
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
        //normal.normalize();

        if (normal.dot(centroid - this->getVertexPosition(tetRemainingVert.at(faceId))) > 0)
        {
            // swap
            int tmpIndex = surfaceTri[faceId][2];
            surfaceTri[faceId][2] = surfaceTri[faceId][1];
            surfaceTri[faceId][2] = tmpIndex;
        }
    }

    // Renumber the vertices
    std::list<int> uniqueVertIdList;
    for (auto &face : surfaceTri)
    {
        uniqueVertIdList.push_back(face[0]);
        uniqueVertIdList.push_back(face[1]);
        uniqueVertIdList.push_back(face[2]);
    }
    uniqueVertIdList.sort();
    uniqueVertIdList.unique();

    int vertId;
    std::list<int>::iterator it;
    std::vector<Vec3d> vertPositions;
    for (vertId=0, it = uniqueVertIdList.begin(); it != uniqueVertIdList.end(); ++vertId, it++)
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

    // add vertices and triangles
    surfaceMesh->initialize(vertPositions, surfaceTri);

    return true;
}

void
TetrahedralMesh::clear()
{
    m_tetrahedraVertices.clear();
    Mesh::clear();
}

void
TetrahedralMesh::print() const
{
    Geometry::print();

    LOG(INFO) << "Number of vertices: " << this->getNumVertices() << "\n";
    LOG(INFO) << "Number of tetrahedra: " << this->getNumTetrahedra() << "\n";

    LOG(INFO) << "Tetrahedra:\n";
    for (auto &tetVerts : this->getTetrahedraVertices())
    {
        LOG(INFO) << "(" << tetVerts[0] << ", " << tetVerts[1] << ", " << tetVerts[2] << ", " << tetVerts[3] << ")\n";
    }

    LOG(INFO) << "Vertex positions:\n";
    for (auto &verts : this->getVerticesPositions())
    {
        LOG(INFO) << "(" << verts.x() << ", " << verts.y() << ", " << verts.z() << ")\n";
    }
}
}
