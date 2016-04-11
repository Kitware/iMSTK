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

#include "imstkSurfaceMesh.h"

namespace imstk {
void
SurfaceMesh::computeVertexNeighborTriangles()
{
    m_vertexNeighborTriangles.resize(m_vertexPositions.size());

    size_t triangleId = 0;

    for (const auto& t : m_triangleVertices)
    {
        m_vertexNeighborTriangles.at(t.at(0)).insert(triangleId);
        m_vertexNeighborTriangles.at(t.at(1)).insert(triangleId);
        m_vertexNeighborTriangles.at(t.at(2)).insert(triangleId);
        triangleId++;
    }
}

void
SurfaceMesh::computeVertexNeighborVertices()
{
    m_vertexNeighborVertices.resize(m_vertexPositions.size());

    if (!m_vertexNeighborTriangles.size())
    {
        this->computeVertexNeighborTriangles();
    }

    for (size_t vertexId = 0; vertexId < m_vertexNeighborVertices.size(); ++vertexId)
    {
        for (const size_t& triangleId : m_vertexNeighborTriangles.at(vertexId))
        {
            for (const size_t& vertexId2 : m_triangleVertices.at(triangleId))
            {
                if (vertexId2 != vertexId)
                {
                    m_vertexNeighborVertices.at(vertexId).insert(vertexId2);
                }
            }
        }
    }
}

void
SurfaceMesh::computeTriangleNormals()
{
    m_triangleNormals.resize(m_triangleVertices.size());

    for (size_t triangleId = 0; triangleId < m_triangleNormals.size(); ++triangleId)
    {
        const auto& t  = m_triangleVertices.at(triangleId);
        const auto& p0 = m_vertexPositions.at(t.at(0));
        const auto& p1 = m_vertexPositions.at(t.at(1));
        const auto& p2 = m_vertexPositions.at(t.at(2));

        m_triangleNormals.at(triangleId) = ((p1 - p0).cross(p2 - p0)).normalized();
    }
}

void
SurfaceMesh::computeVertexNormals()
{
    m_vertexNormals.resize(m_vertexPositions.size());

    if (!m_vertexNeighborTriangles.size())
    {
        this->computeVertexNeighborTriangles();
    }

    if (!m_triangleNormals.size())
    {
        this->computeTriangleNormals();
    }

    for (size_t vertexId = 0; vertexId < m_vertexNormals.size(); ++vertexId)
    {
        for (const size_t& triangleId : m_vertexNeighborTriangles.at(vertexId))
        {
            m_vertexNormals.at(vertexId) += m_triangleNormals.at(triangleId);
        }

        m_vertexNormals.at(vertexId).normalize();
    }
}

void
SurfaceMesh::computeVertexTangents()
{
    /*
       Derived from
       Lengyel, Eric. "Computing Tangent Space Basis Vectors for an Arbitrary
          Mesh".
       Terathon Software 3D Graphics Library, 2001.
       [url]http://www.terathon.com/code/tangent.html[/url]
     */

    std::vector<Vec3d> tan1, tan2;

    tan1.resize(m_vertexPositions.size());
    tan2.resize(m_vertexPositions.size());

    for (const auto& triangle : m_triangleVertices)
    {
        const size_t& id0 = triangle.at(0);
        const size_t& id1 = triangle.at(1);
        const size_t& id2 = triangle.at(2);

        const Vec3d& p0 = m_vertexPositions.at(id0);
        const Vec3d& p1 = m_vertexPositions.at(id1);
        const Vec3d& p2 = m_vertexPositions.at(id2);

        const Vec2f& uv0 = m_textureCoordinates.at(id0);
        const Vec2f& uv1 = m_textureCoordinates.at(id1);
        const Vec2f& uv2 = m_textureCoordinates.at(id2);

        Vec3d  P1 = p1 - p0;
        Vec3d  P2 = p2 - p0;
        double u1 = uv1[0] - uv0[0];
        double u2 = uv2[0] - uv0[0];
        double v1 = uv1[1] - uv0[1];
        double v2 = uv2[1] - uv0[1];

        double div = u1 * v2 - u2 * v1;
        double r   = (div = 0.0f) ? 0.0f : (1.0f / div);

        Vec3d u_dir = (v2 * P1 - v1 * P2) * r;
        Vec3d v_dir = (u2 * P1 - u1 * P2) * r;

        tan1.at(id0) += u_dir;
        tan1.at(id1) += u_dir;
        tan1.at(id2) += u_dir;

        tan2.at(id0) += v_dir;
        tan2.at(id1) += v_dir;
        tan2.at(id2) += v_dir;
    }

    m_vertexTangents.resize(m_vertexPositions.size());

    for (size_t vertexId = 0; vertexId < m_vertexTangents.size(); ++vertexId)
    {
        const Vec3d& n  = m_vertexNormals.at(vertexId);
        const Vec3d& t1 = tan1.at(0);
        const Vec3d& t2 = tan2.at(0);

        // Gram-Schmidt orthogonalize
        Vec3d tangente = (t1 - n * n.dot(t1));
        tangente.normalize();

        m_vertexTangents.at(vertexId)[0] = tangente[0];
        m_vertexTangents.at(vertexId)[1] = tangente[1];
        m_vertexTangents.at(vertexId)[2] = tangente[2];

        // Calculate handedness
        m_vertexTangents.at(vertexId)[3] = ((n.cross(t1)).dot(t2) < 0.0f) ? -1.0f : 1.0f;
    }
}

const std::vector<SurfaceMesh::TriangleArray>&
SurfaceMesh::getTriangleVertices() const
{
    return m_triangleVertices;
}

void
SurfaceMesh::setTriangleVertices(const std::vector<TriangleArray>& triangles)
{
    m_triangleVertices = triangles;
}

const std::vector<Vec2f>&
SurfaceMesh::getTextureCoordinates() const
{
    return m_textureCoordinates;
}

void
SurfaceMesh::setTextureCoordinates(const std::vector<Vec2f>& coords)
{
    m_textureCoordinates = coords;
}

const std::vector<Vec3d>&
SurfaceMesh::getTriangleNormals() const
{
    return m_triangleNormals;
}

const Vec3d&
SurfaceMesh::getTriangleNormal(size_t i) const
{
    return m_triangleNormals.at(i);
}

const std::vector<Vec3d>&
SurfaceMesh::getVertexNormals() const
{
    return m_vertexNormals;
}

const Vec3d&
SurfaceMesh::getVertexNormal(size_t i) const
{
    return m_vertexNormals.at(i);
}

const std::vector<Vec4d>&
SurfaceMesh::getVertexTangents() const
{
    return m_vertexTangents;
}

const Vec4d&
SurfaceMesh::getVertexTangent(size_t i) const
{
    return m_vertexTangents.at(i);
}

const imstk::Vec3d & SurfaceMesh::getVertexInitialPosition(size_t i) const
{
    return m_initialVertexPositions.at(i);
}

const imstk::Vec3d & SurfaceMesh::getVertexPosition(size_t i) const
{
    return m_vertexPositions.at(i);
}

double
SurfaceMesh::getVolume() const
{
    // TODO
    // 1. Check for water tightness
    // 2. Compute volume based on signed distance

    LOG(WARNING) << "Not supported yet, returns 0.0!\n";

    return 0.0;
}

}
