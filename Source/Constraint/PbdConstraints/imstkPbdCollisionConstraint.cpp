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
PbdCollisionConstraint::PbdCollisionConstraint(const unsigned int& n1, const unsigned int& n2)
{
    m_bodiesFirst.resize(n1);
    m_bodiesSecond.resize(n2);
}

void
PbdCollisionConstraint::projectConstraint(const DataArray<double>& invMassA,
                                          const DataArray<double>& invMassB,
                                          VecDataArray<double, 3>& posA,
                                          VecDataArray<double, 3>& posB)
{
    double                  c;
    VecDataArray<double, 3> dcdxA;
    VecDataArray<double, 3> dcdxB;

    bool update = this->computeValueAndGradient(posA, posB, c, dcdxA, dcdxB);
    if (!update)
    {
        return;
    }

    double lambda = 0.0;

    for (size_t i = 0; i < m_bodiesFirst.size(); ++i)
    {
        lambda += invMassA[m_bodiesFirst[i]] * dcdxA[i].squaredNorm();
    }

    for (size_t i = 0; i < m_bodiesSecond.size(); ++i)
    {
        lambda += invMassB[m_bodiesSecond[i]] * dcdxB[i].squaredNorm();
    }

    lambda = c / lambda;

    for (size_t i = 0, vid = 0; i < m_bodiesFirst.size(); ++i)
    {
        vid = m_bodiesFirst[i];
        if (invMassA[vid] > 0.0)
        {
            posA[vid] -= invMassA[vid] * lambda * dcdxA[i] * m_configA->m_stiffness;
        }
    }

    for (size_t i = 0, vid = 0; i < m_bodiesSecond.size(); ++i)
    {
        vid = m_bodiesSecond[i];
        if (invMassB[vid] > 0.0)
        {
            posB[vid] -= invMassB[vid] * lambda * dcdxB[i] * m_configB->m_stiffness;
        }
    }

    return;
}
} // imstk
