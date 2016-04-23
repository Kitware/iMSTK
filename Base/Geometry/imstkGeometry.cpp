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

#include "imstkGeometry.h"

namespace imstk {

void
Geometry::translate(const Vec3d& t)
{
    m_position += t;
}

void
Geometry::translate(const double& x,
                    const double& y,
                    const double& z)
{
    this->translate(Vec3d(x, y, z));
}

void
Geometry::rotate(const Quatd& r)
{
    m_orientation = r * m_orientation;
}

void
Geometry::rotate(const Mat3d& r)
{
    this->rotate(Quatd(r));
}

void
Geometry::rotate(const Vec3d& axis, const double& angle)
{
    this->rotate(Quatd(Eigen::AngleAxisd(angle, axis)));
}

void
Geometry::scale(const double& scaling)
{
    m_scaling *= scaling;
}

void
Geometry::transform(const RigidTransform3d& transform)
{
    this->rotate(transform.rotation());
    this->translate(transform.translation());
}

bool
Geometry::isMesh() const
{
    return (this->m_type == GeometryType::HexahedralMesh ||
            this->m_type == GeometryType::SurfaceMesh ||
            this->m_type == GeometryType::TetrahedralMesh) ? true : false;
}

void
Geometry::print() const
{
    LOG(INFO) << this->getTypeName();
    LOG(INFO) << "Position: " << "(" << this->m_position.x() << ", " << this->m_position.y() << ", " << this->m_position.z() << ")\n";
    LOG(INFO) << "Scaling: " << m_scaling << "\n";
}

const Vec3d&
Geometry::getPosition() const
{
    return m_position;
}

void
Geometry::setPosition(const Vec3d& position)
{
    m_position = position;
}

void
Geometry::setPosition(const double& x,
                      const double& y,
                      const double& z)
{
    this->setPosition(Vec3d(x, y, z));
}

const Quatd&
Geometry::getOrientation() const
{
    return m_orientation;
}

void
Geometry::setOrientation(const Quatd& orientation)
{
    m_orientation = orientation;
}

void
Geometry::setOrientation(const Mat3d& orientation)
{
    this->setOrientation(Quatd(orientation));
}

void
Geometry::setOrientation(const Vec3d& axis, const double& angle)
{
    this->setOrientation(Quatd(Eigen::AngleAxisd(angle, axis)));
}

const double&
Geometry::getScaling() const
{
    return m_scaling;
}

void
Geometry::setScaling(const double& scaling)
{
    m_scaling = scaling;
}

const GeometryType&
Geometry::getType() const
{
    return m_type;
}

const std::string
Geometry::getTypeName() const
{
    switch (m_type)
    {
    case GeometryType::Cube:
        return "Cube";
    case GeometryType::Plane:
        return "Plane";
    case GeometryType::Sphere:
        return "Sphere";
    case GeometryType::SurfaceMesh:
        return "Surface trianglar mesh";
    case GeometryType::TetrahedralMesh:
        return "Tetrahedral mesh";
    case GeometryType::HexahedralMesh:
        return "Hexahedral Mesh";
    default:
        return "Mesh type not determined!";
    }
}

}
