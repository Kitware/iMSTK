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

#ifndef imstkPbdDistanceConstraint_h
#define imstkPbdDistanceConstraint_h

#include "imstkPbdConstraint.h"

namespace imstk
{

///
/// \class DistanceConstraint
///
/// \brief Distance constraints between two nodal points
///
class DistanceConstraint : public PbdConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    DistanceConstraint() : PbdConstraint(2) {}

    ///
    /// \brief Returns PBD constraint of type Type::Distance
    ///
    Type getType() const { return Type::Distance; }

    ///
    /// \brief Initializes the distance constraint
    ///
    void initConstraint(PositionBasedDynamicsModel& model, const unsigned int& pIdx1,
        const unsigned int& pIdx2, const double k = 1e-1);

    ///
    /// \brief Solves the Distance constraint
    ///
    bool solvePositionConstraint(PositionBasedDynamicsModel &model);

public:
    double m_restLength; ///> Rest length between the nodes
    double m_stiffness;  ///> Stiffness of the constaint
};

} // imstk

#endif // imstkPbdDistanceConstraint_h