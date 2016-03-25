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

    const std::vector<Vec3d>& getInitialVertexPositions() const;
    void                      setInitialVertexPositions(const std::vector<Vec3d>& vertices);

    const std::vector<Vec3d>& getVertexPositions() const;
    void                      setVertexPositions(const std::vector<Vec3d>& vertices);

    const std::vector<Vec3d>& getVertexDisplacements() const;
    void                      setVertexDisplacements(const std::vector<Vec3d>& diff);

protected:

    Mesh(GeometryType type) : Geometry(type, ORIGIN, Quatd()) {}

    //   Orientation * Scaling * initialVertexPositions
    // + Position
    // + vertexDisplacements
    // = vertexPositions
    std::vector<Vec3d> m_initialVertexPositions;
    std::vector<Vec3d> m_vertexPositions;
    std::vector<Vec3d> m_vertexDisplacements;
};
}

#endif // ifndef imstkMesh_h
