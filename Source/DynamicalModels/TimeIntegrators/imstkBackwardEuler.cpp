/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkBackwardEuler.h"

namespace imstk
{
void
BackwardEuler::updateStateGivenDv(std::shared_ptr<FeDeformBodyState> prevState,
                                  std::shared_ptr<FeDeformBodyState> currentState,
                                  Vectord&                           dV)
{
    currentState->getQDot() = prevState->getQDot() + dV;
    currentState->getQ()    = prevState->getQ() + m_dT * currentState->getQDot();
}

void
BackwardEuler::updateStateGivenDu(std::shared_ptr<FeDeformBodyState> prevState,
                                  std::shared_ptr<FeDeformBodyState> currentState,
                                  Vectord&                           dU)
{
    currentState->getQ()    = prevState->getQ() + dU;
    currentState->getQDot() = (currentState->getQ() - prevState->getQ()) / m_dT;
}

void
BackwardEuler::updateStateGivenV(std::shared_ptr<FeDeformBodyState> prevState,
                                 std::shared_ptr<FeDeformBodyState> currentState,
                                 Vectord&                           v)
{
    currentState->getQDot() = v;
    currentState->getQ()    = prevState->getQ() + m_dT * currentState->getQDot();
}

void
BackwardEuler::updateStateGivenU(std::shared_ptr<FeDeformBodyState> prevState,
                                 std::shared_ptr<FeDeformBodyState> currentState,
                                 Vectord&                           u)
{
    currentState->getQ()    = u;
    currentState->getQDot() = (currentState->getQ() - prevState->getQ()) / m_dT;
}
} // namespace imstk