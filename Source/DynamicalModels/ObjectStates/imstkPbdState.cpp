/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdState.h"

namespace imstk
{
void
PbdState::setState(std::shared_ptr<PbdState> rhs)
{
    *m_pos = *rhs->getPositions();
    *m_vel = *rhs->getVelocities();
    *m_acc = *rhs->getAccelerations();

    m_pos->postModified();
}
} // namespace imstk