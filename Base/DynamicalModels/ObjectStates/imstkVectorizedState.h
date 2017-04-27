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

#ifndef imstkVectorizedState_h
#define imstkVectorizedState_h

#include <memory>

#include "imstkMath.h"

namespace imstk
{
///
/// \class DeformableBodyState
///
/// \brief This class stores the state of the unknown field variable in vectorized form
///
class VectorizedState
{
public:

    ///
    /// \brief Constructor
    ///
    VectorizedState() = default;
    VectorizedState(const size_t size){ initialize(size); };
    //ProblemState(const Vectord& u, const Vectord& v, const Vectord& a);

    ///
    /// \brief Destructor
    ///
    ~VectorizedState() = default;

    ///
    /// \brief Initialize the problem state
    ///
    void initialize(const size_t numDof);

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const Vectord& u, const Vectord& v, const Vectord& a);

    ///
    /// \brief Set the current state
    ///
    void setU(const Vectord& u);

    ///
    /// \brief Set the time derivative of state
    ///
    void setV(const Vectord& v);

    ///
    /// \brief Set double time derivative of the state
    ///
    void setA(const Vectord& a);

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<VectorizedState> rhs);

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

#endif // ifndef imstkVectorizedState_h
