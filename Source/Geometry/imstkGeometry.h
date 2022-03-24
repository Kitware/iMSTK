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

#include "imstkEventObject.h"
#include "imstkMacros.h"
#include "imstkMath.h"

#include <unordered_map>
#include <string>

namespace imstk
{
///
/// \class Geometry
/// \brief Base class for any geometrical representation
///
class Geometry : public EventObject
{
public:
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

protected:
    Geometry() : m_globalId(getUniqueId()) { }

public:
    Geometry(Geometry& other)
    {
        m_transformApplied = other.m_transformApplied;
        m_transform = other.m_transform;
        m_globalId = Geometry::getUniqueId();
    }
    ~Geometry() override = default;

    void operator=(const Geometry& other)
    {
        m_transformApplied = other.m_transformApplied;
        m_transform = other.m_transform;
        m_globalId = Geometry::getUniqueId();
    }

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    virtual const std::string getTypeName() const = 0;

    // *INDENT-OFF*
    SIGNAL(Geometry, modified);
    // *INDENT-ON*

    ///
    /// \brief Print
    ///
    virtual void print() const;

    ///
    /// \brief Returns the volume of the geometry (if valid)
    ///
    virtual double getVolume() { return 0.0; };

    ///
    /// \brief Compute the bounding box for the geometry
    /// \todo Padding should not be here
    ///
    virtual void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0);

    ///
    /// \brief Returns the bounding box center
    ///
    virtual Vec3d getCenter()
    {
        Vec3d min, max;
        computeBoundingBox(min, max);
        return (min + max) * 0.5;
    }

    ///
    /// \brief Translate the geometry in Cartesian space
    ///@{
    void translate(const Vec3d& t, TransformType type = TransformType::ConcatenateToTransform);
    void translate(double x, double y, double z, TransformType type = TransformType::ConcatenateToTransform);
    ///@}

    ///
    /// \brief Rotate the geometry in Cartesian space
    ///@{
    void rotate(const Quatd& q, TransformType type = TransformType::ConcatenateToTransform);
    void rotate(const Mat3d& m, TransformType type = TransformType::ConcatenateToTransform);
    void rotate(const Vec3d& axis, double radians, TransformType type = TransformType::ConcatenateToTransform);
    ///@}

    ///
    /// \brief Scale in Cartesian directions
    ///@{
    void scale(const Vec3d& scaling, TransformType type = TransformType::ConcatenateToTransform);
    void scale(const double scaling, TransformType type = TransformType::ConcatenateToTransform);
    ///@}

    ///
    /// \brief Applies a rigid transform to the geometry
    ///
    void transform(const Mat4d& T, TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Get/Set translation
    ///@{
    Vec3d getTranslation() const;
    void setTranslation(const Vec3d& t);
    void setTranslation(const double x, const double y, const double z);
    ///@}

    ///
    /// \brief Get/Set rotation
    ///@{
    Mat3d getRotation() const;
    void setRotation(const Mat3d& m);
    void setRotation(const Quatd& q);
    void setRotation(const Vec3d& axis, const double angle);
    ///@}

    ///
    /// \brief Get/Set scaling
    ///@{
    Vec3d getScaling() const;
    void setScaling(const Vec3d& s);
    void setScaling(const double s);
    ///@}

    ///
    /// \brief Get/Set the transform
    ///@{
    const Mat4d& getTransform() const { return m_transform; }
    void setTransform(const Mat4d& m)
    {
        m_transform = m;
        m_transformApplied = false;
    }
    ///@}
    
    ///
    /// \brief Get the global (unique) index of the geometry
    ///
    size_t getGlobalId() const { return m_globalId; }

    ///
    /// \brief Get number of ids/geometries
    ///
    static size_t getNumGlobalIds() { return s_numGlobalIds; }

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    virtual bool isMesh() const { return false; }

    ///
    /// \brief Post modified event
    ///
    void postModified() { this->postEvent(Event(Geometry::modified())); }

    virtual void updatePostTransformData() const { }

protected:
    ///
    /// \brief Directly apply transform to data
    ///
    virtual void applyTransform(const Mat4d& imstkNotUsed(m)) { }

    ///
    /// \brief Get a unique ID for the new generated geometry object
    ///
    static size_t getUniqueId()
    {
        const size_t idx = s_numGlobalIds;
        s_numGlobalIds++;
        return idx;
    }

protected:
    mutable bool m_transformApplied = true; // Internally used for lazy evaluation

    Mat4d m_transform = Mat4d::Identity(); ///> Transformation matrix

    size_t m_globalId; ///> Unique ID assigned to each geometry upon construction

    /// Total number of geometries that have been created in this program
    static std::atomic<size_t> s_numGlobalIds;
};
} // namespace imstk