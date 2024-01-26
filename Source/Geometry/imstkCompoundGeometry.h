/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometry.h"
#include "imstkMath.h"

#include <vector>
#include <memory>

/// Compound Geometry

namespace imstk
{
class AnalyticalGeometry;

/// Class that enables the composition of multiple geometries into on object,
/// useful for creating rigids from multiple shapes.
class CompoundGeometry : public Geometry
{
public:

    IMSTK_TYPE_NAME(CompoundGeometry);

    /// Adds a geometry to the compound geometry
    void add(std::shared_ptr<AnalyticalGeometry> geometry);

    /// Sets a local transform for the object at index, this offsets
    /// the shape from the origin of the overall compound object
    /// \param index index of the subshape
    /// \param transform transform to be used for that shape
    /// if index doesn't exist the operation does nothing
    void setLocalTransform(size_t index, const Mat4d& transform);

    /// \return the number of shapes handled
    size_t count() const { return m_geometries.size(); }

    /// \return the shape with the given index, nullptr otherwise
    std::shared_ptr<Geometry> get(size_t index) const;

    /// \copydoc Geometry::print()
    void print() const override;

    /// \copydoc Geometry::getVolume()
    double getVolume() override;

    /// \copydoc Geometry::computeBoundingBox()
    void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0) override;

    /// \copydoc Geometry::getCenter()
    Vec3d getCenter() override;

    /// \copydoc Geometry::isMesh()
    bool isMesh() const override;

    /// \copydoc Geometry::updatePostTransformData()
    void updatePostTransformData() const override;

    /// \copydoc Geometry::applyTransform()
    void applyTransform(const Mat4d&) override;

private:
    Geometry* cloneImplementation() const override;

    struct LocalGeometry
    {
        std::shared_ptr<Geometry> geometry;
        imstk::Mat4d localTransform = Mat4d::Identity();
    };

    std::vector<LocalGeometry> m_geometries;
};
} // namespace imstk