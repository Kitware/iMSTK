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

void
HexahedralMesh::print() const
{
    Geometry::print();

    LOG(INFO) << "Number of vertices: " << this->getNumVertices() << "\n";
    LOG(INFO) << "Number of Hexahedra: " << this->getNumHexahedra() << "\n";

    LOG(INFO) << "Hexahedra:\n";
    for (auto &hexVerts : this->getHexahedraVertices())
    {
        LOG(INFO) << "(" << hexVerts[0] << ", " << hexVerts[1] << "," << hexVerts[2] <<
            hexVerts[3] << ", " << hexVerts[4] << "," << hexVerts[5] << ", " <<
            hexVerts[6] << "," << hexVerts[7] << ")\n";
    }

    LOG(INFO) << "Vertex positions:\n";
    for (auto &verts : this->getVerticesPositions())
    {
        LOG(INFO) << "(" << verts.x() << ", " << verts.y() << "," << verts.z() << ")\n";
    }
}

void
HexahedralMesh::setHexahedraVertices(const std::vector<HexaArray>& hexahedra)
{
    m_hexahedraVertices = hexahedra;
}

const std::vector<HexahedralMesh::HexaArray>&
HexahedralMesh::getHexahedraVertices() const
{
    return m_hexahedraVertices;
}

const HexahedralMesh::HexaArray&
HexahedralMesh::getHexahedronVertices(const int& hexaNum) const
{
    return m_hexahedraVertices.at(hexaNum);
}

int
HexahedralMesh::getNumHexahedra() const
{
    return m_hexahedraVertices.size();
}

double
HexahedralMesh::getVolume() const
{
    Vec3d v[8];
    Mat3d A;
    Vec3d a, b, c;
    double volume = 0.0;
    for (const HexaArray& hexArray : m_hexahedraVertices)
    {
        for (int i = 0; i < 8; ++i)
        {
            v[i] = this->getVertexPosition(hexArray[i]);
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
