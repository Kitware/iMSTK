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

#include "imstkTimeIntegrator.h"

#include <g3log/g3log.hpp>

namespace imstk {

TimeIntegrator::TimeIntegrator(const Type type)
{
    this->setType(type);
}

void
TimeIntegrator::setType(const Type type)
{
    m_type = type;
    this->setCoefficients(type);
}

const TimeIntegrator::Type&
TimeIntegrator::getType() const
{
    return m_type;
}

void
TimeIntegrator::setCoefficients(const Type type)
{
    switch (type)
    {
    case Type::BackwardEuler:
        m_alpha = {1, 0, 0};
        m_beta = {1, -1, 0};
        m_gamma = {1, -2, -1};
        break;

    case Type::ForwardEuler:
    case Type::NewmarkBeta:
    case Type::CentralDifference:
        LOG(WARNING) << "TimeIntegrator::setCoefficients error: type of the time integrator not supported.";
        break;

    default:
        LOG(WARNING) << "TimeIntegrator::setCoefficients error: type of the time integrator not identified!";
        break;
    }
}
}
