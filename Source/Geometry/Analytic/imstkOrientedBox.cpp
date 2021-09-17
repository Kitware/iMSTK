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

#include "imstkOrientedBox.h"
#include "imstkLogger.h"

namespace imstk
{
void
OrientedBox::print() const
{
    Geometry::print();
    LOG(INFO) << "Extent0: " << m_extents[0];
    LOG(INFO) << "Extent1: " << m_extents[1];
    LOG(INFO) << "Extent2: " << m_extents[2];
}

Vec3d
OrientedBox::getExtents(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_extentsPostTransform;
    }
    return m_extents;
}

void
OrientedBox::setExtents(const Vec3d extents)
{
    if (extents[0] <= 0.0 || extents[1] <= 0.0 || extents[2] <= 0.0)
    {
        LOG(WARNING) << "error: extents should be positive.";
        return;
    }
    if (m_extents == extents)
    {
        return;
    }
    m_extents = extents;
    m_transformApplied = false;
    this->postModified();
}

void
OrientedBox::applyTransform(const Mat4d& m)
{
    AnalyticalGeometry::applyTransform(m);
    Vec3d s = Vec3d(
        m.block<3, 1>(0, 0).norm(),
        m.block<3, 1>(0, 1).norm(),
        m.block<3, 1>(0, 2).norm());
    this->setExtents(m_extents.cwiseProduct(s));
    this->postModified();
}

void
OrientedBox::updatePostTransformData() const
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    const Vec3d s = getScaling();
    m_extentsPostTransform = m_extents.cwiseProduct(s);
    m_transformApplied     = true;
}

double
OrientedBox::getFunctionValue(const Vec3d& pos) const
{
    const Mat3d  rot     = m_orientationPostTransform.toRotationMatrix();
    const Vec3d& extents = m_extentsPostTransform;

    const Vec3d diff = (pos - m_positionPostTransform);
    const Vec3d proj = rot * diff; // dot product/project onto each axes

    bool inside[3] =
    {
        (std::abs(proj[0]) < extents[0]),
        (std::abs(proj[1]) < extents[1]),
        (std::abs(proj[2]) < extents[2])
    };
    bool isInsideCube = inside[0] && inside[1] && inside[2];

    double signedDist = 0.0;
    if (isInsideCube)
    {
        // If inside, find closest face, that is the signed distance
        signedDist = std::numeric_limits<double>::lowest();
        for (int i = 0; i < 3; i++)
        {
            double dist = proj[i];
            if (dist < extents[i] && dist >= 0.0)
            {
                const double unsignedDistToSide = (extents[i] - dist);
                if (-unsignedDistToSide > signedDist)
                {
                    signedDist = -unsignedDistToSide;
                }
            }
            else if (dist > -extents[i] && dist < 0.0)
            {
                const double unsignedDistToSide = (extents[i] + dist);
                if (-unsignedDistToSide > signedDist)
                {
                    signedDist = -unsignedDistToSide;
                }
            }
        }
    }
    else
    {
        // If outside we need to also consider diagonal distance to corners and edges
        // Compute nearest point
        Vec3d axialSignedDists = Vec3d::Zero();
        for (int i = 0; i < 3; i++)
        {
            double dist = proj[i];

            // If distance farther than the box extents, clamp to the box
            if (dist >= extents[i] || dist <= -extents[i])
            {
                axialSignedDists[i] = std::abs(dist) - extents[i];
            }
        }
        signedDist = axialSignedDists.norm();
    }
    return signedDist;
}

void
OrientedBox::computeBoundingBox(Vec3d& min, Vec3d& max, const double imstkNotUsed(paddingPercent))
{
    updatePostTransformData();

    const Mat3d r = m_orientationPostTransform.toRotationMatrix();
    const Vec3d a = r.col(0) * m_extentsPostTransform[0];
    const Vec3d b = r.col(1) * m_extentsPostTransform[1];
    const Vec3d c = r.col(2) * m_extentsPostTransform[2];

    Vec3d pts[8];
    pts[0] = m_positionPostTransform + a + b + c;
    pts[1] = m_positionPostTransform + a + b - c;
    pts[2] = m_positionPostTransform + a - b + c;
    pts[3] = m_positionPostTransform + a - b - c;
    pts[4] = m_positionPostTransform - a + b + c;
    pts[5] = m_positionPostTransform - a + b - c;
    pts[6] = m_positionPostTransform - a - b + c;
    pts[7] = m_positionPostTransform - a - b - c;

    min = pts[0];
    max = pts[0];
    for (int i = 1; i < 8; i++)
    {
        min = min.cwiseMin(pts[i]);
        max = max.cwiseMax(pts[i]);
    }
}
} // imstk
