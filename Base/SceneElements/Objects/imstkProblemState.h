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

namespace imstk
{

///
/// \class ProblemState
///
/// \brief This class stores the state of the unknown field variable of the problem
///
template<class T>
class ProblemState
{
public:

    ///
    /// \brief Constructor
    ///
    ProblemState() = default;
    ProblemState(const T& u, const T& v, const T& a = 0);

    ///
    /// \brief Destructor
    ///
    ~ProblemState(){}

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const T& u, const T& v = 0, const T& a = 0)
    {
        m_q = u;
        m_qDot = v;
        m_qDotDot = a;
    }

    void setState(const ProblemState& rhs)
    {
        m_q = rhs.getQ();
        m_qDot = rhs.getQDot();
        m_qDotDot = rhs.getQDotDot();
    }

    void setState(std::shared_ptr<ProblemState> rhs)
    {
        m_q = rhs->getQ();
        m_qDot = rhs->getQDot();
        m_qDotDot = rhs->getQDotDot();
    }

    ///
    /// \brief Get the state
    ///
    T& getQ() const
    {
        if (!m_q)
        {
            LOG(WARNING) << "State variable is not initialized!"
        }
        return m_q;
    }

    ///
    /// \brief Get the derivative of state w.r.t time
    ///
    T& getQDot() const
    {
        if (!m_qDot)
        {
            LOG(WARNING) << "Time derivative of state variable is not initialized!"
        }
        return m_qDot;
    }

    ///
    /// \brief Get the double derivative of state w.r.t time
    ///
    T& getQDotDot() const
    {
        if (!m_qDotDot)
        {
            LOG(WARNING) << "Double time derivative of state variable is not initialized!"
        }
        return m_qDotDot;
    }

    ///
    /// \brief Get the state
    ///
    T& getState()
    {
        return getQ();
    }

protected:
    T m_q;         // State
    T m_qDot;      // Derivative of state w.r.t time
    T m_qDotDot;   // Double derivative of state w.r.t time
};

} // imstk

#endif // ifndef imstkProblemState_h
