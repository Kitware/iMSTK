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
#include "imstkLogUtility.h"

namespace imstk
{
Geometry::Geometry(const Geometry::Type type, const std::string name) :
    m_type(type), m_name(name), m_geometryIndex(Geometry::getUniqueID())
{
    // If the geometry name is empty, enumerate it by name (which will not be duplicated)
    if (m_name.empty())
    {
        m_name = getTypeName() + std::string("-") + std::to_string(m_geometryIndex);
    }
    else if (s_sGegometryNames.find(m_name) != s_sGegometryNames.end())
    {
        LOG(FATAL) << "The provided geometry name '" + m_name + "' is duplicated";
    }

    // Store geometry name to keep track and avoid duplication
    s_sGegometryNames.insert(m_name);
}

Geometry::~Geometry()
{
    // Remove the string name of this geometry from the global name set so it can be re-used by new geometry
    // Since erasing element from tbb::concurrrent_unordered_set is not thread-safe, spin lock need to be used
    s_GeomGlobalLock.lock();
    s_sGegometryNames.unsafe_erase(m_name);
    s_GeomGlobalLock.unlock();
}

void
Geometry::print() const
{
    LOG(INFO) << this->getTypeName();

    Vec3d t = m_transform.translation();
    Mat3d r = m_transform.rotation();
    LOG(INFO) << "Scaling: " << m_scaling;
    LOG(INFO) << "Translation: " << "(" << t.x() << ", " << t.y() << ", " << t.z() << ")";
    LOG(INFO) << "Rotation:\n" << r;
}

void
Geometry::computeBoundingBox(Vec3d&, Vec3d&, const double)
{
    LOG(FATAL) << "computeBoundingBox() must be called from an instance of a specific geometry class";
}

void
Geometry::translate(const Vec3d& t, TransformType type)
{
    if (t.isApprox(Vec3d::Zero()))
    {
        return;
    }

    if (type == TransformType::ConcatenateToTransform)
    {
        m_transform.translate(t);
        m_transformModified = true;
        m_transformApplied  = false;
    }
    else
    {
        this->applyTranslation(t);
        m_dataModified = true;
    }
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

    if (type == TransformType::ConcatenateToTransform)
    {
        m_transform.rotate(r);
        m_transformModified = true;
        m_transformApplied  = false;
    }
    else
    {
        this->applyRotation(r);
        m_dataModified = true;
    }
}

void
Geometry::rotate(const Quatd& q, TransformType type)
{
    this->rotate(q.toRotationMatrix(), type);
}

void
Geometry::rotate(const Vec3d& axis, double angle, TransformType type)
{
    this->rotate(Rotd(angle, axis).toRotationMatrix(), type);
}

void
Geometry::scale(double s, TransformType type)
{
    if (s <= 0.0)
    {
        LOG(FATAL) << "Geometry::scale error: invalid scaling constant.";
        return;
    }

    if (std::abs(s - 1.0) < 1e-8)
    {
        return;
    }

    if (type == TransformType::ConcatenateToTransform)
    {
        m_scaling          *= s;
        m_transformModified = true;
        m_transformApplied  = false;
    }
    else
    {
        this->applyScaling(s);
        m_dataModified = true;
    }
}

void
Geometry::transform(RigidTransform3d T, TransformType type)
{
    if (type == TransformType::ConcatenateToTransform)
    {
        m_transform         = T * m_transform;
        m_transformModified = true;
        m_transformApplied  = false;
    }
    else
    {
        this->applyTranslation(T.translation());
        this->applyRotation(T.rotation());
        m_dataModified = true;
    }
}

Vec3d
Geometry::getTranslation() const
{
    return m_transform.translation();
}

void
Geometry::setTranslation(const Vec3d t)
{
    m_transform.translation() = t;
    m_transformModified       = true;
    m_transformApplied        = false;
}

void
Geometry::setTranslation(const double x, const double y, const double z)
{
    this->setTranslation(Vec3d(x, y, z));
}

Mat3d
Geometry::getRotation() const
{
    return m_transform.linear();
}

void
Geometry::setRotation(const Mat3d m)
{
    m_transform.linear() = m;
    m_transformModified  = true;
    m_transformApplied   = false;
}

void
Geometry::setRotation(const Quatd q)
{
    this->setRotation(q.toRotationMatrix());
}

void
Geometry::setRotation(const Vec3d axis, const double angle)
{
    this->setRotation(Rotd(angle, axis).toRotationMatrix());
}

double
Geometry::getScaling() const
{
    return m_scaling;
}

void
Geometry::setScaling(double s)
{
    if (s <= 0)
    {
        LOG(WARNING) << "Geometry::setScaling error: scaling should be positive.";
        return;
    }
    m_scaling           = s;
    m_transformModified = true;
    m_transformApplied  = false;
}

Geometry::Type
Geometry::getType() const
{
    return m_type;
}

const std::string
Geometry::getTypeName() const
{
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
    }
    LOG(FATAL) << "Mesh type to string has not been completely implemented";
}

bool
Geometry::isMesh() const
{
    return (this->m_type == Type::HexahedralMesh
            || this->m_type == Type::SurfaceMesh
            || this->m_type == Type::TetrahedralMesh
            || this->m_type == Type::LineMesh
            ) ? true : false;
}

// Static mutex lock
ParallelUtils::SpinLock
Geometry::s_GeomGlobalLock;

// Static counter
uint32_t
Geometry::s_NumGeneratedGegometries = 0;

// Static name set
tbb::concurrent_unordered_set<std::string>
Geometry::s_sGegometryNames;

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
