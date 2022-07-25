/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
