/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCompoundGeometry.h"

#include "imstkAnalyticalGeometry.h"
#include "imstkLogger.h"

#include <algorithm>
#include <numeric>

namespace imstk
{
void
CompoundGeometry::print() const
{
    std::for_each(m_geometries.begin(), m_geometries.end(), [](const auto& geom) { geom.geometry->print(); });
}

double
CompoundGeometry::getVolume()
{
    updatePostTransformData();
    return std::accumulate(m_geometries.begin(), m_geometries.end(), 0.0, [](auto val, auto& geom)
        { return val + geom.geometry->getVolume(); });
}

void
CompoundGeometry::computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent /*= 0.0*/)
{
    Eigen::AlignedBox3d box;
    box.setEmpty();
    updatePostTransformData();
    for (auto geom : m_geometries)
    {
        Vec3d localLowLeft;
        Vec3d localUpRight;
        geom.geometry->computeBoundingBox(localLowLeft, localUpRight, paddingPercent);
        box.extend(Eigen::AlignedBox3d(localLowLeft, localUpRight));
    }
    lowerCorner = box.corner(Eigen::AlignedBox3d::BottomLeftFloor);
    upperCorner = box.corner(Eigen::AlignedBox3d::TopRightCeil);
}

Vec3d
CompoundGeometry::getCenter()
{
    updatePostTransformData();
    return std::accumulate(m_geometries.begin(), m_geometries.end(), Vec3d(0, 0, 0), [](auto val, auto& geom)
        { return val + geom.geometry->getCenter(); }) / m_geometries.size();
}

bool
CompoundGeometry::isMesh() const
{
    return false;
}

void
CompoundGeometry::updatePostTransformData() const
{
    // Take the current transform and offset each
    if (!m_transformApplied)
    {
        std::for_each(m_geometries.begin(), m_geometries.end(), [this](auto& geom) {
                auto transform = m_transform * geom.localTransform;
                CHECK(!transform.hasNaN());
                geom.geometry->setTransform(transform);
                geom.geometry->updatePostTransformData();
                                                                          });
    }
    m_transformApplied = true;
}

void
CompoundGeometry::applyTransform(const Mat4d& mat)
{
    std::for_each(m_geometries.begin(), m_geometries.end(), [&mat](auto& geom) {
            auto transform = mat * geom.localTransform;
            CHECK(!transform.hasNaN());
            geom.geometry->setTransform(transform);
            geom.geometry->updatePostTransformData();
                        });
    m_transformApplied = true;
}

void
CompoundGeometry::transformLocally(size_t index, const Mat4d& transform)
{
    if (index >= m_geometries.size())
    {
        return;
    }
    m_transformApplied = false;
    m_geometries[index].localTransform = transform;
}

void
CompoundGeometry::add(std::shared_ptr<AnalyticalGeometry> geometry)
{
    if (std::find_if(m_geometries.begin(), m_geometries.end(), [this, geometry](auto& geom) { return geom.geometry == geometry; }) == m_geometries.end())
    {
        m_geometries.push_back({ geometry, Mat4d::Identity() });
    }
    else
    {
        LOG(WARNING) << "Added duplicate geometry to compound geometry, ignored " << geometry->getName();
    }
}

Geometry*
imstk::CompoundGeometry::cloneImplementation() const
{
    throw std::logic_error("The method or operation is not implemented.");
}
} // namespace imstk
