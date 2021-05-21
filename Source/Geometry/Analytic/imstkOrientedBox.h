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

#include "imstkAnalyticalGeometry.h"

namespace imstk
{
///
/// \class OrientedBox
///
/// \brief OrientedBox geometry
///
class OrientedBox : public AnalyticalGeometry
{
public:
    OrientedBox(const Vec3d& pos = Vec3d(0.0, 0.0, 0.0), const Vec3d extents = Vec3d(0.5, 0.5, 0.5), const Vec3d& orientationAxis = Vec3d(0.0, 1.0, 0.0),
                const std::string& name = std::string("defaultOrientedBox")) : AnalyticalGeometry(name)
    {
        setPosition(pos);
        setOrientationAxis(orientationAxis);
        setExtents(extents);
    }

    virtual ~OrientedBox() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    virtual const std::string getTypeName() const override { return "OrientedBox"; }

public:
    ///
    /// \brief Print the cube info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cube
    ///
    virtual double getVolume() override { return m_extents[0] * m_extents[1] * m_extents[2] * 8.0; }

    ///
    /// \brief Returns the extents of the cube
    ///
    Vec3d getExtents(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the extents of the cube (half dimensions)
    ///
    void setExtents(const Vec3d extents);

    ///
    /// \brief Sets the width of the cube (half dimensions)
    ///
    void setExtents(const double x, const double y, const double z) { setExtents(Vec3d(x, y, z)); }

    ///
    /// \brief Returns signed distance to surface at pos
    /// \todo Doesn't support orientation yet
    ///
    double getFunctionValue(const Vec3d& pos) const override
    {
        const Mat3d  rot     = Quatd::FromTwoVectors(m_orientationAxisPostTransform, Vec3d(0.0, 1.0, 0.0)).toRotationMatrix();
        const Vec3d& extents = m_extentsPostTransform;

        const Vec3d diff   = (pos - m_positionPostTransform);
        const Mat3d rotInv = rot.transpose();
        const Vec3d proj   = rotInv * diff; // dot product/project onto each axes

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
            Vec3d closestPt = Vec3d::Zero();
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

    ///
    /// \brief Get the min, max of the AABB around the cube
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent = 0.0) override;

    ///
    /// \brief Update the Cube parameters applying the latest transform
    ///
    void updatePostTransformData() const override;

protected:
    void applyTransform(const Mat4d& m) override;

    Vec3d m_extents = Vec3d(0.5, 0.5, 0.5);                      // Half dimensions of the oriented box
    mutable Vec3d m_extentsPostTransform = Vec3d(0.5, 0.5, 0.5); ///> Half dimensions of the oriented box once transform applied
};
}
