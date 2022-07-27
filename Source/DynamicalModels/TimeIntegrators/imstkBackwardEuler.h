/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTimeIntegrator.h"

namespace imstk
{
///
/// \class BackwardEuler
///
/// \brief Backward Euler time integration
///
class BackwardEuler : public TimeIntegrator
{
public:
    BackwardEuler(const double dT = 0.01) : TimeIntegrator(Type::BackwardEuler, dT) { }
    ~BackwardEuler() override     = default;

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
} // namespace imstk