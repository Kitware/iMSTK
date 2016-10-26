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

#include "imstkPbdState.h"

namespace imstk
{

void
PbdState::initialize(const std::shared_ptr<Mesh>& m)
{
    m_pos = m->getVerticesPositions(); // share the same data with Mesh
    const int nP = m->getNumVertices();
    m_initPos.assign(m_pos.begin(), m_pos.end());
    m_vel.resize(nP, Vec3d(0, 0, 0));
    m_acc.resize(nP, Vec3d(0, 0, 0));
    m_oldPos.resize(nP, Vec3d(0, 0, 0));
    m_invMass.resize(nP, 0);
    m_mass.resize(nP, 0);
}

void
PbdState::setUniformMass(const double& val)
{
    if (val != 0.0)
    {
        std::fill(m_mass.begin(), m_mass.end(), val);
        std::fill(m_invMass.begin(), m_invMass.end(), 1 / val);
    }
    else
    {
        std::fill(m_invMass.begin(), m_invMass.end(), 0.0);
        std::fill(m_mass.begin(), m_mass.end(), 0.0);
    }
}

void
PbdState::setParticleMass(const double& val, const unsigned int& idx)
{
    if (idx < m_pos.size())
    {
        m_mass[idx] = val;
        m_invMass[idx] = 1 / val;
    }
}

void
PbdState::setFixedPoint(const unsigned int& idx)
{
    if (idx < m_pos.size())
    {
        m_invMass[idx] = 0;
    }
}

double
PbdState::getInvMass(const unsigned int& idx)
{
    return m_invMass[idx];
}

Vec3d&
PbdState::getInitialVertexPosition(const unsigned int& idx)
{
    return m_initPos.at(idx);
}

void
PbdState::setVertexPosition(const unsigned int& idx, Vec3d& pos)
{
    m_pos.at(idx) = pos;
}

Vec3d&
PbdState::getVertexPosition(const unsigned int& idx)
{
    return m_pos.at(idx);
}

void
PbdState::integratePosition()
{
    for (size_t i = 0; i < m_pos.size(); ++i)
    {
        if (m_invMass[i] != 0.0)
        {
            m_vel[i] += (m_acc[i] + gravity)*dt;
            m_oldPos[i] = m_pos[i];
            m_pos[i] += m_vel[i] * dt;

        }
    }
}

void
PbdState::integrateVelocity()
{
    for (size_t i = 0; i < m_pos.size(); ++i)
    {
        if (m_invMass[i] != 0.0)
        {
            m_vel[i] = (m_pos[i] - m_oldPos[i]) / dt;
        }
    }
}

} // imstk