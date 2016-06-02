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

#ifndef imstkTimeIntegrator_h
#define imstkTimeIntegrator_h

#include <array>
#include "g3log/g3log.hpp"

namespace imstk
{

///
/// \class TimeIntegrator
///
/// \brief This class defines the time integrators of various types.
/// It only sets the rules of how the velocity (or equivalent) and
/// acceleration (or equivalent) of the present time in terms of
/// positions (or equivalent) from previous time steps.
///
class TimeIntegrator
{
    enum class Type
    {
        ForwardEuler,
        BackwardEuler,
        NewmarkBeta,
        CentralDifference
    };

public:
    ///
    /// \brief Constructor
    ///
    TimeIntegrator(const TimeIntegrator::Type type);

    ///
    /// \brief Destructor
    ///
    ~TimeIntegrator() = default;

    ///
    /// \brief Set/Get type of the time integrator
    ///
    void setType(const TimeIntegrator::Type type);
    const Type& getType() const;

    ///
    /// \brief Set coefficients for a given time integrator type
    ///
    void setCoefficients(const TimeIntegrator::Type type);

protected:
    TimeIntegrator::Type m_type; ///> Type of the time integrator

    // Coefficients of the time integrator
    std::array<double,3> m_alpha;
    std::array<double,3> m_gamma;
    std::array<double,3> m_beta;
};

}

#endif // ifndef imstkTimeIntegrator_h
