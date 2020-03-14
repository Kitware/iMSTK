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

#include "imstkLinearProjectionConstraint.h"

namespace  imstk
{
LinearProjectionConstraint::
LinearProjectionConstraint(const size_t& nodeId, const bool isFixed /*= false*/)
{
    m_nodeId = nodeId;
    if (isFixed)
    {
        m_projection = Mat3d::Zero();
        m_isFixedConstraint = true;
    }
}

void
LinearProjectionConstraint::setProjection(const size_t& nodeId, const Vec3d& p, const Vec3d& q /*= Vec3d::Zero()*/)
{
    m_nodeId     = nodeId;
    m_projection = Mat3d::Identity() - p * p.transpose() - q * q.transpose();
}

void
LinearProjectionConstraint::setProjectionToLine(const size_t& nodeId, const Vec3d& p)
{
    m_nodeId = nodeId;
    auto v = p / p.norm();
    m_projection = v * v.transpose();
}

void
LinearProjectionConstraint::setProjectorToDirichlet(const unsigned int& nodeId, const Vec3d z)
{
    m_nodeId     = nodeId;
    m_projection = Mat3d::Zero();
    m_isFixedConstraint = true;
    m_value = z;
}

void
LinearProjectionConstraint::reset()
{
    m_projection = Mat3d::Identity();
    m_value      = Vec3d(0., 0., 0.);
}
} // imstk