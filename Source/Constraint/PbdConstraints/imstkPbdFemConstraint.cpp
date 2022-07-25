/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdFemConstraint.h"

namespace imstk
{
PbdFemConstraint::PbdFemConstraint(const unsigned int cardinality,
                                   MaterialType       mType /*= MaterialType::StVK*/) :
    PbdConstraint(),
    m_initialElementVolume(0),
    m_material(mType),
    m_invRestMat(Mat3d::Identity())
{
    m_vertexIds.resize(cardinality);
    m_dcdx.resize(cardinality);
}
} // namespace imstk