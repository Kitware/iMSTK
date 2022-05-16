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

#include "imstkMath.h"

namespace imstk
{
class EdgeEdgeCCDState
{
public:
    /// Points that belong to the two input line-segments.
    /// const references are maintained to avoid unnecessary copy operations.
    const Vec3d& xi, & xi1, & xj, & xj1;

    /// Vectors that represent the edges / line-segments
    const Vec3d ei, ej;

    const Vec3d w0;

    /// Shortest distance vector between the infinite lines.
    const Vec3d w;

    EdgeEdgeCCDState(const Vec3d& i0, const Vec3d& i1, const Vec3d& j0, const Vec3d& j1);

    /// Parameterized position of closest point on segment xi--xi1 to segment xj--xj1.
    const double si() const
    {
        return m_si;
    }

    /// Parameterized position of closest point on segment xj--xj1 to segment xi--xi1.
    const double sj() const
    {
        return m_sj;
    }

    /// Pi is the closest point on segment xi--xi1 to segment xj--xj1.
    const Vec3d& pi() const
    {
        return m_pi;
    }

    /// Pj is the closest point on segment xj--xj1 to segment xi--xi1.
    const Vec3d& pj() const
    {
        return m_pj;
    }

    Vec3d computeWBar();

    Vec3d computeWBar2() const;

    /// Pi and Pj are closest points on segments xi--xi1 and xj--xj1 respectively.
    Vec3d computePi() const
    {
        return xi + ei * si();
    }

    /// Pi and Pj are closest points on segments xi--xi1 and xj--xj1 respectively.
    Vec3d computePj() const
    {
        return xj + ej * sj();
    }

    void setThickness(double thickness)
    {
        m_thickness = thickness;
    }

    double thickness() const
    {
        return m_thickness;
    }

    void setTolerance(double eps)
    {
        m_epsilon = eps;
    }

    /**
     * @brief Performs a collision test based on two given timesteps that store the state of two lines each.
     * @param prev State of the two lines in a previous timestep
     * @param curr State of the two lines in current timestep
     * @param relativeTimeOfImpact Estimated time of impact in the interval [0, 1] where 0 is prev time and 1 is current time.
     * @return Type of collision: 0 - no collision, 1 - lines overlap within tolerance, 2 - vertex overlap within tolerance, 3 - lines cross each other.
    */
    static int testCollision(const EdgeEdgeCCDState& prev, EdgeEdgeCCDState& curr, double& relativeTimeOfImpact);

private:
    double m_si, m_sj;
    Vec3d  m_pi, m_pj;

    double m_epsilon = 1e-10;

    // Thickness of colliding LineMeshes.
    double m_thickness = 0.0016;

    double a() const
    {
        return ei.dot(ei);
    }

    double b() const
    {
        return ei.dot(ej);
    }

    double c() const
    {
        return ej.dot(ej);
    }

    double d() const
    {
        return ei.dot(w0);
    }

    double e() const
    {
        return ej.dot(w0);
    }

    double denom() const
    {
        return a() * c() - b() * b();
    }

    double computeSi() const;

    double computeSj() const;

    /// Shortest distance vector between the infinite lines.
    Vec3d shortestDistanceVector() const;
};
} // namespace imstk
