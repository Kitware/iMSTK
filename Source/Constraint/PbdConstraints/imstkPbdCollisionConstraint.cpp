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

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
PbdCollisionConstraint::PbdCollisionConstraint(const unsigned int n1, const unsigned int n2)
{
    m_bodiesFirst.resize(n1);
    m_dcdxA.resize(n1);

    m_bodiesSecond.resize(n2);
    m_dcdxB.resize(n2);
}

void
PbdCollisionConstraint::solvePosition()
{
    double     c;
    const bool update = this->computeValueAndGradient(c, m_dcdxA, m_dcdxB);
    if (!update)
    {
        return;
    }

    double lambda = 0.0;

    // Sum the mass (so we can weight displacements)
    for (size_t i = 0; i < m_bodiesFirst.size(); i++)
    {
        lambda += m_bodiesFirst[i].invMass * m_dcdxA[i].squaredNorm();
    }

    for (size_t i = 0; i < m_bodiesSecond.size(); i++)
    {
        lambda += m_bodiesSecond[i].invMass * m_dcdxB[i].squaredNorm();
    }

    if (lambda == 0.0)
    {
        return;
    }

    lambda = c / lambda;

    for (size_t i = 0; i < m_bodiesFirst.size(); i++)
    {
        if (m_bodiesFirst[i].invMass > 0.0)
        {
            (*m_bodiesFirst[i].vertex) +=
                m_bodiesFirst[i].invMass * lambda * m_dcdxA[i] * m_stiffnessA;
        }
    }

    for (size_t i = 0; i < m_bodiesSecond.size(); i++)
    {
        if (m_bodiesSecond[i].invMass > 0.0)
        {
            (*m_bodiesSecond[i].vertex) +=
                m_bodiesSecond[i].invMass * lambda * m_dcdxB[i] * m_stiffnessB;
        }
    }
}

void
PbdCollisionConstraint::correctVelocity(const double friction, const double restitution)
{
    const double fricFrac = 1.0 - friction;

    for (size_t i = 0; i < m_bodiesFirst.size(); i++)
    {
        if (m_bodiesFirst[i].invMass > 0.0)
        {
            const Vec3d n = m_dcdxA[i].normalized();
            Vec3d&      v = *m_bodiesFirst[i].velocity;

            // Separate velocity into normal and tangent components
            const Vec3d vN = n.dot(v) * n;
            const Vec3d vT = v - vN;

            // Put back together fractionally based on defined restitution and frictional coefficients
            v = vN * restitution + vT * fricFrac;
        }
    }

    for (size_t i = 0; i < m_bodiesSecond.size(); i++)
    {
        if (m_bodiesSecond[i].invMass > 0.0)
        {
            const Vec3d n = m_dcdxB[i].normalized();
            Vec3d&      v = *m_bodiesSecond[i].velocity;

            // Separate velocity into normal and tangent components
            const Vec3d vN = n.dot(v) * n;
            const Vec3d vT = v - vN;

            // Put back together fractionally based on defined restitution and frictional coefficients
            v = vN * restitution + vT * fricFrac;
        }
    }
}
} // namespace imstk