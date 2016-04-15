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

#ifndef imstkMesh_h
#define imstkMesh_h

#include "imstkGeometry.h"

namespace imstk {
class Mesh : public Geometry
{
public:

    ~Mesh() = default;

    // Accessors
    void setInitialVerticesPositions(const std::vector<Vec3d>& vertices);
    const std::vector<Vec3d>& getInitialVerticesPositions() const;
    const Vec3d& getInitialVerticePosition(const int& vertNum) const;

    void setVerticesPositions(const std::vector<Vec3d>& vertices);
    const std::vector<Vec3d>& getVerticesPositions() const;
    void setVerticePosition(const int &vertNum, const Vec3d& pos);
    const Vec3d& getVerticePosition(const int& vertNum) const;

    void setVerticesDisplacements(const std::vector<Vec3d>& diff);
    const std::vector<Vec3d>& getVerticesDisplacements() const;
    const Vec3d& getVerticeDisplacement(const int& vertNum) const;

    const int getNumVertices() const;

    void computeBoundingBox(Vec3d& min, Vec3d& max, const double percent = 0.0) const;

protected:

    Mesh(GeometryType type) : Geometry(type, WORLD_ORIGIN, Quatd()) {}

    //   Orientation * Scaling * initialVerticesPositions
    // + Position (Initial translation)
    // + verticesDisplacements
    // = verticesPositions
    std::vector<Vec3d> m_initialVerticesPositions;
    std::vector<Vec3d> m_verticesPositions;
    std::vector<Vec3d> m_verticesDisplacements;
};
}

#endif // ifndef imstkMesh_h
