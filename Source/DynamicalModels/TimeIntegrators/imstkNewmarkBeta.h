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
/// \class NewmarkBeta
///
/// \brief Newmark-beta time integration
///
class NewmarkBeta : public TimeIntegrator
{
public:
    NewmarkBeta(const double dT, const double beta = 0.25, const double gamma = 0.5) : TimeIntegrator(Type::NewmarkBeta, dT), m_gamma(gamma), m_beta(beta)
    {}
    ~NewmarkBeta() override = default;

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
} // namespace imstk