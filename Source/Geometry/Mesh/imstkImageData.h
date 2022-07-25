/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPointSet.h"
#include "imstkTypes.h"

namespace imstk
{
class AbstractDataArray;

///
/// \class ImageData
///
/// \brief Class to represent 1, 2, or 3D image data (i.e. structured points)
///
class ImageData : public PointSet
{
public:
    ImageData() = default;
    ~ImageData() override = default;

    IMSTK_TYPE_NAME(ImageData)

    ///
    /// \brief Print the image data info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume
    ///
    double getVolume() override;

    ///
    /// \brief Returns the dimensions of the image
    ///
    const Vec3i& getDimensions() const { return m_dims; }

    ///
    /// \brief Returns the number of components of the image
    ///
    int getNumComponents() const { return m_numComps; }

    ///
    /// \brief Returns a pointer to the underlying storage of the image
    ///
    void* getVoidPointer();

    ///
    /// \brief Returns index of data in scalar array given structured image coordinate, does no bounds checking
    ///@{
    inline size_t getScalarIndex(int x, int y, int z = 0) { return getScalarIndex(x, y, z, m_dims, m_numComps); }
    inline size_t getScalarIndex(const Vec3i& imgCoord) { return getScalarIndex(imgCoord[0], imgCoord[1], imgCoord[2], m_dims, m_numComps); }
    ///@}

    ///
    /// \brief Returns index of data in scalar array given structured image coordinate, dimensions, and number of components
    /// does no bounds checking
    ///
    inline static size_t getScalarIndex(int x, int y, int z, const Vec3i& dims, int numComps) { return (x + dims[0] * (y + z * dims[1])) * numComps; }

    std::shared_ptr<ImageData> cast(ScalarTypeId type);

    ///
    /// \brief Returns inv spacing of the image
    ///
    const Vec3d& getInvSpacing() const { return m_invSpacing; }

    ///
    /// \brief Returns the scalar type of the image
    ///
    const ScalarTypeId getScalarType() const;

    ///
    /// \brief Returns bounds of the image (voxel to voxel)
    ///
    const Vec6d& getBounds()
    {
        const Vec3d size = Vec3d(
            m_spacing.x() * m_dims[0],
            m_spacing.y() * m_dims[1],
            m_spacing.z() * m_dims[2]);

        m_bounds[0] = m_origin.x();
        m_bounds[1] = m_origin.x() + size.x();
        m_bounds[2] = m_origin.y();
        m_bounds[3] = m_origin.y() + size.y();
        m_bounds[4] = m_origin.z();
        m_bounds[5] = m_origin.z() + size.z();

        return m_bounds;
    }

    void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double imstkNotUsed(paddingPercent)) override
    {
        const Vec6d& bounds = getBounds();
        lowerCorner = Vec3d(bounds[0], bounds[2], bounds[4]);
        upperCorner = Vec3d(bounds[1], bounds[3], bounds[5]);
    }

    ///
    /// \brief Get/Set the scalars
    ///@{
    std::shared_ptr<AbstractDataArray> getScalars() const { return m_scalarArray; }
    void setScalars(std::shared_ptr<AbstractDataArray> scalars, const int numComps, int* dim);
    ///@}

    ///
    /// \brief Get/Set the origin of the image
    ///@{
    const Vec3d& getOrigin() const { return m_origin; }
    void setOrigin(const Vec3d& origin) { m_origin = origin; }
    ///@}

    ///
    /// \brief Get/Set the spacing between pixels/voxels of the image
    ///@{
    const Vec3d& getSpacing() const { return m_spacing; }
    void setSpacing(const Vec3d& spacing)
    {
        m_spacing    = spacing;
        m_invSpacing = Vec3d(1.0 / spacing[0], 1.0 / spacing[1], 1.0 / spacing[2]);
    }

    ///@}

    ///
    /// \brief Allocate image by type
    ///
    void allocate(const ScalarTypeId type, const int numComps, const Vec3i& dims, const Vec3d& spacing = Vec3d(1.0, 1.0, 1.0), const Vec3d& origin = Vec3d(0.0, 0.0, 0.0));

    ///
    /// \brief Generates points in the geometry from the ImageData (center of each voxel)
    ///
    void computePoints();

    ///
    /// \brief Clear the data
    ///
    void clear() override;

private:
    // ImageData does not use transform, consider splitting into separate class
    using Geometry::translate;
    using Geometry::rotate;
    using Geometry::scale;
    using Geometry::transform;
    using Geometry::setRotation;
    using Geometry::setScaling;
    using Geometry::setTranslation;
    using Geometry::setTransform;

protected:
    std::shared_ptr<AbstractDataArray> m_scalarArray = nullptr;
    Vec3i m_dims       = Vec3i(0, 0, 0);
    int   m_numComps   = 1;
    Vec3d m_origin     = Vec3d(0.0, 0.0, 0.0);
    Vec3d m_spacing    = Vec3d(1.0, 1.0, 1.0);
    Vec3d m_invSpacing = Vec3d(1.0, 1.0, 1.0);
    Vec6d m_bounds;
};
} // namespace imstk