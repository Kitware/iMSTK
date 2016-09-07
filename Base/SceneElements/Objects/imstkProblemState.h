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

#ifndef imstkProblemState_h
#define imstkProblemState_h

#include <memory>

#include "imstkMath.h"

namespace imstk
{

///
/// \class ProblemState
///
/// \brief This class stores the state of the unknown field variable of the problem
///
class ProblemState
{
public:

    ///
    /// \brief Constructor
    ///
    ProblemState(){};
    ProblemState(const size_t size){ initialize(size); };
    //ProblemState(const Vectord& u, const Vectord& v, const Vectord& a);

    ///
    /// \brief Destructor
    ///
    ~ProblemState(){}

    ///
    /// \brief Initialize the problem state
    ///
    void initialize(const size_t numDof)
    {
        m_q.resize(numDof);
        m_qDot.resize(numDof);
        m_qDotDot.resize(numDof);

        m_q.setZero();
        m_qDot.setZero();
        m_qDotDot.setZero();
    };

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const Vectord& u, const Vectord& v, const Vectord& a)
    {
        m_q = u;
        m_qDot = v;
        m_qDotDot = a;
    }

    ///
    /// \brief
    ///
    void setU(const Vectord& u)
    {
        m_q = u;
    }

    ///
    /// \brief
    ///
    void setV(const Vectord& v)
    {
        m_qDot = v;
    }

    ///
    /// \brief
    ///
    void setA(const Vectord& a)
    {
        m_qDotDot = a;
    }

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<ProblemState> rhs)
    {
        m_q = rhs->getQ();
        m_qDot = rhs->getQDot();
        m_qDotDot = rhs->getQDotDot();
    }

    ///
    /// \brief Get the state
    ///
    //const Vectord& getQ() const { return m_q; }
    Vectord& getQ() { return m_q; }

    ///
    /// \brief Get the derivative of state w.r.t time
    ///
    //const Vectord& getQDot() const { return m_qDot; }
    Vectord& getQDot() { return m_qDot; }

    ///
    /// \brief Get the double derivative of state w.r.t time
    ///
    //const Vectord& getQDotDot() const { return m_qDotDot; }
    Vectord& getQDotDot() { return m_qDotDot; }

    ///
    /// \brief Get the state
    ///
    //const Vectord& getState() const { return getQ(); }
    Vectord& getState() { return getQ(); }

protected:
    Vectord m_q;         // State
    Vectord m_qDot;      // Derivative of state w.r.t time
    Vectord m_qDotDot;   // Double derivative of state w.r.t time
};

} // imstk

#endif // ifndef imstkProblemState_h
