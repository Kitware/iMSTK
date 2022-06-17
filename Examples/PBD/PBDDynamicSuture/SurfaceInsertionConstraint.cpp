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

#pragma once

#include "SurfaceInsertionConstraint.h"

using namespace imstk;

void
SurfaceInsertionConstraint::initConstraint(
    Vec3d          insertionPoint,
    VertexMassPair ptB1,
    VertexMassPair ptB2,
    VertexMassPair ptB3,
    Vec3d          contactPt,
    Vec3d          barycentricPt,
    double         stiffnessA,
    double         stiffnessB)
{
    m_insertionPoint  = insertionPoint;
    m_contactPt       = contactPt;
    m_barycentricPt   = barycentricPt;
    m_bodiesSecond[0] = ptB1;
    m_bodiesSecond[1] = ptB2;
    m_bodiesSecond[2] = ptB3;
    m_stiffnessA      = stiffnessA;
    m_stiffnessB      = stiffnessB;
}

bool
SurfaceInsertionConstraint::computeValueAndGradient(
    double&             c,
    std::vector<Vec3d>& dcdxA,
    std::vector<Vec3d>& dcdxB) const
{
    // Get current position of puncture point
    // Move triangle to match motion of needle

    Vec3d diff = m_contactPt - m_insertionPoint;

    c = diff.norm();

    // If sufficiently close, do not solve constraint
    if (c < 1E-8)
    {
        return false;
    }

    diff.normalize();// gradient dcdx

    // Weight by berycentric coordinates
    dcdxB[0] = diff * m_barycentricPt[0];
    dcdxB[1] = diff * m_barycentricPt[1];
    dcdxB[2] = diff * m_barycentricPt[2];

    // Dont adjust position of needle, force mesh to follow needle
    dcdxA[0] = Vec3d::Zero();

    return true;
}
