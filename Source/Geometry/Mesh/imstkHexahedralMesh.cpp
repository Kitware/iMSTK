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
HexahedralMesh::HexahedralMesh(const std::string& name) : VolumetricMesh(name),
    m_hexahedraIndices(std::make_shared<VecDataArray<int, 8>>())
{
}

void
HexahedralMesh::initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                           std::shared_ptr<VecDataArray<int, 8>> hexahedra,
                           bool computeAttachedSurfaceMesh)
{
    PointSet::initialize(vertices);
    this->setHexahedraIndices(hexahedra);

    if (computeAttachedSurfaceMesh)
    {
        this->computeAttachedSurfaceMesh();
    }
}

void
HexahedralMesh::clear()
{
    PointSet::clear();
    m_hexahedraIndices->clear();
}

void
HexahedralMesh::print() const
{
    Geometry::print();

    LOG(INFO) << "Number of Hexahedra: " << this->getNumHexahedra();
    LOG(INFO) << "Hexahedra:";
    for (auto& hex : *m_hexahedraIndices)
    {
        LOG(INFO) << hex[0] << ", " << hex[1] << ", "
                  << hex[2] << ", " << hex[3] << ", "
                  << hex[4] << ", " << hex[5] << ", "
                  << hex[6] << ", " << hex[7];
    }
}

double
HexahedralMesh::getVolume()
{
    Vec3d                          v[8];
    Mat3d                          A;
    Vec3d                          a, b, c;
    double                         volume   = 0.0;
    const VecDataArray<double, 3>& vertices = *m_vertexPositions;
    for (const Vec8i& hexArray : *m_hexahedraIndices)
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

void
HexahedralMesh::computeAttachedSurfaceMesh()
{
    this->m_attachedSurfaceMesh = std::make_shared<imstk::SurfaceMesh>();
    if (!this->extractSurfaceMesh(this->m_attachedSurfaceMesh))
    {
        LOG(FATAL) << "error: surface mesh was not extracted.";
    }
}

bool
HexahedralMesh::extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh)
{
    LOG(FATAL) << "error: not implemented.";
    return false;
}

const Vec8i&
HexahedralMesh::getHexahedronIndices(const int hexaNum) const
{
    return (*m_hexahedraIndices)[hexaNum];
}

size_t
HexahedralMesh::getNumHexahedra() const
{
    return m_hexahedraIndices->size();
}
} // imstk
