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

#ifndef imstkBackwardEuler_h
#define imstkBackwardEuler_h

#include "imstkTimeIntegrator.h"

#include <array>
#include "g3log/g3log.hpp"

namespace imstk
{

///
/// \class Backward Euler time integration
///
class BackwardEuler : public TimeIntegrator
{

public:
    ///
    /// \brief Constructor
    ///
    BackwardEuler(const double dT) : TimeIntegrator(dT), m_type(Type::BackwardEuler){};

    ///
    /// \brief Destructor
    ///
    ~BackwardEuler() = default;

    void updateStateGivenDv(std::shared_ptr<ProblemState> prevState, std::shared_ptr<ProblemState> currentState, Vectord& dV)
    {
        currentState->getQDot() = prevState->getQDot() + dV;
        currentState->getQ() = prevState->getQ() + m_dT*currentState->getQDot();
    }

    void updateStateGivenDu(std::shared_ptr<ProblemState> prevState, std::shared_ptr<ProblemState> currentState, Vectord& dU)
    {
        currentState->getQ() = prevState->getQ() + dU;
        currentState->getQDot() = (currentState->getQ() - prevState->getQ())/m_dT;
    }

    void updateStateGivenV(std::shared_ptr<ProblemState> prevState, std::shared_ptr<ProblemState> currentState, Vectord& v)
    {
        currentState->getQDot() = v;
        currentState->getQ() = prevState->getQ() + m_dT*currentState->getQDot();
    }

    void updateStateGivenU(std::shared_ptr<ProblemState> prevState, std::shared_ptr<ProblemState> currentState, Vectord& u)
    {
        currentState->getQ() = u;
        currentState->getQDot() = (currentState->getQ() - prevState->getQ()) / m_dT;
    }
protected:

    // Coefficients of the time integrator
    std::array<double, 3> m_alpha = { { 1, 0, 0 } };
    std::array<double, 3> m_beta = { { 1, -1, 0 } };
    std::array<double, 3> m_gamma = { { 1, -2, -1 } };

    double m_dT; ///> Delta T
};

} // imstk

#endif // ifndef imstkBackwardEuler_h
