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
#include "imstkLogger.h"
#include "imstkParallelUtils.h"

namespace imstk
{
Geometry::Geometry(const Geometry::Type type, const std::string& name) :
    m_type(type), m_name(name), m_geometryIndex(Geometry::getUniqueID()), m_transform(Mat4d::Identity())
{
    // If the geometry name is empty, enumerate it by name (which will not be duplicated)
    if (m_name.empty())
    {
        m_name = getTypeName() + std::string("-") + std::to_string(m_geometryIndex);
    }
}

void
Geometry::print() const
{
    LOG(INFO) << this->getTypeName();
    LOG(INFO) << "Transform: " << m_transform;
}

void
Geometry::computeBoundingBox(Vec3d& imstkNotUsed(min), Vec3d& imstkNotUsed(max), const double imstkNotUsed(padding))
{
    LOG(WARNING) << "computeBoundingBox() must be called from an instance of a specific geometry class";
}

void
Geometry::translate(const Vec3d& t, TransformType type)
{
    if (t.isApprox(Vec3d::Zero()))
    {
        return;
    }

    transform(AffineTransform3d(Eigen::Translation3d(t)).matrix(), type);
}

void
Geometry::translate(double x, double y, double z, TransformType type)
{
    this->translate(Vec3d(x, y, z), type);
}

void
Geometry::rotate(const Mat3d& r, TransformType type)
{
    if (r.isApprox(Mat3d::Identity()))
    {
        return;
    }

    Mat4d m = Mat4d::Identity();
    m.block<3, 3>(0, 0) = r;
    transform(m, type);
}

void
Geometry::rotate(const Quatd& q, TransformType type)
{
    this->rotate(q.toRotationMatrix(), type);
}

void
Geometry::rotate(const Vec3d& axis, double radians, TransformType type)
{
    this->rotate(Rotd(radians, axis).toRotationMatrix(), type);
}

void
Geometry::scale(const Vec3d& s, TransformType type)
{
    Mat4d m = Mat4d::Identity();
    m(0, 0) = s[0];
    m(1, 1) = s[1];
    m(2, 2) = s[2];
    transform(m, type);
}
void
Geometry::scale(const double s, TransformType type)
{
    Mat4d m = Mat4d::Identity();
    m(0, 0) = s;
    m(1, 1) = s;
    m(2, 2) = s;
    transform(m, type);
}

void
Geometry::transform(const Mat4d& T, TransformType type)
{
    if (type == TransformType::ConcatenateToTransform)
    {
        m_transform = T * m_transform;
        m_transformApplied = false;
    }
    else
    {
        applyTransform(T);
        this->postEvent(Event(EventType::Modified));
    }
    m_transformApplied = true;
}

Vec3d
Geometry::getTranslation() const
{
    return m_transform.block<3, 1>(0, 3);
}

void
Geometry::setTranslation(const Vec3d& t)
{
    m_transform.block<3, 1>(0, 3) = t;
    m_transformApplied = false;
}

void
Geometry::setTranslation(const double x, const double y, const double z)
{
    this->setTranslation(Vec3d(x, y, z));
}

void
Geometry::setRotation(const Mat3d& m)
{
    // Decompose trs, getRotation assumes no shear
    const Vec3d s = getScaling();
    const Mat3d r = getRotation();
    const Vec3d t = getTranslation();
    m_transform = Mat4d::Identity();
    m_transform.block<3, 3>(0, 0) = m;
    m_transform.block<3, 1>(0, 3) = t;
    m_transform(0, 0) *= s[0];
    m_transform(1, 1) *= s[1];
    m_transform(2, 2) *= s[2];
    m_transformApplied = false;
}

void
Geometry::setRotation(const Quatd& q)
{
    this->setRotation(q.toRotationMatrix());
}

void
Geometry::setRotation(const Vec3d& axis, const double angle)
{
    this->setRotation(Rotd(angle, axis).toRotationMatrix());
}

void
Geometry::setScaling(const Vec3d& s)
{
    m_transform.block<3, 1>(0, 0) = m_transform.block<3, 1>(0, 0).normalized() * s[0];
    m_transform.block<3, 1>(0, 1) = m_transform.block<3, 1>(0, 1).normalized() * s[1];
    m_transform.block<3, 1>(0, 2) = m_transform.block<3, 1>(0, 2).normalized() * s[2];
    m_transformApplied = false;
}
void
Geometry::setScaling(const double s)
{
    m_transform.block<3, 1>(0, 0) = m_transform.block<3, 1>(0, 0).normalized() * s;
    m_transform.block<3, 1>(0, 1) = m_transform.block<3, 1>(0, 1).normalized() * s;
    m_transform.block<3, 1>(0, 2) = m_transform.block<3, 1>(0, 2).normalized() * s;
    m_transformApplied = false;
}

Mat3d
Geometry::getRotation() const
{
    // Assumes affine, no shear
    const Vec3d& x = m_transform.block<3, 1>(0, 0);
    const Vec3d& y = m_transform.block<3, 1>(0, 1);
    const Vec3d& z = m_transform.block<3, 1>(0, 2);

    Mat3d r;
    r.block<3, 1>(0, 0) = x.normalized();
    r.block<3, 1>(0, 1) = y.normalized();
    r.block<3, 1>(0, 2) = z.normalized();

    return r;
}

Vec3d
Geometry::getScaling() const
{
    return Vec3d(
        m_transform.block<3, 1>(0, 0).norm(),
        m_transform.block<3, 1>(0, 1).norm(),
        m_transform.block<3, 1>(0, 2).norm());
}

const std::string
Geometry::getTypeName() const
{
    // /todo: create an actual object factory
    switch (m_type)
    {
    case Type::Plane: return "Plane";
    case Type::Sphere: return "Sphere";
    case Type::Cylinder: return "Cylinder";
    case Type::Cube: return "Cube";
    case Type::Capsule: return "Capsule";
    case Type::PointSet: return "PointSet";
    case Type::SurfaceMesh: return "SurfaceMesh";
    case Type::TetrahedralMesh: return "TetrahedralMesh";
    case Type::HexahedralMesh: return "HexahedralMesh";
    case Type::LineMesh: return "LineMesh";
    case Type::Decal: return "Decal";
    case Type::DecalPool: return "DecalPool";
    case Type::RenderParticles: return "RenderParticles";
    case Type::ImageData: return "ImageData";
    case Type::SignedDistanceField: return "SignedDistanceField";
    case Type::CompositeImplicitGeometry: return "CompositeImplicitGeometry";
    default:
        LOG(FATAL) << "Mesh type to string has not been completely implemented";
        return "";
    }
}

bool
Geometry::isMesh() const
{
    return (this->m_type == Type::HexahedralMesh
            || this->m_type == Type::SurfaceMesh
            || this->m_type == Type::TetrahedralMesh
            || this->m_type == Type::LineMesh
            || this->m_type == Type::PointSet
            ) ? true : false;
}

// Static mutex lock
ParallelUtils::SpinLock
Geometry::s_GeomGlobalLock;

// Static counter
uint32_t
Geometry::s_NumGeneratedGegometries = 0;

uint32_t
Geometry::getUniqueID()
{
    s_GeomGlobalLock.lock();
    const auto idx = s_NumGeneratedGegometries;
    ++s_NumGeneratedGegometries;
    s_GeomGlobalLock.unlock();
    return idx;
}
} // imstk
