#include "imstkPbdConstraint.h"

namespace imstk
{

void PbdConstraint::projectConstraint(const StdVectorOfReal& invMasses, const double dt, const SolverType& solverType, StdVectorOfVec3d& pos)
{
    double c;
    StdVectorOfVec3d dcdx;

    bool update = this->computeValueAndGradient(pos, c, dcdx);
    if (!update) return;

    double dcMidc = 0.0;
    double lambda = 0.0;
    double alpha;

    for (size_t i=0; i<m_vertexIds.size(); ++i)
    {
        dcMidc += invMasses[m_vertexIds[i]] * dcdx[i].squaredNorm();
    }

    if (dcMidc < VERY_SMALL_EPSILON)
    {
        return;
    }

    switch (solverType)
    {
    case (SolverType::xPBD):
        alpha = m_compliance / (dt * dt );
        lambda = -(c + alpha * m_lambda) / (dcMidc + alpha);
        m_lambda += lambda;
        break;
    case (SolverType::PBD):
        lambda = -c * m_stiffness / dcMidc;
        break;
    default:
        alpha = m_compliance / (dt * dt );
        lambda = -(c + alpha * m_lambda) / (dcMidc + alpha);
        m_lambda += lambda;
    }

    for (size_t i=0, vid=0; i<m_vertexIds.size(); ++i)
    {
        vid = m_vertexIds[i];
        if (invMasses[vid] > 0.0)
        {
            pos[vid] += invMasses[vid] * lambda * dcdx[i];
        }
    }

    return;
}

}
