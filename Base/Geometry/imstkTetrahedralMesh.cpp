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
const std::vector<TetrahedralMesh::TetraArray>&
TetrahedralMesh::getTetrahedronVertices() const
{
    return m_tetrahedronVertices;
}

const imstk::TetrahedralMesh::TetraArray& TetrahedralMesh::getTetrahedronVertices(const int tetraNum) const
{
    return m_tetrahedronVertices.at(tetraNum);
}

void
TetrahedralMesh::setTetrahedronVertices(const std::vector<TetraArray>& tetrahedrons)
{
    m_tetrahedronVertices = tetrahedrons;
}

void
TetrahedralMesh::computeBarycentricWeights(const int closestEle, const imstk::Vec3d& p, std::array<double, 4> weights) const
{
    TetraArray vertIndices = getTetrahedronVertices(closestEle);
    imstk::Vec3d v[4];
    double dets[4];
    double det;

    for (int i = 0; i < 4; i++)
    {
        v[i] = getVertexPosition(vertIndices[i]);
    }

    Eigen::Matrix4d A;
    A << v[0][0], v[0][1], v[0][2], 1,
         v[1][0], v[1][1], v[1][2], 1,
         v[2][0], v[2][1], v[2][2], 1,
         v[3][0], v[3][1], v[3][2], 1;

    det = A.determinant();

    for (int i = 0; i < 4; i++)
    {
        Eigen::Matrix4d B = A;
        B(i, 0) = p[0];
        B(i, 1) = p[1];
        B(i, 2) = p[2];
        weights[i] = B.determinant() / det;
    }
}

void
TetrahedralMesh::computeTetrahedraBoundingBox(imstk::Vec3d& min, imstk::Vec3d& max, const int tetNum) const
{
    auto v1 = getVertexPosition(m_tetrahedronVertices.at(tetNum)[0]);
    auto v2 = getVertexPosition(m_tetrahedronVertices.at(tetNum)[1]);
    auto v3 = getVertexPosition(m_tetrahedronVertices.at(tetNum)[2]);
    auto v4 = getVertexPosition(m_tetrahedronVertices.at(tetNum)[3]);

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

double
TetrahedralMesh::getVolume() const
{
    imstk::Vec3d v[4];
    Eigen::Matrix4d A;
    double volume = 0.0;
    for (auto it = m_tetrahedronVertices.begin(); it != m_tetrahedronVertices.end(); ++it)
    {
        for (int i = 0; i < 4; i++)
        {
            v[i] = getVertexPosition((*it)[i]);
        }

        A << v[0][0], v[0][1], v[0][2], 1,
            v[1][0], v[1][1], v[1][2], 1,
            v[2][0], v[2][1], v[2][2], 1,
            v[3][0], v[3][1], v[3][2], 1;

        double det = A.determinant();
        if (det < 0)
        {
            LOG(WARNING) << "Tetrahedon is inverted, has negative volume!\n";
        }

        volume += std::abs(det)/6;
    }

    return volume;
}

int
TetrahedralMesh::getNumTetrahedra() const
{
    return m_tetrahedronVertices.size();
}

}
