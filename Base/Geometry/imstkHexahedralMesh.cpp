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

#include "imstkHexahedralMesh.h"

namespace imstk {

const std::vector<HexahedralMesh::HexaArray>&
HexahedralMesh::getHexahedronMeshVertices() const
{
    return m_hexahedronVertices;
}

void
HexahedralMesh::setHexahedronMeshVertices(const std::vector<HexaArray>& hexahedrons)
{
    m_hexahedronVertices = hexahedrons;
}


const imstk::HexahedralMesh::HexaArray&
HexahedralMesh::getHexahedronVertices(const int hexaNum) const
{
    return m_hexahedronVertices.at(hexaNum);
}

int
HexahedralMesh::getNumHexahedra() const
{
    return m_hexahedronVertices.size();
}

double
HexahedralMesh::getVolume() const
{
    double volume = 0.0;
    imstk::Vec3d v[8];
    imstk::Vec3d a, b, c;
    imstk::Mat3d A;
    for (int i = 0; i < getNumHexahedra(); ++i)
    {
        auto hexVerts = getHexahedronVertices(i);
        for (int i = 0; i < 8; ++i)
        {
            v[i] = getVertexPosition(hexVerts[i]);
        }

        a = v[7] - v[0];
        b = v[1] - v[0];
        c = v[3] - v[5];

        A << a[0], b[0], c[0],
            a[1], b[1], c[1],
            a[2], b[2], c[2];

        volume += A.determinant();

        b = v[4] - v[0];
        c = v[5] - v[6];

        A << a[0], b[0], c[0],
            a[1], b[1], c[1],
            a[2], b[2], c[2];

        volume += A.determinant();

        b = v[2] - v[0];
        c = v[6] - v[3];

        A << a[0], b[0], c[0],
            a[1], b[1], c[1],
            a[2], b[2], c[2];

        volume += A.determinant();
    }

    return volume/6;
}

}
