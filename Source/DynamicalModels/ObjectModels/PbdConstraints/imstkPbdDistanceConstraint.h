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

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class PbdDistanceConstraint
///
/// \brief Distance constraints between two nodal points
///
class PbdDistanceConstraint : public PbdConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    PbdDistanceConstraint() : PbdConstraint() { m_vertexIds.resize(2); }

    ///
    /// \brief Returns PBD constraint of type Type::Distance
    ///
    inline Type getType() const override { return Type::Distance; }

    ///
    /// \brief Initializes the distance constraint
    ///
    void initConstraint(PbdModel& model, const size_t& pIdx1,
                        const size_t& pIdx2, const double k = 1e-1);

    ///
    /// \brief Solves the Distance constraint
    ///
    bool solvePositionConstraint(PbdModel& model) override;

public:
    double m_restLength = 0.; ///> Rest length between the nodes
    double m_stiffness  = 0.; ///> Stiffness of the constaint
};
} // imstk