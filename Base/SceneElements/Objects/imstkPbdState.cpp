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
    //m_oldPos.assign(m_pos.begin(), m_pos.end());
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

} // imstk