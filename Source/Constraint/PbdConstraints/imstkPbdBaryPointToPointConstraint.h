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

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
///
/// \class PbdBaryPointToPointConstraint
///
/// \brief Constrains two points from two separate elements given via
/// barycentric coordinates.
///
/// This constraint technically is a cache-all implementation of all
/// non-penetration constraints as it works for any elements. However,
/// it is not fast so using for many contacts is not ideal.
/// Note: Template solution avoided as imstk is a dynamic library.
///
/// \todo: Could be altered as a distance constraint
///
class PbdBaryPointToPointConstraint : public PbdCollisionConstraint
{
public:
    PbdBaryPointToPointConstraint() : PbdCollisionConstraint(0, 0) { }
    ~PbdBaryPointToPointConstraint() override = default;

public:
    Vec3d computeInterpolantDifference() const;

    ///
    /// \brief initialize constraint
    ///
    void initConstraint(
        std::vector<VertexMassPair> ptsA,
        std::vector<double> weightsA,
        std::vector<VertexMassPair> ptsB,
        std::vector<double> weightsB,
        double stiffnessA, double stiffnessB);

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient for A
    /// \param[inout] dcdx constraint gradient for B
    ///
    bool computeValueAndGradient(double&             c,
                                 std::vector<Vec3d>& dcdxA,
                                 std::vector<Vec3d>& dcdxB) const override;

protected:
    // Bary weights
    std::vector<double> m_weightsA;
    std::vector<double> m_weightsB;
};
} // namespace imstk