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

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
///
/// \class PbdTriangleEmbeddingConstraint
///
/// \brief Constraint to keep a specified vertex at a given uvw in a triangle
///
class PbdTriangleEmbeddingConstraint : public PbdCollisionConstraint
{
public:
    PbdTriangleEmbeddingConstraint() : PbdCollisionConstraint(1, 3) { }
    ~PbdTriangleEmbeddingConstraint() override = default;

public:
    ///
    /// \brief Returns the type of the pbd collision constraint
    ///
    Type getType() const { return Type::PointTriangle; }

    ///
    /// \brief initialize constraint
    /// \param pIdxA1 index of the point from object1
    /// \param pIdxB1 first point of the triangle from object2
    /// \param pIdxB2 second point of the triangle from object2
    /// \param pIdxB3 third point of the triangle from object2
    /// \return
    ///
    void initConstraint(VertexMassPair ptA,
                        VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
                        double stiffnessA, double stiffnessB,
                        Vec3d* p, Vec3d* q,
                        double friction)
    {
        m_bodiesFirst[0] = ptA;

        m_bodiesSecond[0] = ptB1;
        m_bodiesSecond[1] = ptB2;
        m_bodiesSecond[2] = ptB3;

        m_stiffnessA = stiffnessA;
        m_stiffnessB = stiffnessB;
        m_friction   = friction;

        const Vec3d& x0 = *m_bodiesFirst[0].vertex; // Intersection point
        const Vec3d& x1 = *m_bodiesSecond[0].vertex;
        const Vec3d& x2 = *m_bodiesSecond[1].vertex;
        const Vec3d& x3 = *m_bodiesSecond[2].vertex;

        // Compute the interpolant on the triangle
        {
            // Compute barycentric coordinates of ptA in tri B
            // This effectively puts the point in a local coordinate
            // system of the triangle so when the triangle begins
            // to move and deform the embedded point will as well
            const Vec3d  v0    = x2 - x1;
            const Vec3d  v1    = x3 - x1;
            const Vec3d  v2    = x0 - x1;
            const double d00   = v0.dot(v0);
            const double d01   = v0.dot(v1);
            const double d11   = v1.dot(v1);
            const double d20   = v2.dot(v0);
            const double d21   = v2.dot(v1);
            const double denom = d00 * d11 - d01 * d01;
            const double v     = (d11 * d20 - d01 * d21) / denom;
            const double w     = (d00 * d21 - d01 * d20) / denom;
            const double u     = 1.0 - v - w;
            m_uvw[0] = u;
            m_uvw[1] = v;
            m_uvw[2] = w;
        }
        // Compute the interpolant on the line
        {
            m_p = p;
            m_q = q;
            const Vec3d pq = (*p - *q).normalized();
            const Vec3d d  = x0 - *q;
            m_t = pq.dot(d);
        }
    }

    // Sets the point of vertex
    VertexMassPair& getVertexMassA()
    {
        return m_bodiesFirst[0];
    }

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[in] currVertexPositionsA current positions from object A
    /// \param[in] currVertexPositionsA current positions from object B
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    bool computeValueAndGradient(double&             c,
                                 std::vector<Vec3d>& dcdxA,
                                 std::vector<Vec3d>& dcdxB) const override
    {
        //const Vec3d& x0 = *m_bodiesFirst[0].vertex;
        const Vec3d& x1 = *m_bodiesSecond[0].vertex;
        const Vec3d& x2 = *m_bodiesSecond[1].vertex;
        const Vec3d& x3 = *m_bodiesSecond[2].vertex;

        Vec3d*      p    = m_p;
        Vec3d*      q    = m_q;
        const Vec3d pq   = (*p - *q);
        const Vec3d pq_n = pq.normalized();

        const Vec3d triPos  = x1 * m_uvw[0] + x2 * m_uvw[1] + x3 * m_uvw[2];
        const Vec3d linePos = (*q) + pq_n * m_t;

        // Directly transform to each other
        Vec3d diff = triPos - linePos;
        // Remove normal component to allow slide (don't completely remove to allow a sort of friction)
        diff = diff - diff.dot(pq_n) * pq_n * (1.0 - m_friction);
        const Vec3d n = diff.normalized();

        dcdxA[0] = Vec3d::Zero();
        dcdxB[0] = n;
        dcdxB[1] = n;
        dcdxB[2] = n;

        c = -diff.norm();

        return true;
    }

protected:
    // Interpolant for triangle (uv coords)
    Vec3d m_uvw = Vec3d::Zero();

    // Interpolant for line
    double m_t = 0.0;

    // Line
    Vec3d* m_p = nullptr;
    Vec3d* m_q = nullptr;

    bool m_enabled = true;

    double m_friction = 0.99;
};
}