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

#ifndef imstkPbdAreaConstraint_h
#define imstkPbdAreaConstraint_h

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
    Type getType() const { return Type::Area; }

    ///
    /// \brief Initializes the area constraint
    ///
    void initConstraint(PbdModel& model, const size_t& pIdx1,
                        const size_t& pIdx2, const size_t& pIdx3,
                        const double k = 2.5);

    ///
    /// \brief Solves the area constraint
    ///
    bool solvePositionConstraint(PbdModel &model);

public:
    double m_restArea; ///> Area at the rest position
    double m_stiffness; ///> Stiffness of the area constraint
};

} // imstk

#endif // imstkPbdAreaConstraint_h