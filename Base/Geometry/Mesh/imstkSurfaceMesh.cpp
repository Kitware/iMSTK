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

    void SurfaceMesh::initialize(const std::vector<Vec3d>& vertices,
        const std::vector<TriangleArray>& triangles,
        const std::vector<Vec2f>& texCoords,
        const bool computDerivedData)
    {
        this->clear();
        setInitialVerticesPositions(vertices);
        setInitialVerticesPositions(vertices);
        setTrianglesVertices(triangles);
        setTextureCoordinates(texCoords);

        if (computDerivedData)
        {
            computeVerticesNormals();
            computeTrianglesNormals();
            computeVerticesTangents();
        }
    }

    void
SurfaceMesh::computeVerticesNeighborTriangles()
{
    m_verticesNeighborTriangles.resize(m_verticesPositions.size());

    size_t triangleId = 0;

    for (const auto& t : m_trianglesVertices)
    {
        m_verticesNeighborTriangles.at(t.at(0)).insert(triangleId);
        m_verticesNeighborTriangles.at(t.at(1)).insert(triangleId);
        m_verticesNeighborTriangles.at(t.at(2)).insert(triangleId);
        triangleId++;
    }
}

void
SurfaceMesh::computeVerticesNeighborVertices()
{
    m_verticesNeighborVertices.resize(m_verticesPositions.size());

    if (m_verticesNeighborTriangles.empty())
    {
        this->computeVerticesNeighborTriangles();
    }

    for (size_t vertexId = 0; vertexId < m_verticesNeighborVertices.size(); ++vertexId)
    {
        for (const size_t& triangleId : m_verticesNeighborTriangles.at(vertexId))
        {
            for (const size_t& vertexId2 : m_trianglesVertices.at(triangleId))
            {
                if (vertexId2 != vertexId)
                {
                    m_verticesNeighborVertices.at(vertexId).insert(vertexId2);
                }
            }
        }
    }
}

void
SurfaceMesh::computeTrianglesNormals()
{
    m_trianglesNormals.resize(m_trianglesVertices.size());

    for (size_t triangleId = 0; triangleId < m_trianglesNormals.size(); ++triangleId)
    {
        const auto& t  = m_trianglesVertices.at(triangleId);
        const auto& p0 = m_verticesPositions.at(t.at(0));
        const auto& p1 = m_verticesPositions.at(t.at(1));
        const auto& p2 = m_verticesPositions.at(t.at(2));

        m_trianglesNormals.at(triangleId) = ((p1 - p0).cross(p2 - p0)).normalized();
    }
}

void
SurfaceMesh::computeVerticesNormals()
{
    m_verticesNormals.resize(m_verticesPositions.size());

    if (m_verticesNeighborTriangles.empty())
    {
        this->computeVerticesNeighborTriangles();
    }

    if (m_trianglesNormals.empty())
    {
        this->computeTrianglesNormals();
    }

    for (size_t vertexId = 0; vertexId < m_verticesNormals.size(); ++vertexId)
    {
        for (const size_t& triangleId : m_verticesNeighborTriangles.at(vertexId))
        {
            m_verticesNormals.at(vertexId) += m_trianglesNormals.at(triangleId);
        }

        m_verticesNormals.at(vertexId).normalize();
    }
}

void
SurfaceMesh::computeVerticesTangents()
{
    if (m_textureCoordinates.empty())
    {
        LOG(DEBUG) << "SurfaceMesh::computeVerticesTangents debug: no textureCoordinates, can not compute vertices tangents.";
        return;
    }

    /*
       Derived from
       Lengyel, Eric. "Computing Tangent Space Basis Vectors for an Arbitrary
          Mesh".
       Terathon Software 3D Graphics Library, 2001.
       [url]http://www.terathon.com/code/tangent.html[/url]
     */

    std::vector<Vec3d> tan1, tan2;

    tan1.resize(m_verticesPositions.size());
    tan2.resize(m_verticesPositions.size());

    for (const auto& triangle : m_trianglesVertices)
    {
        const size_t& id0 = triangle.at(0);
        const size_t& id1 = triangle.at(1);
        const size_t& id2 = triangle.at(2);

        const Vec3d& p0 = m_verticesPositions.at(id0);
        const Vec3d& p1 = m_verticesPositions.at(id1);
        const Vec3d& p2 = m_verticesPositions.at(id2);

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

    m_verticesTangents.resize(m_verticesPositions.size());

    if (m_verticesNormals.empty())
    {
        this->computeVerticesNormals();
    }

    for (size_t vertexId = 0; vertexId < m_verticesTangents.size(); ++vertexId)
    {
        const Vec3d& n  = m_verticesNormals.at(vertexId);
        const Vec3d& t1 = tan1.at(0);
        const Vec3d& t2 = tan2.at(0);

        // Gram-Schmidt orthogonalize
        Vec3d tangente = (t1 - n * n.dot(t1));
        tangente.normalize();

        m_verticesTangents.at(vertexId)[0] = tangente[0];
        m_verticesTangents.at(vertexId)[1] = tangente[1];
        m_verticesTangents.at(vertexId)[2] = tangente[2];

        // Calculate handedness
        m_verticesTangents.at(vertexId)[3] = ((n.cross(t1)).dot(t2) < 0.0f) ? -1.0f : 1.0f;
    }
}

const std::vector<SurfaceMesh::TriangleArray>&
SurfaceMesh::getTrianglesVertices() const
{
    return m_trianglesVertices;
}

void
SurfaceMesh::setTrianglesVertices(const std::vector<TriangleArray>& triangles)
{
    m_trianglesVertices = triangles;
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
SurfaceMesh::getTrianglesNormals() const
{
    return m_trianglesNormals;
}

const Vec3d&
SurfaceMesh::getTriangleNormal(size_t i) const
{
    return m_trianglesNormals.at(i);
}

const std::vector<Vec3d>&
SurfaceMesh::getVerticesNormals() const
{
    return m_verticesNormals;
}

const Vec3d&
SurfaceMesh::getVerticeNormal(size_t i) const
{
    return m_verticesNormals.at(i);
}

const std::vector<Vec4d>&
SurfaceMesh::getVerticesTangents() const
{
    return m_verticesTangents;
}

int
SurfaceMesh::getNumTriangles() const
{
    return this->m_trianglesVertices.size();
}

const Vec4d&
SurfaceMesh::getVerticeTangent(size_t i) const
{
    return m_verticesTangents.at(i);
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

void
SurfaceMesh::clear()
{
    m_trianglesVertices.clear();
    m_textureCoordinates.clear();
    m_verticesNeighborTriangles.clear();
    m_verticesNeighborVertices.clear();
    m_trianglesNormals.clear();
    m_verticesNormals.clear();
    m_verticesTangents.clear();

    Mesh::clear();
}

void
SurfaceMesh::print() const
{
    LOG(INFO) << "Number of vertices: " << this->getNumVertices() << "\n";
    LOG(INFO) << "Number of triangles: " << this->getNumTriangles() << "\n";

    LOG(INFO) << "Triangles:\n";
    for (auto &triVerts : this->getTrianglesVertices())
    {
        LOG(INFO) << "(" << triVerts[0] << ", " << triVerts[1] << "," << triVerts[2] << ")\n";
    }

    LOG(INFO) << "Vertex positions:\n";
    for (auto &verts : this->getInitialVerticesPositions())
    {
        LOG(INFO) << "(" << verts.x() << ", " << verts.y() << "," << verts.z() << ")\n";
    }
}

}
