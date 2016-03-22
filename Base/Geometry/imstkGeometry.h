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

#ifndef imstkGeometry_h
#define imstkGeometry_h

#include "imstkMath.h"

namespace imstk {
enum class GeometryType
{
    Plane,
    Sphere,
    Cube,
    SurfaceMesh,
    TetrahedralMesh,
    HexahedralMesh
};

class Geometry
{
public:

    ~Geometry() = default;

    virtual void        translate(const Vec3d& t) = 0;
    virtual void        rotate(const Quatd& r)    = 0;
    virtual void        rotate(const Vec3d & axis,
                               const double& angle) = 0;

    const GeometryType& getType() const;

protected:

    Geometry(GeometryType type) : m_type(type) {}

    GeometryType m_type;
};
}

#endif // ifndef imstkGeometry_h
