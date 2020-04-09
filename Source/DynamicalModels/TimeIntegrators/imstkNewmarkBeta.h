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

#include "imstkTimeIntegrator.h"

#include <array>
#include "imstkLogger.h"

namespace imstk
{
///
/// \class Newmark-beta time integration
///
class NewmarkBeta : public TimeIntegrator
{
public:
    ///
    /// \brief Constructor
    ///
    NewmarkBeta(const double dT, const double beta = 0.25, const double gamma = 0.5) : TimeIntegrator(Type::NewmarkBeta, dT), m_gamma(gamma), m_beta(beta)
    {}

    ///
    /// \brief Destructor
    ///
    ~NewmarkBeta() = default;

    void updateStateGivenDv(std::shared_ptr<FeDeformBodyState> prevState, std::shared_ptr<FeDeformBodyState> currentState, Vectord& dV)
    {
        currentState->getQDot()    = prevState->getQDot() + dV;
        currentState->getQDotDot() = (currentState->getQDot() - prevState->getQDot()) / (m_gamma * m_dT) - (1.0 / m_gamma - 1) * prevState->getQDotDot();
        currentState->getQ()       = prevState->getQ() + m_dT * currentState->getQDot() + 0.5 * m_dT * m_dT * ((1 - 2 * m_beta) * prevState->getQDotDot() + 2 * m_beta * currentState->getQDotDot());
    }

    void updateStateGivenDu(std::shared_ptr<FeDeformBodyState> prevState, std::shared_ptr<FeDeformBodyState> currentState, Vectord& dU)
    {
    }

    void updateStateGivenV(std::shared_ptr<FeDeformBodyState> prevState, std::shared_ptr<FeDeformBodyState> currentState, Vectord& v)
    {
        currentState->getQDot()    = v;
        currentState->getQDotDot() = (currentState->getQDot() - prevState->getQDot()) / (m_gamma * m_dT) - (1.0 / m_gamma - 1) * prevState->getQDotDot();
        currentState->getQ()       = prevState->getQ() + m_dT * currentState->getQDot() + 0.5 * m_dT * m_dT * ((1 - 2 * m_beta) * prevState->getQDotDot() + 2 * m_beta * currentState->getQDotDot());
    }

    void updateStateGivenU(std::shared_ptr<FeDeformBodyState> prevState, std::shared_ptr<FeDeformBodyState> currentState, Vectord& u)
    {
    }

protected:
    double m_beta;
    double m_gamma;

//    // Coefficients of the time integrator
//    std::array<double, 3> m_alpha = { { 1, 0, 0 } };
//    std::array<double, 3> m_beta = { { 1, -1, 0 } };
//    std::array<double, 3> m_gamma = { { 1, -2, -1 } };
};
} // imstk
