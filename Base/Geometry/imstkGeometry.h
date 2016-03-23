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

    void         translate(const Vec3d& t);
    void         translate(const double& x,
                           const double& y,
                           const double& z);
    void         rotate(const Quatd& r);
    void         rotate(const Mat3d& r);
    void         rotate(const Vec3d & axis,
                        const double& angle);

    const Vec3d& getPosition() const;
    void         setPosition(const Vec3d& position);
    void         setPosition(const double& x,
                             const double& y,
                             const double& z);

    const Quatd       & getOrientation() const;
    void                setOrientation(const Quatd& orientation);
    void                setOrientation(const Mat3d& orientation);
    void                setOrientation(const Vec3d & axis,
                                       const double& angle);

    const GeometryType& getType() const;

protected:

    Geometry(GeometryType type,
             const Vec3d& position = Vec3d(),
             const Quatd& orientation = Quatd()) :
        m_type(type),
        m_position(position),
        m_orientation(orientation)
    {}

    GeometryType m_type;
    Vec3d m_position;
    Quatd m_orientation;
};
}

#endif // ifndef imstkGeometry_h
