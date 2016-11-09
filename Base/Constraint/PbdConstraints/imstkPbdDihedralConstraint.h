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

#ifndef imstkPbdDihedralConstraint_h
#define imstkPbdDihedralConstraint_h

#include "imstkPbdConstraint.h"

namespace imstk
{

///
/// \class PbdDihedralConstraint
///
/// \brief Angular constraint between two triangular faces
///
class PbdDihedralConstraint : public PbdConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    PbdDihedralConstraint() : PbdConstraint(4) {}

    ///
    /// \brief Returns PBD constraint of type Type::Dihedral
    ///
    Type getType() const { return Type::Dihedral; }

    ///
    /// \brief initConstraint
    ///        p3
    ///       / | \
                ///      /  |  \
                ///     p0  |  p1
    ///      \  |  /
    ///       \ | /
    ///         p2
    /// \param model
    /// \param pIdx1 index of p0
    /// \param pIdx2 index of p1
    /// \param pIdx3 index of p2
    /// \param pIdx4 index of p3
    /// \param k stiffness
    ///
    void initConstraint(PbdModel &model,
                        const size_t& pIdx1, const size_t& pIdx2,
                        const size_t& pIdx3, const size_t& pIdx4,
                        const double k);

    ///
    /// \brief Solves the dihedral angular constraint
    ///
    bool solvePositionConstraint(PbdModel &model) override;

public:
    double m_restAngle; ///> Rest angle
    double m_stiffness; ///> Angular stiffness
};

} //imstk

#endif // imstkPbdDihedralConstraint_h