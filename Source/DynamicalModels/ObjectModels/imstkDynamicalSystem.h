/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkAbstractDynamicalSystem.h"

namespace imstk
{
///
/// \class DynamicalSystem
///
/// \brief Base class for mathematical model of the physics governing the
/// dynamic object.
///
template<class StateType>
class DynamicalSystem : public AbstractDynamicalSystem
{
public:
    DynamicalSystem(DynamicalModelType type = DynamicalModelType::None) : AbstractDynamicalSystem(type),
        m_initialState(std::make_shared<StateType>()),
        m_currentState(std::make_shared<StateType>()),
        m_previousState(std::make_shared<StateType>())
    {
    }

    ~DynamicalSystem() override = default;

    ///
    /// \brief Return the initial state of the problem
    ///
    std::shared_ptr<StateType> getInitialState() const { return m_initialState; }

    ///
    /// \brief Return the current state of the problem
    ///
    std::shared_ptr<StateType> getCurrentState() const { return m_currentState; }

    ///
    /// \brief Return the previous state of the problem
    ///
    std::shared_ptr<StateType> getPreviousState() const { return m_previousState; }

    ///
    /// \brief Reset the current state to the initial state
    ///
    void resetToInitialState() override
    {
        m_currentState->setState(m_initialState);
        m_previousState->setState(m_initialState);
    }

protected:
    // Body states
    std::shared_ptr<StateType> m_initialState;  ///< Initial state
    std::shared_ptr<StateType> m_currentState;  ///< Current state
    std::shared_ptr<StateType> m_previousState; ///< Previous state
};
} // namespace imstk