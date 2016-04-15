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

#include <array>
#include <set>

#include "imstkMesh.h"

namespace imstk {
class SurfaceMesh : public Mesh
{
public:

    using TriangleArray = std::array<size_t, 3>;
    using NeighborsType = std::set<size_t>;

    SurfaceMesh() : Mesh(GeometryType::SurfaceMesh) {}

    ~SurfaceMesh() = default;

    void computeVerticesNeighborTriangles();
    void computeVerticesNeighborVertices();
    void computeTrianglesNormals();
    void computeVerticesNormals();
    void computeVerticesTangents();

    // Accessors
    void setTrianglesVertices(const std::vector<TriangleArray>& triangles);
    const std::vector<TriangleArray>& getTrianglesVertices() const;

    void setTextureCoordinates(const std::vector<Vec2f>& coords);
    const std::vector<Vec2f>& getTextureCoordinates() const;

    const std::vector<Vec3d>& getTrianglesNormals() const;
    const Vec3d& getTriangleNormal(size_t i) const;

    const std::vector<Vec3d>& getVerticesNormals() const;
    const Vec3d& getVerticeNormal(size_t i) const;

    const std::vector<Vec4d>& getVerticesTangents() const;
    const Vec4d& getVerticeTangent(size_t i) const;

    double getVolume() const;

protected:

    std::vector<TriangleArray> m_trianglesVertices;
    std::vector<Vec2f> m_textureCoordinates;

    std::vector<NeighborsType> m_verticesNeighborTriangles;
    std::vector<NeighborsType> m_verticesNeighborVertices;

    std::vector<Vec3d> m_trianglesNormals;
    std::vector<Vec3d> m_verticesNormals;
    std::vector<Vec4d> m_verticesTangents;
};
}

#endif // ifndef imstkSurfaceMesh_h
