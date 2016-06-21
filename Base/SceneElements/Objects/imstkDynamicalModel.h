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

#ifndef imstkDynamicalModel_h
#define imstkDynamicalModel_h

#include <string>

#include "imstkProblemState.h"

namespace imstk {

///
/// \class DynamicalModel
///
/// \brief Base class for mathematical model of the physics governing the dynamic object
///
class DynamicalModel
{
public:
    enum class Type
    {
        elastoDynamics,
        NavierStokes,
        HeatEquation,
        none
    };

    using kinematicState = ProblemState;

public:
    ///
    /// \brief Constructor
    ///
    DynamicalModel(DynamicalModel::Type type = Type::none) : m_type(type){}

    ///
    /// \brief Destructor
    ///
    virtual ~DynamicalModel() = default;

    ///
    /// \brief Return the current state of the body
    ///
    std::shared_ptr<kinematicState> getInitialState()
    {
        return m_initialState;
    }

    ///
    /// \brief Return the current state of the body
    ///
    std::shared_ptr<kinematicState> getCurrentState()
    {
        return m_currentState;
    }

    ///
    /// \brief Return the current state of the body
    ///
    std::shared_ptr<kinematicState> getPreviousState()
    {
        return m_previousState;
    }

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState()
    {
        m_currentState->setState(m_initialState);
        m_previousState->setState(m_initialState);
    }

    ///
    /// \brief Returns the number of degrees of freedom
    ///
    std::size_t getNumDegreeOfFreedom()
    {
        return m_numDOF;
    }

    ///
    /// \brief Update states
    ///
    virtual void updateBodyStates(const Vectord& q) = 0;

protected:

    Type m_type; ///> Mathematical model type

    // Body states
    std::shared_ptr<kinematicState> m_initialState;      ///> Initial state
    std::shared_ptr<kinematicState> m_currentState;      ///> Current state
    std::shared_ptr<kinematicState> m_previousState;     ///> Previous state

    std::size_t m_numDOF; ///> Total number of degree of freedom
};

}

#endif // ifndef imstkDynamicalModel_h
