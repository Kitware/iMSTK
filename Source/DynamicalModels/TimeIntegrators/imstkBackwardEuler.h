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
    BackwardEuler(const double dT = 0.01) : TimeIntegrator(Type::BackwardEuler, dT)
    {}

    ///
    /// \brief Destructor
    ///
    ~BackwardEuler() = default;

    ///
    /// \brief Update state given change in velocity
    ///
    void updateStateGivenDv(std::shared_ptr<FeDeformBodyState> prevState,
                            std::shared_ptr<FeDeformBodyState> currentState,
                            Vectord&                           dV) override;

    ///
    /// \brief Update state given change in displacement
    ///
    void updateStateGivenDu(std::shared_ptr<FeDeformBodyState> prevState,
                            std::shared_ptr<FeDeformBodyState> currentState,
                            Vectord&                           dU) override;

    ///
    /// \brief Update state given updated velocity
    ///
    void updateStateGivenV(std::shared_ptr<FeDeformBodyState> prevState,
                           std::shared_ptr<FeDeformBodyState> currentState,
                           Vectord&                           v) override;

    ///
    /// \brief Update state given updated displacement
    ///
    void updateStateGivenU(std::shared_ptr<FeDeformBodyState> prevState,
                           std::shared_ptr<FeDeformBodyState> currentState,
                           Vectord&                           u) override;
protected:

//    // Coefficients of the time integrator
//    std::array<double, 3> m_alpha = { { 1, 0, 0 } };
//    std::array<double, 3> m_beta = { { 1, -1, 0 } };
//    std::array<double, 3> m_gamma = { { 1, -2, -1 } };
};
} // imstk
