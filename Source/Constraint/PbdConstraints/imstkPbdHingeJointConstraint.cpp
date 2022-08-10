/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdHingeJointConstraint.h"

namespace imstk
{
void
PbdHingeJointConstraint::initConstraint(
    const PbdParticleId& pIdx0,
    const Vec3d&         hingeAxes,
    const double         k)
{
    m_particles[0] = pIdx0;
    setStiffness(k);

    m_hingeAxes = hingeAxes;
}

bool
PbdHingeJointConstraint::computeValueAndGradient(PbdState& bodies,
                                                 double& c, std::vector<Vec3d>& dcdx)
{
    // Is this the fastest way to get basis vectors?
    const Vec3d up = bodies.getOrientation(m_particles[0]).toRotationMatrix().col(1);

    // Gives rotation
    Vec3d dir = m_hingeAxes.cross(up);
    dcdx[0] = dir.normalized();
    c       = dir.norm();

    return true;
}
} // namespace imstk