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
/// \brief Constrains two points from two separate cells/elements given via
/// barycentric coordinates to be coincident
///
/// Such constraint may be used for grasping (grabbing points on elements,
/// grabbing points with other points), stitching (constraining two
/// points from separate elements together)
///
class PbdBaryPointToPointConstraint : public PbdCollisionConstraint
{
public:
    PbdBaryPointToPointConstraint() : PbdCollisionConstraint(0, 0) { }
    ~PbdBaryPointToPointConstraint() override = default;

    double getRestLength() const { return m_restLength; }
    void setRestLength(const double length) { m_restLength = length; }

    Vec3d computePtA() const;
    Vec3d computePtB() const;
    Vec3d computeInterpolantDifference() const { return computePtB() - computePtA(); }

    ///
    /// \brief initialize constraint with current distance between
    /// the points as the resting length
    /// \param points of cell a
    /// \param barycentric weights for the point in cell a
    /// \param points of cell b
    /// \param barycentric weights for the point in cell b
    /// \param Stiffness which to resolve a
    /// \param stiffness which to resolve b
    ///
    void initConstraintToRest(
        const std::vector<VertexMassPair>& ptsA,
        const std::vector<double>& weightsA,
        const std::vector<VertexMassPair>& ptsB,
        const std::vector<double>& weightsB,
        const double stiffnessA, const double stiffnessB)
    {
        initConstraint(ptsA, weightsA, ptsB, weightsB, stiffnessA, stiffnessB);
        m_restLength = (computePtB() - computePtA()).norm();
    }

    ///
    /// \brief Initialize the constraint with given resting length
    /// \param points of cell a
    /// \param barycentric weights for the point in cell a
    /// \param points of cell b
    /// \param barycentric weights for the point in cell b
    /// \param stiffness which to resolve a
    /// \param stiffness which to resolve b
    /// \param the rest length to resolve to
    ///
    void initConstraint(
        const std::vector<VertexMassPair>& ptsA,
        const std::vector<double>& weightsA,
        const std::vector<VertexMassPair>& ptsB,
        const std::vector<double>& weightsB,
        const double stiffnessA, const double stiffnessB,
        const double restLength = 0.0);

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[inout] c constraint value
    /// \param[inout] dcdxA constraint gradient for A
    /// \param[inout] dcdxB constraint gradient for B
    ///
    bool computeValueAndGradient(double&             c,
                                 std::vector<Vec3d>& dcdxA,
                                 std::vector<Vec3d>& dcdxB) const override;

protected:
    // Bary weights
    std::vector<double> m_weightsA;
    std::vector<double> m_weightsB;

    double m_restLength = 0.0;
};
} // namespace imstk