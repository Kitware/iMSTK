/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkAbstractDynamicalModel.h"

namespace imstk
{
///
/// \class DynamicalModel
///
/// \brief Base class for mathematical model of the physics governing the
/// dynamic object.
///
template<class StateType>
class DynamicalModel : public AbstractDynamicalModel
{
public:
    DynamicalModel(DynamicalModelType type = DynamicalModelType::None) : AbstractDynamicalModel(type) {}
    ~DynamicalModel() override = default;

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
    virtual void resetToInitialState() override
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