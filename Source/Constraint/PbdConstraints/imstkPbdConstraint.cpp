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

#include "imstkPbdConstraint.h"

namespace imstk
{
void
PbdConstraint::projectConstraint(const DataArray<double>& invMasses, const double dt, const SolverType& solverType, VecDataArray<double, 3>& pos)
{
    if (dt == 0.0)
    {
        return;
    }

    double c      = 0.0;
    bool   update = this->computeValueAndGradient(pos, c, m_dcdx);
    if (!update)
    {
        return;
    }

    double dcMidc = 0.0;
    double lambda = 0.0;
    double alpha  = 0.0;

    for (size_t i = 0; i < m_vertexIds.size(); ++i)
    {
        dcMidc += invMasses[m_vertexIds[i]] * m_dcdx[i].squaredNorm();
    }

    if (dcMidc < IMSTK_DOUBLE_EPS)
    {
        return;
    }

    switch (solverType)
    {
    case (SolverType::xPBD):
        alpha     = m_compliance / (dt * dt);
        lambda    = -(c + alpha * m_lambda) / (dcMidc + alpha);
        m_lambda += lambda;
        break;
    case (SolverType::PBD):
        lambda = -c * m_stiffness / dcMidc;
        break;
    default:
        alpha     = m_compliance / (dt * dt);
        lambda    = -(c + alpha * m_lambda) / (dcMidc + alpha);
        m_lambda += lambda;
    }

    for (size_t i = 0, vid = 0; i < m_vertexIds.size(); ++i)
    {
        vid = m_vertexIds[i];
        if (invMasses[vid] > 0.0)
        {
            pos[vid] += invMasses[vid] * lambda * m_dcdx[i];
        }
    }
}
} // namespace imstk
