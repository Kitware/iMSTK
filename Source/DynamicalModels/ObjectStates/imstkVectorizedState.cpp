/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVectorizedState.h"

namespace imstk
{
void
FeDeformBodyState::initialize(const size_t numDof)
{
    m_q.resize(numDof);
    m_qDot.resize(numDof);
    m_qDotDot.resize(numDof);

    m_q.setZero();
    m_qDot.setZero();
    m_qDotDot.setZero();
};

void
FeDeformBodyState::setState(const Vectord& u, const Vectord& v, const Vectord& a)
{
    m_q       = u;
    m_qDot    = v;
    m_qDotDot = a;
}

void
FeDeformBodyState::setU(const Vectord& u)
{
    m_q = u;
}

void
FeDeformBodyState::setV(const Vectord& v)
{
    m_qDot = v;
}

void
FeDeformBodyState::setA(const Vectord& a)
{
    m_qDotDot = a;
}

void
FeDeformBodyState::setState(std::shared_ptr<FeDeformBodyState> rhs)
{
    m_q       = rhs->getQ();
    m_qDot    = rhs->getQDot();
    m_qDotDot = rhs->getQDotDot();
}
} // namespace imstk