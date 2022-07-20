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
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
void
HexahedralMesh::clear()
{
    VolumetricMesh::clear();
}

void
HexahedralMesh::print() const
{
    VolumetricMesh::print();
}

double
HexahedralMesh::getVolume()
{
    Vec3d                          v[8];
    Mat3d                          A;
    Vec3d                          a, b, c;
    double                         volume   = 0.0;
    const VecDataArray<double, 3>& vertices = *m_vertexPositions;
    for (const Vec8i& hexArray : *m_indices)
    {
        for (int i = 0; i < 8; ++i)
        {
            v[i] = vertices[hexArray[i]];
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

    return volume / 6.0;
}

std::shared_ptr<SurfaceMesh>
HexahedralMesh::extractSurfaceMesh()
{
    LOG(FATAL) << "error: not implemented.";
    return nullptr;
}
} // namespace imstk