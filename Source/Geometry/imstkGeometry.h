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

#pragma once

#include "g3log/g3log.hpp"

#include "imstkMath.h"
#include "imstkRenderMaterial.h"
#include "imstkParallelUtils.h"

namespace imstk
{
///
/// \class Geometry
///
/// \brief Base class for any geometrical representation
///
class Geometry
{
public:
    ///
    /// \brief Enumeration for the geometry type
    ///
    enum class Type
    {
        Plane,
        Sphere,
        Cylinder,
        Cube,
        PointSet,
        SurfaceMesh,
        TetrahedralMesh,
        HexahedralMesh,
        LineMesh,
        Capsule,
        Decal,
        DecalPool,
        RenderParticles
    };

    ///
    /// \brief Enumeration for the transformation to apply
    /// \params ApplyToTransform to apply the transformation to the data
    /// \params ApplyToTransform to apply the transformation to the transform matrix
    ///
    enum class TransformType
    {
        ApplyToData,
        ConcatenateToTransform
    };

    ///
    /// \brief Enumeration for the data to retrieve
    /// \params PreTransform for data where transform matrix is not applied
    /// \params PostTransform for data where transform matrix is applied
    ///
    enum class DataType
    {
        PreTransform,
        PostTransform
    };

    ///
    /// \brief Constructor
    ///
    Geometry(const Type type) : m_geometryIdx(Geometry::getUniqueID()), m_type(type) {}

    ///
    /// \brief Destructor
    ///
    virtual ~Geometry() = default;

    ///
    /// \brief Print
    ///
    virtual void print() const;

    ///
    /// \brief Returns the volume of the geometry (if valid)
    ///
    virtual double getVolume() const = 0;

    ///
    /// \brief Compute the bounding box for the geometry
    ///
    virtual void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent = 0.0) const;

    ///
    /// \brief Translate the geometry in Cartesian space
    ///
    void translate(const Vec3d&  t,
                   TransformType type = TransformType::ConcatenateToTransform);
    void translate(double x, double y, double z,
                   TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Rotate the geometry in Cartesian space
    ///
    void rotate(const Quatd&  q,
                TransformType type = TransformType::ConcatenateToTransform);
    void rotate(const Mat3d&  m,
                TransformType type = TransformType::ConcatenateToTransform);
    void rotate(const Vec3d& axis, double angle,
                TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Scale in Cartesian directions
    ///
    void scale(double        scaling,
               TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Applies a rigid transform to the geometry
    ///
    void transform(RigidTransform3d T,
                   TransformType    type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Get/Set translation
    ///
    Vec3d getTranslation() const;
    void setTranslation(const Vec3d t);
    void setTranslation(double x, double y, double z);

    ///
    /// \brief Get/Set rotation
    ///
    Mat3d getRotation() const;
    void setRotation(const Mat3d m);
    void setRotation(const Quatd q);
    void setRotation(const Vec3d axis, const double angle);

    ///
    /// \brief Get/Set scaling
    ///
    double getScaling() const;
    void setScaling(const double s);

    ///
    /// \brief Returns the type of the geometry
    ///
    Type getType() const;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    const std::string getTypeName() const;

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    bool isMesh() const;

    ///
    /// \brief Get the global (unique) index of the geometry
    ///
    unsigned int getGlobalIndex() const { return m_geometryIdx; }

    ///
    /// \brief Get a pointer to geometry that has been registered globally
    ///
    static unsigned int getTotalNumberGeometries() { return s_NumGeneratedGegometries; }

protected:
    ///
    /// \brief Get a unique ID for the object
    ///
    static unsigned int getUniqueID();

    static ParallelUtils::SpinLock s_RegistryLock; ///> Mutex lock for thread-safe registry modification
    static unsigned int s_NumGeneratedGegometries; ///> Total number of geometries that have been created in this program

    friend class VTKRenderer;
    friend class VTKRenderDelegate;

    friend class VulkanLineMeshRenderDelegate;
    friend class VulkanRenderDelegate;
    friend class VulkanSurfaceMeshRenderDelegate;
    friend class VulkanRenderer;

    virtual void applyTranslation(const Vec3d t) = 0;
    virtual void applyRotation(const Mat3d r)    = 0;
    virtual void applyScaling(const double s)    = 0;
    virtual void updatePostTransformData()       = 0;

    const unsigned int m_geometryIdx = 0; ///> Unique ID assigned to each geometry upon construction
    const Type         m_type;            ///> Type of geometry

    ParallelUtils::SpinLock m_dataLock;
    ParallelUtils::SpinLock m_transformLock;
    bool m_dataModified      = false;
    bool m_transformModified = false;
    bool m_transformApplied  = true;

    RigidTransform3d m_transform = RigidTransform3d::Identity(); ///> Transform
    double           m_scaling   = 1.0;
};
} //imstk
