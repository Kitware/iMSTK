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

#ifndef imstkSurfaceMesh_h
#define imstkSurfaceMesh_h

#include <set>

#include "imstkMesh.h"

namespace imstk {
class SurfaceMesh : public Mesh
{
    using TriangleArray = std::array<size_t, 3>;
    using NeighborsType = std::set<size_t>;

public:

    SurfaceMesh() : Mesh(GeometryType::SurfaceMesh) {}

    ~SurfaceMesh() = default;

    void                              computeVertexNeighborTriangles();
    void                              computeVertexNeighborVertices();
    void                              computeTriangleNormals();
    void                              computeVertexNormals();
    void                              computeVertexTangents();

    void                              setTriangleVertices(
        const std::vector<TriangleArray>& triangles);

    void                              setTextureCoordinates(const std::vector<Vec2f>& coords);

    const std::vector<TriangleArray>& getTriangleVertices() const;
    const std::vector<Vec2f>        & getTextureCoordinates() const;
    const std::vector<Vec3d>        & getTriangleNormals() const;
    const Vec3d             & getTriangleNormal(size_t i) const;

    const std::vector<Vec3d>& getVertexNormals() const;
    const Vec3d             & getVertexNormal(size_t i) const;

    const std::vector<Vec4d>& getVertexTangents() const;
    const Vec4d             & getVertexTangent(size_t i) const;

protected:

    std::vector<TriangleArray> m_triangleVertices;
    std::vector<Vec2f> m_textureCoordinates;

    std::vector<NeighborsType> m_vertexNeighborTriangles;
    std::vector<NeighborsType> m_vertexNeighborVertices;

    std::vector<Vec3d> m_triangleNormals;
    std::vector<Vec3d> m_vertexNormals;
    std::vector<Vec4d> m_vertexTangents;
};
}

#endif // ifndef imstkSurfaceMesh_h
