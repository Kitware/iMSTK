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
////
/// \class AreaConstraint
///
/// \brief Area constraint for triangular face
///
class PbdAreaConstraint : public PbdConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    PbdAreaConstraint() : PbdConstraint() { m_vertexIds.resize(3); }

    ///
    /// \brief Returns PBD constraint of type Type::Area
    ///
    Type getType() const override { return Type::Area; }

    ///
    /// \brief Initializes the area constraint
    ///
    void initConstraint(
        const StdVectorOfVec3d& initVertexPositions,
        const size_t& pIdx1, const size_t& pIdx2, const size_t& pIdx3,
        const double k = 2.5);

    bool computeValueAndGradient(const StdVectorOfVec3d& currVertexPositions,
                                 double& c,
                                 StdVectorOfVec3d& dcdx) const override;

public:
    double m_restArea  = 0.; ///> Area at the rest position
};
} // imstk
