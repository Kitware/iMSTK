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
    ///
    /// \brief Constructor
    ///
    ImageData(const std::string& name = std::string(""));

    ///
    /// \brief Destructor
    ///
    virtual ~ImageData() override = default;

public:
    ///
    /// \brief Print the image data info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume
    ///
    double getVolume() const override;

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
    ///
    inline size_t getScalarIndex(int x, int y, int z = 0) { return getScalarIndex(x, y, z, m_dims, m_numComps); }
    inline size_t getScalarIndex(const Vec3i& imgCoord) { return getScalarIndex(imgCoord[0], imgCoord[1], imgCoord[2], m_dims, m_numComps); }

    ///
    /// \brief Returns index of data in scalar array given structured image coordinate, dimensions, and number of components
    /// does no bounds checking
    ///
    inline static size_t getScalarIndex(int x, int y, int z, const Vec3i& dims, int numComps) { return (x + dims[0] * (y + z * dims[1])) * numComps; }

    std::shared_ptr<ImageData> cast(ScalarType type);

    ///
    /// \brief Returns the origin of the image
    ///
    const Vec3d& getOrigin() const { return m_origin; }

    ///
    /// \brief Returns the spacing of the image
    ///
    const Vec3d& getSpacing() const { return m_spacing; }

    ///
    /// \brief Returns inv spacing of the image
    ///
    const Vec3d& getInvSpacing() const { return m_invSpacing; }

    ///
    /// \brief Returns the scalar type of the image
    ///
    const ScalarType getScalarType() const;

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

    ///
    /// \brief Returns point data scalars
    ///
    std::shared_ptr<AbstractDataArray> getScalars() const { return m_scalarArray; }

    ///
    /// \brief Sets the scalars
    ///
    void setScalars(std::shared_ptr<AbstractDataArray> scalars, const int numComps, int* dim);

    ///
    /// \brief Sets the origin of the image
    ///
    void setOrigin(const Vec3d& origin) { m_origin = origin; }

    ///
    /// \brief Sets the spacing between pixels/voxels of the image
    ///
    void setSpacing(const Vec3d& spacing)
    {
        m_spacing    = spacing;
        m_invSpacing = Vec3d(1.0 / spacing[0], 1.0 / spacing[1], 1.0 / spacing[2]);
    }

    ///
    /// \brief Allocate image by type
    ///
    void allocate(const ScalarType type, const int numComps, const Vec3i& dims, const Vec3d& spacing = Vec3d(1.0, 1.0, 1.0), const Vec3d& origin = Vec3d(0.0, 0.0, 0.0));

    ///
    /// \brief Generates points in the geometry from the ImageData (center of each voxel)
    ///
    void computePoints();

    ///
    /// \brief Clear the data
    ///
    void clear() override;

protected:
    friend class VTKImageDataRenderDelegate;

    std::shared_ptr<AbstractDataArray> m_scalarArray = nullptr;
    Vec3i m_dims       = Vec3i(0, 0, 0);
    int   m_numComps   = 1;
    Vec3d m_origin     = Vec3d(0.0, 0.0, 0.0);
    Vec3d m_spacing    = Vec3d(1.0, 1.0, 1.0);
    Vec3d m_invSpacing = Vec3d(1.0, 1.0, 1.0);
    Vec6d m_bounds;
};
} // imstk
