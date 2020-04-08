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

#include "imstkAbstractDynamicalModel.h"

namespace imstk
{
///
/// \class DynamicalModel
///
/// \brief Base class for mathematical model of the physics governing the dynamic object
///
template<class StateType>
class DynamicalModel : public AbstractDynamicalModel
{
public:
    ///
    /// \brief Constructor
    ///
    DynamicalModel(DynamicalModelType type = DynamicalModelType::None) : AbstractDynamicalModel(type) {}

    ///
    /// \brief Destructor
    ///
    virtual ~DynamicalModel() = default;

    ///
    /// \brief Return the initial state of the problem
    ///
    std::shared_ptr<StateType> getInitialState() { return m_initialState; }

    ///
    /// \brief Return the current state of the problem
    ///
    std::shared_ptr<StateType> getCurrentState() { return m_currentState; }

    ///
    /// \brief Return the previous state of the problem
    ///
    std::shared_ptr<StateType> getPreviousState() { return m_previousState; }

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
    std::shared_ptr<StateType> m_initialState;  ///> Initial state
    std::shared_ptr<StateType> m_currentState;  ///> Current state
    std::shared_ptr<StateType> m_previousState; ///> Previous state
};
} // imstk
