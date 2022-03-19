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

#include "imstkMath.h"
#include "imstkVecDataArray.h"

namespace imstk
{
///
/// \struct VertexMassPair
///
/// \brief Represents a point in a PBD mesh with a mass & velocity
///
struct VertexMassPair
{
    Vec3d* vertex;
    double invMass;
    Vec3d* velocity;
};

///
/// \class PbdCollisionConstraint
///
/// \brief Abstract base class for collision constraints. Collision
/// constraints are non penetration constraints solved with PBD.
/// They are formulated with a gradient dcdx and scalar c which must
/// be returned in computeValueAndGradient.
/// These constraints differ from PbdConstraints in that they do not
/// support XPBD (as they are meant to be solved completely) & they
/// are meant to be generated during runtime quickly upon contacts.
///
class PbdCollisionConstraint
{
public:
    ///
    /// \brief Construct with the number of vertices from A and B for each side
    ///
    PbdCollisionConstraint(const unsigned int numVertsA, const unsigned int numVertsB);
    virtual ~PbdCollisionConstraint() = default;

public:
    ///
    /// \brief Get vertex indices of first object
    ///@{
    const std::vector<VertexMassPair>& getVertexIdsFirst() const { return m_bodiesFirst; }
    const std::vector<VertexMassPair>& getVertexIdsSecond() const { return m_bodiesSecond; }
    ///@}

    ///
    /// \brief Get stiffness
    ///@{
    const double getStiffnessA() const { return m_stiffnessA; }
    const double getStiffnessB() const { return m_stiffnessB; }
    ///@}

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    virtual bool computeValueAndGradient(double&             c,
                                         std::vector<Vec3d>& dcdxA,
                                         std::vector<Vec3d>& dcdxB) const = 0;

    ///
    /// \brief Solve the positions given to the constraint
    ///
    virtual void solvePosition();

    ///
    /// \brief Solve the velocities given to the constraint
    ///
    virtual void correctVelocity(const double friction, const double restitution);

protected:
    std::vector<VertexMassPair> m_bodiesFirst;  ///< index of points for the first object
    std::vector<VertexMassPair> m_bodiesSecond; ///< index of points for the second object

    double m_stiffnessA = 1.0;
    double m_stiffnessB = 1.0;

    std::vector<Vec3d> m_dcdxA; ///< Constraint gradients (per vertex)
    std::vector<Vec3d> m_dcdxB; ///< Constraint gradients (per vertex)
};
} // namespace imstk
