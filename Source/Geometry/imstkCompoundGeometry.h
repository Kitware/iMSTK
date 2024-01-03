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

class CompoundGeometry : public Geometry
{
public:

    IMSTK_TYPE_NAME(CompoundGeometry);

    void print() const override;

    double getVolume() override;

    void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0) override;

    Vec3d getCenter() override;

    bool isMesh() const override;

    void updatePostTransformData() const override;

    void applyTransform(const Mat4d&) override;

    void transformLocally(size_t index, const Mat4d& transform);

    void add(std::shared_ptr<AnalyticalGeometry> geometry);

    size_t count() const { return m_geometries.size(); }

    std::shared_ptr<Geometry> get(size_t index) const
    {
        if (index < m_geometries.size())
        {
            return m_geometries[index].geometry;
        }
        else
        {
            return nullptr;
        }
    }

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